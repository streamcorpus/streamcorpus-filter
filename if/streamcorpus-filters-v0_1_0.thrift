


namespace py  streamcorpus_filters
namespace cpp streamcorpus_filters

/**
 * target_id: unique identifier for a filtering target.  Typically a
 * URL that identifies an entity, such a a twitter profile URL or a
 * company homepage.
 */
typedef string target_id

/**
 * name: surface form name string to match for one or more
 * target_id(s).  For example, "John Smith" is a name that has more
 * than one target_id.
 */
typedef string name

struct Filter {
  /**
   * map of target_id to list of name strings.  This reflects how the
   * data is usually gathered: starting from a target_id, one scrapes
   * the profile page for alternate names.
   */
  1: optional map<target_id, list<name>> target_id_to_names

  /**
   * map of name strings to list of target_id strings that share that
   * name.  This is the inverse mapping of target_id_to_names and
   * reflects how the matcher must organize the data.  When a document
   * matches a name with multiple target_ids, the matcher must create
   * multiple streamcorpus.Rating objects for that one name string,
   * i.e. one for each target_id.  This is redundant with the above.
   */
  2: optional map<name, list<target_id>> names_to_target_id

}
