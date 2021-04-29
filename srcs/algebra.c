/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   algebra.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/29 09:01:00 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/29 15:03:55 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minirt.h"

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

t_vec3	mul(t_vec3 vec, float m)
{
	t_vec3	result;

	result.x = vec.x * m;
	result.y = vec.y * m;
	result.z = vec.z * m;
	return (result);
}
