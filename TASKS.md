
Phase I:  Initial Tests
-----------------------

1. Harness: discuss with bauer1j and make this work again:

   https://github.com/trec-kba/streamcorpus/tree/v0.3.0-dev/cpp

   https://github.com/trec-kba/streamcorpus/tree/v0.3.0-dev/examples/cpp


2. Basic unit test: make trivial string matching baseline that is
O(n*m) as basic test for the harness that reads StreamItems --- make a
simple text and match it.  This unit test can construct a StreamItem
from scratch to pass into the code under test.  Use a testing
framework, such as https://code.google.com/p/googletest/

3. Get an estimate of baseline throughput performance using the TREC
KBA 2013 Rated Documents Corpus described in
[streamcorpus-filter/data/trec-kba-2013-rated-documents.md](streamcorpus-filter/data/trec-kba-2013-rated-documents.md)


Phase II:  basic use of MultiFast
---------------------------------

4. Hook up MultiFast such that it passes the basic unit test.

   http://sourceforge.net/p/multifast/code/HEAD/tree/

5. Get an estimate of baseline throughput perf of MultiFast using the
TREC KBA 2013 Rated Documents Corpus.


Phase III:  assess character class issues
-----------------------------------------

6. How many of the StreamItems fail to match a name string in the
provided map?  Pick an example that fails, investigate manually to
check that it does in fact contain a string corresponding to the
expected target_id, and figure out what is required to make it match.  

Expected issues:

 - tokenization between words can vary, e.g. "John Smith" might need
   to be matched as "John[\s\n]Smith" with the "\s" meaning all
   Unicode whitespace:
   http://en.wikipedia.org/wiki/Space_(punctuation)#Spaces_in_Unicode
   http://en.wikipedia.org/wiki/Whitespace_character

 - case: "John Smith" might need to be matched as "JOHN SMiTH"

 - transliterations: it may be necessary to expand some of the
   characters into classes of characters, such as [e\u00E9\u00C9]

Notes from developer of MultiFast:

> I defined AC_ALPHABET_t type in actypes.h ; you can re-define it
> with whatever you want.  I suggest you to define an enum like this:
>
>     enum MyEnum
>     {
>        MYENUM_TOKEN_1,
>        MYENUM_TOKEN_2,
>     ....
>     };
> 
> and then
> 
>     typedef MyEnum AC_ALPHABET_t;
>
> and devise a pre processing state or tokenizing block. you can
> handle spaces and case sensitivity it that state. after that block
> you have string of tokens instead of bytes.



Phase IV:  assess dictionary size issues
----------------------------------------

7. using list of all titles from WP:

 - how large a data structure does MultiFast make?
 - how fast does it load into memory?
 - how does it interact with character class issues (especially tokenization)?

8. measure throughput against a large portion of the WLC


Phase V:  assess multi-core matching
------------------------------------

9. design & implement multi-core shared memory use of MultiFast

10. speed test on AWS EC2 cc2.8xlarge using 32 cores / 64 GB RAM
