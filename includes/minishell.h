/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minishell.h                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sarayapa <sarayapa@student.42bangkok.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/25 14:33:36 by sarayapa          #+#    #+#             */
/*   Updated: 2026/08/01 16:34:57 by sarayapa         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MINISHELL_H
# define MINISHELL_H

# include <stdio.h>
# include <readline/readline.h>
# include <readline/history.h>
# include <unistd.h>
# include <stdlib.h>
# include <fcntl.h>
# include "libft.h"

# define TRUE 1
# define FALSE 0

typedef enum e_quote
{
	Q_NONE,
	Q_SINGLE,
	Q_DOUBLE
}	t_quote;

typedef enum e_token_type
{
	TOKEN_WORD,
	TOKEN_PIPE,
	TOKEN_REDIR_IN,
	TOKEN_REDIR_OUT,
	TOKEN_REDIR_APPEND,
	TOKEN_HEREDOC,
	TOKEN_END
}	t_token_type;

typedef enum e_redir_type
{
	REDIR_IN,
	REDIR_OUT,
	REDIR_APPEND,
	REDIR_HEREDOC
}	t_redir_type;

typedef struct s_token
{
	char			*value;
	t_token_type	type;
	t_quote			qouted;
	struct s_token	*next;
}	t_token;

typedef struct s_redir
{
	t_redir_type	type;
	char			*file;
	int				fd;
	struct s_redir	*next;
}	t_redir;

typedef struct s_env
{
	char			*key;
	char			*value;
	struct s_env	*next;
}	t_env;

typedef struct s_cmd
{
	char			**args;
	t_redir			*redirs;
	struct s_cmd	*next;
}	t_cmd;

typedef struct s_shell
{
	t_env			*env;
	t_cmd			*cmds;
	t_token			*token;
	int			exit_status;
}	t_shell;

void	print_tokens(t_token *tokens);
t_token	*tokenize(char *input);
int		init_shell(t_shell *shell, char **envp);

char 	*env_finder(char *str, char **envp);
void	env_addback(t_env **lst, t_env *new);
t_env 	*env_last(t_env *lst);
t_env 	*new_env(char *envp);
char	*get_env(t_env *env, char *key);
char	*get_promt(t_env *env, char *str);

void	loop(t_shell *shell);
int		check_args(int ac, char **av, char **envp);
int		init_env(t_shell *shell, char **envp);
void	free_env(t_env *env);

t_token	*new_token(char const *value, t_token_type type);
int		find_word_end(const char *s, int start, int *end);
int		is_operator_char(char c);
int		is_quote(char c);
int		is_space(char c);
void	free_tokens(t_token *token);
void	print_tokens(t_token *tokens);
void	token_add_back(t_token **head, t_token *new);

#endif