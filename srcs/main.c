/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sarayapa <sarayapa@student.42bangkok.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/25 14:49:02 by sarayapa          #+#    #+#             */
/*   Updated: 2026/09/09 19:32:16 by sarayapa         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/*
main - Initialize the shell and start the main loop.
Return: 0 on success, 1 on error.
*/
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

/*
loop - Run the shell input loop.
Return: Nothing.
*/
void	loop(t_shell *shell)
{
	char	*input;
	char	*prompt;

	while (1)
	{
		prompt = get_prompt(shell->env, "$ ");
		input = readline(prompt);
		free(prompt);
		if (!input || ft_strncmp(input, "exit", 4) == 0)
		{
			free(input);
			break ;
		}
		shell->token = tokenize(input);
		if (syntax_check(shell->token))
		{
			free_tokens(shell->token);
			shell->token = NULL;
			continue ;
		}
		if (!expand_tokens(shell->token, shell))
		{
			free_tokens(shell->token);
			shell->token = NULL;
			continue ;
		}
		print_tokens(shell->token);
		free_tokens(shell->token);
		free(input);
	}
}

/*
get_promt - Create the shell prompt from the current directory.
Return: Allocated prompt string.
*/
char	*get_prompt(t_env *env, char *str)
{
	char	*prompt;
	char	*temp;

	temp = get_env(env, "PWD");
	prompt = ft_strjoin(temp, str);
	return (prompt);
}

/*
check_args - Check the arguments passed to the shell.
Return: 0 if valid, 1 otherwise.
*/
int	check_args(int ac, char **av, char **envp)
{
	if (ac != 1 || !av[0] || !envp)
		return (1);
	return (0);
}

/*
init_shell - Initialize the shell structure and environment.
Return: 0 on success, 1 on error.
*/
int	init_shell(t_shell *shell, char **envp)
{
	shell->env = NULL;
	shell->cmds = NULL;
	shell->token = NULL;
	if (init_env(shell, envp))
		return (1);
	return (0);
}
