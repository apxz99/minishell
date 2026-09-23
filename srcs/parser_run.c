/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser_run.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sarayapa <sarayapa@student.42bangkok.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/19 19:01:07 by sarayapa          #+#    #+#             */
/*   Updated: 2026/09/23 18:06:44 by sarayapa         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void	handle_line(t_shell *shell, char *input)
{
	t_token	*token;
	t_cmd	*cmds;

	token = tokenize(input);
	if (!token || syntax_check(token))
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
	cmds = build_commands(token);
	free_tokens(token);
	if (!cmds)
		return ;
	execute(cmds, shell);
	free_cmds(cmds);
}
