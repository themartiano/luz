/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   miniRT.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <ejuliao-@42lisboa.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/08 15:12:09 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/10 14:47:00 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "miniRT.h"

void	put_pixel(t_img_data *img_data, int x, int y, int color)
{
	char	*dst;

	dst = img_data->addr + (y * img_data->line_length + x * (img_data->bits_per_pixel / 8));
	*(unsigned int *)dst = color;
}

int	main(int argc, char *argv[])
{
	void		*mlx;
	void		*mlx_window;
	t_img_data	img_data;

	mlx = mlx_init();
	mlx_window = mlx_new_window(mlx, 1920, 1080, WINDOW_TITLE);

	img_data.img = mlx_new_image(mlx, 1920, 1080);
	img_data.addr = mlx_get_data_addr(img_data.img, &img_data.bits_per_pixel, &img_data.line_length, &img_data.endian);

	put_pixel(&img_data, 5, 5, 0x00FF0000);

	mlx_put_image_to_window(mlx, mlx_window, img_data.img, 0, 0);
	mlx_loop(mlx);

	(void)argc;
	(void)argv;
	ft_putstr("Show");
	return (0);
}
