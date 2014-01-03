// Converts filternames.scf names into mmap file
//
// There are 2 output names data/names_data.mmap and data/names_begin.mmap
// Usage:
// 	scf2mmap -f path-to-filter-names.scf


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

// THRIFT -- FILTERNAMES
#include "filternames_types.h"
#include "filternames_constants.h"
	namespace fn = filternames;

// STD
#include <iostream>
	using std::cerr;
	using std::cout;
	using std::endl;

#include <algorithm>
	using std::copy;
#include <iterator>
	using std::back_inserter;
#include <unordered_map>
	using std::unordered_map;
#include <set>
	using std::set;
#include <string>
	using std::string;
#include <vector>
	using std::vector;
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
#include "lvvlib/mmap.h"


int main(int argc, char **argv) {
	
	///////////////////////////////////////////////////////////////////////////////  OPTIONS

	string		filtername_path;

	po::options_description desc("Allowed options");
	desc.add_options()
		("help,h", "help message")
		("filternames,f", po::value<string>(&filtername_path), "filternames file")
	;
	
	// Parse command line options
	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);
	
	if (vm.count("help")) { cout << desc << "\n"; return 1; }
	
	////////////////////////////////////////////////////////////// THRIFT READS FILTERNAMES

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

	unordered_map<string, set<string>> target_text_map;

	
	#ifdef LVV
		// a hack to release some memory
		filter_names.target_id_to_names = std::map<std::string, std::vector<std::string>>();
	#endif

	////////////////////////////////////////////////////////////////////////////////// FILTER AND ADD TO NAMES
	
	vector<char>	names_data;		// cancantenated content of names
	vector<char>	name;                   // temp name
	vector<size_t>	names_begin{0};		// array of indexes in names_data which point to begining of a names. 
						// a name i is from names_begin[i] to names_begin[i+1]

	size_t name_min=9999999999;
	size_t name_max=0;
	size_t good_names = 0; 	  
	
	set<vector<char>> uniq_set;            	// used in check for uniqness
	long total_tokens = 0;


	// for all names in filter_names
	for(auto& pr : filter_names.name_to_target_ids) {
		auto s  = pr.first.data();                             		// name
		size_t sz = pr.first.size();                                    // is size

						#ifdef DISPLAY_NAMES
						cout << "\nscf name: \t(" << string(s,s+sz) << ")\n";
						#endif
		// split into tokens
		pos_t  tb, te = s;             					// token begin, end
		pos_t  e = s + sz;
		name.clear();
		long tokens = 0;
		while ((tb=get_tb(te,e)) != e) {
			te=get_te(tb,e);     
			if (!name.empty())  name.push_back(token_separator);
			copy (tb, te, back_inserter(name));
			++tokens;
		}

		// filter out bad names
		constexpr long	max_name_length = 1000;
		constexpr long	min_name_length = 3;
		constexpr long	min_name_tokens = 1;

		if (
			    ( min_name_length <= name.size()  &&  name.size() <= max_name_length )
			&&  ( min_name_tokens <= tokens )
			&&  ( uniq_set.find(name) == uniq_set.end() )
		) {
			++good_names;
		} else {
			continue;
		}

		uniq_set.insert(name);
						#ifdef DISPLAY_NAMES
						cout << "added name: \t(" << string(name.begin(), name.end()) << ")\n";
						#endif
		// statistics
		name_min = std::min(name_min, name.size());
		name_max = std::max(name_max, name.size());
		total_tokens += tokens;

		// append name to names_data
		copy (name.begin(), name.end(), back_inserter(names_data));
		names_begin.push_back(names_data.size());
	}

	transportScf->close();

	cerr << "NAMES: "  << filter_names.name_to_target_ids.size()
	     << ";\n\t good:       "         << good_names
	     << ";\n\t min length: "         << name_min
	     << ";\n\t max length: "         << name_max
	     << ";\n\t avg length: "         << double(names_data.size())/(names_begin.size()-1)
	     << ";\n\t avg tokens: "         << double(total_tokens)/(names_begin.size()-1)
	     << ";\n\t total names length: " << names_data.size()
	     << endl;


	///////////////////////////////////////////////////////////////////////////////// WRITE MMAP
       
	cerr << "writing names memory map file ... ";

	lvv::mmap_write<char>  ("data/names_data.mmap",  &names_data[0],  names_data.size());
	lvv::mmap_write<size_t>("data/names_begin.mmap", &names_begin[0], names_begin.size());

	cerr << "done\n";
}

// vim: ts=8 sw=8
