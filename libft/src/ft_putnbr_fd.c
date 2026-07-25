/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_putnbr_fd.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sarayapa <sarayapa@student.42bangkok.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/10 11:50:36 by sarayapa          #+#    #+#             */
/*   Updated: 2025/09/14 16:08:09 by sarayapa         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

void	ft_putnbr_fd(int nbr, int fd)
{
	if (fd < 0)
		return ;
	if (nbr < 0)
	{
		ft_putchar_fd('-', fd);
		if (nbr == -2147483648)
		{
			ft_putnbr_fd(2, fd);
			nbr = 147483648;
		}
		else
			nbr = -nbr;
	}
	if (nbr / 10)
		ft_putnbr_fd(nbr / 10, fd);
	ft_putchar_fd(nbr % 10 + '0', fd);
}

//void	ft_putnbr_fd(int nbr, int fd)
//{
//	char	*temp;

//	temp = ft_itoa(nbr);
//	if (!temp)
//		return ;
//	ft_putstr_fd(temp, fd);
//	free(temp);
//}