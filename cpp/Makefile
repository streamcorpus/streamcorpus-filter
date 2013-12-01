
# read config file 
include config.mk

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
      COMMON_FLAGS += -Wall -std=c++0x -DID='"$(LINK)-$(ID)"'
    OPTIMIZE_FLAGS += -O3 -DNDEBUG -march=native
       DEBUG_FLAGS += -O0 -DDEBUG  -ggdb3 -fstack-protector  # -D_GLIBCXX_DEBUG 

ifneq ($(CXX),cl++)
ifneq ($(CXX),clang++)
	DEBUG_FLAGS += -fdelete-null-pointer-checks  -fbounds-check
endif
endif

u-%:   MODE = DEBUG
u-%:   MODE = DEBUG
u-%:   DEBUG_FLAGS += -D_GLIBCXX_DEBUG 
t-%:   DEBUG_FLAGS += -D_GLIBCXX_DEBUG 

          CXXFLAGS += $(COMMON_FLAGS) $($(MODE)_FLAGS)

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

u-naive:   	u-test.o  naive.o
	$(CXX) $(CXXFLAGS) -o $@ $^  $(LDFLAGS) $(LDLIBS)


##  vsearch  ----------

u-vsearch:  u-test.o  vsearch.o
	$(CXX) $(CXXFLAGS) -o $@ $^  $(LDFLAGS) $(LDLIBS)


##  multifast  -----

multifast.o:                  $(MULTIFAST) $(LIBS)
multifast.o:                  CXXFLAGS += -I $(MULTIFAST)/ahocorasick/
%multifast multifast-%:       LDFLAGS  += -L $(MULTIFAST)/ahocorasick/build/
%multifast multifast-%:       LDLIBS   += -l ahocorasick

u-multifast:   u-test.o  multifast.o
	$(CXX) $(CXXFLAGS) -o $@ $^  $(LDFLAGS) $(LDLIBS)


##  test  ----------

test.o:  search.h

test:    $(LIBS) u-naive u-multifast u-vsearch
	./u-naive
	./u-multifast
	./u-vsearch

##########################################  DATA & EXTERNAL LIBS

DATA          = $(NAMES)  $(CORPUS)
LIBS          = $(MULTIFAST)  $(STREAMCORPUS)
EXTERNAL_DEPS = $(DATA)  $(LIBS)


$(NAMES):
	mkdir -p data/
	wget -O - $(NAMES_URL) | zcat > $(NAMES)

$(CORPUS):
	mkdir -p $(CORPUS)
	wget -O - $(CORPUS_URL) | tar -xJ -C $(CORPUS) --strip-components=1 -f -

$(MULTIFAST) :
	svn checkout $(MULTIFAST_REPO) multifast
	CXXFLAGS+=-O3 make -C $(MULTIFAST)/ahocorasick/
	ls                    $(MULTIFAST)/ahocorasick/build/libahocorasick.a
	
$(STREAMCORPUS) :
	git clone -b v0.3.0-dev $(STREAMCORPUS_REPO)
	cd  $(STREAMCORPUS)/cpp;  cmake -F CMakeLists.txt;  CXXFLAGS+=-O3 make;  ls libstreamcorpus.a

########################################### BUILD FILTER

%-filter:   CXXFLAGS += -I$(STREAMCORPUS)/cpp
%-filter:   LDFLAGS  += -L$(STREAMCORPUS)/cpp
%-filter:   LDLIBS   += -lstreamcorpus -lthrift -lboost_program_options

$(filternames_thrift_o): filternames_types.h
filter.o: filternames_types.h $(LIBS) $(STREAMCORPUS)

naive-filter:          filter.o  $(filternames_thrift_o)  naive.o
multifast-filter:      filter.o  $(filternames_thrift_o)  multifast.o
vsearch-filter:        filter.o  $(filternames_thrift_o)  vsearch.o

###########################################  Benchmarks

RUN:	MODE := OPTIMIZE

run:  $(LINK)-filter  | $(DATA)
	@$(POWER_SAVING_OFF)
	$(CAT_CORPUS_TO_STDOUT) | ./$(LINK)-filter -N $(N) -f $(NAMES) > $(OUTPUT)

##########################################
clean:
	$(RM)  *.o filter *-filter u-naive  u-multifast $(output) $(filtername_thrift_cpp) *.gch [tu]-search

#########################################  

vsearch.o : search.h

t-vsearch: t-vsearch.o vsearch.o
t-vsearch: t-vsearch.o vsearch.o
	$(CXX) $(CXXFLAGS) -o $@ $^  $(LDFLAGS) $(LDLIBS)
	
