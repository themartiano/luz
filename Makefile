# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2021/04/08 15:31:37 by ejuliao-          #+#    #+#              #
#    Updated: 2021/06/17 09:16:11 by ejuliao-         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = miniRT
SRCS =	./srcs/minirt.c ./srcs/utils.c ./srcs/scene_reader.c ./srcs/conversions.c ./srcs/readers.c ./srcs/renderer.c ./srcs/exit.c	\
		./srcs/vector_utils.c ./srcs/render_utils.c ./srcs/sphere_utils.c ./srcs/bmp.c ./srcs/plane_utils.c ./srcs/color_utils.c	\
		./srcs/algebra.c ./srcs/cylinder_utils.c ./srcs/light.c ./srcs/triangle_utils.c ./srcs/square_utils.c ./srcs/camera_utils.c	\
		./libraries/get_next_line/get_next_line.c ./libraries/get_next_line/get_next_line_utils.c ./srcs/utils_2.c
OBJS = $(SRCS:.c=.o)
INCLUDES = -Iincludes -Ilibraries/libft -Ilibraries/get_next_line
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
	DEBUG_FLAGS = -fsanitize=address -g
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

all:
	@printf "[\e[1;34mPreparing objects\e[0m]\n\n"
	@$(MAKE) $(NAME) --no-print-directory

%.o: %.c
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

bonus:	all

clean:
	@printf "[\e[1;33mCleaning\e[0m]\n\n"
	rm -f $(NAME)

fclean:	clean
	rm -f $(OBJS)
	@$(MAKE) fclean -C ./libraries/libft
	@$(MAKE) clean -C ./libraries/$(CURR_MLX)
	rm -f ./libmlx.dylib

re:
	@$(MAKE) fclean --no-print-directory
	@printf "\n"
	@$(MAKE) all --no-print-directory

.PHONY: debug re fclean clean bonus all
