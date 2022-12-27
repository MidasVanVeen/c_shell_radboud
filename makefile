FILES=src/main.c src/shell.c src/shell_internal.c src/shell_common.c
CC=clang
INCLUDES=-Iinclude/

all: $(FILES)
	$(CC) $(FILES) $(INCLUDES) -o shell -g
