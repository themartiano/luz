/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_ultoa_bonus.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <ejuliao-@42lisboa.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/03/27 13:58:27 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/08 17:41:36 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static void	ultoa_rec(unsigned long nbr, char *str, int *i)
{
	if (nbr <= 9)
	{
		*(str + *i) = nbr + 48;
		*(str + *i + 1) = '\0';
		*i += 1;
	}
	else
	{
		ultoa_rec(nbr / 10, &str[0], &i[0]);
		ultoa_rec(nbr % 10, &str[0], &i[0]);
	}
}

char	*ft_ultoa(unsigned long n)
{
	char	*str;
	int		i;
	int		mem_size;

	mem_size = ft_ulongdigits(n) + 1;
	str = malloc(mem_size * sizeof(char));
	if (str == NULL)
		return (NULL);
	str[mem_size - 1] = '\0';
	i = 0;
	ultoa_rec(n, &str[0], &i);
	return (str);
}
