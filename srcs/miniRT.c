/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   miniRT.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/08 15:12:09 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/22 12:47:52 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "miniRT.h"

void	init_holder(t_holder *holder)
{
	holder->scene.plane = NULL;
	holder->scene.sphere = NULL;
	holder->scene.square = NULL;
	holder->scene.cylinder = NULL;
	holder->scene.triangle = NULL;
	holder->scene.t_min = 0.001f;
	holder->scene.t_max = FLT_MAX;
	holder->scene.samples = 4;
	holder->scene.min_bounces = 1;
	holder->scene.max_bounces = 8;
}

int	window_key_callback(int keycode, t_holder *holder)
{
	printf("KEY PRESSED: %d\n", keycode);
	if (keycode == KEY_ESC)
	{
		mlx_destroy_window(holder->mlx, holder->window);
		clean_exit(holder);
	}
	return (0);
}

int	start_render(void *p_holder)
{
	static bool	rendered = false;
	t_holder	*holder;

	holder = p_holder;
	if (rendered == false)
	{
		render(holder);
		mlx_put_image_to_window(holder->mlx, holder->window, holder->img.img,
			0, 0);
		rendered = true;
	}
	return (0);
}

void	start_mlx(t_holder *holder, int fd, bool save)
{
	read_scene(fd, holder);
	holder->mlx = mlx_init();
	holder->window = mlx_new_window(holder->mlx, holder->scene.x_res,
			holder->scene.y_res, WINDOW_TITLE);
	mlx_hook(holder->window, 17, 0L, clean_exit, holder);
	mlx_key_hook(holder->window, window_key_callback, holder);
	holder->img.img = mlx_new_image(holder->mlx, holder->scene.x_res,
			holder->scene.y_res);
	holder->img.addr = mlx_get_data_addr(holder->img.img,
			&holder->img.bits_per_pixel, &holder->img.line_length, &holder->img
			.endian);
	mlx_pixel_put(holder->mlx, holder->window, 0, 0, 0x00000000);
	mlx_loop_hook(holder->mlx, start_render, holder);
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
	init_holder(&holder);
	start_mlx(&holder, fd, save);
	return (0);
}
