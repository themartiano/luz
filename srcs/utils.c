/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/10 14:51:57 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/22 16:15:19 by ejuliao-         ###   ########.fr       */
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

void	put_pixel(t_img *img_data, int x, int y, int color)
{
	char	*dst;

	dst = img_data->addr + (y * img_data->line_length + x
			* (img_data->bits_per_pixel / 8));
	*(unsigned int *)dst = color;
}

t_color	get_pixel(t_img *img_data, int x, int y)
{
	char	*dst;

	dst = img_data->addr + (y * img_data->line_length + x
			* (img_data->bits_per_pixel / 8));
	return (hex_to_rgba(*(unsigned int *)dst));
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
