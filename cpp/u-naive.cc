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

using namespace std;

							//#include "ro/ro.h"
							//#include "scc/simple.h"

int main() {
	vector<string>		names_str  {"aaa","bbb"};
	names_t			names;  
	for(string &n:  names_str)  names.insert(n.data(), n.data()+n.size());

	{ ////////////////////////////////////////////////  FIND AAA, BBB
	string		content         = "aaa bbb aaa z";
	vector<long>    results;
	pos_t		b   	   	= content.data();
	pos_t		e          	= b+content.size();
	pos_t		p          	= b;
	pos_t		match_b, match_e;

						// find all instanses of needle
	while (names.search(p, e, match_b, match_e), match_b) {
		results.push_back(match_b - b) ;
                p = match_e;
	}

						// compare result with expected values
	CHECK_ARE_EQUAL (results.size(),  3);
	//CHECK_ARE_EQUAL (results, (vector<long>{0, 4, 8}));
	CHECK (results == (vector<long>{0, 4, 8}));
	}

	{ ////////////////////////////////////////////////  NOT FOUND
	string		content         = "there are only xxx and yyy";
	vector<long>    results;
	pos_t		b   	   	= content.data();
	pos_t		e          	= b+content.size();
	pos_t		p          	= b;
	pos_t		match_b, match_e;

						// find all instanses of needle
	while (names.search(p, e, match_b, match_e), match_b) {
		results.push_back(p-b) ;
                p = match_e;
	}
						// compare result with expected values
	CHECK_ARE_EQUAL (results.size(),  0);
	}

	CHECK_EXIT;
}
