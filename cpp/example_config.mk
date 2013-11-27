
CORPUS_URL := http://aws-publicdatasets.s3.amazonaws.com/trec/kba/trec-kba-2013-rated-chunks-indexed.tar.xz
NAMES_URL  := http://aws-publicdatasets.s3.amazonaws.com/trec/kba/wlc/mention-dump.scf.gz

MULTIFAST_REPO    := svn://svn.code.sf.net/p/multifast/code/trunk
STREAMCORPUS_REPO := http://github.com/trec-kba/streamcorpus

# Path to streamcorpus source repo. It should have built libstreamcorpus.a 
STREAMCORPUS = streamcorpus
#STREAMCORPUS = ../../sc-lvv-v0.3.0-dev/cpp

# Path to multifast source repo.  It should have built libahocorasick.a
MULTIFAST = multifast
#MULTIFAST = ../../multifast-code/

# Path to names file
NAMES = data/names.scf

# Command which sends items text to stdout
CORPUS := data/corpus/
CAT_CORPUS_TO_STDOUT ?= find $(CORPUS) -type f | head -n $(I) | xargs cat
#CAT_CORPUS_TO_STDOUT ?= find ../../corpus/trec-kba-2013-rated-chunks-indexed -type f | head -n $(I) | xargs cat
#CORPUS          = /tr/diffeo/sc-lvv-v0.3.0-dev/test-data/john-smith-tagged-by-lingpipe-0-v0_2_0.sc

# Path to saved output from filter
OUTPUT = /tmp/filtered.sc

# optional: command which turns off powersaving (needed for benchmarking)
POWER_SAVING_OFF = su -c 'cpufreq-set -c0 -g performance'; su -c 'cpufreq-set -c1 -g performance'

