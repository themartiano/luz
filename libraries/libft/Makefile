# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2021/02/08 14:16:30 by ejuliao-          #+#    #+#              #
#    Updated: 2021/04/27 10:25:21 by ejuliao-         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

SRCS =	./ft_substr.c ./ft_strjoin.c ./ft_strtrim.c ./ft_split.c ./ft_itoa.c ./ft_strmapi.c ./ft_putchar_fd.c			\
		./ft_putstr_fd.c ./ft_putendl_fd.c ./ft_putnbr_fd.c ./ft_atoi.c ./ft_tolower.c ./ft_toupper.c ./ft_isprint.c	\
		./ft_isascii.c ./ft_isalnum.c ./ft_isdigit.c ./ft_isalpha.c ./ft_strncmp.c ./ft_strnstr.c ./ft_strchr.c			\
		./ft_strrchr.c ./ft_strlcat.c ./ft_strlcpy.c ./ft_strlen.c ./ft_strdup.c ./ft_memset.c ./ft_bzero.c				\
		./ft_memcpy.c ./ft_memccpy.c ./ft_memmove.c ./ft_memchr.c ./ft_memcmp.c ./ft_calloc.c ./ft_index_of.c			\
		./ft_last_index_of.c
OBJS =	$(SRCS:.c=.o)
BSRCS =	./ft_lstnew.c ./ft_lstadd_front.c ./ft_lstsize.c ./ft_lstlast.c ./ft_lstadd_back.c ./ft_lstdelone.c				\
		./ft_lstclear.c ./ft_lstiter.c ./ft_lstmap.c ./ft_isspace_bonus.c ./ft_putchar_bonus.c ./ft_putstr_bonus.c		\
		./ft_putint_bonus.c ./ft_putuint_bonus.c ./ft_charinset_bonus.c ./ft_intdigits_bonus.c ./ft_uintdigits_bonus.c	\
		./ft_putnstr_bonus.c ./ft_uitoa_bonus.c ./ft_longdigits_bonus.c ./ft_ulongdigits_bonus.c ./ft_ltoa_bonus.c		\
		./ft_ultoa_bonus.c ./ft_longlongdigits_bonus.c ./ft_ulonglongdigits_bonus.c ./ft_lltoa_bonus.c					\
		./ft_ulltoa_bonus.c ./ft_atof_bonus.c
BOBJS = $(BSRCS:.c=.o)
NAME =	libft.a
SO =	libft.so

all:	$(NAME)

$(NAME):
	gcc -Wall -Wextra -Werror -O3 -c $(SRCS)
	ar -rcs $(NAME) $(OBJS)

clean:
	rm -f $(OBJS) $(BOBJS)

soclean:	clean
	rm -f $(SO)

fclean:	clean
	rm -f $(NAME)
	rm -f $(SO)

re:	fclean all

bonus:
	gcc -Wall -Wextra -Werror -O3 -c $(BSRCS)
	ar -rcs $(NAME) $(BOBJS)

so:	$(OBJS) $(BOBJS)
	gcc -shared -o $(SO) $(OBJS) $(BOBJS)

.PHONY: so bonus re fclean soclean clean all
