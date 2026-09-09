/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser_expansion_utils.c                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sarayapa <sarayapa@student.42bangkok.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/09 15:30:22 by sarayapa          #+#    #+#             */
/*   Updated: 2026/09/09 19:31:10 by sarayapa         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/*
append_char - Append one character to the end of a string.
Return: New string, or NULL if allocation fails.
*/
char	*append_char(char *str, char c)
{
	char	*new_str;
	int		len;

	len = ft_strlen(str);
	new_str = malloc(sizeof(char) * (len + 2));
	if (!new_str)
		return (NULL);
	ft_memcpy(new_str, str, len);
	new_str[len] = c;
	new_str[len + 1] = '\0';
	free(str);
	return (new_str);
}

/*
append_string - Append a string to the end of another string.
Return: New string, or NULL if allocation fails.
*/
char	*append_string(char *str, char *append)
{
	char	*new_str;
	int		len1;
	int		len2;

	len1 = ft_strlen(str);
	len2 = ft_strlen(append);
	new_str = malloc(sizeof(char) * (len1 + len2 + 1));
	if (!new_str)
		return (NULL);
	ft_memcpy(new_str, str, len1);
	ft_memcpy(new_str + len1, append, len2);
	new_str[len1 + len2] = '\0';
	free(str);
	return (new_str);
}

/*
is_var_char - Check if a character can be part of variable name.
Return: 1 if valid, 0 otherwise.
*/
int	is_var_char(char c)
{
	if ((c >= 'A' && c <= 'Z')
		|| (c >= 'a' && c <= 'z')
		|| (c >= '0' && c <= '9')
		|| c == '_')
		return (1);
	return (0);
}

int	append_word_char(char *value, int *i, char **result)
{
	*result = append_char(*result, value[*i]);
	if (!*result)
		return (1);
	(*i)++;
	return (0);
}
