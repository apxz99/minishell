# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: sarayapa <sarayapa@student.42bangkok.co    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/08/29 17:32:46 by sarayapa          #+#    #+#              #
#    Updated: 2026/07/25 14:53:57 by sarayapa         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = libft.a

CC = cc
CFLAGS = -Wall -Wextra -Werror
ARCHIVE = ar rcs

FILES = ft_atoi.c  ft_isalnum.c  ft_isalpha.c \
		ft_isascii.c  ft_isdigit.c  ft_isprint.c \
		ft_memset.c  ft_strlen.c  ft_strncmp.c  \
		ft_tolower.c  ft_toupper.c ft_bzero.c \
		ft_memmove.c ft_strlcpy.c ft_strlcat.c \
		ft_strchr.c	ft_strrchr.c ft_memchr.c \
		ft_memcmp.c ft_strnstr.c ft_strdup.c \
		ft_substr.c ft_strjoin.c ft_strtrim.c \
		ft_split.c ft_memcmp.c ft_memcpy.c \
		ft_itoa.c ft_strmapi.c \
		ft_striteri.c ft_putchar_fd.c ft_putstr_fd.c \
		ft_putnbr_fd.c ft_putendl_fd.c \
		ft_calloc.c ft_isalnum.c \
		ft_lstsize_bonus.c ft_lstlast_bonus.c ft_lstiter_bonus.c \
		ft_lstadd_back_bonus.c ft_lstclear_bonus.c ft_lstdelone_bonus.c \
		ft_lstnew_bonus.c ft_lstadd_front_bonus.c ft_lstmap_bonus.c

SRCS = src/
BUILD = build/
INC = -I include

SRCS_O = $(addprefix $(BUILD), $(FILES:.c=.o))

GREEN = \033[1;32m
YELLOW = \033[1;33m
RED = \033[1;31m
RESET = \033[0m

all: $(BUILD) $(NAME)

$(BUILD):
	@mkdir -p $(BUILD)

$(BUILD)%.o: $(SRCS)%.c
	@$(CC) $(CFLAGS) $(INC) -c $< -o $@

$(NAME): $(SRCS_O)
	@$(ARCHIVE) $@ $^

clean:
	@rm -rf $(BUILD)

fclean: clean
	@rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re