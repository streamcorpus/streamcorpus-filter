streamcorpus filter
===================

Before running test, copy "example_config.mk" to "config.mk" and change there config
variables to actual pathes (to data and libraries) that you have on your system.

Targets
-------

Current make targets:

       	test -- run all unit tests
	run  -- run benchmark 
	clean 


Unit tests
----------

Units testing framework is just one include file "check.h".
Until recently this "framework" had 3 LOC. Please look into the
source, it still under 50 LOC, if you want to know how it works.  To test
something, just use CHECK macro:

	CHECK(result==expected_value);


Runing filter benchmark
-----------------------

Obviously you need streamcorpus data and names file.
Set all variables in config.mk

To run test use:

	make MODE=OPTIMIZE LINK=multifast N=500 I=20 run









