
KUL_GIT=master
YAML_GIT=stable
HASH_GIT=stable

CWD:=$(CURDIR)
ifeq ($(strip $(CURDIR)),)
  CWD:=$(.CURDIR)
endif

OS =
CXX=g++ 
CXXFLAGS=-std=c++14 -Os -Wall -fmessage-length=0 
INCS =  -Iinc \
		-Iext/yaml/$(YAML_GIT)/include \
		-Iext/kul/$(KUL_GIT)/inc \
		-Iext/kul/$(KUL_GIT)/os/$(OS)/inc \
		-Iext/kul/$(KUL_GIT)/os/nixish/inc \
		-Iext/sparsehash/$(HASH_GIT)

YAML = ext/yaml/$(YAML_GIT)/bin/libyaml.a
LDFLAGS = -pthread
LINKP= -Wl,--whole-archive -lpthread -Wl,--no-whole-archive

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
		git clone https://github.com/mkn/mkn.kul.git --branch $(KUL_GIT) ext/kul/$(KUL_GIT); \
	fi; 
	@if [ ! -d "./ext/sparsehash/$(HASH_GIT)" ]; then \
		git clone https://github.com/mkn/google.sparsehash.git --branch $(HASH_GIT) ext/sparsehash/$(HASH_GIT); \
	fi;

	@if [ ! -d "ext/yaml/$(YAML_GIT)" ]; then \
		git clone https://github.com/mkn/parse.yaml.git --branch $(YAML_GIT) ext/yaml/$(YAML_GIT); \
	fi;
	@if [ ! -d "ext/yaml/$(YAML_GIT)/bin" ]; then \
		mkdir ext/yaml/$(YAML_GIT)/bin; \
	fi;
	@if [ ! -f "ext/yaml/$(YAML_GIT)/bin/libyaml.a" ]; then \
		$(MAKE) caml; \
		$(MAKE) yaml; \
	fi;

	@if [ ! -d "bin" ]; then \
		mkdir -p bin; \
	fi;
	@for f in $(shell find src -type f -name '*.cpp'); do \
		echo $(CXX) $(CXXFLAGS) $(INCS) -o "bin/$$(basename $$f).o" -c "$$f"; \
		$(CXX) $(CXXFLAGS) $(INCS) -o "bin/$$(basename $$f).o" -c "$$f" || exit 1 ; \
	done;
	@$(CXX) $(CXXFLAGS) $(INCS) -o "bin/cpp.cpp.o" -c "cpp.cpp"
	$(MAKE) link

link:
	$(eval FILES := $(foreach dir,$(shell find bin -type f -name *.o),$(dir)))
	$(CXX) -o "$(EXE)" $(FILES) $(YAML) $(LDFLAGS) 

caml:
	@for f in $(shell find ext/yaml/$(YAML_GIT)/src -type f -name '*.cpp'); do \
		echo $(CXX) $(CXXFLAGS) -Iext/yaml/$(YAML_GIT)/include -o "ext/yaml/$(YAML_GIT)/bin/$$(basename $$f).o" -c "$$f"; \
		$(CXX) $(CXXFLAGS) -Iext/yaml/$(YAML_GIT)/include -o "ext/yaml/$(YAML_GIT)/bin/$$(basename $$f).o" -c "$$f" || exit 1 ; \
	done;	

yaml:
	$(eval FILES := $(foreach dir,$(shell find ext/yaml/$(YAML_GIT)/bin -type f -name *.o),$(dir)))
	ar -r ext/yaml/$(YAML_GIT)/bin/libyaml.a $(FILES)

clean:
	rm -rf ext/yaml/$(YAML_GIT)/bin	
	rm -rf bin

clean-all:
	rm -rf bin ext
