/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_itoa.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/02/09 13:56:38 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/26 16:00:15 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static void	itoa_rec(unsigned int nbr, char *str, int *i)
{
	if (nbr <= 9)
	{
		*(str + *i) = nbr + 48;
		*(str + *i + 1) = '\0';
		*i += 1;
	}
	else
	{
		itoa_rec(nbr / 10, &str[0], &i[0]);
		itoa_rec(nbr % 10, &str[0], &i[0]);
	}
}

char	*ft_itoa(int n)
{
	char			*str;
	int				i;
	int				mem_size;
	unsigned int	nbr;

	i = 0;
	if (n < 0)
	{
		nbr = -n;
		i = 1;
	}
	else
		nbr = n;
	mem_size = ft_intdigits(n) + 1 + i;
	str = malloc(mem_size * sizeof(char));
	if (str == NULL)
		return (NULL);
	if (i == 1)
		str[0] = '-';
	str[mem_size - 1] = '\0';
	itoa_rec(nbr, &str[0], &i);
	return (str);
}
