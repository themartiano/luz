/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <ejuliao-@42lisboa.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/10 14:51:57 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/10 15:22:12 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "miniRT.h"
#include "../includes/typedefs.h"

int	exit_error(char *message)
{
	ft_putstr("Error\n");
	ft_putstr(message);
	return (1);
}

t_xyz	normalize(t_xyz vector)
{
	t_xyz	new_vector;
	float	w;

	w = sqrt(vector.x * vector.x + vector.y * vector.y + vector.z * vector.z);
	if (vector.x == 0)
		new_vector.x = 0;
	else
		new_vector.x /= w;
	if (vector.y == 0)
		new_vector.y = 0;
	else
		new_vector.y /= w;
	if (vector.z == 0)
		new_vector.z = 0;
	else
		new_vector.z /= w;
	return (new_vector);
}

void	put_pixel(t_img *img_data, int x, int y, int color)
{
	char	*dst;

	dst = img_data->addr + (y * img_data->line_length + x
			* (img_data->bits_per_pixel / 8));
	*(unsigned int *)dst = color;
}

unsigned long rgba_to_hex(t_color rgba)
{
	rgba.a = 0;
	return (rgba.a << 24 | rgba.r << 16 | rgba.g << 8 | rgba.b);
}

float	dot(t_xyz u, t_xyz v)
{
	return ((u.x * v.x) + (u.y * v.y) + (u.z * u.y));
}

bool	hit_sphere(t_sphere sphere, t_ray ray)
{
	t_xyz	oc;
	float	a;
	float	b;
	float	c;
	float	d;

	oc.x = ray.origin.x - sphere.transform.position.x;
	oc.y = ray.origin.y - sphere.transform.position.y;
	oc.z = ray.origin.z - sphere.transform.position.z;
	a = dot(ray.direction, ray.direction);
	b = 2.0f * dot(oc, ray.direction);
	c = dot(oc, oc) - (sphere.diameter / 2.0f) * (sphere.diameter / 2.0f);
	d = b * b - 4 * a * c;
	return (d < 0);
}

void	fill_image(t_img *img_data, int width, int height, t_holder *holder)
{
	int			x;
	int			y;
	t_color		new_color;
	t_sphere	sphere;
	t_ray		ray;

	y = 0;
	while (y < height)
	{
		x = 0;
		while (x < width)
		{
			float	u = (float)x / (float)width;
			float	v = (float)y / (float)height;

			new_color.r = 255;
			new_color.g = 255;
			new_color.b = 255;
			ray.origin.x = 0;
			ray.origin.y = 0;
			ray.origin.z = 0;
			ray.direction.x = -2.0f + (u * 4.0f);
			ray.direction.y = -1.0f + v * 2.0f;
			ray.direction.z = -1;
			sphere = holder->scene.sphere;
			if (hit_sphere(sphere, ray))
				new_color = sphere.color;
			int temp = rgba_to_hex(new_color);
			put_pixel(img_data, x, y, temp);
			x++;
		}
		y++;
	}
}
