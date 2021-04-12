/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   miniRT.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <ejuliao-@42lisboa.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/08 15:12:09 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/10 15:21:31 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "miniRT.h"

void	start_mlx(int fd, bool save)
{
	void		*mlx;
	t_window	window;
	t_img_data	img_data;

	read_scene(fd, &window);
	mlx = mlx_init();
	window.mlx_window = mlx_new_window(mlx, window.width, window.height,
			WINDOW_TITLE);
	img_data.img = mlx_new_image(mlx, window.width, window.height);
	img_data.addr = mlx_get_data_addr(img_data.img,
			&img_data.bits_per_pixel, &img_data.line_length, &img_data.endian);
	fill_image(&img_data, window.width, window.height, 0x0000FF00);
	mlx_put_image_to_window(mlx, window.mlx_window, img_data.img, 0, 0);
	mlx_loop(mlx);
	(void)save;
}

int	main(int argc, char *argv[])
{
	bool	save;
	int		fd;

	save = false;
	if (argc <= 1)
		return (exit_error("Scene not specified."));
	else if (argc == 3)
	{
		if (ft_memcmp(argv[2], "--save", 6) != 0)
			return (exit_error("Incorrect save argument."));
		else
			save = true;
	}
	fd = open(argv[1], O_RDWR);
	if (fd == -1)
		return (exit_error("Incorrect scene path."));
	start_mlx(fd, save);
	return (0);
}
