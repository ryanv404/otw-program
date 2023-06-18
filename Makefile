CC := gcc

DATADIR   := data
SRCDIR    := src
BUILDDIR  := build
TARGETDIR := bin

SRCEXT  := c
DATAEXT := dat
DATA    := $(shell find $(DATADIR) -type f -name "*.$(DATAEXT)")
TARGET  := $(TARGETDIR)/otw
SOURCES := $(shell find $(SRCDIR) -type f -name "*.$(SRCEXT)")
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))

CFLAGS := -Wall -Wextra -pedantic -Werror -g -save-temps
INC    := -Iinclude
LIB    := -lssh2

$(TARGET): $(OBJECTS)
	@echo -n "[+] Linking $@: "
	@mkdir -p $(DATADIR)
	@mkdir -p $(TARGETDIR)
	@echo "$(CC) $^ -o $(TARGET) $(LIB)"; $(CC) $^ -o $(TARGET) $(LIB)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	@echo -n "[+] Compiling $@: "
	@mkdir -p $(BUILDDIR)
	@echo "$(CC) $(CFLAGS) $(INC) -c -o $@ $<"; $(CC) $(CFLAGS) $(INC) -c -o $@ $<

.PHONY: clean
clean:
	@echo "[+] Cleaning up..."
	@echo "$(RM) -r $(BUILDDIR) $(TARGETDIR) $(DATA)"; $(RM) -r $(BUILDDIR) $(TARGETDIR) $(DATA)
