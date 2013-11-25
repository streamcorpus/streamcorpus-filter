
#include "ahocorasick.h"
#include "search.h"

#include <iostream>
	using std::cout;
	using std::clog;
	using std::cerr;
	using std::endl;

#include <cassert>


//////////////////////////////////////////////////////////////////////////////  NAMES IMPL


struct  names_t::names_impl {

	AC_AUTOMATA_t   *atm;
	size_t		sz=0;		

	        names_impl () { atm = ac_automata_init (); };
	       ~names_impl () { ac_automata_release (atm); }
	void    post_ctor  () { ac_automata_finalize (atm); };
	void    print      () { ac_automata_display(atm,'s'); };
	size_t  size       () { return sz; };

	void insert(pos_t b, pos_t e) {

		if (e-b >= AC_PATTRN_MAX_LENGTH) {
	       		#ifdef DEBUG 
			cerr << "\twarning: name of length " << (e-b) << " skipped\n";
			#endif
			return;
		}

		AC_PATTERN_t    tmp_pattern;
		tmp_pattern.astring  = b;
		tmp_pattern.length   = e-b;
		AC_STATUS_t rc = ac_automata_add (atm, &tmp_pattern);
		if (rc != ACERR_SUCCESS) {
			cerr << "multifast error: ac_automata_add() exited"
				"\n\twith return code: " << rc <<
				"\n\tfor string of size: " << e-b <<
				"\n\tfor string: (" << std::string(b,e) <<  ")\n";
			exit(1);
		};
		++sz;
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
		
		
						#ifdef DEBUG
						if (matchp->match_num > 1) {
							clog << "warning: " << matchp->match_num <<  " matches @" << matchp->position << "\n";
							
							for (size_t i=0;  i < matchp->match_num; ++i) {
							    clog << '\t' << i <<  '\t' << matchp->patterns[i].astring;
							}
							
							clog << '\n';
						}

       						assert(matchp->match_num >= 1);
						#endif

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
size_t names_t::size       () 						        { return impl->size(); }

