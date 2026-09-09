/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser_expansion_quote.c                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sarayapa <sarayapa@student.42bangkok.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/09 15:30:38 by sarayapa          #+#    #+#             */
/*   Updated: 2026/09/09 18:38:29 by sarayapa         ###   ########.fr       */
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
