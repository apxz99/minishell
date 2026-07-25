/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_itoa.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sarayapa <sarayapa@student.42bangkok.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/09 16:40:13 by sarayapa          #+#    #+#             */
/*   Updated: 2025/09/17 20:25:33 by sarayapa         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

int	ft_count(long i)
{
	int	res;

	res = 0;
	if (i < 0)
	{
		res++;
		i = -i;
	}
	if (i == 0)
		res++;
	while (i != 0)
	{
		i = i / 10;
		res++;
	}
	return (res);
}

char	*malloc_itoa(long *n, int len)
{
	char	*dst;

	dst = (char *)malloc((len + 1) * sizeof(char));
	if (!dst)
		return (NULL);
	if (*n == 0)
	{
		dst[0] = '0';
		dst[1] = '\0';
		return (dst);
	}
	if (*n < 0)
	{
		dst[0] = '-';
		*n = -*n;
	}
	dst[len] = '\0';
	return (dst);
}

char	*ft_itoa(int n)
{
	int		len;
	char	*res;
	long	nbr;

	nbr = n;
	len = ft_count(nbr);
	res = malloc_itoa(&nbr, len);
	len--;
	while (nbr > 0)
	{
		res[len] = '0' + (nbr % 10);
		nbr = nbr / 10;
		len--;
	}
	return (res);
}
