# toolchain & flags
CC := gcc
CFLAGS := -Wall -Wextra -std=c17 -g
LDFLAGS :=
LDLIBS  :=

# project root
TOP := $(abspath ../..)

# local build roots
BUILD := build
BIN   := $(BUILD)/bin
OBJ   := $(BUILD)/obj

# sources from src/ (excluding main.c)
SRCS := $(shell find "$(TOP)/src" -name '*.c' ! -name 'main.c')

# tapi source in tests/
TAPI_SRC := $(TOP)/tests/tapi.c

# tests in this folder
TEST_SRCS := $(wildcard *.c)

# include dirs
INCLUDES := $(shell find "$(TOP)/src" -type d | sort -u)
INCLUDES += $(shell find "$(TOP)/tests" -type d | sort -u)
CFLAGS   += $(addprefix -I, $(INCLUDES))

# object files
OBJS      := $(patsubst $(TOP)/src/%.c,$(OBJ)/src/%.o,$(SRCS))
TAPI_OBJ  := $(OBJ)/tests/tapi.o
TEST_OBJS := $(patsubst %.c,$(OBJ)/tests/unit/%.o,$(TEST_SRCS))
TEST_BINS := $(patsubst %.c,$(BIN)/%,$(TEST_SRCS))

# log files
LOGS := $(wildcard *.log)

# default target
all: $(TEST_BINS)

# link each test binary with src objs + tapi.o
$(BIN)/%: $(OBJ)/tests/unit/%.o $(OBJS) $(TAPI_OBJ)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $@

# build src objects
$(OBJ)/src/%.o: $(TOP)/src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# build tapi object
$(OBJ)/tests/tapi.o: $(TAPI_SRC)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# build test objects
$(OBJ)/tests/unit/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# clean
.PHONY: clean
clean:
	rm -rf "$(BUILD)"
	rm -f $(LOGS)

# run tests
.PHONY: run
run: all
	clear
	@set -e; \
	for t in $(TEST_BINS); do "$$t"; done

# don't auto-delete object files
.PRECIOUS: $(OBJS) $(TAPI_OBJ) $(TEST_OBJS)