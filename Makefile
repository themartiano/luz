#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#
#                                                                           #
#   Makefile                                                                #
#                                                                           #
#    Options: (make FLAG=1/0)                                               #
#     DEBUG -> Compiles with "-g" flag                                      #
#     SANITIZER -> Compiles with "-g" and "-fsanitize=address" flags        #
#     NO_FLAGS -> Compiles without "-Wall", "-Wextra" and "-Werror" flags   #
#                                                                           #
#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#


#######  ~~~   Initial variable setup   ~~~  #######

NAME := Luz
SRCS_DIR := ./srcs
OBJS_DIR := ./objs
SRCS :=	./srcs/Camera.cpp ./srcs/ExitError.cpp ./srcs/main.cpp ./srcs/Scene.cpp ./srcs/Vector3.cpp \
		./srcs/BMP.cpp ./srcs/Renderer/Renderer.cpp ./srcs/Ray.cpp ./srcs/Utilities.cpp ./srcs/AABB.cpp \
		./srcs/Color.cpp ./srcs/Material.cpp ./srcs/Transform.cpp ./srcs/Renderer/SequenceRenderer.cpp \
		./srcs/Forms/Sphere.cpp ./srcs/Clock.cpp ./srcs/BVHNode.cpp ./srcs/Forms/Rectangle.cpp \
		./srcs/Forms/Plane.cpp ./srcs/Atmosphere.cpp ./srcs/Forms/Triangle.cpp ./srcs/OBJReader.cpp \
		./srcs/Forms/Cube.cpp ./srcs/SceneFile.cpp ./srcs/ONB.cpp ./srcs/PDFs/CosinePDF.cpp \
		./srcs/PDFs/HittablePDF.cpp ./srcs/Hittable.cpp ./srcs/PDFs/MixturePDF.cpp ./srcs/Renderer/HitHelper.cpp \
		./srcs/Renderer/Atmospherics.cpp ./srcs/Renderer/Bouncer.cpp ./srcs/Renderer/Threads.cpp \
		./srcs/Renderer/ColorHelper.cpp
OBJS := $(patsubst $(SRCS_DIR)/%.cpp, $(OBJS_DIR)/%.o, $(SRCS))
DPND := $(OBJS:.o=.d)
INCLUDES := -Iincludes
GENERAL_FLAGS := -std=c++17 -pthread
WWW_FLAGS := -Wall -Wextra -Werror -pedantic-errors
OPT_FLAGS := -O3
INC_FLAGS := -MD
TMP_FILE := Makefile.tmp

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

# Configure DEBUG
ifeq ($(DEBUG),1)
	DEBUG_FLAGS = -g
	OPT_FLAGS =
endif

# Configure SANITIZER
ifeq ($(SANITIZER),1)
	DEBUG_FLAGS = -g -fsanitize=address
	OPT_FLAGS =
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

$(shell echo "DEBUG = $(DEBUG)\nSANITIZER = $(SANITIZER)\nNO_FLAGS = $(NO_FLAGS)\nCOMPILER = $(COMPILER)" > $(TMP_FILE))

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
	@$(shell if [[ "$(shell test -d $(OBJS_DIR) && find $(OBJS_DIR) -type f | wc -l)" -eq 0 ]]; then rm -rf $(OBJS_DIR); fi;)

.PHONY: fclean
fclean: clean
	rm -f $(NAME)
	rm -f $(TMP_FILE)

.PHONY: re
re:
	@$(MAKE) fclean --no-print-directory
	@printf "\n"
	@$(MAKE) all --no-print-directory

.PHONY: debug
debug:
	@$(MAKE) all DEBUG=1 --no-print-directory
	@printf "\n[\e[1;34mStarting $(DEBUGGER)\e[0m]\n\n"
	$(DEBUGGER) ./$(NAME)

-include $(DPND)

######################################################