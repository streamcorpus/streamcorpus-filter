// Test search algorithms speed without thrift

#include "lvvlib/mmap.h"
#include "search.h"

#include <vector>
	using std::vector;

#include <string>
	using std::string;

#include <iostream>
	using std::cout;
	using std::clog;
	using std::endl;

#ifdef LVV
#include <ro/ro.h>
#include <scc/simple.h>
#endif

#include <chrono>
	using namespace std::chrono;
//	namespace chrono = std::chrono;


using namespace std;
       
       

int main() {

	cout << ID << endl;
	auto start = high_resolution_clock::now();

	size_t		max_names	= 
		#ifdef  MAX_NAMES 
			MAX_NAMES
		#else
			numeric_limits<size_t>::max();
		#endif
	;

	/////////////////////////////////////////////////  READ NAMES MMAP

	size_t 	names_size;	// number of names (size of names_begin-1)
	size_t 	names_mem;      // size of names_data;
	char 	*names_data  = mmap_read<char>  ("data/names_data.mmap",  names_mem);
	size_t	*names_begin = mmap_read<size_t>("data/names_begin.mmap", names_size);
	names_size--;


	/////////////////////////////////////////////////  CONSTRUCT NAMES_T 

	names_t		names;  

	for (size_t i=0;  i< std::min(max_names,names_size);  ++i) {
						//cerr << "addeing name: " <<  i << " " <<  names_begin[i] <<   names_begin[i+1] 
						//<< " (" << string(names_data+names_begin[i], names_data+names_begin[i+1]) << endl;
		names.insert(names_data+names_begin[i], names_data+names_begin[i+1]); 
	}

	names.post_ctor();


	///////////////////////////////////////////////  TIMER
	{
	auto diff = high_resolution_clock::now() - start;
	double sec = duration_cast<nanoseconds>(diff).count()/1e9;
	cerr << "Names: "  	                 << names_size << "\n"; 
	cerr << "Names used: "  	         << max_names << "\n"; 
	cerr << "Names construction time: "      << sec        << " sec\n";
	}


	////////////////////////////////////////////////  CONTENT
	size_t  corpus_mem;
	const char  *corpus = mmap_read<char>("data/corpus.mmap", corpus_mem);

	start = high_resolution_clock::now();


	pos_t		match_b, match_e;
	size_t 		match_count=0;
						// find all instanses of needle
	names.set_content(corpus, corpus+corpus_mem);


	while (names.find_next(match_b, match_e)) {
	       	#ifdef DISPLAY_MATCH
 	               cout << "with content: " <<  match_b - corpus << "\t("  << string(match_b, match_e) << ")\n\n";
	        #endif
	        match_count++;
	}


	///////////////////////////////////////////////  TIMER
	{
	auto diff = high_resolution_clock::now() - start;
	double sec                 = duration_cast<nanoseconds>(diff).count()/1e9;
	cerr << "matches: "      << match_count  << "\n"
	     << "search time: "  << sec << " sec\n"
	     << "MB/sec: "       << corpus_mem/sec/1000000 << "\n";
	}
}
