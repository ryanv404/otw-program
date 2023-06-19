SHELL := /bin/sh

CC := gcc

DATADIR   := data
SRCDIR    := src
BUILDDIR  := build
BINDIR    := bin
DEPDIR    := $(BUILDDIR)/.deps

SRCEXT    := c
DATAEXT   := dat

DATAFILES := otw_data known_otw_hosts
DATAFILES := $(patsubst %,$(DATADIR)/%.$(DATAEXT),$(DATAFILES))
SOURCES   := $(shell find $(SRCDIR) -type f -name "*.$(SRCEXT)")
OBJECTS   := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))
DEPFILES  := $(patsubst $(SRCDIR)/%,$(DEPDIR)/%,$(SOURCES:.$(SRCEXT)=.d))
TARGET    := $(BINDIR)/otw

CFLAGS   := -Wall -Wextra -pedantic -Werror -g
DEPFLAGS  = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.d
INC      := -Iinclude
LIB      := -lssh

$(TARGET): $(OBJECTS) | $(DATADIR) $(BINDIR)
	$(CC) $^ -o $(TARGET) $(LIB)

$(DATADIR):
	mkdir -p $(DATADIR)

$(BINDIR):
	mkdir -p $(BINDIR)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c $(DEPDIR)/%.d | $(DEPDIR)
	$(CC) $(CFLAGS) $(DEPFLAGS) $(INC) -c -o $@ $<

$(DEPDIR):
	mkdir -p $@

$(DEPFILES):

include $(wildcard $(DEPFILES))

.PHONY: clean
clean:
	rm -f -r $(BUILDDIR) $(BINDIR) $(DATADIR)

.PHONY: cleandata
cleandata:
	rm -f $(DATAFILES)
