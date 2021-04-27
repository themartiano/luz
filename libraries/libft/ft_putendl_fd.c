/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_putendl_fd.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/02/10 10:51:51 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/27 10:27:11 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

void	ft_putendl_fd(char *s, int fd)
{
	int	i;
	int	rtrn;

	if (s == NULL)
		return ;
	i = 0;
	while (s[i])
	{
		rtrn = write(fd, &s[i], 1);
		i++;
	}
	rtrn = write(fd, "\n", 1);
	(void)rtrn;
}
