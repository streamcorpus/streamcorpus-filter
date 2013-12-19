
# read config file if any

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
#CAT_CORPUS_TO_STDOUT ?= find $(CORPUS) -type f | head -n $(I) | xargs cat
CAT_CORPUS_TO_STDOUT ?= lz4c -dc  $(CORPUS) 

# Path to saved output from filter
#OUTPUT = /tmp/filtered.sc
OUTPUT := /dev/null

# optional: command which turns off powersaving (needed for benchmarking)
#POWER_SAVING_OFF = su -c 'cpufreq-set -c0 -g performance'; su -c 'cpufreq-set -c1 -g performance'

# OS Time command
TIME = /usr/bin/time

ifneq ($(wildcard config.mk),)
include config.mk
endif

# benchmarking parameters:  search-library;  max names to use;  max items to use
LINK    ?= multifast
N	?= 500000
I	?= 900


###########################################  HELP

help:
	@echo "Please select specific target:"
	@echo "	  run -- run benchmark"
	@echo "	  test -- unit test"
	@echo "	  clean -- remove computer generated files"

.PHONY: help run test clean 

################################################################################  BUILD ID
_cc := $(CXX)-$(shell $(CXX) -v 2>&1 | sed  -n 's/^.*ersion *\(4[^ ]*\) .*/\1/p')
#_date := $(shell date +'%y%m%d_%H%M%S')
_rev :=$(shell test -d .git && git rev-parse HEAD|sed -n 's/^\(........\).*/\1/p' )
#_cpu=$(shell uname -m -p  |sed 's/Intel(R)//;s/(TM)//;s/@//;s/CPU//;s/ \+/-/g')
_cpu :=$(shell sed -n '/^model name/!d; s/.*: //; s/(tm) Processor//; s/Intel(R)//; s/(TM)//; s/@//; s/^ //; s/ \+/-/g;p;q' /proc/cpuinfo )
_cores := $(shell awk '/^processor/{cores=$$3+1}; END{print cores}' /proc/cpuinfo)
_mhz := $(shell awk '/^cpu MHz/{mhz=$$4}; END{print mhz}' /proc/cpuinfo |sed 's/\.[0-9]\+//')
ID = $(shell echo "$(_rev)-$(_cc)-$(MODE)-$(_cpu)-x$(_cores)@$(_mhz)" | tr -d ' ')

###########################################  COMPILER FLAGS

              MODE ?= OPTIMIZE
      COMMON_FLAGS += -Wall -DID='"$(LINK)-$(ID)"'
    OPTIMIZE_FLAGS += -O3 -DNDEBUG -march=native
       DEBUG_FLAGS += -O0 -DDEBUG  -ggdb3 -fstack-protector  # -D_GLIBCXX_DEBUG 
     PROFILE_FLAGS += -O0 -ggdb3 -pg  -fno-omit-frame-pointer -fno-inline-functions -fno-inline-functions-called-once -fno-optimize-sibling-calls -fno-default-inline -fno-inline

ifneq ($(CXX),cl++)
ifneq ($(CXX),clang++)
	DEBUG_FLAGS += -fdelete-null-pointer-checks  -fbounds-check
endif
endif

u-%:           MODE = DEBUG
u-%:           MODE = DEBUG
u-%:   DEBUG_FLAGS += -D_GLIBCXX_DEBUG 
t-%:   DEBUG_FLAGS += -D_GLIBCXX_DEBUG 

   export CXXFLAGS += $(COMMON_FLAGS) $($(MODE)_FLAGS) -std=c++0x 
   export   CFLAGS += $(COMMON_FLAGS) $($(MODE)_FLAGS)

%-filter:  filter.o
	$(CXX) $(CXXFLAGS) -o $@ $^  $(LDFLAGS) $(LDLIBS)

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

###########################################  UNIT TESTS


##  naive  ----------

%-naive:   	%-test.o  naive.o
	$(CXX) $(CXXFLAGS) -o $@ $^  $(LDFLAGS) $(LDLIBS)


##  vsearch  ----------

%-vsearch:  %-test.o  vsearch.o
	$(CXX) $(CXXFLAGS) -o $@ $^  $(LDFLAGS) $(LDLIBS)


##  multifast  -----

multifast.o:                  $(MULTIFAST)/ahocorasick/build/libahocorasick.a
multifast.o:                  CXXFLAGS += -I $(MULTIFAST)/ahocorasick/
%multifast multifast-%:       LDFLAGS  += -L $(MULTIFAST)/ahocorasick/build/
%multifast multifast-%:       LDLIBS   += -l ahocorasick


%-multifast:   %-test.o  multifast.o
	$(CXX) $(CXXFLAGS) -o $@ $^  $(LDFLAGS) $(LDLIBS)


##  test  ----------

%-test.o:  %-test.cc search.h


test:    $(LIBS) u-naive u-multifast
	./u-naive
	./u-multifast


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


########################################### BUILD FILTER

%-filter mk_mmap_names tts:   CXXFLAGS += -I$(STREAMCORPUS)/cpp
%-filter mk_mmap_names tts:   LDFLAGS  += -L$(STREAMCORPUS)/cpp
%-filter mk_mmap_names tts:   LDLIBS   += -lstreamcorpus -lthrift -lboost_program_options  

$(filternames_thrift_o): filternames_types.h
filter.o mk_mmap_names.o: filternames_types.h $(STREAMCORPUS)/cpp/libstreamcorpus.a 

naive-filter:          filter.o		$(filternames_thrift_o)  naive.o
multifast-filter:      filter.o		$(filternames_thrift_o)  multifast.o
vsearch-filter:        filter.o  	$(filternames_thrift_o)  vsearch.o

mk_mmap_names:         mk_mmap_names.o		$(filternames_thrift_o)  naive.o

###########################################  make names mmap

count_names:  mk_mmap_names | $(DATA)
	$(RM) corpus.txt
	$(CAT_CORPUS_TO_STDOUT) |  ./mk_mmap_names  -f $(NAMES)  -c

names_data.mmap:  mk_mmap_names | $(DATA)
	$(RM) corpus.txt
	$(CAT_CORPUS_TO_STDOUT) |  ./mk_mmap_names  -f $(NAMES) 
	ls -l names_data.mmap 

##########################################  MISC
clean:
	$(RM)  *.o filter *-filter  $(output) $(filtername_thrift_cpp) *.gch [tu]-search mk_mmap_names test_throughput_speed
	$(RM)  gmon.out  *.prof  *.prof.png
	$(RM)  ?-multifast ?-multifast ?-naive ?-vsearch

libclean:  clean
	make -C $(STREAMCORPUS)/cpp/      clean
	make -C $(MULTIFAST)/ahocorasick/ clean
	#$(RM)  $(MULTIFAST)/ahocorasick/build/libahocorasick.a $(STREAMCORPUS)/cpp/libstreamcorpus.a 

gprof:
	gprof $(exe) > $(exe).gprof
	cat $(exe).gprof  |  gprof2dot -s -n 0 -e 0 | dot -Tpng -o $(exe).gprof.png;
	eog $(exe).gprof.png

###########################################  Benchmarks

#RUN:	MODE := OPTIMIZE

run:  $(LINK)-filter | $(DATA)
	@$(POWER_SAVING_OFF)
	$(CAT_CORPUS_TO_STDOUT) | $(TIME) ./$(LINK)-filter -N $(N) -f $(NAMES) -I $(I) --verbose $(RUNFLAGS) > $(OUTPUT)

###########################################  TEST_THROUGHPUT_SPEED

test_throughput_speed:      	$(filternames_thrift_o)

tts: test_throughput_speed  | $(DATA)
	$(TIME) ./test_throughput_speed streamcorpus/test-data/john-smith-tagged-by-lingpipe-0-v0_3_0.sc  /tmp/tts.out

###########################################  MMAP Benchmarks

B-multifast:  b-multifast
	$(TIME) ./b-multifast

#########################################  

vsearch.o : search.h

t-vsearch: t-vsearch.o vsearch.o
t-vsearch: t-vsearch.o vsearch.o
	$(CXX) $(CXXFLAGS) -o $@ $^  $(LDFLAGS) $(LDLIBS)
