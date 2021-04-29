# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2021/04/08 15:31:37 by ejuliao-          #+#    #+#              #
#    Updated: 2021/04/29 12:47:51 by ejuliao-         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

SRCS =	./srcs/minirt.c ./srcs/utils.c ./srcs/scene_reader.c ./srcs/conversions.c ./srcs/readers.c ./srcs/renderer.c ./srcs/exit.c	\
		./srcs/vector_utils.c ./srcs/render_utils.c ./srcs/sphere_utils.c ./srcs/bmp.c ./srcs/plane_utils.c ./srcs/object_utils.c	\
		./srcs/color_utils.c ./srcs/algebra.c ./srcs/cylinder_utils.c
NAME = miniRT
GNL_SRCS = ./libraries/get_next_line/get_next_line.c ./libraries/get_next_line/get_next_line_utils.c
INCLUDES = -Iincludes -Ilibraries/libft -Ilibraries/get_next_line
LIBFT_PATH = ./libraries/libft/libft.a
WWW_FLAGS = -Wall -Wextra -Werror
OPT_FLAGS = -O3

ifeq ($(FLAGS),0)
	WWW_FLAGS =
endif

OS_NAME := $(shell uname -s)
ifeq ($(OS_NAME),Linux)
	CURR_MLX = minilibx_linux
	MLX_FLAGS = -lbsd -Llibraries/$(CURR_MLX) -lmlx -lXext -lX11 -lm -DOS=2
endif
ifeq ($(OS_NAME),Darwin)
	CURR_MLX = minilibx_mms
	MLX_FLAGS = -Ilibraries/$(CURR_MLX) -Llibraries/$(CURR_MLX) -lmlx -DOS=1
	CP_CMD = cp ./libraries/$(CURR_MLX)/libmlx.dylib ./
endif

ifeq ($(DEBUG),1)
	DEBUG_FLAGS = -g
	OPT_FLAGS =
endif

ifeq ($(SANITIZE),1)
	DEBUG_FLAGS = -fsanitize=address -g
	OPT_FLAGS =
endif

$(NAME):
	@# Compiles libft
	@printf "\e[1;34m\nCompiling libft:\e[0m\n\n"
	$(MAKE) -C ./libraries/libft
	$(MAKE) bonus -C ./libraries/libft

	@# Compiles minilibx
	@printf "\e[1;34m\nCompiling minilibx:\e[0m\n\n"
	$(MAKE) -C ./libraries/$(CURR_MLX)
	$(CP_CMD)

	@# Compiles miniRT
	@printf "\e[1;34m\nCompiling miniRT:\e[0m\n\n"
	gcc $(WWW_FLAGS) $(OPT_FLAGS) $(DEBUG_FLAGS) -pthread $(INCLUDES) $(SRCS) $(GNL_SRCS) $(MLX_FLAGS) $(LIBFT_PATH) -o $(NAME)
	
	@printf "\e[0;32m\nCompilation done. miniRT ready.\e[0m\n\n"

all:	$(NAME)

clean:
	@printf "\e[1;33m\nCleaning:\e[0m\n\n"
	rm -f $(NAME)

fclean:	clean
	@printf "\e[1;33m\nCleaning libraries:\e[0m\n\n"
	$(MAKE) fclean -C ./libraries/libft
	$(MAKE) clean -C ./libraries/$(CURR_MLX)
	rm -f ./libmlx.dylib

re:	clean all

.PHONY: re fclean clean all
