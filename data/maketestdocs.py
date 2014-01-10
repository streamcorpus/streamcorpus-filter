#!/usr/bin/env python

import os
import random
import sys


fin = open('dumbnames.txt', 'rb')
raw = fin.read()
lines = raw.splitlines()
fin.close()

if not os.path.exists('testdocs'):
    os.makedirs('testdocs')

goodcount = 0
for line in lines:
    goodcount += 1
    outpath = os.path.join('testdocs', 'good{0}.txt'.format(goodcount))
    fout = open(outpath, 'wb')
    for i in xrange(random.randint(1,10)):
        fout.write('good ')
    fout.write(line)
    for i in xrange(random.randint(1,10)):
        fout.write('good ')
    fout.write('\n')
    fout.close()

usable_chars = [chr(x) for x in xrange(ord(' '), ord('~'))]
badcount = 0
for i in xrange(20):
    badcount += 1
    outpath = os.path.join('testdocs', 'bad{0}.txt'.format(badcount))
    needrbytes = True
    while needrbytes:
        rchars = [random.choice(usable_chars) for cc in xrange(random.randint(100, 500))]
        rbytes = b''.join(rchars)
        needrbytes = False
        for line in lines:
            if line in rbytes:
                needrbytes = True
                sys.stderr.write('found {0!r} in rbytes!\n'.format(line))
                break
    fout = open(outpath, 'wb')
    fout.write('\n')
    fout.close()
