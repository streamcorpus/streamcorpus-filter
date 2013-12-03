
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

u-naive:   	u-test.o  naive.o
	$(CXX) $(CXXFLAGS) -o $@ $^  $(LDFLAGS) $(LDLIBS)


##  vsearch  ----------

u-vsearch:  u-test.o  vsearch.o
	$(CXX) $(CXXFLAGS) -o $@ $^  $(LDFLAGS) $(LDLIBS)


##  multifast  -----

multifast.o:                  $(MULTIFAST)/ahocorasick/build/libahocorasick.a
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
	ls      $(MULTIFAST)/ahocorasick/build/libahocorasick.a
	
$(STREAMCORPUS):
	git clone -b v0.3.0-dev $(STREAMCORPUS_REPO)

$(STREAMCORPUS)/cpp/libstreamcorpus.a : | $(STREAMCORPUS)
	make -f $(PWD)/streamcorpus.mk  -C  $(STREAMCORPUS)/cpp/  libstreamcorpus.a
	#cd  $(STREAMCORPUS)/cpp;  cmake -F CMakeLists.txt;  make  MAKE_VERBOSE=1 -B;  ls libstreamcorpus.a


########################################### BUILD FILTER

%-filter:   CXXFLAGS += -I$(STREAMCORPUS)/cpp
%-filter:   LDFLAGS  += -L$(STREAMCORPUS)/cpp
%-filter:   LDLIBS   += -lstreamcorpus -lthrift -lboost_program_options

$(filternames_thrift_o): filternames_types.h
filter.o: filternames_types.h $(STREAMCORPUS)/cpp/libstreamcorpus.a 

naive-filter:          filter.o  $(filternames_thrift_o)  naive.o
multifast-filter:      filter.o  $(filternames_thrift_o)  multifast.o
vsearch-filter:        filter.o  $(filternames_thrift_o)  vsearch.o

###########################################  Benchmarks

#RUN:	MODE := OPTIMIZE

run:  $(LINK)-filter | $(DATA)
	@$(POWER_SAVING_OFF)
	$(CAT_CORPUS_TO_STDOUT) | $(TIME) ./$(LINK)-filter -N $(N) -f $(NAMES) -I $(I) > $(OUTPUT)

##########################################  MISC
clean:
	$(RM)  *.o filter *-filter u-naive  u-multifast $(output) $(filtername_thrift_cpp) *.gch [tu]-search
	$(RM)  gmon.out  *.prof  *.prof.png

libclean:  clean
	make -C $(STREAMCORPUS)/cpp/      clean
	make -C $(MULTIFAST)/ahocorasick/ clean
	#$(RM)  $(MULTIFAST)/ahocorasick/build/libahocorasick.a $(STREAMCORPUS)/cpp/libstreamcorpus.a 

gprof:
	gprof $(exe) > $(exe).gprof
	cat $(exe).gprof  |  gprof2dot -s -n 0 -e 0 | dot -Tpng -o $(exe).gprof.png;
	eog $(exe).gprof.png

#########################################  

vsearch.o : search.h

t-vsearch: t-vsearch.o vsearch.o
t-vsearch: t-vsearch.o vsearch.o
	$(CXX) $(CXXFLAGS) -o $@ $^  $(LDFLAGS) $(LDLIBS)
