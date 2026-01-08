# compiler and compiler flags
CC := gcc
CFLAGS := -g -O0 -Wall -Wextra -std=c17

# project dirs
SRC_DIR   := src
BUILD_DIR := build
BIN_DIR   := bin

# installation paths
PREFIX ?= /usr/local
BINDIR  := $(PREFIX)/bin

# derive include directories (-I) from header locations in src/
INCLUDES := $(shell find $(SRC_DIR) -type d | sort -u)
CFLAGS   += $(addprefix -I,$(INCLUDES))

# final executables (built into bin/)
CLIENT_TARGET := $(BIN_DIR)/lit
SERVER_TARGET := $(BIN_DIR)/lit-serv

# recursively find all .c files in src/ with exclusions
CLIENT_SRCS := $(shell find $(SRC_DIR) -name '*.c' ! -path '$(SRC_DIR)/server/*')
SERVER_SRCS := $(shell find $(SRC_DIR) -name '*.c' ! -path '$(SRC_DIR)/client/*')

# generate .o file names in build/ directory mirroring src/
CLIENT_OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/client/%.o,$(CLIENT_SRCS))
SERVER_OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/server/%.o,$(SERVER_SRCS))


# default target
.PHONY: all
all: client server

# build client and server targets
.PHONY: client server
client: $(CLIENT_TARGET)
server: $(SERVER_TARGET)

# link objects into final binaries
$(CLIENT_TARGET): $(CLIENT_OBJS)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $^ -o $@

$(SERVER_TARGET): $(SERVER_OBJS)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $^ -o $@


# compile source files into object files (two object trees)
$(BUILD_DIR)/client/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/server/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@


# debug target (build both with debug flags)
.PHONY: debug
debug: CFLAGS += -O0 -g
debug: all


# install / uninstall (installs both)
.PHONY: install
install: all
	@echo "installing $(notdir $(CLIENT_TARGET)) and $(notdir $(SERVER_TARGET)) to $(BINDIR)"
	@mkdir -p $(BINDIR)
	install -m 755 $(CLIENT_TARGET) $(BINDIR)/$(notdir $(CLIENT_TARGET))
	install -m 755 $(SERVER_TARGET) $(BINDIR)/$(notdir $(SERVER_TARGET))
	@echo "installation complete."

.PHONY: uninstall
uninstall:
	@echo "removing $(notdir $(CLIENT_TARGET)) and $(notdir $(SERVER_TARGET)) from $(BINDIR)"
	rm -f $(BINDIR)/$(notdir $(CLIENT_TARGET))
	rm -f $(BINDIR)/$(notdir $(SERVER_TARGET))
	@echo "uninstall complete."


# clean target
.PHONY: clean
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)
