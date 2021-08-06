NAME := miniRT
SRCS_DIR := ./srcs
OBJS_DIR := ./objs
SRCS :=	./srcs/minirt.c ./srcs/utils.c ./srcs/scene_reader.c ./srcs/conversions.c ./srcs/readers.c ./srcs/renderer.c ./srcs/exit.c	\
		./srcs/vector_utils.c ./srcs/render_utils.c ./srcs/sphere_utils.c ./srcs/bmp.c ./srcs/plane_utils.c ./srcs/color_utils.c	\
		./srcs/algebra.c ./srcs/cylinder_utils.c ./srcs/light.c ./srcs/triangle_utils.c ./srcs/square_utils.c ./srcs/camera_utils.c	\
		./srcs/get_next_line.c ./srcs/get_next_line_utils.c ./srcs/utils_2.c
OBJS := $(patsubst $(SRCS_DIR)/%.c,$(OBJS_DIR)/%.o,$(SRCS))
INCLUDES = -Iincludes -Ilibraries/libft
LIBFT_PATH = ./libraries/libft/libft.a
WWW_FLAGS = -Wall -Wextra -Werror
OPT_FLAGS = -O3

####  ~~   Preparing variables   ~~  ####
ifeq ($(NOFLAGS),1)
	WWW_FLAGS =
endif

ifeq ($(DEBUG),1)
	DEBUG_FLAGS = -g
	OPT_FLAGS =
endif

ifeq ($(SANITIZER),1)
	DEBUG_FLAGS = -g -fsanitize=address
	OPT_FLAGS =
endif

ifeq ($(shell uname -s),Linux)
	DEBUGGER = gdb
	CURR_MLX = minilibx_linux
	MLX_FLAGS = -lbsd -Llibraries/$(CURR_MLX) -lmlx -lXext -lX11 -lm -DOS=2
endif
ifeq ($(shell uname -s),Darwin)
	DEBUGGER = lldb
	CURR_MLX = minilibx_mms
	MLX_FLAGS = -Ilibraries/$(CURR_MLX) -Llibraries/$(CURR_MLX) -lmlx -DOS=1
	CP_CMD = cp ./libraries/$(CURR_MLX)/libmlx.dylib ./
endif
#########################################

.PHONY: all
all:
	@printf "[\e[1;34mPreparing objects\e[0m]\n\n"
	@$(MAKE) $(NAME) --no-print-directory

$(OBJS_DIR)/%.o: $(SRCS_DIR)/%.c
	$(shell [ ! -d $(@D) ] && mkdir -p $(@D))
	gcc $(WWW_FLAGS) $(OPT_FLAGS) $(DEBUG_FLAGS) -pthread $(INCLUDES) -c -o $@ $< -Ilibraries/$(CURR_MLX)

$(NAME):	$(OBJS)
	@printf "\n[\e[1;34mCompiling libft\e[0m]\n\n"
	@$(MAKE) -C ./libraries/libft
	@$(MAKE) bonus -C ./libraries/libft

	@printf "\n[\e[1;34mCompiling minilibx\e[0m]\n\n"
	@$(MAKE) -C ./libraries/$(CURR_MLX)
	@$(CP_CMD)

	@printf "\n[\e[1;34mCompiling $(NAME)\e[0m]\n\n"
	gcc $(WWW_FLAGS) $(OPT_FLAGS) $(DEBUG_FLAGS) -pthread $(INCLUDES) $(OBJS) $(MLX_FLAGS) $(LIBFT_PATH) -o $(NAME)

	@printf "\n[\e[0;32mCompilation done. $(NAME) ready.\e[0m]\n"

.PHONY: clean
clean:
	@printf "[\e[1;33mCleaning\e[0m]\n\n"
	$(shell rm -f $(OBJS))
	@$(MAKE) fclean -C ./libraries/libft
	@$(MAKE) clean -C ./libraries/$(CURR_MLX)
	rm -f ./libmlx.dylib
	@$(shell if [[ "$(shell test -d $(OBJS_DIR) && find $(OBJS_DIR) -type f | wc -l)" -eq 0 ]]; then rm -rf $(OBJS_DIR); fi;)

.PHONY: fclean
fclean:	clean
	rm -f $(NAME)

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