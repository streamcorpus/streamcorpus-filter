
					#ifndef  DIFFEO_SEARCH_H
					#define  DIFFEO_SEARCH_H
#include <cstddef>

//////////////////////////////////////////////////////////////////////////////  NAMES IFACE

typedef  const char*   	pos_t;

struct  names_t {                       // pimpl idiom

	       names_t    ();
	      ~names_t    ();
	void   post_ctor  ();
	void   insert     (pos_t b, pos_t e);
	void   set_content(pos_t b, pos_t e);
	bool   find_next  (pos_t& match_b, pos_t& match_e);
	void   print      ();
	size_t size       ();

	struct names_impl;
	names_impl *impl;
};

#define  SEARCH_IFACE_TO_IMPL_FORWARDING \
	       names_t::names_t    ()  					{ impl = new names_impl; }                       \
	       names_t::~names_t   ()  					{ delete impl; }                                 \
	void   names_t::post_ctor  ()                                  	{ impl->post_ctor(); };                          \
	void   names_t::insert     (pos_t b, pos_t e)                   { impl->insert(b, e); }                          \
	void   names_t::set_content(pos_t b, pos_t e) 	 		{ impl->set_content(b, e); }                     \
	bool   names_t::find_next  (pos_t& match_b, pos_t& match_e)  	{ return impl->find_next(match_b, match_e); }    \
	void   names_t::print      () 					{ impl->print(); }                               \
	size_t names_t::size       () 					{ return impl->size(); }                         \

					#endif


