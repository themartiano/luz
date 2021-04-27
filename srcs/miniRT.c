/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   miniRT.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/08 15:12:09 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/27 11:52:28 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "miniRT.h"

static void	init_scene(t_scene *scene)
{
	scene->objects = NULL;
	scene->t_min = 0.001f;
	scene->t_max = FLT_MAX;
	scene->samples = 16;
	scene->max_bounces = 8;
}

static void	init_mlx(t_scene *scene, int fd)
{
	read_scene(fd, scene);
	scene->mlx = mlx_init();
	scene->img.img = mlx_new_image(scene->mlx, scene->x_res,
			scene->y_res);
	scene->img.addr = mlx_get_data_addr(scene->img.img,
			&scene->img.bits_per_pixel, &scene->img.line_length, &scene->img
			.endian);
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

void	start_miniRT(t_scene *scene, int fd, bool save, char *file)
{
	char	*file_no_ext;

	init_mlx(scene, fd);
	scene->window = mlx_new_window(scene->mlx, scene->x_res,
			scene->y_res, WINDOW_TITLE);
	mlx_hook(scene->window, DESTROYNOTIFY, 0L, clean_exit, scene);
	mlx_key_hook(scene->window, window_key_callback, scene);
	mlx_loop_hook(scene->mlx, manage_frames, scene);
	if (save == true)
	{
		file_no_ext = get_file_no_ext(file);
		save_bmp(true);
		bmp_name(file_no_ext);
	}
	mlx_loop(scene->mlx);
}

int	main(int argc, char *argv[])
{
	t_scene	scene;
	bool	save;
	int		fd;

	save = false;
	if (argc <= 1)
		exit_error(COLOR_LIGHT_RED "Scene not specified." COLOR_NC);
	else if (argc == 3)
	{
		if (ft_memcmp(argv[2], "--save", 6) != 0)
			exit_error(COLOR_LIGHT_RED "Incorrect save argument." COLOR_NC);
		else
			save = true;
	}
	fd = open(argv[1], O_RDONLY);
	if (fd == -1)
		exit_error(COLOR_LIGHT_RED "Incorrect scene path." COLOR_NC);
	printf("\n");
	init_scene(&scene);
	start_miniRT(&scene, fd, save, argv[1]);
	return (0);
}
