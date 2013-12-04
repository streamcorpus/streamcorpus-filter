#include "check.h"
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

using namespace std;


int main() {

	/////////////////////////////////////////////////  NAMES
	vector<string>		names_str  {"aaa","bbb"};
	names_t			names;  
	for(string &n:  names_str)  names.insert(n.data(), n.data()+n.size());
	names.post_ctor();

	{ ////////////////////////////////////////////////  FIND AAA, BBB
	string		content         = "aaa bbb aaa z"; //"aaa bbb aaa z aaa aaa bbb";
	vector<long>    results;
	pos_t		b   	   	= content.data();
	pos_t		e          	= b+content.size();
	pos_t		match_b, match_e;

						// find all instanses of needle
	names.set_content(b, e);
	while (names.find_next(match_b, match_e)) {
		results.push_back(match_b - b) ;
	}

						// compare result with expected values
	CHECK_ARE_EQUAL (results.size(),  3);
	CHECK (results == (vector<long>{0, 4, 8}));
	#ifdef LVV
	CHECK_ARE_EQUAL (results, (vector<long>{0, 4, 8}));
	#endif
	}

	{ ////////////////////////////////////////////////  NOT FOUND
	string		content         = "there are only xxx and yyy";
	vector<long>    results;
	pos_t		b   	   	= content.data();
	pos_t		e          	= b+content.size();
	pos_t		match_b, match_e;

						// find all instanses of needle
	names.set_content(b, e);
	while (names.find_next(match_b, match_e)) {
		results.push_back(match_b - b) ;
	}

						// compare result with expected values
	CHECK_ARE_EQUAL (results.size(),  0);
	}

	CHECK_EXIT;
}
