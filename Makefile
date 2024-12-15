# RESET       = \033[0m
# GREEN       = \033[32m
# YELLOW      = \033[33m

NAME = ircserv
CC = c++
CFLAGS = -Wall -Wextra -Werror -std=c++98
SRCS = ircserv.cpp Channel.cpp Server.cpp Extra.cpp Commands.cpp Mode.cpp
OBJS = $(SRCS:.c=.o)

$(NAME): $(OBJS)
	@$(CC) $(CFLAGS) $(OBJS) -o $(NAME)\
	@echo "$(GREEN)"
	@echo "Created: $(words $(OBJS)) object file(s) for FT_IRC"
	@echo "Created: $(NAME)"

%.o: %.c
	@$(CC) $(CFLAGS) -c $< -o $@

all: $(NAME)

clean:
	@rm -f $(OBJS)

fclean: clean
	@rm -f $(NAME)
	@echo "$(YELLOW)"
	@echo "Removed: $(words $(OBJS)) object file(s) for FT_IRC"
	@echo "Removed: $(NAME)"

re: fclean all

.PHONY: all clean fclean re