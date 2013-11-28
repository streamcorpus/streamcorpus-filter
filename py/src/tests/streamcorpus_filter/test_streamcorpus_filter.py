
import os
import json
from streamcorpus import StreamItem, ContentItem
from streamcorpus_filter import Filter
from streamcorpus_filter.ttypes import FilterNames
from streamcorpus_filter.convert import convert_utf8


def test_streamcorpus_filter_save_load():
    filter = Filter()

    ## path to test data
    path_to_json = os.path.join(os.path.dirname(__file__), '../../../../data/test-name-strings.json')
    
    ## path that we will write to --- with our invented file extension "scf"
    path_to_thrift_message = os.path.join(os.path.dirname(__file__), '../../../../data/test-name-strings.scf')
    
    unicode_target_id_to_names = json.load(open(path_to_json))
    utf8_target_id_to_names = convert_utf8(unicode_target_id_to_names)
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

def test_streamcorpus_filter_compile_apply():

    ## path that we will READ to --- with our invented file extension "scf"
    path_to_thrift_message = os.path.join(os.path.dirname(__file__), '../../../../data/test-name-strings.scf')

    filter = Filter()
    
    filter.load_filter_names(path_to_thrift_message)

    filter.compile_filters()

    stream_item = StreamItem(
        body=ContentItem(clean_html='some text with a filter name: Beth Ramsay and more'))

    filter.apply_filters(stream_item)
    
    assert stream_item.ratings
        
    assert stream_item.ratings['streamcorpus-filter'][0].target.target_id == "https://twitter.com/BethMRamsay"
