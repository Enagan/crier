.PHONY: install test
DESTDIR=.

all: test install

install:
	@rm -rf $(DESTDIR)/crier
	@cp -rf include/. $(DESTDIR)

test:
	@cd test && $(MAKE) test

all: test install
