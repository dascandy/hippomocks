#!/usr/bin/make -f
MAKE?=make
CMAKE?=cmake

-include Makefile


Makefile:
	${CMAKE} --version && ${CMAKE} .
	ls $@ || cp -av build.mk $@
