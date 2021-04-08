# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: ejuliao- <ejuliao-@42lisboa.com>           +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2021/04/08 15:31:37 by ejuliao-          #+#    #+#              #
#    Updated: 2021/04/08 15:38:59 by ejuliao-         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

SRCS =
OBJS = $(SRCS:.c=.o)
NAME = miniRT

INCLUDES = -Ilibraries/libft -Ilibraries/minilibx_opengl

%.o: %.c
	gcc -Wall -Wextra -Werror -I. -c -o $@ $<

$(NAME):	$(OBJS)
	# Compiles libft
	$(MAKE) -C ./libraries/libft
	$(MAKE) bonus -C ./libraries/libft
	cp ./libraries/libft/libft.a ./libft.a

	# Compiles minilibx
	$(MAKE) -C ./libraries/minilibx_opengl
	cp ./libraries/minilibx_opengl/libmlx.a ./libmlx.a

	# Compiles miniRT
	gcc -Wall -Wextra -Werror $(INCLUDES) miniRT.c libft.a libmlx.a -o miniRT

all:	$(NAME)

clean:
	rm -rf $(OBJS)

fclean:	clean
	rm -rf $(NAME).a

re:	fclean all

.PHONY: re fclean clean all
