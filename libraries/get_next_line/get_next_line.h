/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_next_line.h                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/02/23 10:04:57 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/10 15:31:18 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef GET_NEXT_LINE_H
# define GET_NEXT_LINE_H

# ifndef BUFFER_SIZE
#  define BUFFER_SIZE 1
# endif

# include <unistd.h>
# include <stdlib.h>
# include <stdbool.h>

char	*prev_nl(char *str);
char	*pos_nl(char *str);
int		get_next_line(int fd, char **line);
bool	str_contains_nl(char *str);
size_t	gnl_strlen(const char *s);
char	*gnl_strjoin(char *s1, const char *s2);

#endif
