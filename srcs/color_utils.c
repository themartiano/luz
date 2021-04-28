/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   color_utils.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/28 16:16:46 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/28 16:25:11 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minirt.h"

t_color	set_color(int r, int g, int b)
{
	t_color	result;

	if (r < 0)
		r = 0;
	if (g < 0)
		g = 0;
	if (b < 0)
		b = 0;
	result.r = r;
	result.g = g;
	result.b = b;
	return (result);
}

t_color	sum_colors(t_color clr1, t_color clr2)
{
	t_color	result;

	result.r = clr1.r + clr2.r;
	result.g = clr1.g + clr2.g;
	result.b = clr1.b + clr2.b;
	return (result);
}

t_color	divide_color(t_color clr, float f)
{
	t_color	result;

	result.r = clr.r / f;
	result.g = clr.g / f;
	result.b = clr.b / f;
	return (result);
}