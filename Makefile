# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: ejuliao- <ejuliao-@42lisboa.com>           +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2021/04/08 15:31:37 by ejuliao-          #+#    #+#              #
#    Updated: 2021/04/10 15:29:04 by ejuliao-         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

SRCS = ./miniRT.c ./utils.c ./scene_reader.c ./conversions.c ./readers.c
OBJS = $(SRCS:.c=.o)
NAME = miniRT

GNL_SRCS = ./libraries/get_next_line/get_next_line.c ./libraries/get_next_line/get_next_line_utils.c

INCLUDES = -I. -Ilibraries/libft -Ilibraries/get_next_line -Ilibraries/minilibx_opengl
MLX_FLAGS = -framework OpenGL -framework AppKit
LIBFT_PATH = ./libraries/libft/libft.a
MLX_PATH = ./libraries/minilibx_opengl/libmlx.a

$(NAME):
	# Compiles libft
	$(MAKE) -C ./libraries/libft
	$(MAKE) bonus -C ./libraries/libft

	# Compiles minilibx
	$(MAKE) -C ./libraries/minilibx_opengl

	# Compiles miniRT
	gcc -Wall -Wextra -Werror $(INCLUDES) $(MLX_FLAGS) $(SRCS) $(GNL_SRCS) $(LIBFT_PATH) $(MLX_PATH) $(DEBUG) -o $(NAME)

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
