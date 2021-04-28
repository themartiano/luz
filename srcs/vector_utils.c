/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   vector_utils.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/21 11:08:33 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/28 16:19:13 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minirt.h"

float	length_sqrt(t_vec3 v)
{
	return (v.x * v.x + v.y * v.y + v.z * v.z);
}

float	length(t_vec3 v)
{
	return (sqrt(length_sqrt(v)));
}

t_vec3	set(float x, float y, float z)
{
	t_vec3	result;

	result.x = x;
	result.y = y;
	result.z = z;
	return (result);
}

t_vec3	scale(t_vec3 v, float f)
{
	return (set(f * v.x, f * v.y, f * v.z));
}

t_vec3	sum(t_vec3 vec1, t_vec3 vec2)
{
	t_vec3	result;

	result.x = vec1.x + vec2.x;
	result.y = vec1.y + vec2.y;
	result.z = vec1.z + vec2.z;
	return (result);
}

t_vec3	sub(t_vec3 vec1, t_vec3 vec2)
{
	t_vec3	result;

	result.x = vec1.x - vec2.x;
	result.y = vec1.y - vec2.y;
	result.z = vec1.z - vec2.z;
	return (result);
}

t_vec3	normalize(t_vec3 v)
{
	float	w;

	w = length(v);
	v.x /= w;
	v.y /= w;
	v.z /= w;
	return (v);
}
