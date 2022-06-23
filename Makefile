######################################################################
# This Makefile is part of a skeleton (diploma) thesis.  If you have
# useful additions, suggestions or questions, please feel free to issue a
# PR or an issue at https://github.com/TUD-OS/latex-template
######################################################################

# main document
DOC_TEX     		= diplom.tex
# sub documents (chapters) without preamble
# (because we do not need checkbiw on the preamble).
DOC_TEX_ADD 		= $(wildcard content/*.tex)
# All Tex-files sorted alphabetically and concatenated to a single string.
# (make's shell utility converts newlines to spaces)
# This makes the "checkbiw"-script more convenient.
DOC_TEX_ALL_SORTED 	= $(shell echo $(DOC_TEX) $(DOC_TEX_ADD) | tr ' ' '\n' | sort)

DOC_PDF     		= $(DOC_TEX:.tex=.pdf)

LATEXMK              	= latexmk
# Add "-shell-escape" when you use "minted" instead of "listings" for beautiful code listings
LATEXMK_COMMON_FLAGS	= -pdf -use-make
LATEXMK_FLAGS			= $(LATEXMK_COMMON_FLAGS) -pdflatex=lualatex -halt-on-error
LATEXMK_WATCH_FLAGS		= $(LATEXMK_COMMON_FLAGS) -pvc -quiet -f -pdflatex="lualatex -synctex=1 -interaction=nonstopmode"

# Path to 'checkbiw'-script.
# Do not forget to execute `git submodule update --init` first.
CHECKBIW = checkbiw/src/checkbiw

# The @-symbol makes the commands quite, i.e., make does not
# output the commands it executes.
QUIET = @

.PHONY: default pdf clean check-french-spacing checkbiw watch

default: pdf

# Builds the PDF and creates a file with the format: "yyyy-MM-dd DRAFT Diplomarbeit - Branch <current branch>.pdf"
pdf:
	$(LATEXMK) $(LATEXMK_FLAGS) $(DOC_TEX)
	cp $(DOC_PDF) "$(shell date +\"%Y-%m-%d_%H%M%S\") DRAFT Diplomarbeit - Branch $(shell git rev-parse --abbrev-ref HEAD).pdf"

# Performs a watch task, i.e. automatically re-builds everything quickly on changes.
# If your PDF viewer supports a reload on file changes (such as the default PDF viewer in GNOME)
# you get a cool productive working environment.
#
# The dependency to "pdf" exists only that in case something doesn't build from the beginning
# the build immediately stops.
#
# Also see: https://paulklemm.com/blog/2016-03-06-watch-latex-documents-using-latexmk/
watch: pdf
	$(LATEXMK) $(LATEXMK_WATCH_FLAGS) $(DOC_TEX)

# Cleans all intermediate files except for the produced pdf files.
clean:
	$(LATEXMK) -C
	# latexmk does not clean up the aux-files produced by the chapters.
	# They are stand-alone compilation units with a dedicated aux file.
	# This comes because we include them with "\include" instead of "\input".
	find "./content" -type f -name "*.aux" | xargs -r rm
	$(QUIET)# Cleanup of "minted" package (if you use it - not included by default)
	$(QUIET)# find "./content" -type f -name "_minted*" | xargs -r rm -rf

# Points out abbreviations and reminds you of escaping
# the space after the period
check-french-spacing: $(DOC_TEX_ALL_SORTED)
	$(QUIET)export GREP_COLOR='1;32'; \
	grep --color=auto "[A-Z]\(2,\)\." $+ || \
	grep --color=auto -e 'e\.g\.' -e 'i\.e\.' -e 'd\.h\.' $+ || \
	true

# check for conformance with "bugs in writing", English only
checkbiw: $(DOC_TEX_ALL_SORTED)
	$(QUIET)$(CHECKBIW) -v -c $+

