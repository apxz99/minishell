/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strlcpy.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sarayapa <sarayapa@student.42bangkok.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/03 18:06:08 by sarayapa          #+#    #+#             */
/*   Updated: 2025/09/10 16:46:37 by sarayapa         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

size_t	ft_strlcpy(char *dst, const char *src, size_t size)
{
	size_t	i;

	i = 0;
	if (!src || !dst)
		return (0);
	if (size == 0)
		return (ft_strlen(src));
	while ((size - 1 > i) && src[i])
	{
		*(dst + i) = *(src + i);
		i++;
	}
	dst[i] = '\0';
	return (ft_strlen(src));
}
