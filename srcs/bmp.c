/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   bmp.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/24 21:21:48 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/26 10:07:46 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "miniRT.h"

char	*generate_bmp_content(t_holder *holder)
{
	char	*content;

	content = "asd";
	(void)holder;
	return (content);
}

int	write_bmp(t_holder *holder, const char *file_name)
{
	int		fd;
	char	*file;
	char	*content;

	file = ft_strjoin(file_name, ".bmp");
	fd = open(file, O_CREAT, S_IRWXU);
	close(fd);
	fd = open(file, O_WRONLY);
	if (fd == -1)
		return (-1);
	content = generate_bmp_content(holder);
	write(fd, content, ft_strlen(content));
	close(fd);
	free(file);
	return (0);
}
