This is slightly modified annotated_example.cpp with boost:regex replaced with
naive multisearch and added timer code. 

Before running test, change pathes at Makefile config section.

There are currently 3 targets:

	run1 -- run benchmark on corpus bundled with streamcorpus repo
      	run2 -- run benchmark on trec-kba-2013-rated-chunks-indexed
       	test -- unit test

Any of above will call cmake generator, run 2nd make, build executables, and run tests.
Benchmark metrics are recored on standard output. 

Units testing framework - just a simple include file "check.h" with couple
macros.   Until recently this "framework" had 3 LOC. Please look into the
source, it still under 50 LOC.  To test something, we just use CHECK macro:

	CHECK(result,expected_value);






