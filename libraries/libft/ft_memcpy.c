/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_memcpy.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <ejuliao-@42lisboa.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/02/15 15:25:38 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/02/24 09:56:07 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

void	*ft_memcpy(void *dest, const void *src, size_t n)
{
	unsigned char	*new_dest;
	unsigned char	*new_src;
	size_t			i;

	if (dest == NULL && src == NULL)
		return (NULL);
	new_dest = (unsigned char *)dest;
	new_src = (unsigned char *)src;
	i = 0;
	while (i < n)
	{
		new_dest[i] = new_src[i];
		i++;
	}
	return (dest);
}
