
CORPUS_URL := http://aws-publicdatasets.s3.amazonaws.com/trec/kba/trec-kba-2013-rated-chunks-indexed.tar.xz
NAMES_URL  := http://aws-publicdatasets.s3.amazonaws.com/trec/kba/wlc/mention-dump.scf.gz

MULTIFAST_REPO    := svn://svn.code.sf.net/p/multifast/code/trunk
STREAMCORPUS_REPO := http://github.com/trec-kba/streamcorpus

# benchmarking parameters:  search-library;  max names to use;  max items to use
LINK    ?= multifast
N	?= 1000
I	?= 20

# Path to streamcorpus source repo.
STREAMCORPUS = streamcorpus

# Path to multifast source repo.
MULTIFAST = multifast

# Path to names file
NAMES = data/names.scf

# Command which sends items text to stdout
CORPUS := data/corpus/
CAT_CORPUS_TO_STDOUT ?= find $(CORPUS) -type f | head -n $(I) | xargs cat

# Path to saved output from filter
OUTPUT = /tmp/filtered.sc

# optional: command which turns off powersaving (needed for benchmarking)
#POWER_SAVING_OFF = su -c 'cpufreq-set -c0 -g performance'; su -c 'cpufreq-set -c1 -g performance'

