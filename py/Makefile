
.PHONY: build
build:
	thrift --gen py ../if/streamcorpus-filter-v0_1_0.thrift
	mv gen-py/streamcorpus_filter/ttypes.py src/streamcorpus_filter/
	mv gen-py/streamcorpus_filter/constants.py src/streamcorpus_filter/
	rm -rf gen-py

install: build
	python setup.py install
