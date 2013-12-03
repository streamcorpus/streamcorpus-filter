
STREAMCORPUS_O := streamcorpus_constants.o streamcorpus_types.o

$(STREAMCORPUS_O): 
	thrift  --out . --gen cpp  filternames.thrift

libstreamcorpus.a: $(STREAMCORPUS_O)
	ar -cvq  $@ $^

clean: 
	$(RM) $(STREAMCORPUS_O) libstreamcorpus.a

