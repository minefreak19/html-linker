WCC = x86_64-w64-mingw32-gcc
CC = gcc
CPP = g++
CFLAGS = -std=c11 -g -pedantic -Werror -Wall

c_source_files = $(wildcard src/*.c)
cpp_source_files = $(wildcard src/*.cpp)

all: windows linux

windows: bin/htmll.exe
linux: ./run

# actual version i'm using, for windows
bin/htmll.exe: $(c_source_files)
	$(WCC) $(CFLAGS) -o $@ $^

# for linux testing
./run: $(c_source_files)
	$(CC) $(CFLAGS) -o $@ $^
