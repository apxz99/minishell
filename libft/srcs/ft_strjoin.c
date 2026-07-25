/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strjoin.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sarayapa <sarayapa@student.42bangkok.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/08 10:02:20 by sarayapa          #+#    #+#             */
/*   Updated: 2025/09/08 10:02:20 by sarayapa         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

char	*ft_strjoin(char const *s1, char const *s2)
{
	char	*res;
	size_t	i;
	size_t	j;
	size_t	k;

	i = 0;
	j = 0;
	k = 0;
	res = malloc(ft_strlen(s1) + ft_strlen(s2) + 1);
	while (j < ft_strlen(s1))
	{
		res[i] = s1[j];
		i++;
		j++;
	}
	while (k < ft_strlen(s2))
	{
		res[i] = s2[k];
		i++;
		k++;
	}
	res[i] = '\0';
	return (res);
}
