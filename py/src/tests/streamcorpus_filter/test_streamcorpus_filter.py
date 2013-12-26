
import os
import re
import sys
import json
from streamcorpus import StreamItem, ContentItem
from streamcorpus import Chunk ## special convenience wrapper around file handles, only available in python
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
    ## reach under the hood and act like we called load_filter_names
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


def test_streamcorpus_filter_token_boundaries():
    filter = Filter()

    ## path to test data
    path_to_json = os.path.join(os.path.dirname(__file__), '../../../../data/test-name-strings-john-smith-token-boundaries.json')
    
    ## path that we will write to --- with our invented file extension "scf"
    path_to_thrift_message = os.path.join(os.path.dirname(__file__), '../../../../data/test-name-strings-john-smith-token-boundaries.scf')
    
    ## path to test streamcorpus.Chunk to create from test examples below
    path_to_stream_items = os.path.join(os.path.dirname(__file__), '../../../../data/test-stream-items-john-smith-token-boundaries.sc')
    
    unicode_target_id_to_names = {
        "http://en.wikipedia.org/wiki/John_F._Smith,_Jr.": \
            [u'John\\WSmith\\Wjr', u'John\\WSmithjr', u'J\\WSmith\\Wjr', u'J\\WSmithjr', u'J\\WSmith', u'John\\WSmith']
            ### There is an implicit \\W at the end of every string
            ## If we want to someday get fancy and parse nested
            ## parens, we could compress this to:
            ## u'(John|J)\\WSmith(\\W|)(jr|)'
        }
    
    utf8_target_id_to_names = convert_utf8(unicode_target_id_to_names)
    filter_names = FilterNames(target_id_to_names=utf8_target_id_to_names)

    filter = Filter()
    ## reach under the hood and act like we called load_filter_names
    ## instea of building it from JSON like we did above
    filter.filter_names = filter_names
    filter.invert_filter_names()
    assert filter.filter_names.name_to_target_ids
    assert isinstance(filter.filter_names, FilterNames)
    filter.save_filter_names(path_to_thrift_message)
    
    filter.load_filter_names(path_to_thrift_message)

    ## search through all unicode characters and register all of those that match python's regex for r'\s'
    # maxunicode = 0x10ffff... depending on python build
    chrs = u''.join(unichr(c) for c in xrange(sys.maxunicode + 1))
    map(filter.register_token_boundary_char, re.findall(r'\s', chrs, re.UNICODE))

    filter.compile_filters()

    ## each test case has exactly one match
    test_texts = ['''
John  Smith is a mention, even though it has two spaces.''',
'''J. Smith is also a mention, even though our pattern does not have the "." symbol
''',
'''J Smithjrthough is not a mention and J Smithjr is.
''',
'''
Sometimes an extraneous newline breaks a mention to John\n
\tSmith, and we still want to match it.
''',
u'''
Other times, special characters intervene John\u205F\u200ASmith
''',
u'''
Any maybe we even want to match no spaces:  John Smithjr.
''',
]

    if os.path.exists(path_to_stream_items):
        os.remove(path_to_stream_items)
    o_chunk = Chunk(path_to_stream_items, mode='wb')
    for test_text in test_texts:

        stream_item = StreamItem(
            body=ContentItem(clean_html=test_text.encode('utf8')))

        ## save this test data in a chunk file
        o_chunk.add(stream_item)

        filter.apply_filters(stream_item)
    
        assert len(stream_item.ratings) == 1, (stream_item.ratings, test_text)
        
    o_chunk.close()
