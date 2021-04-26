/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_last_index_of.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/26 16:10:30 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/26 16:14:14 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

int	ft_last_indexof(const char *s, char c)
{
	int	index;

	index = ft_strlen(s);
	while (--index >= 0)
	{
		if (s[index] == c)
			return (index);
	}
	return (-1);
}
