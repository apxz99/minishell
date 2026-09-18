/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sarayapa <sarayapa@student.42bangkok.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/25 14:49:02 by sarayapa          #+#    #+#             */
/*   Updated: 2026/09/18 20:42:36 by sarayapa         ###   ########.fr       */
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

	if (check_args(ac, av, envp))
		return (1);
	shell = ft_calloc(1, sizeof(t_shell));
	if (!shell)
		return (1);
	if (init_shell(shell, envp))
	{
		free_env(shell->env);
		free(shell);
		return (1);
	}
	loop(shell);
	free_env(shell->env);
	free(shell);
	return (0);
}

void	handle_line(t_shell *shell, char *input)
{
	t_token	*token;

	token = tokenize(input);
	if (!token)
	{
		shell->exit_status = 2;
		return ;
	}
	if (syntax_check(token))
	{
		shell->exit_status = 2;
		free_tokens(token);
		return ;
	}
	if (!expand_tokens(token, shell))
	{
		free_tokens(token);
		return ;
	}
	free_tokens(token);
}

/*
loop - Run the shell input loop.
Return: Nothing.
*/
void	loop(t_shell *shell)
{
	char	*input;

	while (1)
	{
		input = readline("minishell: ");
		if (!input)
		{
			free(input);
			break ;
		}
		handle_line(shell, input);
		free(input);
	}
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
	if (init_env(shell, envp))
		return (1);
	return (0);
}
