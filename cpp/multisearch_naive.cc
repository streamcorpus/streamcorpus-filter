
#include "multisearch.h"

pos_t  multisearch(pos_t b,  pos_t e,  const names_t& names,  name_t*&  matched_name) {
	for (auto pos = b;  pos < e;  ++pos)  {
		for(auto name : names)  {
			if ((name->end() - name->begin()  <= e-pos)  &&  std::equal(name->begin(), name->end(), pos)) {
				matched_name = name;
				return pos;
			}
		}
	}
	matched_name = nullptr;
	return e;
};
