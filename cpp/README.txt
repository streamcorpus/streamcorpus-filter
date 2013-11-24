This is directory for streamcorpus filter benchmarking.

Before running test, copy "example_config.mk" to "config.mk" and change config
variables  to actual pathes that you have on your system.

Directory with streamcorpus repo should have generated thrift files for cpp and 
streamcorpus library (libstreamcorpus.a).

Current make targets:

	run  -- run benchmark on corpus bundled with streamcorpus repo
       	test -- unit test
	clean 

Optionally you can set compile mode with make's MODE variable. Possible
values: DEBUG (default), OPTIMIZATION.
For example to compile filter with debug compilation:

	make MODE=DEBUG filter

Units testing framework is just one include file "check.h".
Until recently this "framework" had 3 LOC. Please look into the
source, it still under 50 LOC, if you want to know how it works.  To test
something, just use CHECK macro:

	CHECK(result==expected_value);






