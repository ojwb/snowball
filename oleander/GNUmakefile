VALGRIND := valgrind
SNOWBALL_DATA := ../../snowball-data
LANGUAGE := english
CXX := g++

# Oleander has dutch_stem.h but the class is dutch_porter (and matches
# Snowball 3's dutch_porter algorithm).
ifeq ($(LANGUAGE),dutch)
SBL_LANG := dutch_porter
HEADER := "dutch_stem.h"
else
SBL_LANG := $(LANGUAGE)
HEADER := "$(LANGUAGE)_stem.h"
endif
CLASS := $(SBL_LANG)_stem

all: stemwords_oleander stemwords_nostem
	$(MAKE) -C.. SNOWBALL_WIDE=1 cxx

OleanderStemmingLibrary/src/stemming.h:
	git clone https://github.com/Blake-Madden/OleanderStemmingLibrary.git

stemwords_oleander: stemwords.cxx OleanderStemmingLibrary/src/stemming.h
	$(CXX) -IOleanderStemmingLibrary/src -Wall -W -O2 stemwords.cxx -o $@ \
	    -DLANGUAGE='$(SBL_LANG)' \
	    -DOLEANDER_CLASS='$(CLASS)' \
	    -DOLEANDER_HEADER='$(HEADER)'

stemwords_nostem: stemwords.cxx
	$(CXX) -Wall -W -O2 -DNO_STEM stemwords.cxx -o $@

benchmark:
	cat $(SNOWBALL_DATA)/$(SBL_LANG)/voc.txt >/dev/null # warm the cache
	for stemwords in ../cxx/stemwords ./stemwords_oleander ./stemwords_nostem ; do \
	  label=`echo "$${stemwords}"|sed 's!.*[_/]!!'` ; \
	  $(VALGRIND) --tool=cachegrind --cache-sim=yes --branch-sim=yes \
	      --cachegrind-out-file="cachegrind.out.$(LANGUAGE)-$${label}" \
	      $${stemwords} -l "$(SBL_LANG)" < "$(SNOWBALL_DATA)/$(SBL_LANG)/voc.txt" ; \
	done
	./cg_estimate \
	    cachegrind.out.$(LANGUAGE)-nostem \
	    cachegrind.out.$(LANGUAGE)-oleander \
	    cachegrind.out.$(LANGUAGE)-stemwords

clean:
	$(MAKE) -C.. clean
	rm -f stemwords_nostem stemwords_oleander
	rm -f cachegrind.out.*
