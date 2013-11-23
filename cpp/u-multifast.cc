#include <stdio.h>

#include "ahocorasick.h"
#include "search.h"
#include <string.h>

#include <cassert>

#include <vector>
	using std::vector;

#include <string>
	using std::string;

#include <iostream>
	using std::cout;
	using std::clog;
	using std::endl;


//////////////////////////////////////////////////////////////////////////////  NAMES IMPL


struct  names_t::names_impl {

	AC_AUTOMATA_t   *atm;

	      names_impl () { atm = ac_automata_init (); };
	     ~names_impl () { ac_automata_release (atm); }
	void  post_ctor  () { ac_automata_finalize (atm); };
	void  print      () { ac_automata_display(atm,'s'); };

	void insert(pos_t b, pos_t e) {
		AC_PATTERN_t    tmp_pattern;
		tmp_pattern.astring  = b;
		tmp_pattern.length   = e-b;
									AC_STATUS_t rc = ac_automata_add (atm, &tmp_pattern);
									assert(rc == ACERR_SUCCESS);
	};
	
	void search  (pos_t b, pos_t e,  pos_t& match_b, pos_t& match_e) {

		// set content
		AC_TEXT_t       tmp_text;
		tmp_text.astring = b;
		tmp_text.length  = e-b;
		ac_automata_settext (atm, &tmp_text, 0); 
				    
						     // struct AC_MATCH_t {
						     //     AC_PATTERN_t * patterns; /* Array of matched pattern */
						     //     long position;           /* The end position of matching pattern(s) in the text */
						     //     unsigned int match_num;  /* Number of matched patterns */
						     // };

		// search
		AC_MATCH_t * matchp  = ac_automata_findnext(atm);

		// return results
		if (!matchp) 	{ match_b = nullptr;  return; }
		
		
						if (matchp->match_num > 1) {
							clog << "warning: " << matchp->match_num <<  " matches @" << matchp->position << "\n";
							
							for (size_t i=0;  i < matchp->match_num; ++i) {
							    clog << '\t' << i <<  '\t' << matchp->patterns[i].astring;
							}
							
							clog << '\n';
						}

       						assert(matchp->match_num >= 1);

		match_e = b + matchp->position;
		match_b = match_e - matchp->patterns[0].length;
	}
};

// names_t definitions

       names_t::names_t    ()  { impl = new names_impl; }
       names_t::~names_t   ()  { delete impl; }

void   names_t::post_ctor  ()                                  		        { impl->post_ctor(); };
void   names_t::insert     (pos_t b, pos_t e)                                   { impl->insert(b, e); }
void   names_t::search     (pos_t b, pos_t e,  pos_t& match_b, pos_t& match_e)  { impl->search(b, e, match_b, match_e); }
void   names_t::print      () 						        { impl->print(); }


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
