/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/10 14:51:57 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/15 13:39:02 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "miniRT.h"

void	set_color(t_color *color, int r, int g, int b)
{
	if (r < 0)
		r = 0;
	if (g < 0)
		g = 0;
	if (b < 0)
		b = 0;
	color->r = r;
	color->g = g;
	color->b = b;
}

t_vec3	normalize(t_vec3 vector)
{
	float	w;

	w = sqrt(vector.x * vector.x + vector.y * vector.y + vector.z * vector.z);
	if (vector.x == 0)
		vector.x = 0;
	else
		vector.x /= w;
	if (vector.y == 0)
		vector.y = 0;
	else
		vector.y /= w;
	if (vector.z == 0)
		vector.z = 0;
	else
		vector.z /= w;
	return (vector);
}

void	put_pixel(t_img *img_data, int x, int y, int color)
{
	char	*dst;

	dst = img_data->addr + (y * img_data->line_length + x
			* (img_data->bits_per_pixel / 8));
	*(unsigned int *)dst = color;
}

unsigned long	rgba_to_hex(t_color rgba)
{
	rgba.a = 0;
	return (rgba.a << 24 | rgba.r << 16 | rgba.g << 8 | rgba.b);
}

float	dot(t_vec3 u, t_vec3 v)
{
	return ((u.x * v.x) + (u.y * v.y) + (u.z * v.z));
}
