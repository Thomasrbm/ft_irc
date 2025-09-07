NAME = botIRC

CC = c++
CFLAGS = -Wall -Wextra -Werror -g3 -std=c++98

OBJDIR = ./objects

SRCS = srcs/main.cpp \
	   srcs/Bot.cpp \
	../srcs/Utils.cpp \

OBJS = $(patsubst %.cpp,$(OBJDIR)/%.o,$(SRCS))

RM = rm -f

all: $(NAME)
	@echo "Compiled successfully!"
	@EXEC_LINE="=> execute ./$(NAME) <port> <password> optional: <ip>(set to localhost by default)"; \
	LEN=$$(echo "$$EXEC_LINE" | wc -c); \
	LEN=$$((LEN - 1)); \
	BORDER_LEN=$$((LEN + 2)); \
	TOP="┌$$(printf '─%.0s' $$(seq 1 $$BORDER_LEN))┐"; \
	MID="│ $$EXEC_LINE │"; \
	BOT="└$$(printf '─%.0s' $$(seq 1 $$BORDER_LEN))┘"; \
	echo "\033[35m$$TOP"; \
	echo "$$MID"; \
	echo "$$BOT\033[0m"

$(NAME): $(OBJS)
	@$(CC) $(CFLAGS) -o $(NAME) $(OBJS)

$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	@if [ -d "$(OBJDIR)" ]; then \
		echo "\033[33mCleaning object files...\033[0m"; \
		$(RM) -r $(OBJDIR); \
		echo "\033[32mClean completed successfully!\033[0m"; \
	else \
		echo "\033[33mNo objects to clean.\033[0m"; \
	fi

fclean: clean
	@if [ -f "$(NAME)" ]; then \
		echo "\033[33mRemoving $(NAME)...\033[0m"; \
		$(RM) $(NAME); \
		echo "\033[32mFull clean completed successfully!\033[0m"; \
	else \
		echo "\033[33mNo executable to clean.\033[0m"; \
	fi

re: fclean all

.PHONY: all clean fclean re
