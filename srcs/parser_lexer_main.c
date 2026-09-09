/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser_lexer_main.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sarayapa <sarayapa@student.42bangkok.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/25 16:26:17 by sarayapa          #+#    #+#             */
/*   Updated: 2026/09/09 15:44:06 by sarayapa         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/*
read_operator - Read an operator and create its token.
Return: New operator token, or NULL on failure.
*/
t_token	*read_operator(const char *input, int *i)
{
	if (input[*i] == '|')
		return (*i += 1, new_token("|", TOKEN_PIPE));
	else if (input[*i] == '<' && input[*i + 1] == '<')
		return (*i += 2, new_token("<<", TOKEN_HEREDOC));
	else if (input[*i] == '<')
		return (*i += 1, new_token("<", TOKEN_REDIR_IN));
	else if (input[*i] == '>' && input[*i + 1] == '>')
		return (*i += 2, new_token(">>", TOKEN_REDIR_APPEND));
	else
		return (*i += 1, new_token(">", TOKEN_REDIR_OUT));
	return (NULL);
}

/*
read_word_token - Read a word and create a word token.
Return: New word token, or NULL on failure.
*/
t_token	*read_word_token(const char *input, int *i)
{
	int		end;
	char	*value;
	t_token	*token;

	if (find_word_end(input, *i, &end))
	{
		print_error(ERR_UNCLOSED_QUOTE, NULL);
		return (NULL);
	}
	value = ft_substr(input, *i, end - *i);
	if (!value)
		return (NULL);
	token = new_token(value, TOKEN_WORD);
	free(value);
	*i = end;
	return (token);
}

/*
tokenize - Convert the input string into a token list.
Return: Token list, or NULL on error.
*/
t_token	*tokenize(char *input)
{
	int		i;
	t_token	*head;
	t_token	*next;

	head = NULL;
	i = 0;
	while (input[i])
	{
		while (is_space(input[i]))
			i++;
		if (!input[i])
			break ;
		if (is_operator_char(input[i]))
			next = read_operator(input, &i);
		else
			next = read_word_token(input, &i);
		if (!next)
			return (free_tokens(head), NULL);
		token_add_back(&head, next);
	}
	next = new_token("", TOKEN_END);
	if (!next)
		return (free_tokens(head), NULL);
	return (token_add_back(&head, next), head);
}
