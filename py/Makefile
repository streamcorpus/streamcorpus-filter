
.PHONY: build
build:
	thrift --gen py ../if/streamcorpus-filters-v0_1_0.thrift
	mv gen-py/streamcorpus_filters/ttypes.py src/streamcorpus_filters/
	mv gen-py/streamcorpus_filters/constants.py src/streamcorpus_filters/
	rm -rf gen-py


