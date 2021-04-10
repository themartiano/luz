# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: ejuliao- <ejuliao-@42lisboa.com>           +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2021/04/08 15:31:37 by ejuliao-          #+#    #+#              #
#    Updated: 2021/04/10 14:51:34 by ejuliao-         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

SRCS = ./miniRT.c ./utils.c
OBJS = $(SRCS:.c=.o)
NAME = miniRT

INCLUDES = -I. -Ilibraries/libft -Ilibraries/minilibx_opengl
MLX_FLAGS = -framework OpenGL -framework AppKit

$(NAME):
	# Compiles libft
	$(MAKE) -C ./libraries/libft
	$(MAKE) bonus -C ./libraries/libft
	cp ./libraries/libft/libft.a ./libft.a

	# Compiles minilibx
	$(MAKE) -C ./libraries/minilibx_opengl
	cp ./libraries/minilibx_opengl/libmlx.a ./libmlx.a

	# Compiles miniRT
	gcc -Wall -Wextra -Werror $(INCLUDES) $(MLX_FLAGS) $(SRCS) libft.a libmlx.a -o miniRT

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
