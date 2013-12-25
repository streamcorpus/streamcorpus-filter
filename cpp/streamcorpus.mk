
%:  %.o
	$(CXX) $(CXXFLAGS) -o $@ $^  $(LDFLAGS) $(LDLIBS)
	
streamcorpus_cpp := streamcorpus_constants.h  streamcorpus_types.h streamcorpus_constants.cpp  streamcorpus_types.cpp
streamcorpus_o   := streamcorpus_constants.o streamcorpus_constants.o  streamcorpus_types.o


$(streamcorpus_cpp): 
	cp ../if/streamcorpus-v0_3_0.thrift /tmp/streamcorpus.thrift
	thrift  --out . --gen cpp:dense /tmp/streamcorpus.thrift

$(streamcorpus_o): $(streamcorpus_cpp)

libstreamcorpus.a: $(streamcorpus_o)
	$(AR) -cvq  $@ $^

clean: 
	$(RM) $(streamcorpus_cpp) l$(streamcorpus_o) libstreamcorpus.a

