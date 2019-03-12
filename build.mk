#!/usr/bin/make -f
CMAKE?=cmake


default: all
	@echo "done: $@: $^"

%:
	$(MAKE) -C HippoMocksTest/ $@

test:
	$(MAKE) -C HippoMocksTest/ runtest


install: HippoMocks/hippomocks.h
	install -d ${DESTDIR}/usr/include/${<D}
	install ${<} ${DESTDIR}/usr/include/${<}

Makefile:
	${CMAKE} --version && ${CMAKE} .
	ls $@ || cp -av build.mk $@
