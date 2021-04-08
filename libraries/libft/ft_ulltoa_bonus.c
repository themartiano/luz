/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_ulltoa_bonus.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <ejuliao-@42lisboa.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/03/27 14:26:17 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/03/27 14:27:02 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static void	ulltoa_rec(unsigned long long nbr, char *str, int *i)
{
	if (nbr <= 9)
	{
		*(str + *i) = nbr + 48;
		*(str + *i + 1) = '\0';
		*i += 1;
	}
	else
	{
		ulltoa_rec(nbr / 10, &str[0], &i[0]);
		ulltoa_rec(nbr % 10, &str[0], &i[0]);
	}
}

char		*ft_ulltoa(unsigned long long n)
{
	char	*str;
	int		i;
	int		mem_size;

	mem_size = ft_ulonglongdigits(n) + 1;
	str = malloc(mem_size * sizeof(char));
	if (str == NULL)
		return (NULL);
	str[mem_size - 1] = '\0';
	i = 0;
	ulltoa_rec(n, &str[0], &i);
	return (str);
}
