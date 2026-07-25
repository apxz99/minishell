/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_atoi.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sarayapa <sarayapa@student.42bangkok.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/31 03:34:16 by sarayapa          #+#    #+#             */
/*   Updated: 2026/02/13 06:24:02 by sarayapa         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

int	ft_atoi(const char *n)
{
	int		neg;
	long	i;
	int		res;

	i = 0;
	res = 0;
	neg = 1;
	while (n[i] == ' ' || (n[i] >= '\t' && n[i] <= '\r'))
		i++;
	if (n[i] == '-' || n[i] == '+')
		neg = ',' - n[i++];
	while (ft_isdigit(n[i]) && n[i])
	{
		res = (res * 10) + (n[i] - '0');
		i++;
	}
	return (res * neg);
}
