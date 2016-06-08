#!/usr/bin/make -f

%:
	$(MAKE) -C HippoMocksTest/ $@

test:
	$(MAKE) -C HippoMocksTest/ runtest


install: HippoMocks/hippomocks.h
	install -d ${DESTDIR}/usr/include/${<D}
	install ${<} ${DESTDIR}/usr/include/${<}
