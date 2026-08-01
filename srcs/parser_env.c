/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser_env.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sarayapa <sarayapa@student.42bangkok.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/28 11:46:45 by sarayapa          #+#    #+#             */
/*   Updated: 2026/08/01 16:52:23 by sarayapa         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	init_env(t_shell *shell, char **envp)
{
	t_env	*node;
	int		i;

	i = 0;
	while (envp[i])
	{
		node = new_env(envp[i]);
		if (!node)
			return (1);
		env_addback(&shell->env, node);
		i++;
	}
	return (0);
}

t_env	*new_env(char *envp)
{
	t_env	*node;
	char	*equal;

	node = ft_calloc(1, sizeof(t_env));
	equal = ft_strchr(envp, '=');
	if (!equal)
	{
		free(node);
		return (NULL);
	}
	node->key = ft_substr(envp, 0, equal - envp);
	if (!node->key)
	{
		free(node);
		return (NULL);
	}
	node->value = ft_strdup(equal + 1);
	if (!node->value)
	{
		free(node->key);
		free(node);
		return (NULL);
	}
	node->next = NULL;
	return (node);
}

t_env	*env_last(t_env *lst)
{
	while (lst && lst->next)
		lst = lst->next;
	return (lst);
}

void	env_addback(t_env **lst, t_env *new)
{
	t_env	*last;

	last = env_last(*lst);
	if (!last)
	{
		*lst = new;
		return ;
	}
	last->next = new;
}

char	*get_env(t_env *env, char *key)
{
	t_env	*temp;

	temp = env;
	while (temp)
	{
		if (ft_strncmp(temp->key, key, ft_strlen(key)) == 0)
			return (temp->value);
		temp = temp->next;
	}
	return (NULL);
}
