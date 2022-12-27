#ifndef _SHELL_H
#define _SHELL_H

#include "shell_common.h"
#include "shell_internal.h"

static void print_shell_prompt(int status);

static char **str_split(char *a_str, const char a_delim, int *size);

static Cmd_t *parse_command(char *input);

static Expr_t parse_expression(char *input);

static void execute_command(Cmd_t *cmd);

static int execute_expression(Expr_t *expr);

int shell_loop(bool show_prompt);

#endif
