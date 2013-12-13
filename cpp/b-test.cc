#include "mmap.h"
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

	constexpr size_t names      = 4446930;
	constexpr size_t max_names  = 2000000;
	constexpr size_t names_mem  = 116942447;
	constexpr size_t corpus_mem = 3117109254;

template<size_t N=names, size_t TOTAL=names_mem, class T=char>
struct names_data_t{
	T data[TOTAL];
	size_t EI[N];

	const static size_t size=N;
	const static size_t total_size=TOTAL;
};


int main() {

	cout << ID << endl;
	auto start = high_resolution_clock::now();
	string		S(100000,'*');
	/////////////////////////////////////////////////  NAMES
	names_data_t<>&  names_data = lvv::mmap_read<names_data_t<>>("data/names_data_N4446930_T116942447.mmap");

	names_t			names;  

	pos_t	b0 = &(names_data.data[0]);
	pos_t	b  = b0;
	size_t	i  = 0;
					//cout << "names: " << (sizeof(names_data.EI) / sizeof(pos_t)) << endl;
	for(size_t ei:  names_data.EI)  {
                pos_t e = b0+ei;
		names.insert(b, e); 
					//cout << i++ << '\t' << (void*)b << '\t' << (void*)e << '\t' << e-b << '\t' << ei << endl; 
		b=e; 
		if (++i > max_names) break;
	} ;
	names.post_ctor();

	{ ///////////////////////////////////////////////  TIMER
	auto diff = high_resolution_clock::now() - start;
	double sec = duration_cast<nanoseconds>(diff).count();
	clog << "Names: "  << (sizeof(names_data.EI) / sizeof(pos_t))
	     << ";  used: "        << max_names 
	     << endl;

	clog << "Names construction time: "      << sec/1e9 << " sec" << endl;
	}


	////////////////////////////////////////////////  CONTENT
	typedef  const char  corpus_t[corpus_mem];
	corpus_t&  corpus = lvv::mmap_read<corpus_t>("data/corpus_I47233_T3117109254.txt");
	start = high_resolution_clock::now();


	pos_t		match_b, match_e;
	size_t 		match_count=0;
						// find all instanses of needle
	names.set_content(corpus, corpus+corpus_mem);

	while (names.find_next(match_b, match_e)) {
	       //cout <<  match_count++ << '\t' << match_b - corpus << "\t("  << S.assign(match_b, match_e) << ")\n";
	       //cout <<  match_count << '\t' << match_b - corpus << endl;
	       if (match_e - match_b > 2000) {
		       cerr << "warning: big match: " << match_e - match_b << "  at: " << match_b - corpus << endl;
	       }
	       match_count++;
	}


	{ ///////////////////////////////////////////////  TIMER
	auto diff = high_resolution_clock::now() - start;
	double nsec                 = duration_cast<nanoseconds>(diff).count();
	double search_time          = nsec/1e9;
	double mb_per_sec           = double(corpus_mem)/1000000 / (nsec/1e9);

	clog << "matches: "          << match_count << endl;
	clog << "search time: "      << search_time << " sec" << endl;
	clog << "MB/sec: "           << mb_per_sec << endl;
	}
}
