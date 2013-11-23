CXXFLAGS	+= -I/home/lvv/p
filtername       = "test-name-strings.scf"
corpus           = /tr/diffeo/sc-lvv-v0.3.0-dev/test-data/john-smith-tagged-by-lingpipe-0-v0_2_0.sc
trec_corpus      = /tr/diffeo/corpus/trec-kba-2013-rated-chunks-indexed
output           = /tmp/filtered.sc
streamcorpus_src = /tr/diffeo/sc-lvv-v0.3.0-dev/cpp
multifast        = /tr/diffeo/multifast-code/
power_saving_off = su -c 'cpufreq-set -c0 -g performance'; su -c 'cpufreq-set -c1 -g performance'

