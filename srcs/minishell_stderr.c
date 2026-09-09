/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minishell_stderr.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sarayapa <sarayapa@student.42bangkok.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/09 14:48:42 by sarayapa          #+#    #+#             */
/*   Updated: 2026/09/09 14:58:08 by sarayapa         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/*
print_error - Print an error message to stderr.
Return: Nothing.
*/
void	print_error(t_error error, char *value)
{
	if (error == ERR_SYNTAX)
	{
		ft_putstr_fd("minishell: syntax error near unexpected token `", 2);
		ft_putstr_fd(value, 2);
		ft_putstr_fd("'\n", 2);
	}
	else if (error == ERR_UNCLOSED_QUOTE)
		ft_putstr_fd("minishell: syntax error: unclosed quote\n", 2);
}
