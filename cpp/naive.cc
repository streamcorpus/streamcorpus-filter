
#include "search.h"

#include <string>
#include <vector>
#include <iostream>


//////////////////////////////////////////////////////////////////////////////  NAMES IMPL

struct  names_t::names_impl {

	         names_impl () {};
	        ~names_impl () {}
	void    post_ctor   () {};
	void    print       () { for(size_t i=0;  i<B.size();  ++i) { std::cout << std::string(B[i], E[i]) << std::endl; } };
	size_t  size        () { return B.size(); };


	void insert(pos_t b, pos_t e) {
		B.push_back(b);
		E.push_back(e);
	};

	void set_content (pos_t b_, pos_t e_)   { b=b_;  e=e_; p=b; };

	bool find_next   (pos_t& match_b, pos_t& match_e) {
		for (; p <= e;  ++p)  {
			for(size_t i=0;  i<B.size();  ++i) {
				if ((E[i] - B[i]  < e-p)  &&  std::equal(B[i], E[i], p)) {
					match_b = p;
					match_e = p + (E[i] - B[i]);
					p = match_e;
					return true;
				}
			}
		}
		match_b = nullptr;
		return false;
	};

	// private
	pos_t  b, e, p;
	std::vector<pos_t> B,E;
};

SEARCH_IFACE_TO_IMPL_FORWARDING
