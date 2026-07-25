/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strnstr.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sarayapa <sarayapa@student.42bangkok.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/05 17:01:45 by sarayapa          #+#    #+#             */
/*   Updated: 2026/02/12 16:06:07 by sarayapa         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

char	*ft_strnstr(const char *dst, const char *str, size_t n)
{
	size_t	len;

	if (!str || !dst)
		return (NULL);
	if (!*str)
		return ((char *)dst);
	len = ft_strlen(str);
	if (!n)
		return (NULL);
	while (*dst != '\0' && len <= n)
	{
		if (*dst == *str && (ft_strncmp(dst, str, len) == 0))
			return ((char *)dst);
		dst++;
		n--;
	}
	return (NULL);
}
