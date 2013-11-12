######################################################################
# This Makefile is part of a skeleton (diploma) thesis.  If you have
# useful additions or suggestions, please feel free to contact me:
# Martin Unzner <munzner@os.inf.tu-dresden.de>
# or alternatively
# Julian Stecklina <jsteckli@os.inf.tu-dresden.de>
######################################################################

# your main document
DOC_TEX      = diplom.tex

# your sub documents (chapters)
DOC_TEX_ADD ?= conclusion.tex futurework.tex design.tex evaluation.tex	\
	       implementation.tex introduction.tex state.tex

# your bibtex databases
DOC_BIB     ?= own.bib

# images
DOC_IMG_JPG = images/squirrel.jpg # you can specify multiple images here
DOC_IMG_PNG =
DOC_IMG_PDF = images/diplom-aufgabe.pdf

# latex stuff
PDFLATEX    ?= pdflatex
BIBER       ?= biber
CHECKBIW    ?= checkbiw/src/checkbiw

######################################################################
# You should not need to adapt stuff below this line ...
######################################################################

DOC_PDF		= $(DOC_TEX:.tex=.pdf)
DOC_BASE	= $(DOC_TEX:.tex=)

DOC_CLEAN = $(DOC_PDF)									\
            $(DOC_BASE).{aux,log,toc,bcf,bbl,blg,ltf,brf,out,lof,nav,snm,acn,glo,ist,lot,run.xml}	\
            *~

VERBOSE = @

.PHONY: default pdf clean check-french-spacing checkbiw

default: pdf

$(DOC_PDF): $(DOC_TEX) $(DOC_TEX_ADD) $(DOC_BIB) $(DOC_IMG_JPG)		\
            $(DOC_IMG_PNG) $(DOC_IMG_PNG) $(DOC_IMG_PDF) Makefile
	$(VERBOSE)$(PDFLATEX) $(DOC_TEX) || \
	    ((grep 'TeX capacity exceeded' $(DOC_PDF:.pdf=.log) && \
	   echo -e "\n\033[31mIncrease pool_size to 200000 in" \
	           "/etc/texmf/texmf.cnf!\033[m\n" && false) || false)
	$(VERBOSE)[ -f $(DOC_BASE).bcf ] && \
	  $(BIBER) $(DOC_BASE).bcf
	$(VERBOSE)(export size=1 ; touch $(DOC_PDF);\
	  until [ $$size -eq `ls -o $(DOC_PDF) | awk '{print $$4}'` ]; do\
	    export size=`ls -o $(DOC_PDF) | awk '{print $$4}'` ;\
	    $(PDFLATEX) $(DOC_TEX) ;\
	  done)
# one more time, just to be sure ...
	$(VERBOSE)$(PDFLATEX) $(DOC_TEX)

pdf: $(DOC_PDF)

clean:
	rm -f $(DOC_CLEAN)

# Points out abbreviations and reminds you of escaping
# the space after the period
check-french-spacing:
	$(VERBOSE)export GREP_COLOR='1;32'; \
	export GREP_OPTIONS='--color=auto'; \
	grep "[A-Z]\{2,\}\." $(DOC_TEX) $(DOC_TEX_ADD) || \
	grep -e 'e\.g\.' -e 'i\.e\.' -e 'd\.h\.' $(DOC_TEX) $(DOC_TEX_ADD) || \
	true

# check for conformance with "bugs in writing", english only
checkbiw:
	$(VERBOSE)$(CHECKBIW) -v -c -d emdash $(DOC_TEX) $(DOC_TEX_ADD)

# EOF
