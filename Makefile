# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: sarayapa <sarayapa@student.42bangkok.co    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/07/25 14:42:29 by sarayapa          #+#    #+#              #
#    Updated: 2026/08/01 17:08:22 by sarayapa         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = minishell

CC = cc
CFLAGS = -Wall -Wextra -Werror

FILES = minishell_main.c \
		parser_env.c \
		parser_env_utils.c \
		parser_lexer.c \
		parser_lexer_utils.c \
		parser_token.c \
#		parser_token_utils.c \

SRCS = srcs/
BUILD = builds/
INC = -Iincludes -Ilibft/includes

SRCS_O = $(addprefix $(BUILD), $(FILES:.c=.o))

GREEN = \033[1;32m
YELLOW = \033[1;33m
RED = \033[1;31m
BLUE = \033[1;34m
WHITE = \033[1m
RESET = \033[0m

all: libft $(BUILD) $(NAME)

libft:
#	@echo "$(YELLOW)Building libft...$(RESET)"
	@make -s --no-print-directory -C libft

$(BUILD):
	@mkdir -p $(BUILD)

$(BUILD)%.o: $(SRCS)%.c
	@echo "$(YELLOW)Compiling file -> $@ $(RESET)"
	@$(CC) $(CFLAGS) $(INC) -c $< -o $@

$(NAME): $(SRCS_O)
	@echo "$(YELLOW)Linking $(NAME)...$(RESET)"
	@$(CC) $(CFLAGS) $^ libft/libft.a $(LIB) -o $@ -lreadline
	@echo "$(GREEN)✔ $(NAME) ready Location: $(BLUE)$$(pwd)/$(YELLOW)$(NAME)$(RESET)"

norm:
	@echo "========= INCLUDES =========="
	@norminette ./includes
	@echo "========= SOURCES ==========="
	@norminette $(SRCS)
	@echo "========= LIBFT ============="
	@norminette ./libft

clean: libclean
	@rm -rf $(BUILD)
	@echo "$(RED)Cleaned objects!$(RESET)"

libclean:
	@make -s clean -C libft
	@echo "$(RED)Cleaned libft objects!$(RESET)"

libfclean:
	@make -s fclean -C libft
	@echo "$(RED)Cleaned libft!$(RESET)"

fclean: libfclean
	@rm -rf $(BUILD)
	@echo "$(RED)Cleaned objects!$(RESET)"
	@rm -f $(NAME)
	@echo "$(RED)Removed $(NAME)!$(RESET)"

re: fclean all

.PHONY: all clean fclean re libft norm libclean
