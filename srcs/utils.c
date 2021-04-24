/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/10 14:51:57 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/24 19:07:47 by ejuliao-         ###   ########.fr       */
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

void	put_pixel(t_img *img, int x, int y, int color)
{
	char	*dst;

	dst = img->addr + (y * img->line_length + x
			* (img->bits_per_pixel / 8));
	*(unsigned int *)dst = color;
}

t_vec3	normalize(t_vec3 vector)
{
	float	w;

	w = sqrt(vector.x * vector.x + vector.y * vector.y + vector.z * vector.z);
	vector.x /= w;
	vector.y /= w;
	vector.z /= w;
	return (vector);
}

float	dot(t_vec3 u, t_vec3 v)
{
	return (u.x * v.x + u.y * v.y + u.z * v.z);
}

t_vec3	cross(t_vec3 vec1, t_vec3 vec2)
{
	t_vec3	result;

	result.x = vec1.y * vec2.z - vec1.z * vec2.y;
	result.y = -(vec1.x * vec2.z - vec1.z * vec2.x);
	result.z = vec1.x * vec2.y - vec1.y * vec2.x;
	return (result);
}
