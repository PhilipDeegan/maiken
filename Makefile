
CWD:=$(CURDIR)
ifeq ($(strip $(CURDIR)),)
  CWD:=$(.CURDIR)
endif

OS =
CXX=g++
CXXFLAGS=-std=c++17 -O0 -Wall -fmessage-length=0 -fPIC -Iinc -g3 -fno-omit-frame-pointer \
		-Iext/parse/yaml/p/include \
		-Iext/mkn/kul/inc \
		-Iext/mkn/kul/os/$(OS)/inc \
		-Iext/mkn/kul/os/nixish/inc
LDFLAGS = -pthread -rdynamic -ldl

entry:
	@@echo "Options include"
	@@echo "make nix"
	@@echo "make bsd"

nix:
	$(MAKE) build OS=nix LDFLAGS="-pthread -rdynamic -Wl,--no-as-needed -ldl"

bsd:
	$(MAKE) build OS=bsd

build:
	$(CXX) $(LDFLAGS) $(CXXFLAGS) -o mkn mkn.cpp $(shell find src ext/parse/yaml/p/src -type f -name '*.cpp')
