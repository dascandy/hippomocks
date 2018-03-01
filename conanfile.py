#!/usr/bin/env python

from conans import ConanFile

class HippomocksConan(ConanFile):
    name = "hippomocks-cpp11"
    version = "5.0"
    license = "LGPLv2"
    url = "https://github.com/dascandy/hippomocks"
    description = "C++ mocking library"
    exports_sources = "*"

    def package(self):
        # If the CMakeLists.txt has a proper install method, the steps below may be redundant
        # If so, you can replace all the steps below with the word "pass"
        self.copy(pattern="LICENSE", dst="license")
        self.copy(pattern="hippomocks.h", dst="include/HippoMocks", src="SingleInclude")

    def package_id(self):
        self.info.header_only()
