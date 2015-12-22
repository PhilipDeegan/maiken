
KUL_GIT=master
YAML_GIT=no-boost
HASH_GIT=2.0.3

OS =
CXX=g++ 
CXXFLAGS=-std=c++14 -g3 -g0 -Wall -c -fmessage-length=0 
LDFLAGS=
INCS =  -I$(CURDIR)/inc \
		-I$(CURDIR)/ext/yaml/$(YAML_GIT)/include \
		-I$(CURDIR)/ext/kul/$(KUL_GIT)/inc \
		-I$(CURDIR)/ext/kul/$(KUL_GIT)/os/$(OS)/inc \
		-I$(CURDIR)/ext/kul/$(KUL_GIT)/os/nixish/inc \
		-I$(CURDIR)/ext/kul/$(KUL_GIT)/ext/sparsehash/$(HASH_GIT)/include

LIBS = -lyaml
LINK = -pthread -static
LINKP= -Wl,--whole-archive -lpthread -Wl,--no-whole-archive

PATHS = -L$(CURDIR)/ext/kul/$(KUL_GIT)/bin \
		-L$(CURDIR)/ext/yaml/$(YAML_GIT)/bin
EXE=mkn
WHICH=which

entry:
	@@echo "Options include"
	@@echo "make nix"
	@@echo "make bsd"
	@@echo "make clean 	- removes bin dirs of kul/maiken"
	@@echo "make clean-all 	- removes bin dirs of kul/maiken/yaml"

nix:
	@@echo "Making for nix"
	$(eval CXXFLAGS := $(CXXFLAGS))
	$(eval OS := nix)
	$(MAKE) general OS=$(OS)
	@@rm -rf ext bin
	
bsd:
	@@echo "Making for bsd"
	$(eval CXXFLAGS := $(CXXFLAGS))
	$(eval OS := bsd)
	$(MAKE) general OS=$(OS)
	@@rm -rf ext bin

prechecks:
	@if [ -z "$$($(WHICH) git)" ]; then echo "git NOT FOUND - EXITING"; exit 1; fi

general:
	$(MAKE) prechecks

	@if [ ! -d "./ext/kul/$(KUL_GIT)" ]; then \
		git clone http://github.com/mkn/mkn.kul.git --branch $(KUL_GIT) ext/kul/$(KUL_GIT); \
	fi; 
	$(MAKE) -C ./ext/kul/$(KUL_GIT)

	@if [ ! -d "ext/yaml/$(YAML_GIT)" ]; then \
		git clone https://github.com/mkn/parse.yaml.git --branch $(YAML_GIT) ext/yaml/$(YAML_GIT); \
	fi;
	@if [ ! -d "ext/yaml/$(YAML_GIT)/bin" ]; then \
		mkdir ext/yaml/$(YAML_GIT)/bin; \
	fi;
	$(MAKE) caml
	$(MAKE) yaml

	@if [ ! -d "$(CURDIR)/bin" ]; then \
		mkdir -p $(CURDIR)/bin; \
	fi;
	@for f in $(shell find src -type f -name '*.cpp'); do \
		echo $(CXX) $(CXXFLAGS) $(INCS) -o "$(CURDIR)/bin/$$(basename $$f).o" -c "$(CURDIR)/$$f"; \
		$(CXX) $(CXXFLAGS) $(INCS) -o "$(CURDIR)/bin/$$(basename $$f).o" -c "$(CURDIR)/$$f"; \
	done;
	@$(CXX) $(CXXFLAGS) $(INCS) -o "$(CURDIR)/bin/rel.cpp.o" -c "$(CURDIR)/dbg.cpp"
	$(MAKE) link

link:
	$(eval FILES := $(foreach dir,$(shell find $(CURDIR)/bin -type f -name *.o),$(dir)))
	$(CXX) $(PATHS) -o "$(EXE)" $(FILES) $(LIBS) $(LINK) $(LDFLAGS) 

caml:
	@for f in $(shell find ext/yaml/$(YAML_GIT)/src -type f -name '*.cpp'); do \
		echo $(CXX) $(CXXFLAGS) -Iext/yaml/$(YAML_GIT)/include -o "ext/yaml/$(YAML_GIT)/bin/$$(basename $$f).o" -c "$(CURDIR)/$$f"; \
		$(CXX) $(CXXFLAGS) -Iext/yaml/$(YAML_GIT)/include -o "ext/yaml/$(YAML_GIT)/bin/$$(basename $$f).o" -c "$(CURDIR)/$$f"; \
	done;	

yaml:
	$(eval FILES := $(foreach dir,$(shell find $(CURDIR)/ext/yaml/$(YAML_GIT)/bin -type f -name *.o),$(dir)))
	ar -r $(CURDIR)/ext/yaml/$(YAML_GIT)/bin/libyaml.a $(FILES)

clean:
	@if [ -d "./ext/kul/$(KUL_GIT)" ]; then \
		$(MAKE) -C ./ext/kul/$(KUL_GIT) clean; \
	fi;
	rm -rf ext/yaml/$(YAML_GIT)/bin	
	rm -rf bin

clean-all: clean
	@if [ -d "./ext/kul/$(KUL_GIT)" ]; then \
		$(MAKE) -C ./ext/kul/$(KUL_GIT) clean-all; \
	fi;
