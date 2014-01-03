// converts scf names into mmap file
//

#include "search.h"
#include "mmap.h"


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

// LVVLIB (http://github.com/lvv/lvvlib)
//#ifdef LVV
//#include <ro/ro.h>
//#include <scc/simple.h>
//#endif
#include "lvvlib/token.h"




//////////////////////////////////////////////////////////////////////////////////////



template<size_t N=4015601, size_t MEM=80177319, class T=char> 	// <-----------  change value according to count_names run
struct names_data_t{
	size_t EI[N];   // end of string at address:   std::begin(data) + EI[i]
	T data[MEM];
	const static size_t size=N;	// number of names
	const static size_t mem=MEM;    // total number of bytes occupied by names
};


bool  good_name(const pos_t b, const pos_t e) {
	constexpr long	max_name_length = 64;
	constexpr long	min_name_length = 3;
	//constexpr long	min_name_tokens = 1;

	if ( e-b > max_name_length ) return false;
	if ( e-b < min_name_length ) return false;

        pos_t tb = get_tb(b,e);
	if (tb==e) return false;
	return true;
};

int main(int argc, char **argv) {
	
	///////////////////////////////////////////////////////////////////////////////  OPTIONS

	// options
	string		text_source	="clean_visible";
	string		filtername_path;
	long		max_names	= numeric_limits<long>::max();
	long		max_items	= numeric_limits<long>::max();
	bool		count_only	= false;

	po::options_description desc("Allowed options");

	desc.add_options()
		("help,h", "help message")
		("text_source,t", po::value<string>(&text_source), "text source in stream item")
		("filternames,f", po::value<string>(&filtername_path), "filternames file")
		("max-names,N", po::value<long>(&max_names), "maximum number of names to use")
		("count,c", "count names")
	;
	
	// Parse command line options
	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);
	
	if (vm.count("help")) {
		cout << desc << "\n";
		return 1;
	}

	if (vm.count("count")) count_only = true;


	
	////////////////////////////////////////////////////////////// BUILD/CPU ID
	cerr << "mk_mmap_names  " << ID ; 

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

							//filter_names.name_to_target_ids["John Smith"] = vector<string>();

	unordered_map<string, set<string>> target_text_map;

	
	#ifdef LVV
		// a hack to release some memory
		filter_names.target_id_to_names = std::map<std::string, std::vector<std::string>>();
	#endif

	size_t name_min=9999999999;
	size_t name_max=0;
	size_t total_names_length=0;

	size_t good_names = 0; 	  
	size_t bad_names = 0; 	  
	
	names_data_t<>*  names_data = new names_data_t<>();
	char*  b0 = &(names_data->data[0]);     // beginning of data 
	pos_t  b  =b0;				// beginning of current name
	char*  d = b0;          		// pointer to  end of written data (and beginning of free data)
	size_t i = 0; 				// index in EI 
							// clog << "begin: " << (void*)b << endl;

	// for all names in filter_names
	for(auto& pr : filter_names.name_to_target_ids) {
		auto s  = pr.first.data();
		size_t sz = pr.first.size();

		if (good_name(s, s+sz)) {

			++good_names;
			name_min = std::min(name_min, sz);
			name_max = std::max(name_max, sz);

			if (!count_only) {
				// copy a name to names_data 
				assert (i < names_data->size);
				assert(0 < sz  &&  sz < 10000);

				std::copy(s, s+sz, d);
				d += sz;
				names_data->EI[i] = d - b0;

				assert(names_data->EI[i]  <=  names_data->mem);
				++i;
					//cerr << i << "\t(" << string(s,s+sz) << ")\n";
			}
			total_names_length += sz;
		} else {
			++bad_names;
		}

	}
	transportScf->close();


	clog << "NAMES: "  << filter_names.name_to_target_ids.size()
	     << ";\n\t in file:    "         << names_data->size
	     << ";\n\t good:       "         << good_names
	     << ";\n\t bad:        "         << bad_names
	     << ";\n\t min length: "         << name_min
	     << ";\n\t max length: "         << name_max
	     << ";\n\t avg length: "         << double(total_names_length)/names_data->size
	     << ";\n\t total names length: " << total_names_length
	     << endl;

	if (count_only)  exit(0);

	cerr << "writing names memory map file\n";
	mmap_write("names_data.mmap", *names_data);

	exit(0);


	///////////////////////////////////////////////////////////////////////////////  STREAMCORPUS
					/*// check data
					 *
					for(const auto& pr : filter_names.target_id_to_names) {
						clog << pr.first << endl;
						for(auto& name : pr.second) {
							clog << '\t' << name << endl;
						}
					}*/
	// Setup thrift reading and writing from stdin and stdout
	int input_fd = 0;
	
	// input
	boost::shared_ptr<att::TFDTransport>	        innerTransportInput(new att::TFDTransport(input_fd));
	boost::shared_ptr<att::TBufferedTransport>	transportInput(new att::TBufferedTransport(innerTransportInput));
	boost::shared_ptr<atp::TBinaryProtocol>		protocolInput(new atp::TBinaryProtocol(transportInput));
	transportInput->open();


	///////////////////////////////////////////////////////////////////////////////  SC ITEMS READ CYCLE
	sc::StreamItem stream_item;
	long total_content_size=0;
	int  stream_items_count=0;
	
	// content file
	std::ofstream  corpus_file("corpus.txt");
	std::string 	buf(100000,0);

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
			buf.assign(b,e);
	    		
			corpus_file << "\n<item-" <<  stream_items_count << "> " << buf.assign(b,e) << '\n';
	    		
	    		// Increment count of stream items processed
	    		stream_items_count++;

			cerr << "item: " << stream_items_count << '\n';
			//if (stream_items_count >= max_items)  throw att::TTransportException();

	    	}

		//----------------------------------------------------------------------------  items read cycle exit

		catch (...) {
			clog << "Total stream items processed: " << stream_items_count << endl;
			break;
		}
	}
}

