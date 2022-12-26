#ifndef _SHELL_COMMON_H
#define _SHELL_COMMON_H

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define NOT_INTERNAL 305

typedef struct Expr Expr_t;


/*
  The Command structure is a linked list
 */
typedef struct Cmd {
  Expr_t *parent_expr;
  struct Cmd *next; // NULL if last command
  char **parts;     // argv
  int numParts;     // argc
  bool head;        // Set to True if this is the first command in the chain
  int pipe[2];
} Cmd_t;


/*
  The expression structure holds a pointer to the head of the command list
  and information on the amount of commands in the list.
  It also holds information of the input and output files (if any)
 */
typedef struct Expr {
  Cmd_t *cmd;
  int numCmds;
  char *inputFromFile;
  char *outputToFile;
  bool background;
  bool valid;
} Expr_t;

char *fix_filename(char *filename);

#endif
