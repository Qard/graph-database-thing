CXX_FLAGS = -std=c++14 -luv -Iinclude
CXX_FILES = src/guid.cpp src/main.cpp

ifeq ($(OS),Windows_NT)
    CXX_FLAGS += -D WIN32 -DGUID_WINDOWS
    ifeq ($(PROCESSOR_ARCHITEW6432),AMD64)
        CXX_FLAGS += -D AMD64
    else
        ifeq ($(PROCESSOR_ARCHITECTURE),AMD64)
            CXX_FLAGS += -D AMD64
        endif
        ifeq ($(PROCESSOR_ARCHITECTURE),x86)
            CXX_FLAGS += -D IA32
        endif
    endif
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        CXX_FLAGS += -D LINUX -DGUID_LIBUUID -luuid
    endif
    ifeq ($(UNAME_S),Darwin)
        CXX_FLAGS += -D OSX -DGUID_CFUUID -framework CoreFoundation -lleveldb -lmsgpackc
    endif
    UNAME_P := $(shell uname -p)
    ifeq ($(UNAME_P),x86_64)
        CXX_FLAGS += -D AMD64
    endif
    ifneq ($(filter %86,$(UNAME_P)),)
        CXX_FLAGS += -D IA32
    endif
    ifneq ($(filter arm%,$(UNAME_P)),)
        CXX_FLAGS += -D ARM
    endif
endif

.PHONY: all clean

all: bin/graph

clean:
	rm -rf bin
	# rm -rf graphdb

bin:
	mkdir bin

bin/graph: bin
	g++ $(CXX_FLAGS) $(CXX_FILES) -o bin/graph

run: clean all
	./bin/graph
