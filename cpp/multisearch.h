
#include <string>
#include <vector>

typedef  std::string::const_iterator   	pos_t;
typedef  std::string	       		name_t;
typedef	 std::vector<const name_t*>	names_t;

pos_t  multisearch(pos_t b,  pos_t e,  const names_t& names,  const name_t*&  matched_name);
