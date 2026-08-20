Benchmarking Snowball and the Oleander stemmers
===============================================

This is a hacky branch to benchmark Snowball and the Oleander stemmers.

The Oleander stemming library works in wide characters, and this branch
compares it with Snowball's generated C++ stemmers in wide-character
mode, so it's testing equivalent functionality (except that some of the
Snowball stemming algorithms may be slightly updated since the versions
Oleander currently implements; in these cases the Snowball stemmer
typically has an additional rule or two so we'd expect it to be slightly
slower as a result).

To run the benchmarking, you need snowball-data checked out as a
sibling directory to your snowball checkout, e.g. like this:
::

    $ ls -l ~/dev
    total 8
    drwxr-xr-x  57 olly olly      20480 Aug 20 15:25 snowball
    drwxr-xr-x  50 olly olly      12288 Aug 18 16:17 snowball-data

The benchmarking is done with valgrind's cachegrind tool (which
gives an estimated cycle count), so you need to have valgrind installed.
The big advantage of cachegrind is that you don't need to worry if
a difference is noise due to other load on the machine at the time,
etc.

By default the makefile included will clone the Oleandar stemmers repo
as a subdirectory if it doesn't already exist.  If you want to use a
checkout you already have you should be able to symlink it in before
you do anything else - e.g. to use ~/dev/OleanderStemmingLibrary
run this from your snowball git repo directory:
::

    cd oleander
    ln -s ~/dev/OleanderStemmingLibrary OleanderStemmingLibrary

To build:
::

    cd oleandar
    make

Then to run the benchmark (English by default):
::

    make benchmark

The benchmark also measures the stemwords harness without any actual
stemming, and subtracts that so we're comparing just the stemming
part.

You can run it for other languages (use "dutch" for Dutch, which is
handled specially in the makefile), e.g. you can build and benchmark
for French in one command:
::

    make LANGUAGE=french clean all benchmark

I found compiling in all the Oleander stemmers in the same build gave
me slower stemming, so it seemed fairer to test just one per build as that
is a valid use case.

For english I get:
::

    cachegrind.out.english-oleander: 128082261 - 44484427 = 83597834 estimated cycles
    cachegrind.out.english-stemwords: 113091721 - 44484427 = 68607294 estimated cycles
    cachegrind.out.english-stemwords is 17.9317325374722% faster

The difference varies by stemmer, but Snowball git main is faster in
every case (by 9% for german up to 83% for portuguese).

I'm testing on Debian unstable with GCC as the compiler and using -O2
for both Snowball and Oleander.  (I think cachegrind defaults to simulating
based on the CPU you have so you may not get identical numbers to me even
if you use the same OS and compiler.)

You can specify the C++ compiler to use by setting ``CXX``, e.g.
::

    make CXX=clang++ LANGUAGE=french clean all benchmark
