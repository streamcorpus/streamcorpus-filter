
# set pathes in config.mk
include config.mk

###########################################  Compiler flags
              MODE ?= DEBUG
      COMMON_FLAGS += -Wall -std=c++0x
OPTIMIZATION_FLAGS += -O3 -DNDEBUG -march=native
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

%-filter:   CXXFLAGS += -I$(streamcorpus_src)
%-filter:   LDFLAGS  += -L$(streamcorpus_src)
%-filter:   LDLIBS   += -lstreamcorpus -lthrift -lboost_program_options

filter.o $(filternames_thrift_o): $(filtername_thrift_cpp)

naive-filter:   filter.o $(filternames_thrift_o) naive.o
ac-filter:      filter.o $(filternames_thrift_o) multifast.o

###########################################  Unit tests


### NAIVE

naive.o:  search.h
u-naive:  naive.o u-naive.o


### MULTIFAST

multifast.o:                  CXXFLAGS += -I $(multifast)/ahocorasick/
%multifast ac-%:              LDFLAGS  += -L $(multifast)/ahocorasick/build/
%multifast ac-%:              LDLIBS   += -l ahocorasick

multifast.o:   search.h
u-multifast:  u-multifast.o  multifast.o


### RUN TESTS

test:     u-naive
	./u-naive

###########################################  Benchmarks

LINK ?=naive

run1:	  $(LINK)-filter
	$(power_saving_off)
	./$(LINK)-filter -f $(filtername) < $(corpus) > $(output)

run2:	$(LINK)-filter
	$(power_saving_off)
	find $(trec_corpus) -type f |head -n 20 |  xargs cat | ./$(LINK)-filter --max-items 20 -f $(filtername) > $(output)


##########################################
clean:
	$(RM)  *.o filter u-naive  u-multifast $(output) $(filtername_thrift_cpp) *.gch
