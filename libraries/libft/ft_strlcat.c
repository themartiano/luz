/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strlcat.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/02/15 12:00:59 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/08 17:39:39 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

size_t	ft_strlcat(char *dst, const char *src, size_t size)
{
	char		*i_dst;
	const char	*i_src;
	size_t		dst_len;
	size_t		i;

	i_dst = dst;
	i_src = src;
	i = size;
	while (i-- != 0 && *i_dst != '\0')
		i_dst++;
	dst_len = i_dst - dst;
	i = size - dst_len;
	if (i == 0)
		return (dst_len + ft_strlen(src));
	while (*src != '\0')
	{
		if (i > 1)
		{
			*i_dst++ = *src;
			i--;
		}
		src++;
	}
	*i_dst = '\0';
	return (dst_len + (src - i_src));
}
