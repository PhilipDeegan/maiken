
KUL_GIT=master
YAML_GIT=master

CWD:=$(CURDIR)
ifeq ($(strip $(CURDIR)),)
  CWD:=$(.CURDIR)
endif

OS =
CXX=g++
CXXFLAGS=-std=c++17 -Os -Wall -fmessage-length=0 -fPIC -g3
INCS =  -Iinc \
		-Iext/parse/yaml/$(YAML_GIT)/p/include \
		-Iext/mkn/kul/$(KUL_GIT)/inc \
		-Iext/mkn/kul/$(KUL_GIT)/os/$(OS)/inc \
		-Iext/mkn/kul/$(KUL_GIT)/os/nixish/inc

YAML = ext/parse/yaml/$(YAML_GIT)/p/bin/libyaml.a
LDFLAGS = -pthread -ldl

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

	@if [ ! -d "./ext/mkn/kul/$(KUL_GIT)" ]; then \
		git clone --depth 1 https://github.com/mkn/mkn.kul.git --branch $(KUL_GIT) ext/mkn/kul/$(KUL_GIT); \
	fi;

	@if [ ! -d "ext/parse/yaml/$(YAML_GIT)" ]; then \
		git clone --depth 1 https://github.com/mkn/parse.yaml.git --branch $(YAML_GIT) ext/parse/yaml/$(YAML_GIT); \
		cd ext/parse/yaml/$(YAML_GIT); \
		./mkn.sh;  \
		cd ../../..; \
	fi;
	@if [ ! -d "ext/parse/yaml/$(YAML_GIT)/p/bin" ]; then \
		mkdir ext/parse/yaml/$(YAML_GIT)/p/bin; \
	fi;
	@if [ ! -f "ext/parse/yaml/$(YAML_GIT)/p/bin/libyaml.a" ]; then \
		$(MAKE) caml; \
		$(MAKE) yaml; \
	fi;

	@mkdir -p bin;

	@for f in $(shell find src -type f -name '*.cpp'); do \
		echo $(CXX) $(CXXFLAGS) $(INCS) -o "bin/$$(basename $$f).o" -c "$$f"; \
		$(CXX) $(CXXFLAGS) $(INCS) -o "bin/$$(basename $$f).o" -c "$$f" || exit 1 ; \
	done;
	@$(CXX) $(CXXFLAGS) $(INCS) -o "bin/mkn.cpp.o" -c "mkn.cpp"
	$(MAKE) link

link:
	$(eval FILES := $(foreach dir,$(shell find bin -type f -name *.o),$(dir)))
	$(CXX) -o "$(EXE)" $(FILES) $(YAML) $(LDFLAGS)

caml:
	@for f in $(shell find ext/parse/yaml/$(YAML_GIT)/p/src -type f -name '*.cpp'); do \
		echo $(CXX) $(CXXFLAGS) -Iext/parse/yaml/$(YAML_GIT)/p/include -o "ext/parse/yaml/$(YAML_GIT)/p/bin/$$(basename $$f).o" -c "$$f"; \
		$(CXX) $(CXXFLAGS) -Iext/parse/yaml/$(YAML_GIT)/p/include -o "ext/parse/yaml/$(YAML_GIT)/p/bin/$$(basename $$f).o" -c "$$f" || exit 1 ; \
	done;

yaml:
	$(eval FILES := $(foreach dir,$(shell find ext/parse/yaml/$(YAML_GIT)/p/bin -type f -name *.o),$(dir)))
	ar -r ext/parse/yaml/$(YAML_GIT)/p/bin/libyaml.a $(FILES)

clean:
	rm -rf bin ext
