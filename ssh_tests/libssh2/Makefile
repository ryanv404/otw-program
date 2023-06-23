SHELL := /bin/sh

CC := gcc

BUILDDIR  := build
DEPDIR    := $(BUILDDIR)/.deps
DATADIR   := data

SRCEXT    := c
SOURCES   := libssh2_client.c
OBJECTS   := $(patsubst %,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))
DEPFILES  := $(patsubst %,$(DEPDIR)/%,$(SOURCES:.$(SRCEXT)=.d))
TARGET    := ssh2client

CFLAGS    := -Wall -Wextra -pedantic -Werror -g
DEPFLAGS   = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.d
INC       := -I. -I../../include
LIB       := -lssh2

$(TARGET): $(OBJECTS)
	$(CC) $^ -o $(TARGET) $(LIB)

$(BUILDDIR)/%.o: %.c $(DEPDIR)/%.d | $(DEPDIR)
	$(CC) $(CFLAGS) $(DEPFLAGS) $(INC) -c -o $@ $<

$(DEPDIR):
	@mkdir -p $(DATADIR)
	@mkdir -p $@

$(DEPFILES):

include $(wildcard $(DEPFILES))

.PHONY: clean
clean:
	rm -f -r $(BUILDDIR) $(TARGET) $(DATADIR)
