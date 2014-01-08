// Search algo unit tests

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

#include "normalize.h"


int test_normalize() {
    vector<string> johnsmiths {
"john smith jr",
"john smithjr",
"j smith jr",
"j smithjr",
"j smith",
"john smith"
};

    // each test case has exactly one match
    vector<string> test_texts {
"\nJohn  Smith is a mention, even though it has two spaces.",
"J. Smith is also a mention, even though our pattern does not have the \".\" symbol\n",
"J Smithjrthough is not a mention and J Smithjr is.\n",
"\nSometimes an extraneous newline breaks a mention to John\n\tSmith, and we still want to match it.\n",
#if 0
    // TODO: unicode strings, use ICU?
"\nOther times, special characters intervene John\u205F\u200ASmith\n",
#endif
"\nAny maybe we even want to match no spaces:  John Smithjr.\n"
    };

    names_t names;
    for (string& tname: johnsmiths) {
	names.insert(tname.data(), tname.data() + tname.size());
    }
    names.post_ctor();

    for (string& content: test_texts) {
	char* normtext = NULL;
	size_t* offsets = NULL;
	size_t normlen;
	size_t rawlen = content.size();
	offsets = new size_t[rawlen];
	normtext = new char[rawlen];
	normlen = normalize(content.data(), rawlen, offsets, normtext);
	names.set_content(normtext, normtext + normlen);
	
	int match_count = 0;
	pos_t match_b, match_e;
	while (names.find_next(match_b, match_e)) {
	    match_count++;
	    size_t startoffset = offsets[match_b - normtext];
	    size_t endoffset = offsets[match_e - normtext];
	    cout << "found=" << std::string(match_b, match_e) << " orig["<< startoffset<<"]=" << content.substr(startoffset, endoffset - startoffset) << endl;
	}
	//assert(match_count == 1);
	CHECK_ARE_EQUAL(match_count, 1);
	if (match_count != 1) {
	    cout << "found " << match_count << " matches in: " << content << endl;
	}

	delete [] offsets;
	delete [] normtext;
    }


    return 0;
}


int main() {
    test_normalize();
    

	/////////////////////////////////////////////////  NAMES
	vector<string>		names_str  {
		"aaa",
		"bbb"
	};
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
