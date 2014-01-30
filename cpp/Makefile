
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

all:	filter-multifast filter-multifast-sc2

sc3/sc3.thrift:	streamcorpus/if/streamcorpus-v0_3_0.thrift
	mkdir -p sc3
	patch -p0 -o $@ $^ sc3.diff

sc3/sc3_constants.cpp sc3/sc3_constants.h sc3/sc3_types.cpp sc3/sc3_types.h:	sc3/sc3.thrift
	$(THRIFT)  --out sc3 --gen cpp $^

# regenerated diff
sc3.diff:	streamcorpus/if/streamcorpus-v0_3_0.thrift sc3/sc3.thrift
	diff -u streamcorpus/if/streamcorpus-v0_3_0.thrift sc3/sc3.thrift > sc3.diff

sc2/sc2.thrift:	streamcorpus/if/streamcorpus-v0_2_0.thrift
	mkdir -p sc2
	patch -p0 -o $@ $^ sc2.diff

# regenerated diff
sc2.diff:	streamcorpus/if/streamcorpus-v0_2_0.thrift sc2/sc2.thrift
	diff -u streamcorpus/if/streamcorpus-v0_2_0.thrift sc2/sc2.thrift > sc2.diff

sc2/sc2_types.cpp sc2/sc2_constants.cpp sc2/sc2_types.h sc2/sc2_constants.h:	sc2/sc2.thrift
	mkdir -p sc2
	$(THRIFT)  --out sc2 --gen cpp $^

multifast multifast/ahocorasick/node.c multifast/ahocorasick/ahocorasick.c multifast/ahocorasick/ahocorasick.h:
	svn co svn://svn.code.sf.net/p/multifast/code/trunk@31 multifast
	cat multifast.patch | (cd multifast && patch -p0)

filternames.thrift:	../if/streamcorpus-filter-v0_1_0.thrift
	cp $^ $@

filternames_constants.cpp filternames_types.cpp filternames_types.h filternames_constants.h:	filternames.thrift
	$(THRIFT)  --out . --gen cpp  filternames.thrift

#streamcorpus_constants.o:	streamcorpus/cpp/streamcorpus_constants.cpp
#	$(CXX) $(CXXFLAGS) -c -o $@ $^

#streamcorpus_types.o:	streamcorpus/cpp/streamcorpus_types.cpp
#	$(CXX) $(CXXFLAGS) -c -o $@ $^

MULTIFAST_OBJS = multifast.o normalize.o multifast/ahocorasick/node.o multifast/ahocorasick/ahocorasick.o filternames_constants.o filternames_types.o
MULTIFAST_OBJS += sc3/sc3_constants.o sc3/sc3_types.o
MULTIFAST_OBJS += sc2/sc2_constants.o sc2/sc2_types.o
# streamcorpus_constants.o streamcorpus_types.o

FILTER_MULTIFAST_OBJS = $(MULTIFAST_OBJS) filter.o

filter-multifast:	${FILTER_MULTIFAST_OBJS}
	$(CXX) $(CXXFLAGS) ${FILTER_MULTIFAST_OBJS} $(LDFLAGS) -licuuc -lthrift -lboost_program_options -o $@

UNIT_OBJS = $(MULTIFAST_OBJS) unit.o

unit-multifast:	$(UNIT_OBJS)
	$(CXX) $(CXXFLAGS) $(UNIT_OBJS) $(LDFLAGS) -licuuc -lthrift -lboost_program_options -o $@

clean:
	rm -rf ${FILTER_MULTIFAST_OBJS}

multifast.o:	multifast/ahocorasick/ahocorasick.h
filter.o:	filternames_types.h filternames_constants.h sc3/sc3_types.h

filter2.o:	filter.cc filternames_types.h filternames_constants.h sc2/sc2_types.h
	$(CXX) $(CXXFLAGS) -DUSE_STREAMCORPUS_2=1 -c -o $@ filter.cc

filter-multifast-sc2:	filter2.o $(MULTIFAST_OBJS)
	$(CXX) $(CXXFLAGS) filter2.o $(MULTIFAST_OBJS) $(LDFLAGS) -licuuc -lthrift -lboost_program_options -o $@


test:	filter-multifast unit-multifast .PHONY
	./unit-multifast
	python test_filter.py

.PHONY:
