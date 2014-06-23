// Search algo unit tests

#include <assert.h>

#include "check.h"
#include "search.h"

#include <vector>
	using std::vector;

#include <string>
	using std::string;

#include <sstream>
using std::stringstream;

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
	//"john smith jr",
    //"john smithjr",
    //"j smith jr",
    //"j smithjr",
"j smith",
"john smith",
"j\xc3\xb8hn smith",  // Jo/hn Smith
"m\xc3\xb8\xc3\xb8se"  // mo/o/se
};

    // each test case has exactly one match
    vector<string> test_texts {
"\nJohn  Smith is a mention, even though it has two spaces.",
    // "J. Smith is also a mention, even though our pattern does not have the \".\" symbol\n", // normalize doesn't filter out '.', do we really want it to?
    // "J Smithjrthough is not a mention and J Smithjr is.\n", // Actually, AC matcher doesn't detect word boundaries, but that's okay, a few false positives are okay.
"\nSometimes an extraneous newline breaks a mention to John\n\tSmith, and we still want to match it.\n",
"\nOther times, special characters intervene John\u205F\u200ASmith\n",
    "\nAny maybe we even want to match no spaces:  John Smithjr.\n",
    //"J\xc3\xb6hn   \t  \tw\xc3\x85t\tSm\xc3\xaeth",
    "space J\xc3\xb6hn   \t  \t\tSm\xc3\xaeth space", // J&ouml;hn Sm&iacute;th
    "space J\xc3\xb8hn \tSmith space ", // Jo/hn Smith
    "my sister was once bitten by a m\xc3\xb8\xc3\xb8se, no reeli!"
    };

    names_t names;
    for (string& tname: johnsmiths) {
	names.insert(tname.data(), tname.data() + tname.size());
    }
    names.post_ctor();

    for (string& content: test_texts) {
	size_t normlen;
	string normstr;
	std::vector<size_t> offsets;
	assert(normstr.empty());
	assert(offsets.empty());
	normalize(content, &normstr, &offsets, NULL);
	const char* normtext = normstr.data();
	normlen = normstr.size();
	names.set_content(normtext, normtext + normlen);
	
	int match_count = 0;
	pos_t match_b, match_e;
	vector<string> messages;
	while (names.find_next(match_b, match_e)) {
	    match_count++;
	    size_t startoffset = offsets[match_b - normtext];
	    size_t endoffset = offsets[match_e - normtext];
	    std::stringstream msg;
	    msg << "found=" << std::string(match_b, match_e) << " orig[" << startoffset << "]=" << content.substr(startoffset, endoffset - startoffset) << endl;
	    messages.push_back(msg.str());
	}
	CHECK_ARE_EQUAL(match_count, 1);
	if (match_count != 1) {
	    for (string& mt : messages) {
		cout << mt;
	    }
	    cout << "found " << match_count << " matches in: " << content << endl << "\tnormtext: " << normstr << endl;
	}
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
