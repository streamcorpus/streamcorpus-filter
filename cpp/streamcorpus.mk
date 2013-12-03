
STREAMCORPUS_O := streamcorpus_constants.o streamcorpus_types.o

libstreamcorpus.a: $(STREAMCORPUS_O)
	ar -cvq  $@ $^

clean: 
	$(RM) $(STREAMCORPUS_O) libstreamcorpus.a

