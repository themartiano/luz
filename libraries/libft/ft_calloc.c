/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_calloc.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <ejuliao-@42lisboa.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/02/15 17:25:16 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/02/17 16:16:52 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

void	*ft_calloc(size_t nmemb, size_t size)
{
	char *new_mem;

	new_mem = malloc(nmemb * size);
	if (new_mem == NULL)
		return (NULL);
	ft_bzero(new_mem, nmemb * size);
	return (new_mem);
}
