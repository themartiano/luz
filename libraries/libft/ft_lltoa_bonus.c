/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_lltoa_bonus.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/03/27 14:30:08 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/08 16:55:00 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static void	lltoa_rec(unsigned long long nbr, char *str, int *i)
{
	if (nbr <= 9)
	{
		*(str + *i) = nbr + 48;
		*(str + *i + 1) = '\0';
		*i += 1;
	}
	else
	{
		lltoa_rec(nbr / 10, &str[0], &i[0]);
		lltoa_rec(nbr % 10, &str[0], &i[0]);
	}
}

char	*ft_lltoa(long long n)
{
	char				*str;
	int					i;
	int					mem_size;
	unsigned long long	nbr;

	i = 0;
	if (n < 0)
	{
		nbr = -n;
		i = 1;
	}
	else
		nbr = n;
	mem_size = ft_longlongdigits(n) + 1 + i;
	str = malloc(mem_size * sizeof(char));
	if (str == NULL)
		return (NULL);
	if (i == 1)
		str[0] = '-';
	str[mem_size - 1] = '\0';
	lltoa_rec(nbr, &str[0], &i);
	return (str);
}
