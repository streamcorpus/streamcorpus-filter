#include "search.h"

// LIBC
#include <fcntl.h>
#include <time.h>

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
#include "streamcorpus_types.h"
#include "streamcorpus_constants.h"
	namespace sc = streamcorpus;

// THRIFT -- FILTERNAMES
#include "filternames_types.h"
#include "filternames_constants.h"
	namespace fn = filternames;


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


// BOOST
#include <boost/program_options.hpp>
	namespace po = boost::program_options;



int main(int argc, char **argv) {
	
	///////////////////////////////////////////////////////////////////////////////  OPTIONS

	// options
	string		text_source	="clean_visible";
	string		filtername_path;
	bool		negate		= false;
	long		max_names	= numeric_limits<long>::max();
	long		max_items	= numeric_limits<long>::max();

	po::options_description desc("Allowed options");

	desc.add_options()
		("help,h", "help message")
		("text_source,t", po::value<string>(&text_source), "text source in stream item")
		("negate,n", po::value<bool>(&negate)->implicit_value(true), "negate sense of match")
		("filternames,f", po::value<string>(&filtername_path), "filternames file")
		("max-names,N", po::value<long>(&max_names), "maximum number of names to use")
		("max-items,I", po::value<long>(&max_items), "maximum number of items to process")
	;
	
	// Parse command line options
	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);
	
	if (vm.count("help")) {
		cout << desc << "\n";
		return 1;
	}

	
	////////////////////////////////////////////////////////////// BUILD/CPU ID
	cerr << ID << endl; 

	#ifdef DEBUG 
		cerr << "MODE=DEBUG\n";
	#endif

	#ifdef OPTIMIZE 
		cerr << "MODE=OPTIMIZE\n";
	#endif

	////////////////////////////////////////////////////////////// READ FILTERNAMES
	
	auto start = chrono::high_resolution_clock ::now();

	int scf_fh = open(filtername_path.c_str(), O_RDONLY);

					if(scf_fh==-1)  {
						cerr << "error: cann't open scf file -- '" << filtername_path << "'\n";
						exit(1);
					}


	boost::shared_ptr<att::TFDTransport>		innerTransportScf(new att::TFDTransport(scf_fh));
	boost::shared_ptr<att::TBufferedTransport>	transportScf(new att::TBufferedTransport(innerTransportScf));
	boost::shared_ptr<atp::TBinaryProtocol>		protocolScf(new atp::TBinaryProtocol(transportScf));
	transportScf->open();
	
	fn::FilterNames filter_names;
	filter_names.read(protocolScf.get());
	names_t  names;

							//filter_names.name_to_target_ids["John Smith"] = vector<string>();

	unordered_map<string, set<string>> target_text_map;

	
	#ifdef LVV
		// a hack to release some memory
		filter_names.target_id_to_names = std::map<std::string, std::vector<std::string>>();
	#endif

	size_t name_min=9999999999;
	size_t name_max=0;
	size_t total_name_length=0;
      
	for(auto& pr : filter_names.name_to_target_ids) {
		if ((long)names.size() >= max_names) break;
		auto p  = pr.first.data();
		auto sz = pr.first.size();
		names.insert(p , p+sz);

		// names stats
		name_min = std::min(name_min,sz);
		name_max = std::max(name_max,sz);
		total_name_length += sz;
	}
	names.post_ctor();
	transportScf->close();

	{
	auto diff = chrono::high_resolution_clock ::now() - start;
	double sec = chrono::duration_cast<chrono::nanoseconds>(diff).count();
	clog << "Names in file: "  << filter_names.name_to_target_ids.size()
	     << ";  used: "        << names.size()
	     << ";  min: "         << name_min
	     << ";  max: "         << name_max
	     << ";  avg: "         << double(total_name_length)/names.size()
	     << endl;

	clog << "Names construction time: "      << sec/1e9 << " sec" << endl;
	}

					/*// check data
					for(const auto& pr : filter_names.target_id_to_names) {
						clog << pr.first << endl;
						for(auto& name : pr.second) {
							clog << '\t' << name << endl;
						}
					}*/
	/////////////////////////////////////////////////////////////////////////////////// SC Objects
	

	// Create annotator object
	sc::Annotator annotator;
	sc::AnnotatorID annotatorID;
	annotatorID = "example-matcher-v0.1";
	
	// Annotator identifier
	annotator.annotator_id = "example-matcher-v0.1";
	
	// Time this annotator was started
	sc::StreamTime streamtime;
	time_t seconds;
	seconds = time(NULL);
	streamtime.epoch_ticks = seconds;
	streamtime.zulu_timestamp = ctime(&seconds);
	annotator.__set_annotation_time(streamtime);

	
	// Setup thrift reading and writing from stdin and stdout
	int input_fd = 0;
	int output_fd = 1;
	
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
	int  stream_items_count=0;
	int  matches=0;
	int  written=0;
	#ifdef DEBUG
	clog << "Reading stream item content from : " << text_source << endl;
	#endif
	
	while (true) {
		try {
 	    		//------------------------------------------------------------------   get item content
	    		stream_item.read(protocolInput.get());
		       
	    		string content;
	    		string actual_text_source = text_source;
	    		if (text_source == "clean_visible") {
	    			content = stream_item.body.clean_visible;
	    		} else if (text_source == "clean_html") {
	    			content = stream_item.body.clean_html;
	    		} else if (text_source == "raw") {
	    			content = stream_item.body.raw;
	    		} else {
	    			cerr << "Bad text_source :" << text_source <<endl;
	    			exit(-1);
	    		}
	    		
	    		if (content.size() <= 0) {
	    			// Fall back to raw if desired text_source has no content.
	    			content = stream_item.body.raw;
	    			actual_text_source = "raw";
	    			if (content.size() <= 0) {
	    				// If all applicable text sources are empty, we have a problem and exit with an error
	    				cerr << stream_items_count << " Error, doc id:" << stream_item.doc_id << " was empty." << endl;
	    				exit(-1);
	    			}
	    		}

			total_content_size += content.size();
	    		
            		
	    		//------------------------------------------------------------------   multisearch cycle

			pos_t		b   	   	= content.data();
			pos_t		e          	= b+content.size();
			pos_t		p          	= b;
			pos_t		match_b, match_e;

			names.set_content(p, e);

			while (names.find_next(match_b, match_e)) {
			
				// found
				#ifdef DEBUG
				clog << stream_items_count << " \tdoc-id:" << stream_item.doc_id;
				clog << "   pos:" << match_b-b << " \t" << std::string(match_b, match_e) << "\n";
				#endif
			
			
				// mapping between canonical form of target and text actually found in document
			
				// For each of the current matches, add a label to the 
				// list of labels.  A label records the character 
				// positions of the match.
				matches++;
				
				// Add the target identified to the label.  Note this 
				// should be identical to what is in the rating 
				// data we add later.
				sc::Target target;
				target.target_id = "1";
			
				sc::Label label;
				label.target = target;
			
				// Add the actual offsets 
				sc::Offset offset;
				offset.type = sc::OffsetType::CHARS;
				
				offset.first = match_b - b;
				offset.length = match_e - match_b;
				offset.content_form = std::string(match_b, match_e);
			
				label.offsets[sc::OffsetType::CHARS] = offset;
				label.__isset.offsets = true;
			
				// Add new label to the list of labels.
				stream_item.body.labels[annotatorID].push_back(label);
			
				// Map of actual text mapped 
				target_text_map[target.target_id].insert(std::string(match_b, match_e));

				// advance pos to begining of unsearched content
				p = match_e;
			}
	    		
	    		//------------------------------------------------------------------   sc processing
			
	    		// Add the rating object for each target that matched in a document
	    		for ( auto match=target_text_map.begin(); match!=target_text_map.end(); ++match) {
	    			// Construct new rating
	    			sc::Rating rating;
	    			
	    			// Flag that it contained a mention
	    			rating.__set_contains_mention(true);
	    			
	    			// Construct a target object for each match
	    			sc::Target target;
	    			target.target_id = match->first;
	    			rating.target = target;
	    			
	    			// Copy all the strings that matched into the mentions field
	    			copy(match->second.begin(), match->second.end(), back_inserter(rating.mentions));
	    			
	    			// Subtle but vital, we need to do the following for proper serialization.
	    			rating.__isset.mentions = true;
	    			
	    			// Add the annotator from above.
	    			rating.annotator = annotator;
	    			
	    			// Push the new rating onto the rating list for this annotator.
	    			stream_item.ratings[annotatorID].push_back(rating);
	    		}
	    		
	    		if (not negate) { 
	    		        // Write stream_item to stdout if it had any ratings
	    		        if (target_text_map.size() > 0) {
	    		            stream_item.write(protocolOutput.get());
	    		            written++;
	    		        }
	    		} else if (target_text_map.size() == 0) {
	    			// Write stream_item to stdout if user requested
	    		       	// to show ones that didn't have any matches
	    			stream_item.write(protocolOutput.get());
	    			written++;
	    		}
	    		
	    		// Increment count of stream items processed
	    		stream_items_count++;
			if (stream_items_count >= max_items)  throw att::TTransportException();
	    	}

		//----------------------------------------------------------------------------  items read cycle exit

		//catch (att::TTransportException e) {
		catch (...) {
			// Vital to flush the buffered output or you will lose the last one
			transportOutput->flush();
			clog << "Total stream items processed: " << stream_items_count << endl;
			clog << "Total matches: "                << matches << endl;
			clog << "Total stream items written: "   << written << endl;
			if (negate) {
				clog << " (Note, stream items written were non-matching ones)" << endl;
			}
			break;
		}
	}

	/////////////////////////////////////////////////////////////////////////// TIMING RESULTS
	
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

}

