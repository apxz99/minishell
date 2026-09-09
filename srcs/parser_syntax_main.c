/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser_syntax_main.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sarayapa <sarayapa@student.42bangkok.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/04 15:01:36 by sarayapa          #+#    #+#             */
/*   Updated: 2026/09/09 18:45:46 by sarayapa         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	syntax_check(t_token *token)
{
	t_token	*prev;

	prev = NULL;
	while (token->type != TOKEN_END)
	{
		if (token->type == TOKEN_PIPE)
		{
			if (prev == NULL
				|| prev->type != TOKEN_WORD
				|| token->next->type != TOKEN_WORD)
				return (print_error(ERR_SYNTAX, token->value), 1);
		}
		else if (token->type == TOKEN_REDIR_IN
			|| token->type == TOKEN_REDIR_OUT
			|| token->type == TOKEN_REDIR_APPEND
			|| token->type == TOKEN_HEREDOC)
		{
			if (token->next->type != TOKEN_WORD)
				return (print_error(ERR_SYNTAX, token->value), 1);
		}
		prev = token;
		token = token->next;
	}
	return (0);
}
