/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minishell_token.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sarayapa <sarayapa@student.42bangkok.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/01 16:12:52 by sarayapa          #+#    #+#             */
/*   Updated: 2026/09/18 15:10:06 by sarayapa         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/*
new_token - Create a new token with a value and type.
Return: New token, or NULL on failure.
*/
t_token	*new_token(char const *value, t_token_type type)
{
	t_token	*token;

	token = malloc(sizeof(t_token));
	if (!token)
		return (NULL);
	token->value = ft_strdup(value);
	if (!token->value)
	{
		free(token);
		return (NULL);
	}
	token->type = type;
	token->heredoc_quoted = 0;
	token->next = NULL;
	return (token);
}

/*
token_add_back - Add a token to the end of the token list.
Return: Nothing.
*/
void	token_add_back(t_token **head, t_token *new)
{
	t_token	*last;

	if (!*head)
	{
		*head = new;
		return ;
	}
	last = *head;
	while (last->next)
		last = last->next;
	last->next = new;
}

/*
free_tokens - Free the entire token list.
Return: Nothing.
*/
void	free_tokens(t_token *token)
{
	t_token	*next;

	while (token)
	{
		next = token->next;
		free(token->value);
		free(token);
		token = next;
	}
}
