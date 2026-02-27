NAME        = codexion

CC          = cc
CFLAGS      = -Wall -Wextra -Werror -pthread

# Ajout de tous les fichiers .c présents dans ton tree
SRCS        = codexion.c \
              utils.c \
              heap.c \
              heap_utils.c \
              init.c \
              simulation.c \
              thread.c

OBJS        = $(SRCS:.c=.o)

# Le header pour forcer la recompilation en cas de changement dans le .h
HEADER      = codexion.h

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

# On ajoute le HEADER en dépendance pour être plus carré
%.o: %.c $(HEADER)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)
	rm -f codexion.h.gch # On nettoie aussi le fichier précompilé inutile

fclean: clean
	rm -f $(NAME)
	rm -f a.out

re: fclean all

.PHONY: all clean fclean re