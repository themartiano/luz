/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   miniRT.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/08 15:12:09 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/05/14 06:43:23 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minirt.h"

static void	init_scene(t_scene *scene)
{
	scene->mlx = mlx_init();
	scene->window = NULL;
	scene->thread = (pthread_t) NULL;
	scene->objects = NULL;
	scene->lights = NULL;
	scene->cameras = NULL;
	scene->x_res = 0;
	scene->y_res = 0;
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
		clean_exit(scene, 0);
	}
	else if (keycode == KEY_J)
		change_camera(scene);
	else if (keycode == KEY_W || keycode == KEY_A || keycode == KEY_S
		|| keycode == KEY_D || keycode == KEY_Q || keycode == KEY_E)
		move_camera(scene, keycode);
	else if (keycode == KEY_UP || keycode == KEY_LEFT || keycode == KEY_DOWN
		|| keycode == KEY_RIGHT)
		rotate_camera(scene, keycode);
	return (0);
}

static void	start_minirt(t_scene *scene, bool save, bool window, char *file)
{
	char	*file_no_ext;

	scene->img.img = mlx_new_image(scene->mlx, scene->x_res,
			scene->y_res);
	scene->img.addr = mlx_get_data_addr(scene->img.img, &scene->img
			.bits_per_pixel, &scene->img.line_length, &scene->img.endian);
	if (save == true)
	{
		file_no_ext = get_file_no_ext(file);
		save_bmp(true);
		bmp_name(file_no_ext);
	}
	if (window == true || save == false)
	{
		scene->window = mlx_new_window(scene->mlx, scene->x_res,
				scene->y_res, WINDOW_TITLE);
		mlx_hook(scene->window, DESTROYNOTIFY, 0L, clean_exit, scene);
		mlx_key_hook(scene->window, window_key_callback, scene);
		mlx_loop_hook(scene->mlx, render_manager, scene);
		printf(COLOR_YELLOW "Starting rendering thread...\n" COLOR_NC);
		print_render_message(scene);
		mlx_loop(scene->mlx);
	}
	else
		render_manager(scene);
}

static void	read_arguments(t_scene *scene, char *argv[], bool *save,
bool *show_window)
{
	int	i;

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
		else if (ft_memcmp(argv[i], "--save", 6) == 0)
			*save = true;
		else if (ft_memcmp(argv[i], "--no-window", 11) == 0)
			*show_window = false;
		i++;
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
		exit_error(&scene, "Scene not specified.");
	fd = open(argv[1], O_RDONLY);
	if (fd == -1)
		exit_error(&scene, "The specified scene file could not be opened.");
	init_scene(&scene);
	read_scene(fd, &scene);
	if (argc >= 3)
		read_arguments(&scene, argv, &save, &show_window);
	start_minirt(&scene, save, show_window, argv[1]);
	return (0);
}
