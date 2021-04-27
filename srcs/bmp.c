/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   bmp.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/24 21:21:48 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/27 10:58:27 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "miniRT.h"
#include "bmp.h"

static int	write_headers(t_scene *scene, int fd)
{
	t_file_h	file_h;
	t_info_h	info_h;

	file_h.type = 0x4D42;
	file_h.size = 54 + 4 * scene->x_res * scene->y_res;
	file_h.reserved = 0;
	file_h.offset = 54;
	if (write(fd, (void *)&file_h, 14) < 0)
		return (-1);
	info_h.size = 40;
	info_h.width = scene->x_res;
	info_h.height = scene->y_res;
	info_h.planes = 1;
	info_h.bits_per_pxl = 32;
	info_h.compression = 0;
	info_h.image_size = 0;
	info_h.pxls_per_meter_x = 0;
	info_h.pxls_per_meter_y = 0;
	info_h.color_used = 0;
	info_h.color_important = 0;
	if (write(fd, (void *)&info_h, 40) < 0)
		return (-1);
	return (0);
}

int	write_bmp(t_scene *scene, const char *file_name)
{
	int		fd;
	char	*file;

	file = ft_strjoin(file_name, ".bmp");
	fd = open(file, O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR);
	if (fd < 0 || write_headers(scene, fd) == -1)
		return (-1);
	while (--scene->y_res >= 0)
	{
		if (write(fd, &scene->img.addr[scene->y_res * scene->img.line_length],
				scene->img.line_length) < 0)
		{
			return (-1);
		}
	}
	close(fd);
	free(file);
	return (0);
}
