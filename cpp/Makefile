

.PHONY: help run test clean test bench
######################################  DEFAULT CONFIG

CORPUS_URL := http://aws-publicdatasets.s3.amazonaws.com/trec/kba/trec-kba-2013-rated-chunks-indexed.tar.xz
NAMES_URL  := http://aws-publicdatasets.s3.amazonaws.com/trec/kba/wlc/mention-dump.scf.gz

MULTIFAST_REPO    := svn://svn.code.sf.net/p/multifast/code/trunk
STREAMCORPUS_REPO := http://github.com/trec-kba/streamcorpus

# Path to streamcorpus source repo.
STREAMCORPUS := streamcorpus

# Path to multifast source repo.
MULTIFAST := multifast

# Path to names file
NAMES := data/names.scf

# Command which sends items text to stdout
CORPUS := data/corpus.lz4
#CAT_CORPUS_TO_STDOUT ?= find $(CORPUS) -type f | head -n $(MAX_ITEMS) | xargs cat
CAT_CORPUS_TO_STDOUT ?= lz4c -dc  $(CORPUS) 

# Path to saved output from filter
#OUTPUT = /tmp/filtered.sc
OUTPUT := /dev/null

# optional: command which turns off powersaving (needed for benchmarking)
#POWER_SAVING_OFF = su -c 'cpufreq-set -c0 -g performance'; su -c 'cpufreq-set -c1 -g performance'

# time(1) command
TIME = /usr/bin/time

# read config file if any
ifneq ($(wildcard config.mk),)
include config.mk
endif

# benchmarking parameters:  search-library;  max names to use;  max items to use
#LINK    ?= multifast
MAX_NAMES	?= 2000000
MAX_ITEMS	?= 900


###########################################  HELP

help:
	@echo "Please select specific target:"
	@echo "	  run -- run benchmark"
	@echo "	  test -- unit test"
	@echo "	  clean -- remove computer generated files"


############################################  BUILD ID
_cc := $(CXX)-$(shell $(CXX) -v 2>&1 | sed  -n 's/^.*ersion *\(4[^ ]*\) .*/\1/p')
#_date := $(shell date +'%y%m%d_%H%M%S')
_rev := $(shell git rev-parse --git-dir 2> /dev/null 1>/dev/null && git rev-parse HEAD|sed -n 's/^\(......\).*/\1/p' )
#_cpu=$(shell uname -m -p  |sed 's/Intel(R)//;s/(TM)//;s/@//;s/CPU//;s/ \+/-/g')
_cpu := $(shell sed -n '/^model name/!d; s/.*: //; s/Processor//; s/Intel//; s/(TM)//; s/(R)//g; s/CPU//; s/@//; s/^ //; s/ \+/-/g;p;q' /proc/cpuinfo )
_cores := $(shell awk '/^processor/{cores=$$3+1}; END{print cores}' /proc/cpuinfo)
_mhz := $(shell awk '/^cpu MHz/{mhz=$$4}; END{print mhz}' /proc/cpuinfo | sed 's/\.[0-9]\+//')
ID := $(shell echo "$(_rev)-$(_cc)-$(MODE)-$(_cpu)-x$(_cores)@$(_mhz)" | tr -d ' ')

###########################################  COMPILER FLAGS

              MODE ?= OPTIMIZE
      COMMON_FLAGS += -Wall -Wno-unused-function -UID -DID='"$(ID)"'
    OPTIMIZE_FLAGS += -O3 -DNDEBUG -march=native
       DEBUG_FLAGS += -O0 -DDEBUG  -ggdb3 -fstack-protector  # -D_GLIBCXX_DEBUG 
     PROFILE_FLAGS += -O0 -ggdb3 -pg  -fno-omit-frame-pointer -fno-inline-functions -fno-inline-functions-called-once -fno-optimize-sibling-calls -fno-default-inline -fno-inline

ifneq ($(CXX),cl++)
ifneq ($(CXX),clang++)
	DEBUG_FLAGS += -fdelete-null-pointer-checks  -fbounds-check
endif
endif

unit-%:           MODE = DEBUG
bench-%:          MODE = OPTIMIZE
filter-%:         MODE = OPTIMIZE

#unit-%:   DEBUG_FLAGS += -D_GLIBCXX_DEBUG 
#t-%:   DEBUG_FLAGS += -D_GLIBCXX_DEBUG 

export CXXFLAGS += $(COMMON_FLAGS) $($(MODE)_FLAGS) -std=c++0x 

%:  %.o
	$(CXX) $(CXXFLAGS) -o $@ $^  $(LDFLAGS) $(LDLIBS)
	

# additianal debug fascilities, if available
ifneq ($(wildcard /home/lvv/p),)
CXXFLAGS += -DLVV -I /home/lvv/p
endif

########################################### THRIFT: FLTERNAMES 

filtername_thrift_cpp= \
	filternames_constants.cpp  \
	filternames_constants.h  \
	filternames_types.cpp  \
	filternames_types.h 

filternames_thrift_o = \
	filternames_constants.o \
	filternames_types.o

$(filtername_thrift_cpp): filternames.thrift
	thrift  --out . --gen cpp  filternames.thrift

###########################################  PRIFIX-LIB COMBOS

##  naive  ----------

%-naive:   %.o naive.o $(DEPS)
	$(CXX) $(CXXFLAGS) -o $@ $^  $(LDFLAGS) $(LDLIBS)

##  %-vsearch  ----------

%-vsearch:  %.o  vsearch.o
	$(CXX) $(CXXFLAGS) -o $@ $^  $(LDFLAGS) $(LDLIBS)

##  %-multifast  -----

multifast.o:       $(MULTIFAST)/ahocorasick/build/libahocorasick.a
multifast.o:       CXXFLAGS += -I $(MULTIFAST)/ahocorasick/
%-multifast:       LDFLAGS  += -L $(MULTIFAST)/ahocorasick/build/
%-multifast:       LDLIBS   += -l ahocorasick

%-multifast:   %.o  multifast.o
	$(CXX) $(CXXFLAGS) -o $@ $^  $(LDFLAGS) $(LDLIBS)

## filter 
filter-%:   LDLIBS += $(filternames_thrift_o)
filter.o:             $(filternames_thrift_o)


## all 

unit.o:  unit.cc search.h
bench.o:  bench.cc search.h
filter.o:  filter.cc search.h

##########################################  DATA & EXTERNAL LIBS

DATA          = $(NAMES)  $(CORPUS)
LIBS          = $(MULTIFAST)  $(STREAMCORPUS)
EXTERNAL_DEPS = $(DATA)  $(LIBS)


$(NAMES):
	mkdir -p data/
	wget -c -O - $(NAMES_URL) | zcat > $(NAMES)

$(CORPUS):
	mkdir -p data/
	wget -c -O - $(CORPUS_URL) | tar -xJ -O -f - | lz4c > $(CORPUS)
	#mkdir -p $(CORPUS)
	#wget -O - $(CORPUS_URL) | tar -xJ -C $(CORPUS) --strip-components=1 -f -

$(MULTIFAST) :
	svn checkout $(MULTIFAST_REPO) multifast

$(MULTIFAST)/ahocorasick/build/libahocorasick.a : $(MULTIFAST)
	make -BC $(MULTIFAST)/ahocorasick/
	ls   -l  $(MULTIFAST)/ahocorasick/build/libahocorasick.a
	
$(STREAMCORPUS):
	git clone -b v0.3.0-dev $(STREAMCORPUS_REPO)

$(STREAMCORPUS)/cpp/libstreamcorpus.a : | $(STREAMCORPUS)
	make -f $(PWD)/streamcorpus.mk  -C  $(STREAMCORPUS)/cpp/  libstreamcorpus.a;

	#cd  $(STREAMCORPUS)/cpp;  cmake -F CMakeLists.txt;  make  MAKE_VERBOSE=1 -B;  ls libstreamcorpus.a


########################################### LIBS

filter.o    scf2mmap.o  test_throughput_speed.o:   CXXFLAGS += -I$(STREAMCORPUS)/cpp
filter-%    scf2mmap    test_throughput_speed:     LDFLAGS  += -L$(STREAMCORPUS)/cpp
filter-%    scf2mmap    test_throughput_speed:     LDLIBS   += -lstreamcorpus -lthrift -lboost_program_options  

$(filternames_thrift_o):   filternames_types.h
filter.o scf2mmap.o:  filternames_types.h  $(STREAMCORPUS)/cpp/libstreamcorpus.a 


###########################################  make names mmap

scf2mmap:         scf2mmap.o		$(filternames_thrift_o) 

data/names_data.mmap: scf2mmap
	scf2mmap -f $(NAMES)


##########################################  MISC
clean:
	$(RM)  *.o  $(filtername_thrift_cpp) *.gch  mk_mmap_names test_throughput_speed
	$(RM)  gmon.out  *.prof  *.prof.png
	$(RM)  *-multifast *-naive *-vsearch

libclean:  clean
	-make -C $(MULTIFAST)/ahocorasick/ clean
	-make -f $(PWD)/streamcorpus.mk  -C  $(STREAMCORPUS)/cpp/  clean

gprof:
	gprof $(exe) > $(exe).gprof
	cat $(exe).gprof  |  gprof2dot -s -n 0 -e 0 | dot -Tpng -o $(exe).gprof.png;
	eog $(exe).gprof.png

update_lvvlib:
	cp -v ~/p/lvvlib/{check,mmap,token}.h lvvlib/


##########################################

test:    $(LIBS) unit-naive unit-multifast
	./unit-naive
	./unit-multifast


###########################################  BENCHMARKS

#RUN:	MODE := OPTIMIZE

run:  filter-$(LINK) | $(DATA)
	@$(POWER_SAVING_OFF)
	$(CAT_CORPUS_TO_STDOUT) | $(TIME) ./filter-$(LINK) -N $(MAX_NAMES) -f $(NAMES) -I $(MAX_ITEMS) --verbose $(RUNFLAGS) > $(OUTPUT)

###########################################  TEST_THROUGHPUT_SPEED

test_throughput_speed:      $(filternames_thrift_o)
test_throughput_speed.o:    $(STREAMCORPUS)/cpp/libstreamcorpus.a 

TTS: test_throughput_speed  | $(DATA)
	$(TIME) ./test_throughput_speed streamcorpus/test-data/john-smith-tagged-by-lingpipe-0-v0_3_0.sc  /tmp/tts.out

TTS-pv: test_throughput_speed  | $(DATA)
	$(TIME) ./test_throughput_speed streamcorpus/test-data/john-smith-tagged-by-lingpipe-0-v0_3_0.sc - | pv > /dev/null

###########################################  MMAP Benchmarks

bench.o: CXXFLAGS+=-DMAX_NAMES=$(MAX_NAMES)

BENCH-%:  bench-%
	$(TIME) $<

