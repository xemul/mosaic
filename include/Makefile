INCLUDEDIR=/usr/include/mosaic
INCLUDES=uapi/mosaic.h

install: $(INCLUDES)
	install -d $(DESTDIR)/$(INCLUDEDIR)
	install -m 644 $(INCLUDES) $(DESTDIR)/$(INCLUDEDIR)

clean:

.PHONY: install clean
