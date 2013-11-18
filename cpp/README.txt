This is benchmark for filter directory.

Before running test, copy "example_config.mk" to "config.mk" and change in it config
variables  to actual pathes that you have on your system.

Directory with streamcorpus repo should have generated thrift files for cpp and 
streamcorpus library (libstreamcorpus.a).

Current make targets:

	run1 -- run benchmark on corpus bundled with streamcorpus repo
      	run2 -- run benchmark on trec-kba-2013-rated-chunks-indexed
       	test -- unit test
	clean 


Any of above will call cmake generator, run 2nd make, build executables, and run tests.
Benchmark metrics are recored on standard output. 

Units testing framework - just a simple include file "check.h" with couple
macros.   Until recently this "framework" had 3 LOC. Please look into the
source, it still under 50 LOC, if you want to know how it works.  To test
something, we just use CHECK macro:

	CHECK(result==expected_value);






