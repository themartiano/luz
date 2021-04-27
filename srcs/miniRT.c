/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   miniRT.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/08 15:12:09 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/27 18:07:53 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "miniRT.h"

static void	init_scene(t_scene *scene)
{
	scene->mlx = mlx_init();
	scene->window = NULL;
	scene->objects = NULL;
	scene->t_min = 0.001f;
	scene->t_max = FLT_MAX;
	scene->samples = 20;
	scene->max_bounces = 12;
}

int	window_key_callback(int keycode, t_scene *scene)
{
	printf(COLOR_LIGHT_BLUE "KEY PRESSED: " COLOR_NC "%d\n", keycode);
	if (keycode == KEY_ESC)
	{
		printf(COLOR_CYAN "\nExiting...\n" COLOR_NC);
		mlx_destroy_window(scene->mlx, scene->window);
		clean_exit(scene);
	}
	return (0);
}

static void	start_miniRT(t_scene *scene, bool save, bool window, char *file)
{
	char	*file_no_ext;

	scene->img.img = mlx_new_image(scene->mlx, scene->x_res,
			scene->y_res);
	scene->img.addr = mlx_get_data_addr(scene->img.img,
			&scene->img.bits_per_pixel, &scene->img.line_length, &scene->img
			.endian);
	if (save == true)
	{
		file_no_ext = get_file_no_ext(file);
		save_bmp(true);
		bmp_name(file_no_ext);
	}
	if (window == true)
	{
		scene->window = mlx_new_window(scene->mlx, scene->x_res,
				scene->y_res, WINDOW_TITLE);
		mlx_hook(scene->window, DESTROYNOTIFY, 0L, clean_exit, scene);
		mlx_key_hook(scene->window, window_key_callback, scene);
		mlx_loop_hook(scene->mlx, render_manager, scene);
		mlx_loop(scene->mlx);
	}
	else
		render_manager(scene);
}

int	main(int argc, char *argv[])
{
	t_scene	scene;
	bool	save;
	bool	show_window;
	int		fd;

	save = false;
	show_window = true;
	if (argc <= 1)
		exit_error("Scene not specified.\n");
	else if (argc >= 3)
	{
		if (ft_memcmp(argv[2], "--save", 6) == 0)
			save = true;
		else
			exit_error("Incorrect save argument.\n");
		if (argc >= 4 && ft_memcmp(argv[3], "--no-window", 11) == 0)
			show_window = false;
	}
	fd = open(argv[1], O_RDONLY);
	if (fd == -1)
		exit_error("The specified scene file could not be opened.\n");
	init_scene(&scene);
	read_scene(fd, &scene);
	start_miniRT(&scene, save, show_window, argv[1]);
	return (0);
}
