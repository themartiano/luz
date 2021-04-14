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

unsigned long	rgba_to_hex(t_color rgba)
{
	rgba.a = 0;
	return (rgba.a << 24 | rgba.r << 16 | rgba.g << 8 | rgba.b);
}

float	dot(t_xyz u, t_xyz v)
{
	return ((u.x * v.x) + (u.y * v.y) + (u.z * u.y));
}
