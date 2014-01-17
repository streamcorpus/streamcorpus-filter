#!/usr/bin/env python


"""
Achieve this:
xz -d < ../data/nametesttexts.sc.xz | (./filter-multifast --normalize --names=../data/dumbnames.txt 2> /tmp/${USER}_filterlog.txt) | xz > /tmp/${USER}_filter.sc.xz 

check that certain log messages appear
"""


import argparse
import logging
import os
import re
import subprocess
import sys


try:
    # python2_7
    import backports.lzma as lzma
except:
    try:
        # python3
        import lzma
    except:
        lzma = None

assert lzma


def check_re(data, pattern, expected):
    err = 0
    m = pattern.search(data)
    if not m:
        logging.error('failed to find %r', pattern.pattern)
        err = 1
    else:
        if m.group(1) != expected:
            logging.error('%r was %r, wanted %r', pattern.pattern, m.group(1), expected)
            err = 1
        else:
            logging.debug('OK: %r got %r', pattern.pattern, expected)
    return err


processed_re = re.compile(r'Total stream items processed: (\d+)')
out_re = re.compile(r'Total stream items written: (\d+)')


def test_cmd(cmd, rawin):
    err = 0
    logging.info('running %r', cmd)
    p = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=False)
    stdoutdata, stderrdata = p.communicate(rawin)
    if not stdoutdata:
        logging.error('got no stdout data')
        err = 1
    if p.returncode != 0:
        logging.error('filter exited with status %r', p.returncode)
        err = 1
    #errlines = stderrdata.splitlines()
    err2 = check_re(stderrdata, processed_re, '684')

    err3 = check_re(stderrdata, out_re, '664')

    err = err or err2 or err3

    if err != 0:
        sys.stderr.write('FAILURE underlying stderr:\n\n')
        sys.stderr.write(stderrdata);
        sys.stderr.write('\n\n')
    return err


def main():
    '''
    The test is effectively:
xz -d < ../data/nametesttexts.sc.xz | (./filter-multifast --normalize --names=../data/dumbnames.txt 2> /tmp/${USER}_filterlog.txt) | xz > /tmp/${USER}_filter.sc.xz
grep 'Total stream items processed: 684' /tmp/${USER}_filterlog.txt
grep 'Total stream items written: 664' /tmp/${USER}_filterlog.txt

'''
    err = 0
    ap = argparse.ArgumentParser()
    ap.add_argument('--verbose', action='store_true', default=False)
    options = ap.parse_args()
    if options.verbose:
        logging.basicConfig(level=logging.DEBUG)
    else:
        logging.basicConfig(level=logging.INFO)
    indatapath = os.path.abspath(os.path.join(os.path.dirname(__file__), '../data/nametesttexts.sc.xz'))
    rawin = None
    with lzma.open(indatapath, 'rb') as xzin:
        rawin = xzin.read()
    assert rawin
    # ../data/dumbnames.txt
    namespath = os.path.abspath(os.path.join(os.path.dirname(__file__), '../data/dumbnames.txt'))
    binpath = os.path.abspath(os.path.join(os.path.dirname(__file__), 'filter-multifast'))
    cmd = [binpath, '--normalize', '--names=' + namespath, '--verbose']

    err1 = test_cmd(cmd, rawin)
    err2 = test_cmd([binpath, '--normalize', '--names=' + namespath, '--verbose', "--threads=2"], rawin)

    err = err1 or err2

    if err == 0:
        sys.stderr.write('OK\n')
    sys.exit(err)


#Total stream items processed: 684
#Total matches: 720
#Total stream items written: 664
#search time: 0.614188 sec


if __name__ == '__main__':
    main()
