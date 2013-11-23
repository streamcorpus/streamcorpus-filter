
#include "search.h"

#include <string>
#include <vector>
#include <iostream>



//////////////////////////////////////////////////////////////////////////////  NAMES IMPL


struct  names_t::names_impl {

	std::vector<pos_t> B,E;

	         names_impl () {};
	        ~names_impl () {}
	void    post_ctor   () {};
	void    print       () { for(size_t i=0;  i<B.size();  ++i) { std::cout << std::string(B[i], E[i]) << std::endl; } };
	size_t  size        () { return B.size(); };


	void insert(pos_t b, pos_t e) {
		B.push_back(b);
		E.push_back(e);
	};

	
	void search  (pos_t b, pos_t e,  pos_t& match_b, pos_t& match_e) {
		for (pos_t p = b;  p <= e;  ++p)  {
			for(size_t i=0;  i<B.size();  ++i) {
				if ((E[i] - B[i]  < e-p)  &&  std::equal(B[i], E[i], p)) {
					match_b = p;
					match_e = p + (E[i] - B[i]);
					return;
				}
			}
		}
		match_b = nullptr;
	};
};

// names_t definitions

       names_t::names_t    ()  { impl = new names_impl; }
       names_t::~names_t   ()  { delete impl; }

void   names_t::post_ctor  ()                                  		        { impl->post_ctor(); };
void   names_t::insert     (pos_t b, pos_t e)                                   { impl->insert(b, e); }
void   names_t::search     (pos_t b, pos_t e,  pos_t& match_b, pos_t& match_e)  { impl->search(b, e, match_b, match_e); }
void   names_t::print      () 						        { impl->print(); }
size_t names_t::size       () 						        { return impl->size(); }

