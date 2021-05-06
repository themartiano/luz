/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   miniRT.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/08 15:12:09 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/05/06 15:39:29 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minirt.h"

static void	init_scene(t_scene *scene)
{
	scene->mlx = mlx_init();
	scene->window = NULL;
	scene->thread = (pthread_t)NULL;
	scene->objects = NULL;
	scene->crrnt_pxl.x = 0;
	scene->crrnt_pxl.y = 0;
	scene->t_min = 0.001f;
	scene->t_max = FLT_MAX;
	scene->samples = 48;
	scene->max_bounces = 12;
	scene->amb_light.color = set_color(0, 0, 0);
	scene->amb_light.brightness = 0.0f;
}

int	window_key_callback(int keycode, t_scene *scene)
{
	printf(COLOR_PURPLE "\nKEY PRESSED: " COLOR_WHITE "%d\n" COLOR_NC,
		keycode);
	if (keycode == KEY_ESC)
	{
		mlx_destroy_window(scene->mlx, scene->window);
		clean_exit(scene);
	}
	return (0);
}

static void	start_minirt(t_scene *scene, bool save, bool window, char *file)
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

static void	read_arguments(t_scene *scene, int argc, char *argv[])
{
	int		i;

	if (argc >= 3)
	{
		i = 2;
		while (argv[i])
		{
			if (ft_memcmp(argv[i], "-s", 2) == 0)
			{
				argv[i] += 2;
				scene->samples = ft_atoi(argv[i]);
			}
			else if (ft_memcmp(argv[i], "-mb", 3) == 0)
			{
				argv[i] += 3;
				scene->max_bounces = ft_atoi(argv[i]);
			}
			i++;
		}
	}
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
		if (argc >= 4 && ft_memcmp(argv[3], "--no-window", 11) == 0)
			show_window = false;
	}
	fd = open(argv[1], O_RDONLY);
	if (fd == -1)
		exit_error("The specified scene file could not be opened.\n");
	init_scene(&scene);
	read_scene(fd, &scene);
	read_arguments(&scene, argc, argv);
	start_minirt(&scene, save, show_window, argv[1]);
	return (0);
}
