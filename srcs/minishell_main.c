/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minishell_main.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sarayapa <sarayapa@student.42bangkok.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/25 14:49:02 by sarayapa          #+#    #+#             */
/*   Updated: 2026/08/01 16:52:05 by sarayapa         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	main(int ac, char **av, char **envp)
{
	t_shell	*shell;

	shell = ft_calloc(1, sizeof(t_shell));
	if (check_args(ac, av, envp) || init_shell(shell, envp))
		return (1);
	loop(shell);
	free_env(shell->env);
	free(shell);
	return (0);
}

void	loop(t_shell *shell)
{
	char	*input;
	char	*promt;

	while (1)
	{
		promt = get_promt(shell->env, "$ ");
		input = readline(promt);
		free(promt);
		if (!input || ft_strncmp(input, "exit", 4) == 0)
		{
			free(input);
			break ;
		}
		shell->token = tokenize(input);
		print_tokens(shell->token);
		free_tokens(shell->token);
		free(input);
	}
}

char	*get_promt(t_env *env, char *str)
{
	char	*promt;
	char	*temp;

	temp = get_env(env, "PWD");
	promt = ft_strjoin(temp, str);
	return (promt);
}

int	check_args(int ac, char **av, char **envp)
{
	if (ac != 1 || !av[0] || !envp)
		return (1);
	return (0);
}

int	init_shell(t_shell *shell, char **envp)
{
	shell->env = NULL;
	shell->cmds = NULL;
	shell->token = NULL;
	if (init_env(shell, envp))
		return (1);
	return (0);
}
