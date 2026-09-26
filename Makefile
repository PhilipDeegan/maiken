
CWD:=$(CURDIR)
ifeq ($(strip $(CURDIR)),)
  CWD:=$(.CURDIR)
endif

OS =
CXX=g++
CXXFLAGS=-std=c++20 -O0 -Wall -fmessage-length=0 -fPIC -g3 -fno-omit-frame-pointer
INC=-Iinc \
		-Iext/parse/yaml/p/include \
		-Iext/mkn/kul/inc \
		-Iext/mkn/kul/os/$(OS)/inc \
		-Iext/mkn/kul/os/nixish/inc
LDFLAGS = -pthread -rdynamic -ldl
NIX_LDFLAGS = -pthread -rdynamic -Wl,--no-as-needed -ldl
ifeq ($(origin LDFLAGS),command line)
  NIX_LDFLAGS = $(LDFLAGS)
endif

entry:
	@@echo "Options include"
	@@echo "make nix"
	@@echo "make bsd"

nix:
	$(MAKE) build OS=nix LDFLAGS="$(NIX_LDFLAGS)"

bsd:
	$(MAKE) build OS=bsd

build:
	$(CXX) $(LDFLAGS) $(CXXFLAGS) $(INC) -o mkn mkn.cpp $(shell find src ext/parse/yaml/p/src -type f -name '*.cpp')
