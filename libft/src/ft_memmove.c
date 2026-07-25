/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_memmove.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sarayapa <sarayapa@student.42bangkok.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/31 10:09:14 by sarayapa          #+#    #+#             */
/*   Updated: 2025/09/10 19:08:15 by sarayapa         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

void	*ft_memmove(void *d, const void *s, size_t n)
{
	if (!d && !s)
	{
		return (NULL);
	}
	while (d < s)
	{
		return (ft_memcpy(d, s, n));
	}
	while (d > s && n--)
	{
		((unsigned char *)d)[n] = ((unsigned char *)s)[n];
	}
	return (d);
}
