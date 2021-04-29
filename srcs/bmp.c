/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   bmp.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/24 21:21:48 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/29 11:40:37 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minirt.h"
#include "bmp.h"

char	*bmp_name(char *file)
{
	static char	*file_name = "render";

	if (ft_memcmp(file_name, "render", 6) == 0)
		file_name = file;
	return (file_name);
}

bool	save_bmp(bool save)
{
	static bool	shall_save = false;

	if (shall_save == true)
		return (true);
	else
		shall_save = save;
	return (false);
}

static void	init_headers(t_scene *scene, t_file_h *file_h, t_info_h *info_h)
{
	file_h->type = 0x4D42;
	file_h->size = 54 + 4 * scene->x_res * scene->y_res;
	file_h->reserved = 0;
	file_h->offset = 54;
	info_h->size = 40;
	info_h->width = scene->x_res;
	info_h->height = scene->y_res;
	info_h->planes = 1;
	info_h->bits_per_pxl = 32;
	info_h->compression = 0;
	info_h->image_size = 0;
	info_h->pxls_per_meter_x = 0;
	info_h->pxls_per_meter_y = 0;
	info_h->color_used = 0;
	info_h->color_important = 0;
}

static int	write_headers(t_scene *scene, int fd)
{
	t_file_h	file_h;
	t_info_h	info_h;

	init_headers(scene, &file_h, &info_h);
	if (write(fd, (void *)&file_h.type, 2) < 0
		|| write(fd, (void *)&file_h.size, 4) < 0
		|| write(fd, (void *)&file_h.reserved, 4) < 0
		|| write(fd, (void *)&file_h.offset, 4) < 0
		|| write(fd, (void *)&info_h.size, 4) < 0
		|| write(fd, (void *)&info_h.width, 4) < 0
		|| write(fd, (void *)&info_h.height, 4) < 0
		|| write(fd, (void *)&info_h.planes, 2) < 0
		|| write(fd, (void *)&info_h.bits_per_pxl, 2) < 0
		|| write(fd, (void *)&info_h.compression, 4) < 0
		|| write(fd, (void *)&info_h.image_size, 4) < 0
		|| write(fd, (void *)&info_h.pxls_per_meter_x, 4) < 0
		|| write(fd, (void *)&info_h.pxls_per_meter_y, 4) < 0
		|| write(fd, (void *)&info_h.color_used, 4) < 0
		|| write(fd, (void *)&info_h.color_important, 4) < 0)
		return (-1);
	return (0);
}

int	write_bmp(t_scene *scene)
{
	int		fd;
	char	*file;
	char	*file_name;

	if (save_bmp(false))
	{
		file_name = bmp_name("render");
		printf(COLOR_YELLOW "Writing render to " COLOR_LIGHT_BLUE "%s.bmp"
			COLOR_YELLOW "...\n" COLOR_NC, file_name);
		file = ft_strjoin(file_name, ".bmp");
		free(file_name);
		fd = open(file, O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR);
		if (fd < 0 || write_headers(scene, fd) == -1)
			return (-1);
		while (--scene->y_res >= 0)
		{
			if (write(fd, &scene->img.addr[scene->y_res
						* scene->img.line_length], scene->img.line_length) < 0)
				return (-1);
		}
		close(fd);
		free(file);
		printf(COLOR_LIGHT_GREEN "File ready.\n\n" COLOR_NC);
	}
	return (0);
}
