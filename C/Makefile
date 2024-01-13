CC = cc
CFLAGS = -Wall -Wextra -Werror -g
RM = rm -f

NAME = syncLoop

SRC = 	syncLoop.c \
		utils.c \

OBJ_SRC = $(SRC:.c=.o)

all: $(NAME)

$(NAME): $(OBJ_SRC)
	$(CC) $(CFLAGS) $(OBJ_SRC) -o $(NAME)

clean:
	@$(RM) $(OBJ_SRC)

fclean: clean
	@$(RM) $(NAME)

re: fclean all