######################################################################
# This Makefile is part of a skeleton (diploma) thesis.  If you have
# useful additions or suggestions, please fell free to contact me:
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

######################################################################
# You should not need to adapt stuff below this line ...
######################################################################

DOC_PDF		= $(DOC_TEX:.tex=.pdf)
DOC_BASE	= $(DOC_TEX:.tex=)

DOC_CLEAN = $(DOC_PDF)									\
            $(DOC_BASE).{aux,log,toc,bbl,blg,ltf,brf,out,lof,nav,snm,acn,glo,ist,lot}	\
            *~

VERBOSE = @

.PHONY: pdf clean

default: pdf

$(DOC_PDF): $(DOC_TEX) $(DOC_TEX_ADD) $(DOC_BIB) $(DOC_IMG_JPG)		\
            $(DOC_IMG_PNG) $(DOC_IMG_PNG) $(DOC_IMG_PDF) Makefile
	$(VERBOSE)$(PDFLATEX) $(DOC_TEX) || \
	    ((grep 'TeX capacity exceeded' $(DOC_PDF:.pdf=.log) && \
	   echo -e "\n\033[31mIncrease pool_size to 200000 in" \
	           "/etc/texmf/texmf.cnf!\033[m\n" && false) || false)
	$(VERBOSE)grep '\citation' $(DOC_PDF:.pdf=.aux) && \
	  bibtex $(basename $(DOC_PDF)) || true
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

# EOF
