#include <stdio.h>

#include "search.h"

#include <vector>
	using std::vector;

#include <string>
	using std::string;

#include <iostream>
	using std::cout;
	using std::clog;
	using std::endl;


//////////////////////////////////////////////////////////////////////////////////////  MAIN
int main () {
    
	vector<string> names_str {
	    "simplicity",
	    "the ease",
	    "city",
	    " and ",
	    "simplicity of",
	    "experience",
	    "multifast",
	    "simp",
	    "good one",
	    "whatever",
	    "ever",
	    "a good one",
	    " of ",
	    "clutter",
	    "utter",
	    "clu",
	    "oneout",
	};



	// construct names
	names_t			names;    

	for(string name:  names_str)  { names.insert(name.data(), name.data()+name.size());   }
	names.post_ctor();
			

	////////////////////////////////////////////////
	string content = "experience the ease and simplicity of multifast"
			"whatever you are be a good one"
			"out of clutter, find simplicity";
	//string		content    	="aaa bbb aaa z";
	pos_t		b   	   	= content.data();
	pos_t		e          	= b+content.size();
	pos_t		p          	= b;
	pos_t		match_b = nullptr, match_e = nullptr;
	vector<int>     results;

						// find all instanses of needle
	while (names.search(p, e, match_b, match_e), match_b) {
		results.push_back(p-b);
                p = match_e;
				clog << p-b << ":" << std::string(match_b, match_e) << endl;
	}
}
