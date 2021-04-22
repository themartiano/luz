/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   vector_utils.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/21 11:08:33 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/22 12:09:51 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "miniRT.h"

t_vec3	set(float x, float y, float z)
{
	t_vec3	result;

	result.x = x;
	result.y = y;
	result.z = z;
	return (result);
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
