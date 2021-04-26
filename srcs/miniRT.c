/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   miniRT.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/08 15:12:09 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/26 17:08:58 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "miniRT.h"

void	init_holder(t_holder *holder)
{
	holder->scene.objects = NULL;
	holder->scene.t_min = 0.001f;
	holder->scene.t_max = FLT_MAX;
	holder->scene.samples = 4;
	holder->scene.max_bounces = 8;
}

void	init_mlx(t_holder *holder, int fd)
{
	read_scene(fd, holder);
	holder->mlx = mlx_init();
	holder->img.img = mlx_new_image(holder->mlx, holder->scene.x_res,
			holder->scene.y_res);
	holder->img.addr = mlx_get_data_addr(holder->img.img,
			&holder->img.bits_per_pixel, &holder->img.line_length, &holder->img
			.endian);
}

int	window_key_callback(int keycode, t_holder *holder)
{
	printf("\nKEY PRESSED: %d\n", keycode);
	if (keycode == KEY_ESC)
	{
		mlx_destroy_window(holder->mlx, holder->window);
		clean_exit(holder);
	}
	return (0);
}

void	start_miniRT(t_holder *holder, int fd, bool save, char *file)
{
	char	*file_no_ext;

	init_mlx(holder, fd);
	if (save == true)
	{
		printf("\n" RENDERING_MSG "\n");
		file_no_ext = get_file_no_ext(file);
		render(holder);
		printf("Writing .bmp file...\n");
		if (write_bmp(holder->scene, holder->img, file_no_ext) == -1)
			printf("An error occurred while writing the .bmp file.\n");
		else
			printf("File ready.\n");
		free(file_no_ext);
	}
	else
	{
		holder->window = mlx_new_window(holder->mlx, holder->scene.x_res,
				holder->scene.y_res, WINDOW_TITLE);
		mlx_hook(holder->window, DESTROYNOTIFY, 0L, clean_exit, holder);
		mlx_key_hook(holder->window, window_key_callback, holder);
		mlx_loop_hook(holder->mlx, start_render, holder);
		mlx_loop(holder->mlx);
	}
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
	printf("\n");
	init_holder(&holder);
	start_miniRT(&holder, fd, save, argv[1]);
	return (0);
}
