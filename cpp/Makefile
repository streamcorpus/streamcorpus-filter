
# set pathes in config.mk
include config.mk

###########################################  Compiler flags
              MODE ?= DEBUG
      COMMON_FLAGS += -Wall -std=c++0x
OPTIMIZATION_FLAGS += -O3 -DNDEBUG -march=native
       DEBUG_FLAGS += -O0 -DDEBUG  -ggdb3 -fdelete-null-pointer-checks  -fstack-protector -fbounds-check # -D_GLIBCXX_DEBUG 
          CXXFLAGS += $(COMMON_FLAGS) $($(MODE)_FLAGS) 

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

########################################### Build filter

filter:   CXXFLAGS += -I$(streamcorpus_src)
filter:   LDFLAGS  += -L$(streamcorpus_src)
filter:   LDLIBS   += -lstreamcorpus -lthrift -lboost_program_options

filter.o $(filternames_thrift_o): $(filtername_thrift_cpp)

filter:  $(filternames_thrift_o) multisearch_naive.o filter.o
	$(CXX) $(CXXFLAGS) -o $@  $^ $(LDFLAGS) $(LDLIBS)

###########################################  Unit tests

multisearch_naive.o:   multisearch.h

u-multisearch_naive:  multisearch_naive.o u-multisearch_naive.o
	$(CXX) $(CXXFLAGS) -o $@ $^  $(LDFLAGS) $(LDLIBS)

test:     u-multisearch_naive
	./u-multisearch_naive


###########################################  Benchmarks

run1:	filter
	./filter -f $(filtername) < $(corpus) > $(output)

run2:	filter
	find $(trec_corpus) -type f | head -n 20 | xargs cat | ./filter -f $(filtername) > $(output)


##########################################
clean:
	$(RM)  *.o filter u-multisearch_naive  $(output) $(filtername_thrift_cpp) 
