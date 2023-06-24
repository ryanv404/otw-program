SHELL := /bin/sh

CC := gcc

DATADIR   := data
SRCDIR    := src
BUILDDIR  := build
DEPDIR    := $(BUILDDIR)/.deps
BINDIR    := bin

SRCEXT    := c
SOURCES   := $(shell find $(SRCDIR) -type f -name "*.$(SRCEXT)")
OBJECTS   := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))
DEPFILES  := $(patsubst $(SRCDIR)/%,$(DEPDIR)/%,$(SOURCES:.$(SRCEXT)=.d))
DATAFILES := otw_data.dat otw_knownhosts
DATAFILES := $(patsubst %,$(DATADIR)/%,$(DATAFILES))
TARGET    := $(BINDIR)/otw1

CFLAGS    := -Wall -Wextra -pedantic -Werror -g
DEPFLAGS1  = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.d
INC       := -Iinclude
LIB       := -lssh

$(TARGET): $(OBJECTS) | $(BINDIR) $(DATADIR)
	$(CC) $^ -o $(TARGET) $(LIB)

$(BINDIR):
	mkdir -p $(BINDIR)

$(DATADIR):
	mkdir -p $(DATADIR)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c $(DEPDIR)/%.d | $(DEPDIR)
	$(CC) $(CFLAGS) $(DEPFLAGS) $(INC) -c -o $@ $<

$(DEPDIR):
	mkdir -p $@

$(DEPFILES):

include $(wildcard $(DEPFILES))

.PHONY: clean cleandata
clean:
	rm -f -r $(BUILDDIR) $(BINDIR) $(DATADIR)

cleandata:
	rm -f $(DATAFILES)
