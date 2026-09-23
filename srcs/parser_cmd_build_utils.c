/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser_cmd_build_utils.c                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sarayapa <sarayapa@student.42bangkok.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/20 17:56:10 by sarayapa          #+#    #+#             */
/*   Updated: 2026/09/23 17:36:11 by sarayapa         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	need_cmd(t_cmd **cmd)
{
	if (*cmd == NULL)
		*cmd = cmd_new();
	return (*cmd == NULL);
}
