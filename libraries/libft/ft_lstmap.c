/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_lstmap.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <ejuliao-@42lisboa.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/02/18 15:34:54 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/02/18 18:32:49 by ejuliao-         ###   ########.fr       */
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
		if ((new_lst = ft_lstnew((*f)(lst->content))) == NULL)
		{
			ft_lstclear(&strt_lst, (*del));
			return (NULL);
		}
		ft_lstadd_back(&strt_lst, new_lst);
		lst = lst->next;
	}
	return (strt_lst);
}
