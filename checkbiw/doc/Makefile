PKGDIR  ?= ..
L4DIR   ?= $(PKGDIR)/../../l4
include $(L4DIR)/mk/Makeconf

prefix = $(DROPS_STDDIR)/tool

TXTFILES	:= mustread.txt manpage.txt
BIBFILES	:= own.bib

GOSHDIR		:= $(dir $(shell PATH=$(L4DIR)/tool/gosh:$$PATH which gosh))
GOSH		:= $(GOSHDIR)/gosh
TXT2TEX_OPT	:= --style manual.gosh
TXT2MAN_OPT	:= --style $(GOSHDIR)/man.gosh

TEXFILES	:= $(patsubst %.txt, %.tex,$(TXTFILES))
PDFFILES	:= $(TEXFILES:.tex=.pdf)
DVIFILES	:= $(TEXFILES:.tex=.dvi)

MANPAGES	:= checkbiw.1 bugsinwriting.7

all:: mustread.pdf $(MANPAGES)

# Make sure the corresponding txt files are the first dependency of
# each manpage -- the rule to create the manpages depends on this.
checkbiw.1: manpage.txt
bugsinwriting.7: mustread.txt
$(MANPAGES): $(GOSHDIR)/man.gosh $(GOSH) 

$(TEXFILES): manual.gosh $(GOSH)
$(PDFFILES) $(DVIFILES): $(BIBFILES) 

install:: $(MANPAGES)
	install -d $(prefix)/man/man1
	install -c -m 644 checkbiw.1 $(prefix)/man/man1
	install -d $(prefix)/man/man7
	install -c -m 644 bugsinwriting.7 $(prefix)/man/man7

clean::
	rm -f *.aux *.bbl *.blg *.log *.ps *.toc *.pdf *.dvi *.idx *.new \
	  $(TEXFILES) $(MANPAGES) 

cleanall:: clean
	rm -f *~

$(MANPAGES):
	$(GOSH) $(TXT2MAN_OPT) $< > $@.new
	mv $@.new $@

.PRECIOUS: .tex
%.tex: %.txt
	$(GOSH) $(TXT2TEX_OPT) $< > $@.new
	mv $@.new $@

%.pdf: %.tex 
	pdflatex $<
	bibtex $(basename $<) || true
	pdflatex $<
	pdflatex $<

.PRECIOUS: .dvi
%.dvi: %.tex 
	latex $<
	bibtex $(basename $<) || true
	latex $<
	latex $<

%.ps: %.dvi
	dvips -o $@ $<
