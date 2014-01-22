'''python example of filtering through strings from a
streamcorpus.StreamItem to find names from a FilterName

'''

from __future__ import absolute_import

import os
import sys
from cStringIO import StringIO

## import the thrift library
from thrift import Thrift
from thrift.transport.TTransport import TBufferedTransport
from thrift.protocol.TBinaryProtocol import TBinaryProtocol

## pure python TBinaryProtocol is slow, so attempt to replace it with
## faster implementation
from thrift.protocol.TBinaryProtocol import TBinaryProtocolAccelerated
fastbinary_import_failure = None
try:
    from thrift.protocol import fastbinary
    raise Exception()  ## don't let this happen
    ## use faster C program to read/write
    TBinaryProtocol = TBinaryProtocolAccelerated

except Exception, exc:
    fastbinary_import_failure = exc
    ## fall back to pure python

## thrift message classes from core streamcorpus library
from streamcorpus import StreamItem, Rating, Label, Annotator, Offset, OffsetType, Target

## thrift message class from this package
from streamcorpus_filter.ttypes import FilterNames

class Filter(object):

    def __init__(self):
        self.filter_names = None
        self._names = None
        self.token_boundary_chars = set([' ', '\n', '\s', '\t', '\r'])

    def load_filter_names(self, path_to_thrift_message):
        '''reads a FilterNames message from a flat file
        '''
        if not os.path.exists(path_to_thrift_message):
            raise Exception('path does not exist: %r' % path_to_thrift_message)
        fh = open(path_to_thrift_message)
        fh = StringIO(fh.read())
        i_transport = TBufferedTransport(fh)
        i_protocol = TBinaryProtocol(i_transport)
        self.filter_names = FilterNames()
        self.filter_names.read(i_protocol)
        ## not actually required in CPython
        fh.close()

    def save_filter_names(self, path_to_thrift_message=None, file_obj=None):
        '''writes a FilterNames message to a flat file
        '''
        if path_to_thrift_message:
            if os.path.exists(path_to_thrift_message):
                print('warning: overwriting: %r' % path_to_thrift_message)
            o_transport = open(path_to_thrift_message, 'wb')
        elif file_obj:
            o_transport = file_obj
        else:
            raise Exception('must specify either path_to_thrift_message or file_obj')
        o_protocol = TBinaryProtocol(o_transport)
        self.filter_names.write(o_protocol)
        o_transport.close()

    def invert_filter_names(self):
        '''constructs FilterNames.name_to_target_ids from
        FilterNames.target_id_to_names

        '''
        if self.filter_names.name_to_target_ids:
            print('warning: replacing existing FilterNames.name_to_target_ids')

        self.filter_names.name_to_target_ids = dict()
        for target_id in self.filter_names.target_id_to_names:
            for  name in self.filter_names.target_id_to_names[target_id]:
                if name not in self.filter_names.name_to_target_ids:
                    self.filter_names.name_to_target_ids[name] = list()
                self.filter_names.name_to_target_ids[name].append(target_id)

        print('%d names, %d target_ids' % (len(self.filter_names.name_to_target_ids),
                                           len(self.filter_names.target_id_to_names)))


    def compile_filters(self):
        if not self.filter_names:
            raise Exception('must first load FilterNames')
        ## for this simple example, all we do is convert the utf-8
        ## from FilterNames into unicode
        self._names = dict()
        for name in self.filter_names.name_to_target_ids:
            self._names[name.decode('utf8')] = self.filter_names.name_to_target_ids[name]


    def register_token_boundary_char(self, char):
        '''
        add a unicode character to the set of symbols consider
        equivalent to "token boundary"
        '''
        self.token_boundary_chars.add(char)

                
    def advance_passed_boundary(self):
        '''
        advanced self.text_position passed the current boundary, if
        possible, and detect if a token boundary was passed.
        '''
        start_text_position = self.text_position
        while self.text_position < len(self.text) and  self.text[self.text_position] in self.token_boundary_chars:
            self.text_position += 1

        if self.text_position == len(self.text):
            return False
        elif start_text_position != self.text_position:
            ## consumed some characters, so we must have reached a new token
            return True
        elif self.text_position == 0:
            ## special case for start of document
            return True
        else:
            ## have not passed a boundary
            return False

                
    def advance_to_boundary(self):
        '''
        advanced self.text_position to the next boundary or end of self.text
        '''
        while self.text_position < len(self.text) and self.text[self.text_position] not in self.token_boundary_chars:
            self.text_position += 1

    def apply_filters(self, stream_item, content_form='clean_html'):
        '''iterates over the characters in stream_item.body.<content_form>
looking for strings that exact match keys in
self.filter_names.name_to_target_ids'''
        if not self._names:
            raise Exception('must first have a compiled set of filters')

        annotator_id = 'streamcorpus-filter'
        annotator = Annotator(annotator_id=annotator_id)

        text = getattr(stream_item.body, content_form)

        ## pass text around by reference as a property on this class instance
        self.text = text.decode('utf8')

        ## inefficient brute force loop for each name
        for name in self._names:

            name_tokens = name.split('\\W')
            name_token_i = 0

            self.text_position = 0
            first_char_position = None

            while name_tokens and self.text_position < len(self.text):

                #print 'starting', self.text_position, name_token_i, name_tokens

                reached_start_of_new_token = self.advance_passed_boundary()

                if not reached_start_of_new_token:
                    self.advance_to_boundary()
                    continue
            
                name_tok = name_tokens[name_token_i]

                #print 'on a token', self.text_position, name_token_i, name_tok, self.text[self.text_position:self.text_position + len(name_tok)]

                if name_tok != self.text[self.text_position : self.text_position + len(name_tok)]:
                    name_token_i = 0
                    first_char_position = None
                    self.text_position += 1

                else:
                    name_token_i += 1
                    self.text_position += len(name_tok)

                    if  first_char_position is None:
                        first_char_position = self.text_position


                if name_token_i == len(name_tokens):
                    print 'found one!'

                    ## reset state machine for next possible match in this document
                    name_token_i = 0

                    for target_id in self._names[name]:

                        target = Target(target_id=target_id)

                        rating = Rating(annotator=annotator, target=target)
                        label  = Label( annotator=annotator, target=target)
                        label.offsets[OffsetType.CHARS] = Offset(
                            type=OffsetType.CHARS,
                            first=first_char_position,
                            length=self.text_position)

                        if annotator_id not in stream_item.body.labels:
                            stream_item.body.labels[annotator_id] = list()
                        stream_item.body.labels[annotator_id].append(label)

                        if annotator_id not in stream_item.ratings:
                            stream_item.ratings[annotator_id] = list()
                        stream_item.ratings[annotator_id].append(rating)

