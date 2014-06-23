#!/usr/bin/env python


import atexit
import logging
import os
import random
import signal
import subprocess
import tempfile
import time
import urllib


from streamcorpus_pipeline.stages import BatchTransform


logger = logging.getLogger(__name__)



# TODO: recast as an IncrementalTransform with persistent subprocess daemon?
# That could potentially save on startup time by allowing one run of the subprocess to handle multiple chunk files worth of input.
#
# This is made available to yaml as "textfilter_batch" by the
# entry_points in this project's setup.py
class FastFilterBatch(BatchTransform):
    config_name = 'textfilter_batch'

    default_config = {
        'names_scf': None,
        'names_simple': None,
        'min_name_length': None,
        'max_name_length': None,
        'threads': None,
        'timeout_sec': 3600,
        'bin_path': None,
    }

    def __init__(self, config):
        self.config = config
        self.names_scf = config.get('names_scf')
        self.names_simple = config.get('names_simple')
        assert self.names_scf or self.names_simple, 'need names_scf or names_simple'
        self.min_name_length = config.get('min_name_length')
        self.max_name_length = config.get('max_name_length')
        self.threads = config.get('threads')
        self.timeout_sec = int(config.get('timeout_sec', 3600))
        # this will hold the Popen object
        self.proc = None

        self.temp_file_path = None

    def _cmd(self):
        # TODO: replace this url special-case with a general purpose
        # application level virtual filesystem
        if self.names_simple and (self.names_simple.startswith('http:') or
                                  self.names_simple.startswith('https:')):
            fd, self.temp_file_path = tempfile.mkstemp(suffix='.txt', prefix='names_')
            atexit.register(os.remove, self.temp_file_path)
            os.close(fd)
            logger.info('downloading names %r -> %r', self.names_simple, self.temp_file_path)
            urllib.urlretrieve(self.names_simple, self.temp_file_path)
            self.names_simple = self.temp_file_path

        cmd = [self.get_bin_path()]
        if self.names_scf:
            cmd += ['--names-scf', self.names_scf]
        elif self.names_simple:
            cmd += ['--names', self.names_simple]
        if self.min_name_length is not None:
            cmd += ['--min-name-length', str(self.min_name_length)]
        if self.max_name_length:
            cmd += ['--max-name-length', str(self.max_name_length)]
        if self.threads:
            cmd += ['--threads', str(self.threads)]
        return cmd

    def process_path(self, chunk_path):
        '''
        process streamcorpus chunk file at chunk_path.
        work in place with results at same path (with tempfile and rename if needed)
        '''
        cmd = self._cmd()
        tmp_path = chunk_path + '_tmp_{0:x}'.format(random.randint(1,999999999))
        cmd += ['--input', chunk_path, '--output', tmp_path]
        logger.info('going to run filter cmd: %r', cmd)

        start = time.time()
        self.proc = subprocess.Popen(cmd, shell=False)

        retcode = None
        while True:
            retcode = self.proc.poll()
            if retcode is not None:
                break
            dt = time.time() - start
            if dt > self.timeout_sec:
                logger.error('cmd timed out after %s: %r', dt, cmd)
                self.proc.send_signal(signal.SIGKILL)
                raise Exception('filter timed out')
            time.sleep(1.0)
        
        self.proc = None
        if retcode != 0:
            raise Exception('filter returned code: {0}'.format(retcode))

        logger.debug('clobber tmp file back onto chunk: mv %r %r', tmp_path, chunk_path)
        os.rename(tmp_path, chunk_path)

        logger.debug('filter done')

    def shutdown(self):
        if self.proc:
            try:
                self.proc.send_signal(signal.SIGTERM)
            except:
                logger.error('error terminating fast filter subprocess', exc_info=True)
            self.proc = None
            # TODO? sleep 1; kill -9 ?

    def get_bin_path(self):
        # this file is
        # py/src/streamcorpus_filter/pipeline_stage.py
        filter_binary = self.config.get('bin_path')
        if filter_binary:
            if not os.path.isfile(filter_binary):
                logger.error('config bin_path set but no file there: %r', filter_binary)
            else:
                return filter_binary
        py_src_streamcorpus_filter_dir = os.path.dirname(__file__)
        filter_binary = os.path.abspath(
            os.path.join(
                py_src_streamcorpus_filter_dir,
                '../../../cpp/filter-multifast'))
        return filter_binary
