
CXXFLAGS += -std=c++0x

CFLAGS += -DHAVE_ICU=1
CXXFLAGS += -DHAVE_ICU=1

COMMON_CFLAGS = -Imultifast/ahocorasick -Istreamcorpus/cpp -Wall -DID="\"foo\""
CFLAGS += ${COMMON_CFLAGS}
CXXFLAGS += ${COMMON_CFLAGS}


PROFILE_CFLAGS = -O0 -ggdb3 -pg  -fno-omit-frame-pointer -fno-inline-functions -fno-inline-functions-called-once -fno-optimize-sibling-calls -fno-default-inline -fno-inline -DPROFILE

#CFLAGS += ${PROFILE_CFLAGS}
#CXXFLAGS += ${PROFILE_CFLAGS}

OPTIMIZE_CFLAGS = -O3 -DNDEBUG -march=native -DOPTIMIZE

CFLAGS += ${OPTIMIZE_CFLAGS}
CXXFLAGS += ${OPTIMIZE_CFLAGS}

DEBUG_CFLAGS = -ggdb3 -O0

#CFLAGS += ${DEBUG_CFLAGS}
#CXXFLAGS += ${DEBUG_CFLAGS}

THRIFT=thrift

-include local.make

all:	filter-multifast

multifast multifast/ahocorasick/node.c multifast/ahocorasick/ahocorasick.c multifast/ahocorasick/ahocorasick.h:
	svn co svn://svn.code.sf.net/p/multifast/code/trunk multifast

filternames_constants.cpp filternames_types.cpp filternames_types.h filternames_constants.h:	filternames.thrift
	$(THRIFT)  --out . --gen cpp  filternames.thrift

streamcorpus_constants.o:	streamcorpus/cpp/streamcorpus_constants.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $^

streamcorpus_types.o:	streamcorpus/cpp/streamcorpus_types.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $^

MULTIFAST_OBJS = multifast.o normalize.o multifast/ahocorasick/node.o multifast/ahocorasick/ahocorasick.o filternames_constants.o filternames_types.o streamcorpus_constants.o streamcorpus_types.o

FILTER_MULTIFAST_OBJS = $(MULTIFAST_OBJS) filter.o

filter-multifast:	${FILTER_MULTIFAST_OBJS}
	$(CXX) $(CXXFLAGS) ${FILTER_MULTIFAST_OBJS} $(LDFLAGS) -licuuc -lthrift -lboost_program_options -o $@

UNIT_OBJS = $(MULTIFAST_OBJS) unit.o

unit-multifast:	$(UNIT_OBJS)
	$(CXX) $(CXXFLAGS) ${UNIT_OBJS} $(LDFLAGS) -licuuc -lthrift -lboost_program_options -o $@

clean:
	rm -rf ${FILTER_MULTIFAST_OBJS}

multifast.o:	multifast/ahocorasick/ahocorasick.h
filter.o:	filternames_types.h filternames_constants.h

test:	filter-multifast unit-multifast .PHONY
	./unit-multifast
	python test_filter.py

.PHONY:
