/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scene_reader.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <ejuliao-@42lisboa.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/12 11:04:06 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/12 11:04:08 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "miniRT.h"

void	read_scene(int fd, t_window *window)
{
	char	*line;
	char	**values;
	int		rv;

	rv = get_next_line(fd, &line);
	if (rv != -1)
	{
		values = ft_split(line, ' ');
		if (ft_memcmp(values[0], "R", 1) == 0)
		{
			window->width = ft_atoi(values[1]);
			window->height = ft_atoi(values[2]);
		}
	}
}
