
import os
import json
from streamcorpus import StreamItem, ContentItem
from streamcorpus_filters import Filter
from streamcorpus_filters.ttypes import FilterNames


def test_streamcorpus_filters_save_load():
    filter = Filter()

    ## path to test data
    path_to_json = os.path.join(os.path.dirname(__file__), '../../../../data/test-name-strings.json')
    
    ## path that we will write to --- with our invented file extension "scf"
    path_to_thrift_message = os.path.join(os.path.dirname(__file__), '../../../../data/test-name-strings.scf')
    
    unicode_target_id_to_names = json.load(open(path_to_json))
    ## convert all the strings to utf-8, as is required for all thrift strings
    utf8_target_id_to_names = dict()
    for target_id in unicode_target_id_to_names:
        utf8_target_id_to_names[target_id] = list()
        for name in unicode_target_id_to_names[target_id]:
            utf8_target_id_to_names[target_id].append(name.encode('utf8'))

    filter_names = FilterNames(target_id_to_names=utf8_target_id_to_names)

    filter = Filter()
    ## reach under the head and act like we called load_filter_names
    ## instea of building it from JSON like we did above
    filter.filter_names = filter_names
    filter.invert_filter_names()
    assert filter.filter_names.name_to_target_ids
    assert isinstance(filter.filter_names, FilterNames)
    filter.save_filter_names(path_to_thrift_message)
    
    filter.load_filter_names(path_to_thrift_message)

def test_streamcorpus_filters_compile_apply():

    ## path that we will READ to --- with our invented file extension "scf"
    path_to_thrift_message = os.path.join(os.path.dirname(__file__), '../../../../data/test-name-strings.scf')

    filter = Filter()
    
    filter.load_filter_names(path_to_thrift_message)

    filter.compile_filters()

    stream_item = StreamItem(
        body=ContentItem(clean_html='some text with a filter name: Beth Ramsay and more'))

    filter.apply_filters(stream_item)
    
    assert stream_item.ratings
        
    assert stream_item.ratings['streamcorpus-filters'][0].target.target_id == "https://twitter.com/BethMRamsay"
