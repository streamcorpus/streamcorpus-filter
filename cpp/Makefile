
# set pathes in config.mk
include config.mk

###########################################  Compiler flags
              MODE ?= DEBUG
      COMMON_FLAGS += -Wall -std=c++0x
    OPTIMIZE_FLAGS += -O3 -DNDEBUG -march=native
       DEBUG_FLAGS += -O0 -DDEBUG  -ggdb3 -fdelete-null-pointer-checks  -fstack-protector -fbounds-check # -D_GLIBCXX_DEBUG 
          CXXFLAGS += $(COMMON_FLAGS) $($(MODE)_FLAGS) 

%-filter:  filter.o
	$(CXX) $(CXXFLAGS) -o $@ $^  $(LDFLAGS) $(LDLIBS)

%:  %.o
	$(CXX) $(CXXFLAGS) -o $@ $^  $(LDFLAGS) $(LDLIBS)
	
###########################################

help:
	@echo "Please select specific target:"
	@echo "	  run1 -- run benchmark on corpus bundled with streamcorpus repo"
	@echo "	  run2 -- run benchmark on trec-kba-2013-rated-chunks-indexed"
	@echo "	  test -- unit test"
	@echo "	  clean -- remove computer generated files"

########################################### thrift: flternames 

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

########################################### build filter

%-filter:   CXXFLAGS += -I$(STREAMCORPUS)
%-filter:   LDFLAGS  += -L$(STREAMCORPUS)
%-filter:   LDLIBS   += -lstreamcorpus -lthrift -lboost_program_options

filter.o $(filternames_thrift_o): filternames_types.h

naive-filter:   filter.o $(filternames_thrift_o) naive.o
multifast-filter:      filter.o $(filternames_thrift_o) multifast.o

###########################################  Unit tests


### NAIVE

naive.o:  search.h
u-naive:  naive.o u-naive.o


### MULTIFAST

multifast.o:                  CXXFLAGS += -I $(MULTIFAST)/ahocorasick/
%multifast multifast-%:       LDFLAGS  += -L $(MULTIFAST)/ahocorasick/build/
%multifast multifast-%:       LDLIBS   += -l ahocorasick

multifast.o:   search.h
u-multifast:  u-multifast.o  multifast.o


### RUN TESTS

test:     u-naive
	./u-naive

###########################################  Benchmarks

LINK    ?= naive
N	?= 10000	# max names
I	?= 20	# max items


run:	$(LINK)-filter
	$(POWER_SAVING_OFF)
	$(CORPUS) | ./$(LINK)-filter -N $(N) -f $(NAMES) > $(OUTPUT)

##########################################
clean:
	$(RM)  *.o filter *-filter u-naive  u-multifast $(output) $(filtername_thrift_cpp) *.gch
