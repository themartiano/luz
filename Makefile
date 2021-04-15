# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2021/04/08 15:31:37 by ejuliao-          #+#    #+#              #
#    Updated: 2021/04/15 19:52:16 by ejuliao-         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

SRCS = ./srcs/miniRT.c ./srcs/utils.c ./srcs/scene_reader.c ./srcs/conversions.c ./srcs/readers.c ./srcs/renderer.c
OBJS = $(SRCS:.c=.o)
NAME = miniRT

GNL_SRCS = ./libraries/get_next_line/get_next_line.c ./libraries/get_next_line/get_next_line_utils.c

INCLUDES = -Iincludes -Ilibraries/libft -Ilibraries/get_next_line -Ilibraries/minilibx_opengl

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
	CURR_MLX = minilibx_linux
	MLX_FLAGS = -lbsd -Llibraries/$(CURR_MLX) -lmlx -lXext -lX11 -lm -DOS=2
endif
ifeq ($(UNAME_S),Darwin)
	CURR_MLX = minilibx_opengl
	MLX_FLAGS = -Ilibraries/$(CURR_MLX) -framework OpenGL -framework AppKit -DOS=1
endif

LIBFT_PATH = ./libraries/libft/libft.a
MLX_PATH = ./libraries/$(CURR_MLX)/libmlx.a

$(NAME):
	# Compiles libft
	$(MAKE) -C ./libraries/libft
	$(MAKE) bonus -C ./libraries/libft

	# Compiles minilibx
	$(MAKE) -C ./libraries/$(CURR_MLX)

	# Compiles miniRT
	gcc -Wall -Wextra -Werror $(INCLUDES) $(SRCS) $(GNL_SRCS) $(MLX_FLAGS) $(LIBFT_PATH) $(MLX_PATH) $(DEBUG) -o $(NAME)

all:	$(NAME)

libclean:
	$(MAKE) fclean -C ./libraries/libft
	$(MAKE) clean -C ./libraries/minilibx_opengl

clean:
	rm -f $(OBJS)

fclean:	clean
	rm -rf $(NAME)

re:	fclean all

.PHONY: re fclean clean all
