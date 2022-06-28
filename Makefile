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
#                                                                           #
#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#


#######  ~~~   Initial variable setup   ~~~  #######

NAME := Luz
SRCS_DIR := ./srcs
OBJS_DIR := ./objs
SRCS :=	AABB.cpp \
		Atmosphere.cpp \
		Camera.cpp \
		Charts/Bar.cpp \
		Charts/Charts.cpp \
		Clock.cpp \
		Color.cpp \
		ExitError.cpp \
		Font.cpp \
		Hittables/BVHNode.cpp \
		Hittables/Cloud.cpp \
		Hittables/ConstantVolume.cpp \
		Hittables/Cube.cpp \
		Hittables/Hittable.cpp \
		Hittables/Landscape.cpp \
		Hittables/Mesh.cpp \
		Hittables/PerlinSphere.cpp \
		Hittables/Plane.cpp \
		Hittables/Procedural.cpp \
		Hittables/Rectangle.cpp \
		Hittables/Sphere.cpp \
		Hittables/Triangle.cpp \
		Hittables/WaterBody.cpp \
		ImageFiles/BMP.cpp \
		ImageFiles/TIFF.cpp \
		Image.cpp \
		Materials/Dielectric.cpp \
		Materials/Emissive.cpp \
		Materials/Isotropic.cpp \
		Materials/Lambertian.cpp \
		Materials/Material.cpp \
		Materials/Metal.cpp \
		Noise/Perlin.cpp \
		OBJReader.cpp \
		ONB.cpp \
		PDFs/CosinePDF.cpp \
		PDFs/HittablePDF.cpp \
		PDFs/MixturePDF.cpp \
		PDFs/SpherePDF.cpp \
		Random.cpp \
		Ray/Ray.cpp \
		Renderer/Atmospherics.cpp \
		Renderer/ColorHelper.cpp \
		Renderer/HitHelper.cpp \
		Renderer/RayHelper.cpp \
		Renderer/Renderer.cpp \
		Renderer/SequenceRenderer.cpp \
		Renderer/Threads.cpp \
		Scene.cpp \
		SceneFile/Materials.cpp \
		SceneFile/Objects.cpp \
		SceneFile/Scene.cpp \
		SceneFile/SceneFile.cpp \
		SceneFile/Settings.cpp \
		Transform.cpp \
		Utilities.cpp \
		Vector3.cpp \
		main.cpp
OBJS := $(patsubst %.cpp, $(OBJS_DIR)/%.o, $(SRCS))
DPND := $(OBJS:.o=.d)

INCLUDES := -Iincludes
GENERAL_FLAGS := -std=c++2a
WWW_FLAGS := -Wall -Wextra -Werror
OPT_FLAGS := -Ofast -ffast-math # ARM: -ffp-model=fast # X86: -ffast-math
INC_FLAGS := -MD

TMP_FILE := Makefile.tmp
SHELL := /bin/bash


####################################################


#######  ~~~   Compilation setup   ~~~  #######

# Ensures that TMP_FILE exists before calling awk
$(shell touch $(TMP_FILE))

# Reads DEBUG option from TMP_FILE if user hasn't provided it
ifndef DEBUG
	ifeq ($(shell awk 'NR==1 {print $$3}' $(TMP_FILE)),1)
		DEBUG = 1
	else
		DEBUG = 0
	endif
else
	PRE_EXECUTE = clean
endif

# Reads SANITIZER option from TMP_FILE if user hasn't provided it
ifndef SANITIZER
	ifeq ($(shell awk 'NR==2 {print $$3}' $(TMP_FILE)),1)
		SANITIZER = 1
	else
		SANITIZER = 0
	endif
else
	PRE_EXECUTE = clean
endif

# Reads NO_FLAGS option from TMP_FILE if user hasn't provided it
ifndef NO_FLAGS
	ifeq ($(shell awk 'NR==3 {print $$3}' $(TMP_FILE)),1)
		NO_FLAGS = 1
	else
		NO_FLAGS = 0
	endif
else
	PRE_EXECUTE = clean
endif

# Reads COMPILER option from TMP_FILE if user hasn't provided it
# x86_64-w64-mingw32-g++ => 64bit
ifndef COMPILER
	ifeq ($(shell awk 'NR==4 {print $$3}' $(TMP_FILE)),1)
		COMPILER = 1
	else
		COMPILER = 0
	endif
else
	PRE_EXECUTE = clean
endif

# Reads FAST_COMPILATION option from TMP_FILE if user hasn't provided it
ifndef FAST_COMPILATION
	ifeq ($(shell awk 'NR==5 {print $$3}' $(TMP_FILE)),1)
		FAST_COMPILATION = 1
	else
		FAST_COMPILATION = 0
	endif
else
	PRE_EXECUTE = clean
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

# Configure COMPILER
ifeq ($(COMPILER),1)
	_COMPILER = i686-w64-mingw32-g++
	COMPILER_FLAGS = -U__STRICT_ANSI__ -static-libgcc -static-libstdc++
else
	_COMPILER = clang++
endif

# Configure FAST_COMPILATION
ifeq ($(FAST_COMPILATION),1)
	OPT_FLAGS =
endif

$(shell echo -e -n "DEBUG = $(DEBUG)\nSANITIZER = $(SANITIZER)\nNO_FLAGS = $(NO_FLAGS)\nCOMPILER = $(COMPILER)\nFAST_COMPILATION = $(FAST_COMPILATION)" > $(TMP_FILE))

ifeq ($(shell uname -s),Linux)
	DEBUGGER = gdb
else ifeq ($(shell uname -s),Darwin)
	DEBUGGER = lldb
endif


###############################################


#######  ~~~   Rules and commands setup   ~~~  #######

.PHONY: all
all: $(PRE_EXECUTE)
	@printf "[\e[1;34mPreparing objects\e[0m]\n\n"
	@$(MAKE) $(NAME) --no-print-directory

$(OBJS_DIR)/%.o: $(SRCS_DIR)/%.cpp
	$(shell [ ! -d $(@D) ] && mkdir -p $(@D))
	$(_COMPILER) $(COMPILER_FLAGS) $(WWW_FLAGS) $(GENERAL_FLAGS) $(OPT_FLAGS) $(INC_FLAGS) $(DEBUG_FLAGS) $(INCLUDES) -DOS=$(COMPILER) -c $< -o $@

$(NAME): $(OBJS)
	@printf "\n[\e[1;34mCompiling $(NAME)\e[0m]\n\n"
	$(_COMPILER) $(COMPILER_FLAGS) $(WWW_FLAGS) $(GENERAL_FLAGS) $(OPT_FLAGS) $(INC_FLAGS) $(DEBUG_FLAGS) $(INCLUDES) $(OBJS) -DOS=$(COMPILER) -o $(NAME)

	@printf "\n[\e[0;32mCompilation done. $(NAME) ready.\e[0m]\n"

.PHONY: clean
clean:
	@printf "[\e[1;33mCleaning\e[0m]\n\n"
	rm -f $(OBJS)
	rm -f $(DPND)
	@$(shell if [[ "$(shell find $(OBJS_DIR) -type f | wc -l)" -eq "0" ]]; then rm -rf $(OBJS_DIR); fi;)

.PHONY: fclean
fclean: clean
	rm -f $(NAME)
	rm -f $(TMP_FILE)

.PHONY: re
re:
	@$(MAKE) fclean --no-print-directory
	@printf "\n"
	@$(MAKE) all --no-print-directory

.PHONY: noflags
noflags:
	@$(MAKE) all NO_FLAGS=1 --no-print-directory

.PHONY: debugger
debugger:
	@$(MAKE) all DEBUG=1 --no-print-directory
	@printf "\n[\e[1;34mStarting $(DEBUGGER)\e[0m]\n\n"
	$(DEBUGGER) ./$(NAME)

.PHONY: debug
debug:
	@$(MAKE) all DEBUG=1 --no-print-directory

.PHONY: sanitizer
sanitizer:
	@$(MAKE) all SANITIZER=1 --no-print-directory

.PHONY: windows
windows:
	@$(MAKE) all COMPILER=1 --no-print-directory

.PHONY: unix
unix:
	@$(MAKE) all COMPILER=0 --no-print-directory

.PHONY: fast
fast:
	@$(MAKE) all FAST_COMPILATION=1 --no-print-directory

.PHONY: test
test:
	@printf "\n[\e[1;34mStarting tests\e[0m]\n\n"
	@docker build -t luz-test -f ./tests/Dockerfile .
	# @docker run --interactive --tty --entrypoint /bin/sh luz-test
	@docker run --memory=1g --memory-swap=1g --kernel-memory=1g --memory-swappiness=0 --cpus=2 luz-test
	# 1gb ram, no swap, 2 cores
	# set the same seed for the random generator

-include $(DPND)


######################################################