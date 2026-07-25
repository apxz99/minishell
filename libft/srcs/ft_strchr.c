/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strchr.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sarayapa <sarayapa@student.42bangkok.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/04 18:34:27 by sarayapa          #+#    #+#             */
/*   Updated: 2025/09/13 20:56:14 by sarayapa         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

char	*ft_strchr(const char *s, int c)
{
	size_t	i;
	char	*temp;

	if (!s)
		return (NULL);
	temp = (char *)s;
	i = 0;
	while (temp[i])
	{
		if (temp[i] == (char)c)
			return (temp + i);
		i++;
	}
	if ((char)c == '\0')
		return (temp + i);
	return (NULL);
}
