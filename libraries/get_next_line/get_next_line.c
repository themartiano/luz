/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_next_line.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <ejuliao-@42lisboa.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/02/23 10:03:15 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/10 15:42:03 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "get_next_line.h"

int	get_next_line(int fd, char **line)
{
	static char	*strg[1024];
	char		*str;
	int			rd;

	if (line == NULL || BUFFER_SIZE < 1 || read(fd, 0, 0) == -1)
		return (-1);
	str = malloc((BUFFER_SIZE + 1) * sizeof(char));
	if (str == NULL)
		return (-1);
	rd = 1;
	while (!str_contains_nl(strg[fd]) && rd != 0)
	{
		rd = read(fd, str, BUFFER_SIZE);
		if (rd == -1)
		{
			free(str);
			return (-1);
		}
		str[rd] = '\0';
		strg[fd] = gnl_strjoin(strg[fd], str);
	}
	free(str);
	*line = prev_nl(strg[fd]);
	strg[fd] = pos_nl(strg[fd]);
	if (rd != 0)
		rd = 1;
	return (rd);
}

char	*prev_nl(char *str)
{
	char	*prev_str;
	int		i;

	if (str == NULL)
		return (NULL);
	i = 0;
	while (str[i] != '\0' && str[i] != '\n')
		i++;
	prev_str = malloc((i + 1) * sizeof(char));
	if (prev_str == NULL)
		return (NULL);
	i = 0;
	while (str[i] != '\0' && str[i] != '\n')
	{
		prev_str[i] = str[i];
		i++;
	}
	prev_str[i] = '\0';
	return (prev_str);
}

char	*pos_nl(char *str)
{
	char	*pos_str;
	int		i;
	int		j;

	if (str == NULL)
		return (0);
	i = 0;
	while (str[i] != '\0' && str[i] != '\n')
		i++;
	if (str[i] == '\0')
	{
		free(str);
		return (0);
	}
	pos_str = malloc((gnl_strlen(str) - i + 1) * sizeof(char));
	if (pos_str == NULL)
		return (NULL);
	i++;
	j = 0;
	while (str[i] != '\0')
		pos_str[j++] = str[i++];
	pos_str[j] = '\0';
	free(str);
	return (pos_str);
}
