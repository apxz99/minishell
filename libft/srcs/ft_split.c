/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_split.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sarayapa <sarayapa@student.42bangkok.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/08 18:58:25 by sarayapa          #+#    #+#             */
/*   Updated: 2025/09/13 21:45:56 by sarayapa         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static	int	ft_countsplit(const char *str, char c)
{
	int	count;
	int	x;

	count = 0;
	x = 0;
	while (*str)
	{
		if (*str != c && x == 0)
		{
			x = 1;
			count++;
		}
		else if (*str == c)
			x = 0;
		str++;
	}
	return (count);
}

static char	*ft_fillword(const char *s, int s_word, int n)
{
	char	*str;
	size_t	i;

	i = 0;
	str = malloc((n - s_word + 1) * sizeof(char));
	if (!str)
		return (NULL);
	while (s_word < n)
	{
		str[i] = s[s_word];
		s_word++;
		i++;
	}
	str[i] = '\0';
	return (str);
}

static void	*ft_free(char **strs, int count)
{
	int	i;

	i = 0;
	while (i < count)
	{
		free(strs[i]);
		i++;
	}
	free(strs);
	return (NULL);
}

char	**ft_split(char const *s, char c)
{
	size_t	i;
	int		j;
	int		s_word;
	char	**dst;

	i = -1;
	j = 0;
	s_word = -1;
	dst = ft_calloc((ft_countsplit(s, c) + 1), sizeof(char *));
	if (!dst)
		return (NULL);
	while (++i <= ft_strlen(s))
	{
		if (s[i] != c && s_word < 0)
			s_word = i;
		else if ((s[i] == c || i == ft_strlen(s)) && s_word >= 0)
		{
			dst[j] = ft_fillword(s, s_word, i);
			if (!dst[j])
				return (ft_free(dst, j));
			s_word = -1;
			j++;
		}
	}
	return (dst);
}
