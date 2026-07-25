/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_lstmap_bonus.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sarayapa <sarayapa@student.42bangkok.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/14 10:35:22 by sarayapa          #+#    #+#             */
/*   Updated: 2025/09/14 13:36:05 by sarayapa         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

t_list	*ft_lstmap(t_list *lst, void *(*f)(void *), void (*del)(void *))
{
	t_list	*temp_content;
	t_list	*temp_list;

	temp_list = NULL;
	if (!lst || !f || !del)
		return (temp_list);
	while (lst)
	{
		temp_content = ft_lstnew(f(lst -> content));
		if (!temp_content)
		{
			ft_lstclear(&temp_list, del);
			return (NULL);
		}
		ft_lstadd_back(&temp_list, temp_content);
		lst = lst -> next;
	}
	return (temp_list);
}
