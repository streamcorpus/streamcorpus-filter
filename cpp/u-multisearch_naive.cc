#include "check.h"
#include "multisearch.h"
using namespace std;

int main() {
	vector<name_t>		names_str  {"aaa","bbb"};
	names_t			names_p;   for(name_t &n:  names_str)  names_p.push_back(&n);
	const name_t		*matched_name;

	{ ////////////////////////////////////////////////  FIND AAA, BBB
	string			content    ="aaa bbb aaa z";
	vector<pos_t>        	results;
	pos_t			pos        = content.begin();

	while ((pos = multisearch(pos, content.end(), names_p, matched_name)) != content.end()) {
		results.push_back(pos) ;
                pos += matched_name->size();
	}

	//CHECK_ARE_EQUAL (results, (vector<pos_t>{content.begin()+0, content.begin()+4, content.begin()+8}));
	CHECK_ARE_EQUAL (results.size(),  3);
	CHECK (results == (vector<pos_t>{content.begin()+0, content.begin()+4, content.begin()+8}));
	}


	{ ////////////////////////////////////////////////  NOT FOUND
	string			content    ="there are only xxx and yyy";
	vector<pos_t>        	results;
	pos_t			pos        = content.begin();

	while ((pos = multisearch(pos, content.end(), names_p, matched_name)) != content.end()) {
		results.push_back(pos) ;
                pos += matched_name->size();
	}

	CHECK_ARE_EQUAL (results.size(),  0);
	}

	CHECK_EXIT;
}




	

