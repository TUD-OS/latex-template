PKGDIR  ?= ..
L4DIR 	?= $(PKGDIR)/../../l4
include $(L4DIR)/mk/Makeconf

TARGET = checkbiw.inst
RULEFILES = biw lang-american emdash-nospace emdash-space cite-space passive

runprefix = $(DROPS_INSTDIR)/tool
runbindir = $(runprefix)/bin
runsharedir = $(runprefix)/share/checkbiw

instprefix = $(DROPS_STDDIR)/tool
instbindir = $(instprefix)/bin
instsharedir = $(instprefix)/share/checkbiw

all:: $(TARGET)

install:: $(TARGET)
	install -d $(instbindir)
	install -c -m 755 $(TARGET) $(instbindir)/checkbiw
	install -d $(instsharedir)
	install -c -m 644 $(RULEFILES) $(instsharedir)

clean::
	rm -rf checkbiw.inst checkbiw.inst.new

cleanall:: clean

# ......................................................................

checkbiw.inst: checkbiw
	sed -e 's,^\(DICTIONPATH=\).*$$,\1$${DICTIONPATH:-$(runbindir)},' \
	    -e 's,^\(CHECKBIWPATH=\).*$$,\1.:$${CHECKBIWPATH:+$${CHECKBIWPATH}:}$(runsharedir),' $< > $@.new
	mv $@.new $@
