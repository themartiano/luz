/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strnstr.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <ejuliao-@42lisboa.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/02/11 15:08:55 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/02/18 11:32:46 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

char	*ft_strnstr(const char *big, const char *little, size_t len)
{
	size_t	i;
	size_t	j;
	size_t	size;
	char	*new_big;

	i = 0;
	new_big = (char *)big;
	size = ft_strlen(little);
	if (size == 0 || big == little)
		return (new_big);
	while (new_big[i] != '\0' && i < len)
	{
		j = 0;
		while (new_big[i + j] != '\0' && little[j] != '\0'
		&& new_big[i + j] == little[j] && i + j < len)
			j++;
		if (j == size)
			return (new_big + i);
		i++;
	}
	return (0);
}
