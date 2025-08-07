# compiler and compiler flags
CC := gcc
CFLAGS := -Wall -Wextra -std=c17

# installation paths
PREFIX ?= /usr/local
BINDIR = $(PREFIX)/bin

# recursively find all .c files in src/
SRCS := $(shell find src/ -name '*.c')

# generate .o file names in a build/ directory mirroring src/
OBJS := $(patsubst src/%.c, build/%.o, $(SRCS))

# derive include directories (-I) from header locations in src/
INCLUDES := $(shell find src -type d | sort -u)
CFLAGS += $(addprefix -I, $(INCLUDES))

# final executable
TARGET := lit

# default target
all: $(TARGET)


# link objects into the final binary
$(TARGET): $(OBJS)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $^ -o $@

# compile source files into object files
build/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# install target
.PHONY: install
install: $(TARGET)
	@echo "installing $(TARGET) to $(BINDIR)"
	@mkdir -p $(BINDIR)
	install -m 755 $(TARGET) $(BINDIR)/$(TARGET)
	@echo "installation complete. you can now run '$(TARGET)' from anywhere."

# uninstall target
.PHONY: uninstall
uninstall:
	@echo "removing $(TARGET) from $(BINDIR)"
	rm -f $(BINDIR)/$(TARGET)
	@echo "uninstall complete."


# clean target
.PHONY: clean
clean:
	rm -rf .lit/
	rm -rf build $(TARGET)
