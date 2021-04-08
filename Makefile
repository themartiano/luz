SRCS = miniRT.c
OBJS = $(SRCS:.c=.o)
NAME = miniRT
LIBFT = libft

%.o: %.c
	gcc -Wall -Wextra -Werror -I. -Ilibraries/libft -Ilibraries/minilibx_opengl -c -o $@ $<

$(NAME):	$(OBJS)
	$(MAKE) -C ./libraries/libft
	$(MAKE) bonus -C ./libraries/libft
	cp ./libraries/libft/$(LIBFT).a ./$(NAME).a
	ar -rcs $(NAME).a $(OBJS)

all:	$(NAME)

clean:
	rm -rf $(OBJS)

fclean:	clean
	rm -rf $(NAME).a

re:	fclean all

.PHONY: re fclean clean all
