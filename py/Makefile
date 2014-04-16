
clean: 
	rm -rf build dist src/streamcorpus_filter.egg-info

.PHONY: build
# build:	src/streamcorpus_filter/constants.py src/streamcorpus_filter/ttypes.py
# build:	
	python setup.py build


install: build
	pip install .
	python setup.py install_test

test: install
	python setup.py test

src/streamcorpus_filter/constants.py src/streamcorpus_filter/ttypes.py:	../if/streamcorpus-filter-v0_1_0.thrift
	thrift --gen py -out src ../if/streamcorpus-filter-v0_1_0.thrift
