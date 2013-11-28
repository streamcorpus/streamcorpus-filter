streamcorpus filter
===================

Before running test, copy "example_config.mk" to "config.mk" and change  config
variables if necessary.  Editing config.mk is now optional, as all data and
libraries are auto-downloaded and auto-build starting with P2. 

Targets
-------

Current make targets:

       	test -- run all unit tests
	run  -- run benchmark 
	clean 


Running filter benchmark
-----------------------

Streamcorpus and names file will be downloaded automatically (as dependency). 
To run test use with 500 names used and to precess 20 items:

	make N=500 I=20 run

N and I parameters are optional.   They have default values set in config.mk.


Unit tests
----------

Units testing framework is just one include file "check.h".
Until recently this "framework" had 3 LOC. Please look into the
source, it still under 50 LOC, if you want to know how it works.  To test
something, use CHECK macro:

	CHECK(result==expected_value);
