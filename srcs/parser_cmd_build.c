/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser_cmd_build.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sarayapa <sarayapa@student.42bangkok.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/20 14:44:33 by sarayapa          #+#    #+#             */
/*   Updated: 2026/09/20 17:51:31 by sarayapa         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	collect_args(t_cmd *cmd, t_token *tokens)
{
	int	n;
	int	i;

	n = count_args(tokens);
	i = 0;
	if (cmd->args == NULL)
	{
		cmd->args = ft_calloc(n + 1, sizeof(char *));
		if (!cmd->args)
			return (1);
	}
	while (cmd->args[i] != NULL)
		i++;
	cmd->args[i] = ft_strdup(tokens->value);
	if (cmd->args[i] == NULL)
		return (1);
	return (0);
}

t_redir	*redir_new(t_token *token)
{
	t_redir	*node;

	node = ft_calloc(1, sizeof(t_redir));
	if (!node)
		return (NULL);
	if (token->type == TOKEN_REDIR_IN)
		node->type = REDIR_IN;
	else if (token->type == TOKEN_REDIR_OUT)
		node->type = REDIR_OUT;
	else if (token->type == TOKEN_REDIR_APPEND)
		node->type = REDIR_APPEND;
	else
		node->type = REDIR_HEREDOC;
	node->file = ft_strdup(token->next->value);
	if (!node->file)
	{
		free(node);
		return (NULL);
	}
	return (node);
}
int	collect_redir(t_cmd *cmd, t_token *token)
{
	t_redir	*current;
	t_redir *node;

	node = redir_new(token);
	if (!node)
		return 1;
	if (cmd->redirs == NULL)
		cmd->redirs = node;
	else
	{
		current = cmd->redirs;
		while (current->next)
			current = current->next;
		current->next = node;
	}
	return (0);
}

t_cmd	*build_fail(t_cmd *head, t_cmd *cmd)
{
	free_cmds(head);
	free_cmds(cmd);
	return (NULL);
}

t_cmd	*build_commands(t_token *tokens)
{
	t_cmd	*cmd;
	t_cmd	*head;

	cmd = NULL;
	head = NULL;
	while (tokens->type != TOKEN_END)
	{
		if (tokens->type == TOKEN_WORD)
		{
			if (cmd == NULL)
				cmd = cmd_new();
			if (cmd == NULL)
				return (build_fail(head, cmd));
			if (collect_args(cmd, tokens))
				return (build_fail(head, cmd));	
		}
		else if (tokens->type == TOKEN_PIPE)
		{
			cmd_add_back(&head, cmd);
			cmd = NULL;
		}
		else
		{
			if (cmd == NULL)
				cmd = cmd_new();
			if (cmd == NULL)
				return (build_fail(head, cmd));
			if (collect_redir(cmd, tokens))
				return (build_fail(head, cmd));
			tokens = tokens->next;
		}
		tokens = tokens->next;
	}
	if (cmd)
		cmd_add_back(&head, cmd);
	return (head);
}
