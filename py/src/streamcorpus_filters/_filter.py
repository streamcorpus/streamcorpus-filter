'''python example of filtering through strings from a
streamcorpus.StreamItem to find names from a FilterName

'''

from __future__ import absolute_import

import os
import sys

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
    ## use faster C program to read/write
    TBinaryProtocol = TBinaryProtocolAccelerated

except Exception, exc:
    fastbinary_import_failure = exc
    ## fall back to pure python

## thrift message classes from core streamcorpus library
from streamcorpus import StreamItem, Rating, Offset

## thrift message class from this package
from streamcorpus_filters.ttypes import FilterNames

class Filter(object):

    def __init__(self):
        self.filter_names = None

    def load_filter_names(self, path_to_thrift_message):
        '''reads a FilterNames message from a flat file
        '''
        if not os.path.exists(path_to_thrift_message):
            sys.exit('path does not exist: %r' % path_to_thrift_message)
        fh = open(path_to_thrift_message)
        i_transport = TBufferedTransport(fh)
        i_protocol = TBinaryProtocol(i_transport)
        self.filter_names = FileNames.read(i_protocol)
        ## not actually required in CPython
        fh.close()

    def save_filter_names(self, path_to_thrift_message):
        '''writes a FilterNames message to a flat file
        '''
        if os.path.exists(path_to_thrift_message):
            print('warning: overwriting: %r' % path_to_thrift_message)
        fh = open(path_to_thrift_message, 'wb')
        o_transport = TBufferedTransport(fh)
        o_protocol = TBinaryProtocol(i_transport)
        self.filter_names.write(i_protocol)
        fh.close()

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

    
