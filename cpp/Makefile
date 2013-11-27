
# read config file 
include config.mk

###########################################  HELP

help:
	@echo "Please select specific target:"
	@echo "	  run -- run benchmark"
	@echo "	  test -- unit test"
	@echo "	  clean -- remove computer generated files"

.PHONY: help run test clean 

###########################################  COMPILER FLAGS

              MODE ?= OPTIMIZE
      COMMON_FLAGS += -Wall -std=c++0x
    OPTIMIZE_FLAGS += -O3 -DNDEBUG -march=native
       DEBUG_FLAGS += -O0 -DDEBUG  -ggdb3 -fdelete-null-pointer-checks  -fstack-protector -fbounds-check # -D_GLIBCXX_DEBUG 
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

########################################### BUILD FILTER

%-filter:   CXXFLAGS += -I$(STREAMCORPUS)/cpp
%-filter:   LDFLAGS  += -L$(STREAMCORPUS)/cpp
%-filter:   LDLIBS   += -lstreamcorpus -lthrift -lboost_program_options

$(filternames_thrift_o): filternames_types.h
filter.o: filternames_types.h $(LIBS) $(STREAMCORPUS)

naive-filter:          filter.o  $(filternames_thrift_o)  naive.o
multifast-filter:      filter.o  $(filternames_thrift_o)  multifast.o


###########################################  UNIT TESTS


##  naive  ----------

u-naive:   	u-test.o  naive.o
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

test:    $(LIBS) u-naive u-multifast
	./u-naive
	./u-multifast

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

###########################################  Benchmarks


run:  $(LINK)-filter  | $(DATA)
	@$(POWER_SAVING_OFF)
	$(CAT_CORPUS_TO_STDOUT) | ./$(LINK)-filter -N $(N) -f $(NAMES) > $(OUTPUT)

##########################################
clean:
	$(RM)  *.o filter *-filter u-naive  u-multifast $(output) $(filtername_thrift_cpp) *.gch
