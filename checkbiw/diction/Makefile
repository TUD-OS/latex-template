PKGDIR  ?= ..
L4DIR 	?= $(PKGDIR)/../../l4
include $(L4DIR)/mk/Makeconf

runprefix  =$(DROPS_INSTDIR)/tool
instprefix =$(DROPS_STDDIR)/tool

TARGET = OBJ

all:: all-done

all-done: configure-done
	cd $(TARGET) && $(MAKE) all
	touch $@

install:: all-done
	cd $(TARGET) && $(MAKE) prefix=$(instprefix) $@

configure-done:
	mkdir -p $(TARGET)
	cd $(TARGET) && \
	  ../../contrib/diction/configure --prefix=$(runprefix)
	touch $@

clean::
	rm -f all-done
	if [ -f configure-done ]; then cd $(TARGET) && $(MAKE) clean; fi

cleanall::
	rm -rf $(TARGET) configure-done all-done

