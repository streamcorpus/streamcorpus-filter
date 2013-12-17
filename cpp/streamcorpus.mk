
%:  %.o
	$(CXX) $(CXXFLAGS) -o $@ $^  $(LDFLAGS) $(LDLIBS)
	
streamcorpus_cpp := streamcorpus_constants.h  streamcorpus_types.h
streamcorpus_o   := streamcorpus_constants.o  streamcorpus_types.o

$(streamcorpus_cpp): 
	thrift  --out . --gen cpp   ../if/streamcorpus-v0_3_0.thrift

$(streamcorpus_o): $(streamcorpus_cpp)

libstreamcorpus.a: $(streamcorpus_o)
	$(AR) -cvq  $@ $^

clean: 
	$(RM) $(streamcorpus_o) libstreamcorpus.a

