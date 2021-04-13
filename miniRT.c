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

int	window_key_callback(int keycode, t_holder *holder)
{
	printf("KEY PRESSED: %d\n", keycode);
	if (keycode == KEY_ESC)
	{
		mlx_destroy_window(holder->mlx, holder->window);
		exit(0);
	}
	return (0);
}

void	start_mlx(t_holder *holder, int fd, bool save)
{
	read_scene(fd, holder);
	holder->mlx = mlx_init();
	holder->window = mlx_new_window(holder->mlx, holder->scene.x_res,
			holder->scene.y_res, WINDOW_TITLE);
	holder->img.img = mlx_new_image(holder->mlx, holder->scene.x_res,
			holder->scene.y_res);
	holder->img.addr = mlx_get_data_addr(holder->img.img,
			&holder->img.bits_per_pixel, &holder->img.line_length, &holder->img
			.endian);
	fill_image(&holder->img, holder->scene.x_res, holder->scene.y_res,
		0x0000FF00);
	mlx_put_image_to_window(holder->mlx, holder->window, holder->img.img, 0, 0);
	mlx_key_hook(holder->window, window_key_callback, holder);
	mlx_loop(holder->mlx);
	(void)save;
}

int	main(int argc, char *argv[])
{
	t_holder	holder;
	bool		save;
	int			fd;

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
	fd = open(argv[1], O_RDONLY);
	if (fd == -1)
		return (exit_error("Incorrect scene path."));
	start_mlx(&holder, fd, save);
	return (0);
}
