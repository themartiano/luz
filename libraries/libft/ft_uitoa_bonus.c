/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_uitoa_bonus.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/03/22 11:36:38 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/26 16:00:15 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static void	uitoa_rec(unsigned int nbr, char *str, int *i)
{
	if (nbr <= 9)
	{
		*(str + *i) = nbr + 48;
		*(str + *i + 1) = '\0';
		*i += 1;
	}
	else
	{
		uitoa_rec(nbr / 10, &str[0], &i[0]);
		uitoa_rec(nbr % 10, &str[0], &i[0]);
	}
}

char	*ft_uitoa(unsigned int n)
{
	char	*str;
	int		i;
	int		mem_size;

	mem_size = ft_uintdigits(n) + 1;
	str = malloc(mem_size * sizeof(char));
	if (str == NULL)
		return (NULL);
	str[mem_size - 1] = '\0';
	i = 0;
	uitoa_rec(n, &str[0], &i);
	return (str);
}
