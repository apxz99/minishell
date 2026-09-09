/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser_expansion_main.c                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sarayapa <sarayapa@student.42bangkok.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/09 15:30:25 by sarayapa          #+#    #+#             */
/*   Updated: 2026/09/09 19:33:00 by sarayapa         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"	

t_token	*expand_tokens(t_token *tokens, t_shell *shell)
{
	t_token	*current;
	t_token	*prev;

	current = tokens;
	prev = NULL;
	while (current->type != TOKEN_END)
	{
		if (current->type == TOKEN_WORD)
		{
			if (!prev || prev->type != TOKEN_HEREDOC)
			{
				if (expand_word(current, shell))
					return (NULL);
			}
		}
		prev = current;
		current = current->next;
	}
	return (tokens);
}

/*
expand_word - Expand variables and remove quotes from one WORD token.
Return: 0 on success, 1 on allocation error.
*/
int	expand_word(t_token *token, t_shell *shell)
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
		if (is_quote(token->value[i]))
			quote = update_quote(quote, token->value[i++]);
		else if (token->value[i] == '$' && quote != Q_SINGLE)
		{
			if (expand_dollar(token->value, &i, &result, shell))
				return (free(result), 1);
		}
		else if (append_word_char(token->value, &i, &result))
			return (free(result), 1);
	}
	free(token->value);
	token->value = result;
	return (0);
}

int	expand_status(int *i, char **result, t_shell *shell)
{
	char	*status;

	status = ft_itoa(shell->exit_status);
	if (!status)
		return (1);
	*result = append_string(*result, status);
	free(status);
	if (!*result)
		return (1);
	*i += 2;
	return (0);
}

int	expand_variable(char *value, int *i, char **result, t_shell *shell)
{
	char	*key;
	char	*env_value;
	int		start;
	int		len;

	start = *i + 1;
	len = 0;
	while (is_var_char(value[start + len]))
		len++;
	key = ft_substr(value, start, len);
	if (!key)
		return (1);
	env_value = get_env(shell->env, key);
	free(key);
	if (env_value)
		*result = append_string(*result, env_value);
	if (!env_value)
		*result = append_string(*result, "");
	if (!*result)
		return (1);
	*i = start + len;
	return (0);
}

/*
expand_dollar - Expand variable starting at '$'.
Return: 0 on success, 1 on error.
*/
int	expand_dollar(char *value, int *i, char **result, t_shell *shell)
{
	if (value[*i + 1] == '?')
		return (expand_status(i, result, shell));
	if (!is_var_char(value[*i + 1]))
	{
		*result = append_char(*result, '$');
		if (!*result)
			return (1);
		(*i)++;
		return (0);
	}
	return (expand_variable(value, i, result, shell));
}
