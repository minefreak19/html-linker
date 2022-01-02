# CFLAGS = -std=c11 -g -Wpedantic -Werror -Wall -Wextra -D_DEBUG -Wno-missing-braces -Wno-format
CFLAGS = -std=c11 -O3

BINDIR = bin

SRCDIR = src
INCDIR = include

INCDIRS = $(sort $(dir $(wildcard $(INCDIR)/**/)))

c_source_files = $(wildcard $(SRCDIR)/*.c) $(wildcard $(SRCDIR)/**/*.c)
include_files = $(wildcard $(INCDIR)/*.h) $(wildcard $(INCDIR)/**/*.h)

INCARGS = $(foreach incdir,$(INCDIRS),$(addprefix -I,$(incdir)))

.PHONY: all clean

# run: all
#	 ./bin/htmll test/src/index.html -o test/out/index.html

all: $(BINDIR)/htmll $(include_files) # recompile if include files change

$(BINDIR)/%: $(c_source_files)
	$(CC) $(CFLAGS) -o $@ $^ $(INCARGS)

clean: 
	rm $(wildcard $(BINDIR)/*)
