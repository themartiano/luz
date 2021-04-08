/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_lstadd_back.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <ejuliao-@42lisboa.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/02/18 14:22:33 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/02/18 18:20:37 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

void	ft_lstadd_back(t_list **lst, t_list *new)
{
	t_list	*tmp_lst;

	tmp_lst = ft_lstlast(*lst);
	if (tmp_lst != NULL)
	{
		tmp_lst->next = new;
	}
	else
	{
		*lst = new;
	}
}
