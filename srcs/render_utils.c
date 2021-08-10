/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   render_utils.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/21 11:58:52 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/08/09 17:48:56 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minirt.h"

void	clear_rendering_thread(t_scene *scene)
{
	char	thread_s;
	int		x;
	int		y;

	free_threads(scene);
	if (scene->thread_count != 2)
		thread_s = 's';
	else
		thread_s = '\0';
	printf(COLOR_YELLOW "Restarting rendering thread%c...\n" COLOR_NC, thread_s);
	y = 0;
	while (y < scene->y_res)
	{
		x = 0;
		while (x < scene->x_res)
		{
			put_pixel(&scene->img, x, y, 0x00000000);
			x++;
		}
		y++;
	}
}

void	print_render_message(t_scene *scene)
{
	char	sample_s;
	char	bounce_s;
	char	thread_s;

	if (scene->samples != 1)
		sample_s = 's';
	else
		sample_s = '\0';
	if (scene->max_bounces != 1)
		bounce_s = 's';
	else
		bounce_s = '\0';
	if (scene->thread_count != 2)
		thread_s = 's';
	else
		thread_s = '\0';
	printf(COLOR_YELLOW "Rendering..." COLOR_YELLOW
		" (" COLOR_WHITE "%d " COLOR_CYAN "thread%c, " COLOR_WHITE "%d " COLOR_CYAN "sample%c, " COLOR_WHITE "%d" COLOR_CYAN " max light bounce%c" COLOR_YELLOW ")\n" COLOR_NC,
		scene->thread_count - 1, thread_s, scene->samples, sample_s, scene->max_bounces, bounce_s);
}

bool	get_hit_color(t_scene *scene, t_hit_record *hit_rec, int x, int y)
{
	t_vec2	pxl;
	float	brightness;
	float	random;

	pxl.x = (float)(x + drand48()) / (float)scene->x_res;
	pxl.y = (float)(y + drand48()) / (float)scene->y_res;
	if (get_camera(scene) != NULL
		&& check_ray_hits(scene, gen_ray(scene, pxl,
				get_camera(scene)->transform.position,
				get_camera(scene)->transform.orientation), hit_rec))
	{
		brightness = (hit_rec->color.r
				+ hit_rec->color.g
				+ hit_rec->color.b) / 765.0f;
		random = drand48();
		if (brightness < random - 0.001f || brightness > random + 0.001f)
			light_bouncer(scene, pxl, hit_rec);
		return (true);
	}
	else
		return (false);
}

static bool	check_intersects(t_scene *scene, t_ray ray, t_hit_record *hit_rec, float *closest)
{
	bool	hit;

	if (scene->objects == NULL)
		return (false);
	hit = false;
	if (scene->objects->type == 0)
		hit = hit_sphere(scene, &ray, hit_rec, *closest);
	else if (scene->objects->type == 1)
		hit = hit_plane(scene, &ray, hit_rec, *closest);
	else if (scene->objects->type == 2)
		hit = hit_square(scene, &ray, hit_rec, *closest);
	else if (scene->objects->type == 3)
		hit = hit_cylinder(scene, &ray, hit_rec, *closest);
	else if (scene->objects->type == 4)
		hit = hit_triangle(scene, &ray, hit_rec, *closest);
	if (hit == true)
	{
		hit_rec->hit = true;
		*closest = hit_rec->t;
	}
	if (scene->objects->next == NULL)
		return (false);
	else
		scene->objects = scene->objects->next;
	return (true);
}

bool	check_ray_hits(t_scene *scene, t_ray ray, t_hit_record *hit_rec)
{
	float	closest;

	hit_rec->hit = false;
	hit_rec->hit_color = set_color(0, 0, 0);
	closest = scene->t_max;
	while (true)
	{
		if (!check_intersects(scene, ray, hit_rec, &closest))
			break ;
	}
	if (hit_rec->hit == true)
	{
		set_hit_color(scene, hit_rec);
	}
	while (scene->objects != NULL && scene->objects->prev != NULL)
		scene->objects = scene->objects->prev;
	return (hit_rec->hit);
}
