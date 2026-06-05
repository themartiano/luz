#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#
#                                                                           #
#   Makefile                                                                #
#    by Brhaka                                                              #
#                                                                           #
#    Options: (make OPTION)                                                 #
#     debug -> Compiles with "-g" flag                                      #
#     debugger -> Compiles with "-g" flag and opens LLDB / GDB              #
#     sanitizer -> Compiles with "-g" and "-fsanitize=address" flags        #
#     noflags -> Compiles without "-Wall", "-Wextra" and "-Werror" flags    #
#     release-native -> Compiles with native CPU tuning and LTO             #
#                                                                           #
#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#


#######  ~~~   Initial variable setup   ~~~  #######

NAME := Luz
TEST_NAME := LuzTests
SRCS_DIR := ./src
OBJS_DIR := ./objs
MAIN_SRC := $(SRCS_DIR)/cli/main.cpp
LIB_SRCS := $(shell find $(SRCS_DIR) -name '*.cpp' ! -path '$(MAIN_SRC)' | sort)
SRCS := $(LIB_SRCS) $(MAIN_SRC)
TEST_SRCS := $(LIB_SRCS) tests/Tests.cpp
OBJS := $(patsubst $(SRCS_DIR)/%.cpp, $(OBJS_DIR)/%.o, $(SRCS))
DPND := $(OBJS:.o=.d)

COMPILER = clang++
INCLUDES := -Iinclude/luz
GENERAL_FLAGS := -std=c++2a
WWW_FLAGS := -Wall -Wextra -Werror
OPT_FLAGS = -O3
INC_FLAGS := -MD
LINK_FLAGS =
SHELL := /bin/bash
DEBUG ?= 0
SANITIZER ?= 0
NO_FLAGS ?= 0
FAST_COMPILATION ?= 0
WINDOWS ?= 0
NATIVE ?= 1
LTO ?= 1
BENCH_IMAGE ?= luz-benchmark-image
BENCH_CPUS ?= 1
BENCH_CPUSET ?=
BENCH_MEMORY ?= 1g
BENCH_REPEAT ?= 20
BENCH_WARMUP ?= 2
BENCH_THREADS ?= 1
BENCH_SEED ?= 424242424
BENCH_CASES ?= default many-objects mesh-bvh diffuse postprocess atmosphere
BENCH_WIDTH ?=
BENCH_HEIGHT ?=
BENCH_SAMPLES ?=
BENCH_BOUNCES ?=
BENCH_GAMMA ?=
BENCH_TONEMAPPING ?=
BENCH_BLOOM ?=
BENCH_GIT_COMMIT ?= $(shell git rev-parse --short HEAD 2>/dev/null || echo unknown)
BENCH_GIT_STATUS ?= $(shell if [ -n "$$(git status --short 2>/dev/null)" ]; then echo dirty; else echo clean; fi)


####################################################


#######  ~~~   Compilation setup   ~~~  #######

ifeq ($(shell uname -m),x86_64)
	OPT_FLAGS += -ffast-math
else ifeq ($(shell uname -m),i686)
	OPT_FLAGS += -ffast-math
else ifeq ($(shell uname -m),arm64)
	OPT_FLAGS += -ffp-model=fast
endif

# Configure DEBUG
ifeq ($(DEBUG),1)
	DEBUG_FLAGS = -g
	OPT_FLAGS = -Og
endif

# Configure SANITIZER
ifeq ($(SANITIZER),1)
	DEBUG_FLAGS = -g -fsanitize=address
	OPT_FLAGS = -Og
endif

# Configure NO_FLAGS
ifeq ($(NO_FLAGS),1)
	WWW_FLAGS =
endif

# Set flags for Windows compilation
ifeq ($(WINDOWS),1)
	COMPILER = x86_64-w64-mingw32-g++-posix
	COMPILER_FLAGS = -U__STRICT_ANSI__ -static-libgcc -static-libstdc++ -static
endif

ifeq ($(NATIVE),1)
	OPT_FLAGS += -march=native
endif

ifeq ($(LTO),1)
	OPT_FLAGS += -flto
	LINK_FLAGS += -flto
endif

ifneq ($(BENCH_CPUSET),)
	BENCH_CPUSET_FLAG = --cpuset-cpus=$(BENCH_CPUSET)
endif

# Configure FAST_COMPILATION
ifeq ($(FAST_COMPILATION),1)
	OPT_FLAGS =
endif

ifeq ($(shell uname -s),Linux)
	DEBUGGER = gdb
else ifeq ($(shell uname -s),Darwin)
	DEBUGGER = lldb
endif

###############################################


#######  ~~~   Rules and commands setup   ~~~  #######

.PHONY: all
all:
	@printf "[\e[1;34mPreparing objects\e[0m]\n\n"
	@$(MAKE) $(NAME) --no-print-directory

$(OBJS_DIR)/%.o: $(SRCS_DIR)/%.cpp
	$(shell [ ! -d $(@D) ] && mkdir -p $(@D))
	$(COMPILER) $(COMPILER_FLAGS) $(WWW_FLAGS) $(GENERAL_FLAGS) $(OPT_FLAGS) $(INC_FLAGS) $(DEBUG_FLAGS) $(INCLUDES) -c $< -o $@

$(NAME): $(OBJS)
	@printf "\n[\e[1;34mCompiling $(NAME)\e[0m]\n\n"
	$(COMPILER) $(COMPILER_FLAGS) $(WWW_FLAGS) $(GENERAL_FLAGS) $(OPT_FLAGS) $(INC_FLAGS) $(DEBUG_FLAGS) $(INCLUDES) $(OBJS) $(LINK_FLAGS) -o $(NAME)

	@printf "\n[\e[0;32mCompilation done. $(NAME) ready.\e[0m]\n"

.PHONY: release
release:
	@$(MAKE) all --no-print-directory

.PHONY: release-native
release-native:
	@$(MAKE) clean --no-print-directory
	@$(MAKE) all NATIVE=1 LTO=1 --no-print-directory

.PHONY: test
test:
	@printf "[\e[1;34mCompiling $(TEST_NAME)\e[0m]\n\n"
	$(COMPILER) $(COMPILER_FLAGS) $(WWW_FLAGS) $(GENERAL_FLAGS) $(OPT_FLAGS) $(DEBUG_FLAGS) $(INCLUDES) $(TEST_SRCS) $(LINK_FLAGS) -o $(TEST_NAME)
	@printf "\n[\e[1;34mRunning tests\e[0m]\n\n"
	./$(TEST_NAME)

.PHONY: clean
clean:
	@printf "[\e[1;33mCleaning\e[0m]\n\n"
	rm -f $(OBJS)
	rm -f $(DPND)
	@if [[ -d "$(OBJS_DIR)" && "$$(find $(OBJS_DIR) -type f | wc -l)" -eq "0" ]]; then rm -rf $(OBJS_DIR); fi;

.PHONY: fclean
fclean: clean
	rm -f $(NAME)
	rm -f $(TEST_NAME)

.PHONY: re
re:
	@$(MAKE) fclean --no-print-directory
	@printf "\n"
	@$(MAKE) all --no-print-directory

.PHONY: noflags
noflags:
	@$(MAKE) clean --no-print-directory
	@$(MAKE) all NO_FLAGS=1 --no-print-directory

.PHONY: debugger
debugger:
	@$(MAKE) clean --no-print-directory
	@$(MAKE) all DEBUG=1 --no-print-directory
	@printf "\n[\e[1;34mStarting $(DEBUGGER)\e[0m]\n\n"
	$(DEBUGGER) ./$(NAME)

.PHONY: debug
debug:
	@$(MAKE) clean --no-print-directory
	@$(MAKE) all DEBUG=1 --no-print-directory

.PHONY: sanitizer
sanitizer:
	@$(MAKE) clean --no-print-directory
	@$(MAKE) all SANITIZER=1 --no-print-directory

.PHONY: windows
windows:
	@$(MAKE) clean --no-print-directory
	@$(MAKE) all WINDOWS=1 --no-print-directory

.PHONY: unix
unix:
	@$(MAKE) all --no-print-directory

.PHONY: fast
fast:
	@$(MAKE) clean --no-print-directory
	@$(MAKE) all FAST_COMPILATION=1 --no-print-directory

.PHONY: benchmark
benchmark:
	@printf "[\e[1;34mBuilding container...\e[0m]\n\n" >&2

	@docker build -t $(BENCH_IMAGE) -f ./docker/benchmark/Dockerfile . >&2

	@printf "\n[\e[1;34mStarting container...\e[0m]\n\n" >&2

	@docker run --rm --network none --cpus=$(BENCH_CPUS) $(BENCH_CPUSET_FLAG) --memory=$(BENCH_MEMORY) \
		-e BENCH_REPEAT=$(BENCH_REPEAT) \
		-e BENCH_WARMUP=$(BENCH_WARMUP) \
		-e BENCH_THREADS=$(BENCH_THREADS) \
		-e BENCH_SEED=$(BENCH_SEED) \
		-e BENCH_CASES="$(BENCH_CASES)" \
		-e BENCH_WIDTH=$(BENCH_WIDTH) \
		-e BENCH_HEIGHT=$(BENCH_HEIGHT) \
		-e BENCH_SAMPLES=$(BENCH_SAMPLES) \
		-e BENCH_BOUNCES=$(BENCH_BOUNCES) \
		-e BENCH_GAMMA="$(BENCH_GAMMA)" \
		-e BENCH_TONEMAPPING="$(BENCH_TONEMAPPING)" \
		-e BENCH_BLOOM="$(BENCH_BLOOM)" \
		-e BENCH_GIT_COMMIT=$(BENCH_GIT_COMMIT) \
		-e BENCH_GIT_STATUS=$(BENCH_GIT_STATUS) \
		-e BENCH_CPUS=$(BENCH_CPUS) \
		$(BENCH_IMAGE)

	@printf "\n[\e[0;32mDone.\e[0m]\n" >&2

.PHONY: benchmark-compare
benchmark-compare:
	@if [ -z "$(BEFORE)" ] || [ -z "$(AFTER)" ]; then \
		echo "Usage: make benchmark-compare BEFORE=before.csv AFTER=after.csv" >&2; \
		exit 1; \
	fi
	@bash benchmarks/compare.sh "$(BEFORE)" "$(AFTER)"

-include $(DPND)


######################################################
