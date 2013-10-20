TREC KBA 2013 Rated Documents Corpus
====================================

This tarball contains 47,233 flat files in a directory hierarchy.   

http://aws-publicdatasets.s3.amazonaws.com/trec/kba/trec-kba-2013-rated-chunks-indexed.tar.xz.gpg

The GPG key is available from NIST to anyone who submits a
research-only use restriction agreement.

stream_id uniquely identifies each StreamItem.  Every StreamItem in
this tarball can be found in this list of judgments, which also
associates each StreamItem with at least one target_id string, which
is a URL to a profile in twitter or Wikipedia:

https://github.com/trec-kba/kba-scorer/blob/master/data/trec-kba-ccr-judgments-2013-09-26-expanded-with-ssf-inferred-vitals-plus-len-clean_visible.before-and-after-cutoff.filter-run.txt

Every text in the tarball SHOULD match one of the name strings in
streamcorpus-filter/data/test-name-strings.json which is a mapping
from target_id to a list of name strings.

This python function shows how to load a StreamItem using the
stream_id from the directory hierarchy in the tarball:

def find_stream_item(stream_id):
     '''
     Load a StreamItem from a directory built from stream_id, which is
a unique identifier.  The flat files in this directory tree are
one-item-long "chunk" files.

For example:

  stream_id=1352256037-857c0a52772cb30600f281b2ac5bae71

--> loads:  trec-kba-2013-rated-chunks-indexed/8/5/7/1352256037-857c0a52772cb30600f281b2ac5bae71.sc

     '''
     epoch_ticks, doc_id = stream_id.split('-')
     path_1, path_2, path_3 = list(doc_id[:3])
     i_path = os.path.join('trec-kba-2013-rated-chunks-indexed', path_1, path_2, path_3, stream_id) + '.sc'
     if not os.path.exists(i_path):
         return None

     ## Thrift is self-delimiting, so applying python's list
     ## constructor executes the __iter__method on Chunk and
     ## instantiates all of the StreamItem objects in the file loaded
     ## from location "i_path".  In this case, we know there is only
     ## one, so we take the first (and only item) from the list.
     si = list(streamcorpus.Chunk(i_path))[0]
     assert si.stream_id == stream_id
     return si


What is a chunk file?
---------------------

Generally, a chunk file could contain multiple StreamItem objects
serialized and concatenated.  The streamcorpus.Chunk class is a
convenience wrapper that we created for python to more easily
write/read StreamItem objects in/out of flat files.  Other languages
must start at a lower level and use the code generated from this
Thrift definition to read/write from files/streams.

https://github.com/trec-kba/streamcorpus/blob/v0.3.0-dev/if/streamcorpus-v0_3_0.thrift

Note the branch "v0.3.0-dev" is not the "master" branch.


Where is the text?
------------------

StreamItem.body is a ContentItem instance, which has a .raw property
containing a byte array of the original data, usually fetched from a
web server over the 'net.  StreamItem.body.clean_html has a UTF-8
sanitized HTML string.  StreamItem.body.clean_visible has the same
text with all the tags replaced by whitespace, so the byte (and
character) offsets align with clean_html.

StreamItem.body.encoding has a guess at the character encoding of
StreamItem.body.raw, and StreamItem.body.media_type has a guess at the
MIME type of .raw

