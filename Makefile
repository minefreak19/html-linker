WCC = x86_64-w64-mingw32-gcc
CC = gcc
# CFLAGS = -std=c11 -g -pedantic -Werror -Wall -D_DEBUG

BINDIR = bin

SRCDIR = src
INCDIR = include

INCDIRS = $(sort $(dir $(wildcard $(INCDIR)/**/)))

c_source_files = $(wildcard $(SRCDIR)/*.c) $(wildcard $(SRCDIR)/**/*.c)
include_files = $(wildcard $(INCDIR)/*.h) $(wildcard $(INCDIR)/**/*.h)

INCARGS = $(foreach incdir,$(INCDIRS),$(addprefix -I,$(incdir)))

.PHONY: all run clean

run: all
	./bin/htmll test/src/index.html -o test/out/index.html

all: $(BINDIR)/htmll $(include_files) # recompile if include files change

$(BINDIR)/%: $(c_source_files)
	$(CC) $(CFLAGS) -o $@ $^ $(INCARGS)

clean: 
	rm $(wildcard $(BINDIR)/*)
