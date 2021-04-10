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

int	main(int argc, char *argv[])
{
	void		*mlx;
	void		*mlx_window;
	t_img_data	img_data;

	mlx = mlx_init();
	mlx_window = mlx_new_window(mlx, 1920, 1080, WINDOW_TITLE);
	img_data.img = mlx_new_image(mlx, 1920, 1080);
	img_data.addr = mlx_get_data_addr(img_data.img,
			&img_data.bits_per_pixel, &img_data.line_length, &img_data.endian);
	fill_image(&img_data, 1920, 1080, 0x0000FF00);
	mlx_put_image_to_window(mlx, mlx_window, img_data.img, 0, 0);
	mlx_loop(mlx);
	(void)argc;
	(void)argv;
	ft_putstr("Show");
	return (0);
}
