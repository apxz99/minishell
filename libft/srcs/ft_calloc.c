/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_calloc.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sarayapa <sarayapa@student.42bangkok.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/05 20:51:15 by sarayapa          #+#    #+#             */
/*   Updated: 2025/09/13 23:24:37 by sarayapa         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

void	*ft_calloc(size_t n, size_t size)
{
	void	*temp;

	if (n == 0 || size == 0)
		return (malloc(0));
	if (n > INT_MAX || size > INT_MAX || n * size > INT_MAX)
		return (NULL);
	temp = malloc(n * size);
	if (!temp)
		return (NULL);
	ft_bzero((unsigned char *)temp, n * size);
	return (temp);
}
