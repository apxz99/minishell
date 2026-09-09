/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser_lexer_utils.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sarayapa <sarayapa@student.42bangkok.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/01 16:11:42 by sarayapa          #+#    #+#             */
/*   Updated: 2026/09/09 17:14:34 by sarayapa         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"
/*
is_space - Check if a character is whitespace.
Return: 1 if space or tab, 0 otherwise.
*/
int	is_space(char c)
{
	return (c == ' ' || c == '\t');
}

/*
is_operator_char - Check if a character is an operator.
Return: 1 if '|', '<', or '>', 0 otherwise.
*/
int	is_operator_char(char c)
{
	return (c == '|' || c == '<' || c == '>');
}

/*
is_quote - Check if a character is a quote.
Return: 1 if single or double quote, 0 otherwise.
*/
int	is_quote(char c)
{
	return (c == '\'' || c == '"');
}

/*
find_word_end - Find the end of a word while handling quotes.
Return: 1 if quote is unclosed, 0 otherwise.
*/
int	find_word_end(const char *s, int start, int *end)
{
	int		i;
	char	quote;

	i = start;
	quote = 0;
	while (s[i])
	{
		if (quote)
		{
			if (s[i] == quote)
				quote = 0;
		}
		else if (is_quote(s[i]))
			quote = s[i];
		else if (is_space(s[i]) || is_operator_char(s[i]))
			break ;
		i++;
	}
	*end = i;
	if (quote != 0)
		return (1);
	return (0);
}
