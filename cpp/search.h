
#include <cstddef>

//////////////////////////////////////////////////////////////////////////////  NAMES IFACE

typedef  const char*   	pos_t;

struct  names_t {                            // pimpl idiom

	       names_t    ();
	      ~names_t    ();
	void   post_ctor  ();
	void   insert     (pos_t b, pos_t e);
	void   search     (pos_t b, pos_t e, pos_t& match_b, pos_t& match_e);
	void   print      ();
	size_t size       ();

	struct names_impl;
	names_impl *impl;
};


