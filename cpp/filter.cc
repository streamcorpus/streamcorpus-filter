#include "ahocorasick.h"

#include <pthread.h>

// LIBC
#include <fcntl.h>
#include <time.h>

// getrusage
#include <sys/time.h>
#include <sys/resource.h>

// thrift is too stupid to include headers it needs for 'htonl'
#include <arpa/inet.h>
// THRIFT
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/protocol/TDenseProtocol.h>
#include <thrift/protocol/TJSONProtocol.h>
#include <thrift/transport/TTransportUtils.h>
#include <thrift/transport/TFDTransport.h>
#include <thrift/transport/TFileTransport.h>
	namespace atp = apache::thrift::protocol;
	namespace att = apache::thrift::transport;

// THRIFT -- STREAMCORUPS

#ifdef USE_STREAMCORPUS_2
#include "sc2/sc2_types.h"
#include "sc2/sc2_constants.h"
	namespace sc = streamcorpus2;
#else
// version 3
#include "sc3/sc3_types.h"
#include "sc3/sc3_constants.h"
	namespace sc = streamcorpus3;
#endif

// THRIFT -- FILTERNAMES
#include "filternames_types.h"
#include "filternames_constants.h"
	namespace fn = streamcorpus_filter;


// STD
#include <iostream>
	using std::clog;
	using std::cerr;
	using std::cout;
	using std::endl;

#include <set>
	using std::set;
#include <unordered_map>
	using std::unordered_map;
#include <string>
	using std::string;
#include <vector>
	using std::vector;
#include <chrono>
	namespace chrono = std::chrono;
#include <limits>
	using std::numeric_limits;
#include <fstream>

// std::queue
#include <queue>


// BOOST
#include <boost/program_options.hpp>
	namespace po = boost::program_options;

// LVVLIB

#include "lvvlib/mmap.h"


#include "normalize.h"


static const string ANNOTATOR_ID = "streamcorpus-filter-faststrstr";


#ifdef GPERF
// Optionally use google performance sampler:
// http://code.google.com/p/gperftools/
#include <gperftools/profiler.h>

#define GPERF_START() ProfilerStart("fm.prof")
#else
#define GPERF_START()
#define ProfilerStop()
#define ProfilerFlush()
#endif


// "RAII"
class Locker {
	public:
	Locker(pthread_mutex_t* it) {
		mutex = it;
		pthread_mutex_lock(mutex);
	}
	~Locker() {
		pthread_mutex_unlock(mutex);
	}

	private:
	pthread_mutex_t* mutex;
};

// TODO: there might be a better threadsafe queue out there somewhere
template<class T> class TQueue {
	public:
	TQueue() : TQueue(0) {}
	TQueue(int limit);
	~TQueue();
	
	void push(T& it);
	bool pop(T* out, bool block=true);
	void close() {
		auto l = Locker(&mutex);
		open = false;
		pthread_cond_broadcast(&cond);
	}
	bool is_open() const { return open; }

	private:
	bool open;
	pthread_mutex_t mutex;
	pthread_cond_t cond; // wait on pop
	pthread_cond_t pushcond; // wait when full to push
	std::queue<T> they;
	int _limit;
};

template<class T>
TQueue<T>::TQueue(int limit)
	: open(true), _limit(limit)
{
	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&cond, NULL);
	pthread_cond_init(&pushcond, NULL);
}

template<class T>
TQueue<T>::~TQueue() {
	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&cond);
}

template<class T>
void TQueue<T>::push(T& it) {
	auto l = Locker(&mutex);
	while ((_limit > 0) && (they.size() >= (size_t)_limit) && open) {
		pthread_cond_wait(&pushcond, &mutex);
	}
	they.push(it);
	pthread_cond_signal(&cond);
}

template<class T>
bool TQueue<T>::pop(T* out, bool block) {
	auto l = Locker(&mutex);
	while (they.empty() && block && open) {
		pthread_cond_wait(&cond, &mutex);
	}
	//if (!open) { return false; }
	if (!they.empty()) {
		*out = they.front();
		they.pop();
		pthread_cond_signal(&pushcond);
		return true;
	}
	return false;
}


static inline double tv_to_double(const struct timeval& tv) {
    return (tv.tv_usec / 1000000.0) + tv.tv_sec;
}

// out = a - b
static struct timeval operator-(const struct timeval& a, const struct timeval& b) {
	struct timeval out;
	if (a.tv_usec < b.tv_usec) {
		out.tv_usec = (1000000 - b.tv_usec) + a.tv_usec;
		out.tv_sec = (a.tv_sec - b.tv_sec) - 1;
	} else {
		out.tv_sec = a.tv_sec - b.tv_sec;
		out.tv_usec = a.tv_usec - b.tv_usec;
	}
	return out;
}

// out = a - b
static struct rusage operator-(const struct rusage& a, const struct rusage& b) {
	struct rusage out;
#define O_A_B(field) out.field = a.field - b.field
	O_A_B(ru_utime);
	O_A_B(ru_stime);
	O_A_B(ru_maxrss);
	O_A_B(ru_ixrss);
	O_A_B(ru_idrss);
	O_A_B(ru_isrss);
	O_A_B(ru_minflt);
	O_A_B(ru_majflt);
	O_A_B(ru_nswap);
	O_A_B(ru_inblock);
	O_A_B(ru_oublock);
	O_A_B(ru_msgsnd);
	O_A_B(ru_msgrcv);
	O_A_B(ru_nsignals);
	O_A_B(ru_nvcsw);
	O_A_B(ru_nivcsw);
#undef O_A_B
	return out;
}

static void logrusage(std::ostream& out, const struct rusage& ru) {
	out << "my CPU=" << tv_to_double(ru.ru_utime)
		<< " sys CPU=" << tv_to_double(ru.ru_stime)
		<< "; minor faults=" << ru.ru_minflt
		<< ", major faults=" << ru.ru_majflt
		<< ", swaps=" << ru.ru_nswap
		<< ", rss=" << (ru.ru_idrss + ru.ru_isrss) << " (d=" << ru.ru_idrss << ", s=" << ru.ru_isrss << ", max=" << ru.ru_maxrss
		<< "), blocks in=" << ru.ru_inblock
		<< " out=" << ru.ru_oublock
		<< ", msg in=" << ru.ru_msgrcv << " out=" << ru.ru_msgsnd
		<< ", signals=" << ru.ru_nsignals
		<< ", cx v=" << ru.ru_nvcsw << " i=" << ru.ru_nivcsw
		<< endl;
}


typedef std::vector<string> TargetIdList;


static const bool debug_matches = false;


class FilterContext {
	public:
	string text_source;
	int matches;
	int written;
	bool do_normalize;
	bool verbose;
	size_t max_names;
	size_t max_items;

	AC_AUTOMATA_t *atm;

	size_t name_min;
	size_t name_max;
	size_t total_name_length;
	size_t 	names_size;	// number of names (size of names_begin-1)
	long total_content_size;

	size_t min_name_length;
	size_t max_name_length;

	int rating_target_count;
	bool empty_stream_item_instead_of_drop;

	chrono::high_resolution_clock::time_point start;
	
	FilterContext()
		: matches(0), written(0), do_normalize(false), verbose(false),
		  max_names(numeric_limits<size_t>::max()),
		  name_min(numeric_limits<size_t>::max()), name_max(0),
		  total_name_length(0), names_size(0), total_content_size(0),
		  min_name_length(5), max_name_length(1000),
		  rating_target_count(0)
	{
		start = chrono::high_resolution_clock ::now();
		 atm = ac_automata_init ();
	};

    bool _sizeok(size_t size) {
		if ((size > AC_PATTRN_MAX_LENGTH) ||
			(size > max_name_length) ||
			(size < min_name_length)) {
#ifdef DEBUG 
			cerr << "\twarning: name of length " << size << " skipped\n";
#endif
			return false;
		}
		return true;
    }

    void _add_stats(size_t size) {
	names_size++; // count
	name_min = std::min(name_min, size);
	name_max = std::max(name_max, size);
	total_name_length += size;
    }

    // does not normalize(), do externally if do_normalize
    void add_name(const string& name, TargetIdList* targets=NULL) {
		if (name.empty()) {
			return;
		}
		if (!_sizeok(name.size())) { return; }

		AC_PATTERN_t tmp_pattern;
		tmp_pattern.astring = name.data();
		tmp_pattern.length = name.size();
		// rep.stringy is our data which will be returned to us in match
		tmp_pattern.rep.stringy = (const char*)targets;
		
		AC_STATUS_t rc = ac_automata_add (atm, &tmp_pattern);
		if (rc == ACERR_DUPLICATE_PATTERN) {
		    // okay, drop duplicate silently.
		    return;
		} else if (rc != ACERR_SUCCESS) {
			cerr << "multifast error: ac_automata_add() exited"
				"\n\twith return code: " << rc <<
				"\n\tfor string of size: " << name.size() <<
				"\n\tfor string: (" << name <<  ")\n";
			exit(1);
		};
		_add_stats(name.size());
	}

    // Also contains normalization step if do_normalize
	void add_name(const char* data, size_t size, TargetIdList* targets=NULL) {
	    if (do_normalize) {
		string raw(data, size);
		string normed;
		normalize(raw, &normed, NULL);
		add_name(normed, targets);
		return;
	    }
		if (data == NULL) { return; }
		if (!_sizeok(size)) { return; }

		AC_PATTERN_t tmp_pattern;
		tmp_pattern.astring = data;
		tmp_pattern.length = size;
		// rep.{stringy,number} are our data returned to us in match
		tmp_pattern.rep.stringy = (const char*)targets;
		
		AC_STATUS_t rc = ac_automata_add (atm, &tmp_pattern);
		if (rc == ACERR_DUPLICATE_PATTERN) {
		    // okay, drop duplicate silently.
		    return;
		} else if (rc != ACERR_SUCCESS) {
			cerr << "multifast error: ac_automata_add() exited"
				"\n\twith return code: " << rc <<
				"\n\tfor string of size: " << size <<
				"\n\tfor string: (" << string(data, size) <<  ")\n";
			exit(1);
		};
		_add_stats(size);
	}

	int read_simple_names(const string& names_simple_path) {
		// read a file of one name per line
		// OR name [\t target_id]+ \n
		size_t names_mem;      // size of names_data;
		names_size = 0;
		char* names_data  = mmap_read<char>(names_simple_path.c_str(), names_mem);

		unsigned int count = 0;
		size_t startpos = 0;
		size_t pos = 0;
		while (pos < names_mem) {
		    // TODO: probably not UTF-8 Safe
			if (names_data[pos] == '\n') {
			    count++;
			    add_name(names_data + startpos, pos - startpos);
				startpos = pos + 1;

				if (count >= max_names) {
				    clog << "hit max_names " << max_names << endl;
				    break;
				}
			}
			pos++;
		}
		// TODO: munmap() and close!
		return 0;
	}

	int read_scf_names(const string& scf_path) {
	    fn::FilterNames filter_names;

	    {
		int scf_fh = open(scf_path.c_str(), O_RDONLY);
		if (scf_fh < 0) {
		    cerr << "could not open name scf\n";
		    perror(scf_path.c_str());
		    return -1;
		}

		boost::shared_ptr<att::TFDTransport> innerTransportScf(new att::TFDTransport(scf_fh));
		boost::shared_ptr<att::TBufferedTransport> transportScf(new att::TBufferedTransport(innerTransportScf));
		boost::shared_ptr<atp::TBinaryProtocol> protocolScf(new atp::TBinaryProtocol(transportScf));
		transportScf->open();

		// load it all into memory here
		filter_names.read(protocolScf.get());

		//transportScf->close();
		close(scf_fh);
	    }

	    unsigned int count = 0;
	    for(auto& pr : filter_names.name_to_target_ids) {
		const string& name = pr.first;
		TargetIdList* matchtargets = new TargetIdList(pr.second);
#if 0  /* almost always too verbose to dump all the names */
		if (verbose) {
		    clog << name << "\t" << matchtargets->size() << endl;
		}
#endif

		count++;
		if (do_normalize) {
		    string normed;
		    normalize(name, &normed, NULL);
		    add_name(normed, matchtargets);
		} else {
		    add_name(name, matchtargets);
		}

		if (count >= max_names) {
		    clog << "hit max_names " << max_names << endl;
		    break;
		}
	    }


	    return 0;
	}

	int read_mmap_names(const string& names_path) {
		string names_begin_path = string("");

		// construct names_begin_path
		size_t ext_pos = names_path.rfind(".mmap");
		copy(names_path.begin(), names_path.begin()+ext_pos, back_inserter(names_begin_path));
		names_begin_path.append("_begin.mmap");

		if (verbose) {
			cerr << "\tnames_path: " << names_path << endl;
			cerr << "\tnames_begin_path: " << names_begin_path << endl;
		}

		size_t names_size; // yes, this shadows FilterContext.names_size

		/////////////////////////////////////////////////  READ NAMES MMAP

		size_t 	names_mem;      // size of names_data;
		char 	*names_data  = mmap_read<char>  (names_path.c_str(),       names_mem);
		size_t	*names_begin = mmap_read<size_t>(names_begin_path.c_str(), names_size);
		names_size--;


		/////////////////////////////////////////////////  CONSTRUCT NAMES_T 


		for (size_t i=0;  i< std::min(max_names,names_size);  ++i) {
		    add_name(names_data+names_begin[i], names_begin[i+1]-names_begin[i]);
		}
		// TODO: munmap() and close!

		return 0;
	}

	void compile_names() {
		ac_automata_finalize(atm);

		// and log stuff about it
		auto diff = chrono::high_resolution_clock ::now() - start;
		double sec = chrono::duration_cast<chrono::nanoseconds>(diff).count();
		clog << "Names: "  	   << names_size
			 << ";  min: "         << name_min
			 << ";  max: "         << name_max
			//			 << ";  avg: "         << double(total_name_length)/names.size()
			 << "; total length: " << total_name_length
			 << endl;

		clog << "Names construction time: "      << sec/1e9 << " sec" << endl;
	}

	int get_best_content(const sc::StreamItem& stream_item, string* content) const {
		if (text_source == "clean_visible") {
			*content = stream_item.body.clean_visible;
		} else if (text_source == "clean_html") {
			*content = stream_item.body.clean_html;
		} else if (text_source == "raw") {
			*content = stream_item.body.raw;
		} else {
			cerr << "Bad text_source :" << text_source <<endl;
			exit(-1);
		}

		if (content->size() <= 0) {
			// Fall back to raw if desired text_source has no content.
			*content = stream_item.body.raw;
			//actual_text_source = "raw";
			if (content->size() <= 0) {
				// If all applicable text sources are empty, we have a problem and exit with an error
				cerr/* << stream_items_count*/ << " Error, doc id:" << stream_item.doc_id << " was empty." << endl;
				return -1;
				//exit(-1);
			}
		}
		return 0;
	}

	//void check_content(const string& content) {
	// return true if any name matches the content
	bool check_streamitem(sc::StreamItem* stream_item) {

		string content;

		int err = get_best_content(*stream_item, &content);
		if (err != 0) {
			return false;
		}

		total_content_size += content.size();

		string normstr;
		std::vector<size_t> offsets;
		const char* normtext = NULL;
		size_t textlen;

		if (do_normalize) {
			normalize(content, &normstr, &offsets);
			normtext = normstr.data();
			textlen = normstr.size();
		} else {
			normtext = content.data();
			textlen = content.size();
		}

		bool any_match = false;
		AC_TEXT_t tmp_text;
		tmp_text.astring = normtext;
		tmp_text.length = textlen;
		AC_FIND_CONTEXT_t findcontext;
		findnext_context_settext(atm, &findcontext, &tmp_text, 0);

		AC_MATCH_t match;
		int did_match = ac_automata_findnext_r(atm, &findcontext, &match);

		set<string> target_ids;

		while (did_match) {
		    if (debug_matches && verbose && !any_match) {
				// first match for stream_item
				clog << stream_item->doc_id << endl;
		    }
			any_match = true;
			matches += match.match_num;

			for (unsigned int i = 0; i < match.match_num; ++i) {
			    TargetIdList* tids = (TargetIdList*)(match.patterns[i].rep.stringy);
			    if (tids != NULL) {
					if (debug_matches && verbose) {
						clog << "[" << (match.position - match.patterns[i].length) << "] " << string(normtext + (match.position - match.patterns[i].length), match.patterns[i].length) << "\t";
					}
					for (string& targetid : *tids) {
						target_ids.insert(targetid);
						if (debug_matches && verbose) {
							clog << targetid << " ";
						}
					}
					if (debug_matches && verbose) {
						clog << endl;
					}
			    } else {
					if (debug_matches && verbose) {
						clog << string(normtext + (match.position - match.patterns[i].length), match.patterns[i].length) << " no targets\n";
					}
			    }
			}

#if 0
			// We don't (currently) record the per mention
			// label annotations of the things we match,
			// just the document scope Rating objects
			// later. But we might want to bring this back
			// someday.
			//
			// TODO: load and use actual target id
			// Add the target identified to the label.  Note this 
			// should be identical to what is in the rating 
			// data we add later.
			sc::Target target;
			target.target_id = "1";
				
			sc::Label label;
			label.target = target;
				
			// Add the actual offsets 
			sc::Offset offset;
			offset.type = sc::OffsetType::BYTES;

			size_t startpos, endpos;
			if (do_normalize) {
				startpos = offsets[match.position - match.patterns[0].length];
				endpos = offsets[match.position];
			} else {
				startpos = match.position - match.patterns[0].length;
				endpos = match.position;
			}

			offset.first = startpos;
			offset.length = endpos - startpos;
				
			label.offsets[sc::OffsetType::BYTES] = offset;
			label.__isset.offsets = true;
				
			// Add new label to the list of labels.
			stream_item->body.labels[ANNOTATOR_ID].push_back(label);
				
			// Map of actual text mapped 
			//target_text_map[target.target_id].insert(std::string(match_b, match_e));
#endif /* No per-mention labels */

			did_match = ac_automata_findnext_r(atm, &findcontext, &match);
		}

		if (verbose) {
			clog << stream_item->doc_id << " len(target_ids)=" << target_ids.size() << endl;
		}

		// Set document "Rating" objects to note that a
		// target_id was found.
		if (!target_ids.empty()) {
		    vector<sc::Rating> ratings;
		    sc::Annotator annotator;
		    annotator.annotator_id = ANNOTATOR_ID;
		    rating_target_count += target_ids.size();
		    for (const string& target_id : target_ids) {
			sc::Target targ;
			targ.target_id = target_id;

			sc::Rating rat;
			rat.annotator = annotator;
			rat.target = targ;
			ratings.push_back(rat);
		    }
		    stream_item->ratings[ANNOTATOR_ID] = ratings;
		}

		return any_match;
	}

    size_t run_threads(int nthreads, atp::TBinaryProtocol* input, atp::TBinaryProtocol* output);

}; // FilterContext


// Return a new empty item that has empty values for all the thrift-required fields.
sc::StreamItem* newEmptyItem() {
    sc::StreamItem* item;
    item = new sc::StreamItem();
    item->doc_id = "";
    item->stream_id = "";
    item->stream_time = sc::StreamTime();
    item->stream_time.epoch_ticks = 0;
    item->stream_time.zulu_timestamp = "";
    return item;
}


class FilterThread {
	public:
	FilterContext* fcontext;
	TQueue<sc::StreamItem*>* items_in;
	TQueue<sc::StreamItem*>* items_out;
	bool empty_stream_item_instead_of_drop;

	FilterThread(FilterContext* fc, TQueue<sc::StreamItem*>* in, TQueue<sc::StreamItem*>* out)
	    : fcontext(fc), items_in(in), items_out(out),
	      empty_stream_item_instead_of_drop(false)
	{}

	void* run() {
		sc::StreamItem* item;
		int count = 0, mcount = 0;
		while (items_in->pop(&item)) {
			count ++;
			//clog << "ftitem\n";
			bool matched = fcontext->check_streamitem(item);
			if (matched) {
				mcount++;
				items_out->push(item);
			} else if (empty_stream_item_instead_of_drop) {
			    delete item;
			    item = newEmptyItem();
			    items_out->push(item);
			} else {
			    delete item;
			}
		}
		//clog << "f thread ending. items=" << count << " matches=" << mcount << endl;
		clog << "Total matches: " << mcount << endl;
		return NULL;
	}
};

// target for pthread_create
void* filter_thread(void* arg) {
	FilterThread* ft = (FilterThread*)arg;
	return ft->run();
}


class StreamItemWriter {
	public:
	StreamItemWriter(TQueue<sc::StreamItem*>* input,
					 atp::TBinaryProtocol* output)
		: _input(input), _output(output)
	{}

    int run() {
		sc::StreamItem* item;
		int count = 0;
		while (_input->pop(&item)) {
			count++;
			item->write(_output);
			delete item;
		}
		//clog << "writer thread ending, count=" << count << endl;
		clog << "Total stream items written: " << count << endl;
		return count;
	}

	private:
	TQueue<sc::StreamItem*>* _input;
	atp::TBinaryProtocol* _output;
};

// target for pthread_create
void* pthread_writer(void* arg) {
	StreamItemWriter* it = (StreamItemWriter*)arg;
	uintptr_t written = it->run();
	return (void*)written;
}


size_t FilterContext::run_threads(int nthreads, atp::TBinaryProtocol* input, atp::TBinaryProtocol* output) {
	// make queues
	TQueue<sc::StreamItem*> in_queue(10);
	TQueue<sc::StreamItem*> out_queue(10);

	// start theads
	auto filterer = FilterThread(this, &in_queue, &out_queue);
	filterer.empty_stream_item_instead_of_drop = empty_stream_item_instead_of_drop;
	auto writer = StreamItemWriter(&out_queue, output);
	pthread_t* fthreads = new pthread_t[nthreads];
	pthread_t othread;

	int err;
	for (int i = 0; i < nthreads; i++) {
	    err = pthread_create(&(fthreads[i]), NULL, filter_thread, &filterer);
	    if (err != 0) {
		perror("opening filter thread");
		exit(1);
	    }
	}
	err = pthread_create(&othread, NULL, pthread_writer, &writer);
	if (err != 0) {
		perror("opening writer thread");
		exit(1);
	}

	size_t stream_items_count = 0;
	// be the read thread
	while (true) {
		sc::StreamItem* si = new sc::StreamItem();
		try {
			si->read(input);
		} catch (att::TTransportException e) {
			clog << "stream ended. count=" << stream_items_count << endl;
			break;
		}
		in_queue.push(si);
		stream_items_count++;
		if ((max_items > 0) && (stream_items_count >= max_items)) {
			clog << "hit item limit at " << stream_items_count << endl;
			break;
		}
	}
	in_queue.close();
	// NOTE: these log lines are checked in test_filter.py
	clog << "Total stream items processed: " << stream_items_count << endl;

	// join threads
	for (int i = 0; i < nthreads; i++) {
	    pthread_join(fthreads[i], NULL);
	}

	out_queue.close();
	void* writecount;
	pthread_join(othread, &writecount);
	int written = (uintptr_t)writecount;
	clog << "Added " << rating_target_count << " ratings, "
	     << ((1.0 * rating_target_count) / written)
	     << " ratings-per-written-item\n";

	return stream_items_count;
}


int main(int argc, char **argv) {

	 ///////////////////////////////////////////////////////////////////////////////  OPTIONS

	 // options
	 string		text_source	= "clean_visible";
	 string		names_path	/*= "data/names.mmap"*/;	// default location of names mmap
	 string		names_begin_path= ""; 			// default will be "data/names_begin.mmap";
	 string          names_simple_path;
	 string names_scf_path;
	 bool		negate		= false;
	 size_t		max_names	= numeric_limits<size_t>::max();
	 size_t		max_items	= numeric_limits<size_t>::max();
	 bool		verbose		= false;
	 bool		do_normalize	= false;
	 int        threads = 1;
	 int min_name_length = 5;
	 int max_name_length = 1000;
	 string in_path;
	 string out_path;
	 bool empty_stream_item_instead_of_drop = false;

	 po::options_description desc("Allowed options");

	 desc.add_options()
		 ("input", po::value<string>(&in_path), "path to input streamcorpus file (default stdin)")
		 ("output", po::value<string>(&out_path), "path to ouput streamcorpus file (default stdout)")
		 ("help,h",                                          "help message")
		 ("text_source,t", po::value<string>(&text_source),  "text source in stream item")
		 ("negate,n",	po::value<bool>(&negate)->implicit_value(true), "negate sense of match")
		 ("names-mmap,n", po::value<string>(&names_path), "path to names mmap file (and names_begin")
		 ("names-scf,n", po::value<string>(&names_scf_path), "path to names scf file")
		 ("names,n", po::value<string>(&names_simple_path), "path to names text file")
		 ("max-names,N", po::value<size_t>(&max_names), "maximum number of names to use")
		 ("max-items,I", po::value<size_t>(&max_items), "maximum number of items to process")
		 ("threads,j", po::value<int>(&threads), "number of threads to run (default 1)")
		 ("min-name-length", po::value<int>(&min_name_length), "discard names shorter than this (default 5)")
		 ("max-name-length", po::value<int>(&max_name_length), "discard names longer than this (default 1000)")
		 ("verbose",	"performance metrics every 100 items")
		 ("normalize",	"collapse spaces of input")
		 ("no-search",	"do not search - pass through every item")
		 ("emit-empties", "instead of just dropping non-matches, emit an empty item in its place")
	 ;

	 // Parse command line options
	 po::variables_map vm;
	 po::store(po::parse_command_line(argc, argv, desc), vm);
	 po::notify(vm);

	 if (vm.count("help")) {
		 cout << desc << "\n";
		 return 1;
	 }
	 if (vm.count("verbose"))	verbose=true;
	 if (vm.count("normalize"))	do_normalize=true;
	 if (vm.count("emit-empties"))	empty_stream_item_instead_of_drop=true;

	 FilterContext fcontext;
	 fcontext.text_source = text_source;
	 fcontext.verbose = verbose;
	 fcontext.do_normalize = do_normalize;
	 fcontext.max_names = max_names;
	 fcontext.min_name_length = min_name_length;
	 fcontext.max_name_length = max_name_length;
	 fcontext.max_items = max_items;
	 fcontext.empty_stream_item_instead_of_drop = empty_stream_item_instead_of_drop;

	 if (!names_path.empty()) {
	     // construct names_begin_path
	     size_t ext_pos = names_path.rfind(".mmap");
	     copy(names_path.begin(), names_path.begin()+ext_pos, back_inserter(names_begin_path));
	     names_begin_path.append("_begin.mmap");

	     if (verbose) {
		 cerr << "\tnames_path: " << names_path << endl;
		 cerr << "\tnames_begin_path: " << names_begin_path << endl;
	     }
	 } else if (!names_simple_path.empty()) {
	 } else if (!names_scf_path.empty()) {
	 } else {
		 cerr << "need one of --names-mmap, --names-scf, or --names" << endl;
		 return 1;
	 }


	 ////////////////////////////////////////////////////////////// BUILD/CPU ID
	 cerr << "filter  " << ID ; 

	 #ifdef DEBUG 
		 cerr << "   MODE=DEBUG";
	 #endif

	 #ifdef OPTIMIZE 
		 cerr << "   MODE=OPTIMIZE";
	 #endif

	 #ifdef PROFILE 
		 cerr << "   MODE=PROFILE";
	 #endif

	 cerr << "   max-names: " << max_names << "   max-items: " << max_items << endl;

	 ////////////////////////////////////////////////////////////// READ FILTERNAMES
	 GPERF_START();

	 auto start = chrono::high_resolution_clock ::now();

	 //unordered_map<string, set<string>> target_text_map;

	 if (!names_simple_path.empty()) {
		 fcontext.read_simple_names(names_simple_path);
	 } else if (!names_scf_path.empty()) {
		 fcontext.read_scf_names(names_scf_path);
	 } else {
		 fcontext.read_mmap_names(names_path);
	 }

	 fcontext.compile_names();
	 struct rusage post_names_rusage;
	 getrusage(RUSAGE_SELF, &post_names_rusage);
	 clog << "rusage so far: ";
	 logrusage(clog, post_names_rusage);

	 // Time this annotator was started
	 sc::StreamTime streamtime;
	 time_t seconds;
	 seconds = time(NULL);
	 streamtime.epoch_ticks = seconds;
	 streamtime.zulu_timestamp = ctime(&seconds);
	 //annotator.__set_annotation_time(streamtime);

	 //////////////////////////////////////////////////////////////////////////// OPEN IN / OUT SC STREAMS

	 // Setup thrift reading and writing from stdin and stdout
	 int input_fd = 0;
	 int output_fd = 1;

	 if (!in_path.empty()) {
	     input_fd = open(in_path.c_str(), O_RDONLY);
	 }
	 if (!out_path.empty()) {
	     output_fd = open(out_path.c_str(), O_WRONLY|O_CREAT, 0666);
	 }

	 // input
	 boost::shared_ptr<att::TFDTransport>	        innerTransportInput(new att::TFDTransport(input_fd));
	 boost::shared_ptr<att::TBufferedTransport>	transportInput(new att::TBufferedTransport(innerTransportInput));
	 boost::shared_ptr<atp::TBinaryProtocol>		protocolInput(new atp::TBinaryProtocol(transportInput));
	 transportInput->open();

	 // output 
	 boost::shared_ptr<att::TFDTransport>		transportOutput(new att::TFDTransport(output_fd));
	 boost::shared_ptr<atp::TBinaryProtocol>		protocolOutput(new atp::TBinaryProtocol(transportOutput));
	 transportOutput->open();


	 ///////////////////////////////////////////////////////////////////////////////  SC ITEMS READ CYCLE
	 start = chrono::high_resolution_clock ::now();
	 sc::StreamItem stream_item;
	 long total_content_size=0;
	 size_t  stream_items_count=0;
	 int  matches=0;
	 int  written=0;
	 #ifdef DEBUG
	 clog << "Reading stream item content from : " << text_source << endl;
	 #endif

	 auto start100 = chrono::high_resolution_clock ::now();

	 if (threads > 1) {
		 stream_items_count = fcontext.run_threads(
		     threads, protocolInput.get(), protocolOutput.get());
	 } else {

	 while (true) {
		 try {
			 //target_text_map.clear();

				 //------------------------------------------------------------------   get item content
			 stream_item.read(protocolInput.get());

			 if (verbose   &&   stream_items_count % 100 == 0  &&  stream_items_count) {
				 auto diff  = chrono::high_resolution_clock ::now() - start100;
				 double sec = chrono::duration_cast<chrono::nanoseconds>(diff).count()/1e9;

				 cerr	<< "-- item: " << stream_items_count 
					 << "   \tavg time per item: "  << sec/100 << " sec"
					 << "   \titems/sec: "  << 100 / sec << endl;

				 start100 = chrono::high_resolution_clock ::now();
			 }


				 string content;

			bool any_match = false;

			any_match = fcontext.check_streamitem(&stream_item);
	    		
			if (any_match) { matches++; }

				if ((any_match && (! negate)) ||
					((! any_match) && negate)) {
					stream_item.write(protocolOutput.get());
					written++;
				} else if (empty_stream_item_instead_of_drop) {
				    static sc::StreamItem* empty_item;
				    if (empty_item == NULL) {
					empty_item = newEmptyItem();
				    }
				    empty_item->write(protocolOutput.get());
				}
	    		
	    		stream_items_count++;

				if (stream_items_count >= max_items) {
					clog << "hit item limit at " << stream_items_count << endl;
					break;
				}
	    	}

		//----------------------------------------------------------------------------  items read cycle exit

		//catch (att::TTransportException e) {
		catch (...) {
			clog << "exception" << endl;
			break;
		}
	 } // while(true) loop over items
	 // NOTE: these log lines are checked in test_filter.py
	 clog << "Total stream items processed: " << stream_items_count << endl;
	 clog << "Total matches: "                << matches << endl;
	 clog << "Total stream items written: "   << written << endl;
	 clog << "Added " << fcontext.rating_target_count << " ratings, "
	      << ((1.0 * fcontext.rating_target_count) / written)
	      << " ratings-per-written-item\n";

	 } // single thread inline execution

	 // Vital to flush the buffered output or you will lose the last one
	 transportOutput->flush();
	 if (negate) {
		 clog << " (Note, stream items written were non-matching ones)" << endl;
	 }

	/////////////////////////////////////////////////////////////////////////// TIMING RESULTS
	
	struct rusage end_rusage;
	getrusage(RUSAGE_SELF, &end_rusage);
	struct rusage match_rusage = end_rusage - post_names_rusage;
	clog << "match usage: ";
	logrusage(clog, match_rusage);
	clog << "total usage: ";
	logrusage(clog, end_rusage);

	{
	auto diff = chrono::high_resolution_clock ::now() - start;
	double nsec                 = chrono::duration_cast<chrono::nanoseconds>(diff).count();
	double search_time          = nsec/1e9;
	double stream_items_per_sec = double(stream_items_count) / (nsec/1e9);
	double mb_per_sec           = double(total_content_size)/1000000 / (nsec/1e9);

	clog << "search time: "      << search_time << " sec" << endl;
	clog << "stream items/sec: " << stream_items_per_sec << endl;
	clog << "MB/sec: "           << mb_per_sec << endl;

	std::ofstream  log("log",std::ofstream::app);       
	if (!log)  { cerr << "log file open failed\n" ; exit(2); }

	log << stream_items_count << '\t' <<  mb_per_sec << '\t' << stream_items_per_sec << endl;
	}

	ProfilerFlush();

	return 0;
}
