CC       := gcc
SRCDIR   := src
BUILDDIR := build
TARGET   := otw
SOURCES  := $(shell find $(SRCDIR) -type f -name "*.c")
SOURCES  := $(filter-out $(SRCDIR)/ssh_connect.c, $(SOURCES))
OBJECTS  := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.c=.o))
CFLAGS   := -Wall -Wextra -Wpedantic -Werror -g
INC      := -Iinclude

$(TARGET): $(OBJECTS)
	@echo " $(CC) $(CFLAGS) $^ -o $(TARGET)"; $(CC) $(CFLAGS) $^ -o $(TARGET)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(BUILDDIR)
	@echo " $(CC) $(CFLAGS) $(INC) -c -o $@ $< -save-temps"; $(CC) $(CFLAGS) $(INC) -c -o $@ $< -save-temps

.PHONY: clean
clean:
	@echo " Cleaning up the build directory and executable..."
	@echo " $(RM) -r $(BUILDDIR) $(TARGET)"; $(RM) -r $(BUILDDIR) $(TARGET)
