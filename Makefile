CC        := gcc
DATADIR   := data
SRCDIR    := src
BUILDDIR  := build
TARGETDIR := bin
DATA      := $(DATADIR)/otw-data.csv
TARGET    := $(TARGETDIR)/otw
SOURCES   := $(shell find $(SRCDIR) -type f -name "*.c")
SOURCES   := $(filter-out $(SRCDIR)/ssh_connect.c, $(SOURCES))
OBJECTS   := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.c=.o))
CFLAGS    := -Wall -Wextra -Wpedantic -Werror -g
INC       := -Iinclude

$(TARGET): $(OBJECTS)
	@touch $(DATA)
	@mkdir -p $(TARGETDIR)
	@echo "$(CC) $(CFLAGS) $^ -o $(TARGET)"; $(CC) $(CFLAGS) $^ -o $(TARGET)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(BUILDDIR)
	@echo "$(CC) $(CFLAGS) $(INC) -c -o $@ $< -save-temps"; $(CC) $(CFLAGS) $(INC) -c -o $@ $< -save-temps

.PHONY: clean
clean:
	@echo "Cleaning up the build files..."
	@echo "$(RM) -r $(BUILDDIR) $(TARGETDIR) $(DATA)"; $(RM) -r $(BUILDDIR) $(TARGETDIR) $(DATA)
