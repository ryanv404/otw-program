SHELL := /bin/sh

CC := gcc

BUILDDIR  := build
DEPDIR    := $(BUILDDIR)/.deps

SRCEXT    := c
SOURCES   := authentication.c knownhosts.c connect_ssh.c libssh_client.c
OBJECTS   := $(patsubst %,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))
DEPFILES  := $(patsubst %,$(DEPDIR)/%,$(SOURCES:.$(SRCEXT)=.d))
TARGET    := sshclient

CFLAGS    := -Wall -Wextra -pedantic -Werror -g
DEPFLAGS   = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.d
INC       := -I. -I../../include
LIB       := -lssh

$(TARGET): $(OBJECTS)
	$(CC) $^ -o $(TARGET) $(LIB)

$(BUILDDIR)/%.o: %.c $(DEPDIR)/%.d | $(DEPDIR)
	$(CC) $(CFLAGS) $(DEPFLAGS) $(INC) -c -o $@ $<

$(DEPDIR):
	mkdir -p $@

$(DEPFILES):

include $(wildcard $(DEPFILES))

.PHONY: clean
clean:
	rm -f -r $(BUILDDIR) $(TARGET)
