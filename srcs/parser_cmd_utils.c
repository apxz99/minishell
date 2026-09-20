/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser_cmd_utils.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sarayapa <sarayapa@student.42bangkok.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/20 16:45:31 by sarayapa          #+#    #+#             */
/*   Updated: 2026/09/20 17:20:06 by sarayapa         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	count_args(t_token *tokens)
{
	int				n;
	t_token_type	prev_type;

	prev_type = TOKEN_WORD;
	n = 0;
	while (tokens->type != TOKEN_PIPE && tokens->type != TOKEN_END)
	{
		if (tokens->type == TOKEN_WORD)
		{
			if (prev_type == TOKEN_WORD || prev_type == TOKEN_PIPE)
				n++;
		}
		else
			tokens = tokens->next;
		prev_type = tokens->type;
		tokens = tokens->next;
	}
	return (n);
}

t_cmd	*cmd_new(void)
{
	t_cmd	*cmd;

	cmd = ft_calloc(1, sizeof(t_cmd));
	if (!cmd)
		return (NULL);
	return (cmd);
}

void	cmd_add_back(t_cmd **head, t_cmd *new)
{
	t_cmd	*current;

	if (!*head)
	{
		*head = new;
		return ;
	}
	current = *head;
	while (current->next)
		current = current->next;
	current->next = new;
}

void	free_cmds(t_cmd *cmd)
{
	t_cmd	*next;
	int		i;

	while (cmd)
	{
		next = cmd->next;
		i = 0;
		while (cmd->args != NULL && cmd->args[i])
		{
			free(cmd->args[i]);
			i++;
		}
		free(cmd->args);
		free_redirs(cmd->redirs);
		free(cmd);
		cmd = next;
	}
}

void	free_redirs(t_redir *r)
{
	t_redir	*next;

	while (r)
	{
		next = r->next;
		free(r->file);
		free(r);
		r = next;
	}
}
