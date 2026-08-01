/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser_lexer_utils.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sarayapa <sarayapa@student.42bangkok.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/01 16:11:42 by sarayapa          #+#    #+#             */
/*   Updated: 2026/08/01 16:18:44 by sarayapa         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

#include "minishell.h"

int	is_space(char c)
{
	return (c == ' ' || c == '\t');
}

int	is_operator_char(char c)
{
	return (c == '|' || c == '<' || c == '>');
}

int	is_quote(char c)
{
	return (c == '\'' || c == '"');
}

/*
** find_word_end:
**	หาตำแหน่ง index ที่ word จบ (ยังไม่ตัด string จริง)
**	เดินทีละตัวอักษร ถ้าเจอ quote ให้ "ล็อก" โหมด quote ไว้
**	ระหว่างล็อกอยู่ ห้ามหยุดแม้เจอ space หรือ | < >
**	คืนค่า 1 = เจอ error (quote ไม่ปิด), 0 = ปกติ
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
