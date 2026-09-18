/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser_expansion_quote.c                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sarayapa <sarayapa@student.42bangkok.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/09 15:30:38 by sarayapa          #+#    #+#             */
/*   Updated: 2026/09/18 21:02:43 by sarayapa         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/*
update_quote - Update quote state when a quote is found.
Return: Updated quote state.
*/
t_quote	update_quote(t_quote quote, char c)
{
	if (c == '\'' && quote == Q_NONE)
		return (Q_SINGLE);
	if (c == '\'' && quote == Q_SINGLE)
		return (Q_NONE);
	if (c == '"' && quote == Q_NONE)
		return (Q_DOUBLE);
	if (c == '"' && quote == Q_DOUBLE)
		return (Q_NONE);
	return (quote);
}

int	quote_step(t_quote *quote, char c)
{
	t_quote	next;

	next = update_quote(*quote, c);
	if (next == *quote)
		return (0);
	*quote = next;
	return (1);
}

int	strip_quote(t_token *token)
{
	t_quote	quote;
	char	*result;
	int		i;

	quote = Q_NONE;
	result = ft_strdup("");
	if (!result)
		return (1);
	i = 0;
	while (token->value[i])
	{
		if (quote_step(&quote, token->value[i]))
			i++;
		else if (append_word_char(token->value, &i, &result))
			return (free(result), 1);
	}
	free(token->value);
	token->value = result;
	return (0);
}
