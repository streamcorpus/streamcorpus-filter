###########################################  CONFIG
filtername="test-name-strings.scf"
corpus=/tr/sc-lvv-v0.3.0-dev/test-data/john-smith-tagged-by-lingpipe-0-v0_2_0.sc
output=/tmp/filtered.sc
streamcorpus_src=/tr/sc-lvv-v0.3.0-dev/cpp


###########################################  Compile flags
COMMON   = -Wall -std=c++11
O_FLAGS  +=  -O3  -march=native -DNDEBUG  
D_FLAGS  +=  -O0  -DDEBUG -ggdb3 -D_GLIBCXX_DEBUG -fdelete-null-pointer-checks  -fstack-protector -ftrapv -fbounds-check
CXXFLAGS += $(COMMON) $(D_FLAGS) 


####################################################

thrift_generated= \
	build/filternames_constants.cpp  \
	build/filternames_constants.h  \
	build/filternames_types.cpp  \
	build/filternames_types.h 


run:	filter
	./filter -f $(filtername) < $(corpus) > $(output)
	
CMakeLists.txt:   $(thrift_generated)

build/Makefile:	Makefile CMakeLists.txt   $(thrift_generated) 
	cd build && cmake -DSC=$(streamcorpus_src)  -DCMAKE_BUILD_TYPE=Debug .. 

build/filter: filter.cc build/Makefile  $(thrift_generated)   Makefile
	rm -f filter
	cd build && make filter

filter:	build/filter
	ln -s build/filter filter

$(thrift_generated): filternames.thrift
	mkdir -p build 
	thrift  --out build --gen cpp  filternames.thrift

###########################################  Unit tests                   


multisearch_naive.o:   multisearch.h multisearch_naive.cc 

u-multisearch_naive:   multisearch_naive.o u-multisearch_naive.o 
	$(CXX) $(CXXFLAGS) -o $@ $?

test:     u-multisearch_naive
	./u-multisearch_naive


#############################################
clean:
	rm -rf build/
	rm -f filter u-multisearch_naive
