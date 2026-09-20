/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser_run.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sarayapa <sarayapa@student.42bangkok.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/19 19:01:07 by sarayapa          #+#    #+#             */
/*   Updated: 2026/09/20 15:41:22 by sarayapa         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void	handle_line(t_shell *shell, char *input)
{
	t_token	*token;

	token = tokenize(input);
	if (!token)
	{
		shell->exit_status = 2;
		return ;
	}
	if (syntax_check(token))
	{
		shell->exit_status = 2;
		free_tokens(token);
		return ;
	}
	if (!expand_tokens(token, shell))
	{
		free_tokens(token);
		return ;
	}
	//build_commands(token);
	free_tokens(token);
}
