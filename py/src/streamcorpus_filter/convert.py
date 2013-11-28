'''
constructs new FilterNames instance from JSON
'''
import os
import sys
import json
from _filter import Filter
from ttypes import FilterNames

def log(m):
    sys.stderr.write(m)
    sys.stderr.write('\n')
    sys.stderr.flush()


def convert_utf8(dict_of_unicode):
    _new_dict = dict()
    for key, values in dict_of_unicode.items():
        _new_dict[key.encode('utf8')] = [val.encode('utf8') for val in values]
    return _new_dict
            

if __name__ == '__main__':
    import argparse
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('input', nargs='?', help='path to JSON file that is either name-->[target_id, ...], or target_id-->[name, ...], defaults to stdin')
    parser.add_argument('--target-id-to-names', default=False, action='store_true',
                        help='pass this flag if input JSON file has target_id string as the primary key')
    parser.add_argument('output', nargs='?', default=None, help='path to output file for large thrift message, defaults to stdout.')
    args = parser.parse_args()

    filter = Filter()

    if args.input:
        i_fh = open(args.input)
    else:
        i_fh = sys.stdin

    log('loading %s' % args.input)
    if args.target_id_to_names:
        unicode_target_id_to_names = json.load(i_fh)
        log('%d target_ids loaded' % len(unicode_target_id_to_names))

        utf8_target_id_to_names = convert_utf8( unicode_target_id_to_names )

        filter_names = FilterNames(target_id_to_names=utf8_target_id_to_names)

        ## reach under the head and act like we called load_filter_names
        ## instea of building it from JSON like we did above
        filter.filter_names = filter_names
        filter.invert_filter_names()

    else:
        unicode_name_to_target_ids = json.load(i_fh)
        target_ids = set()
        for target_ids_list in unicode_name_to_target_ids.values():
            target_ids.update(target_ids_list)
        log('%d names, %d target_ids loaded' % (len(unicode_name_to_target_ids), len(target_ids)))
        utf8_name_to_target_ids = convert_utf8(unicode_name_to_target_ids)
        filter_names = FilterNames(name_to_target_ids=utf8_name_to_target_ids)

        ## reach under the head and act like we called load_filter_names
        ## instea of building it from JSON like we did above
        filter.filter_names = filter_names
        
    if args.output:
        o_fh = open(args.output, mode='wb')
    else:
        o_fh = sys.stdout

    filter.save_filter_names(file_obj=o_fh)

    log('flushing to %s' % (args.output or 'stdout'))
    o_fh.close()
