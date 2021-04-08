/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_memmove.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <ejuliao-@42lisboa.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/02/15 16:49:01 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/02/18 19:10:09 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

void	*ft_memmove(void *dest, const void *src, size_t n)
{
	char	*new_dest;
	char	*new_src;

	new_dest = (char *)dest;
	new_src = (char *)src;
	if (new_src < new_dest)
	{
		while (n--)
		{
			new_dest[n] = new_src[n];
		}
	}
	else if (new_src != NULL)
	{
		ft_memcpy(new_dest, new_src, n);
	}
	return (dest);
}
