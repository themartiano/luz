/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_lstmap.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/02/18 15:34:54 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/26 16:00:15 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

t_list	*ft_lstmap(t_list *lst, void *(*f)(void *), void (*del)(void *))
{
	t_list	*new_lst;
	t_list	*strt_lst;

	strt_lst = NULL;
	while (lst != NULL)
	{
		new_lst = ft_lstnew((*f)(lst->content));
		if (new_lst == NULL)
		{
			ft_lstclear(&strt_lst, (*del));
			return (NULL);
		}
		ft_lstadd_back(&strt_lst, new_lst);
		lst = lst->next;
	}
	return (strt_lst);
}
