#include "../include/shell.h"

#define VERBOSE 0

/*
 * Print the shell prompt.
 * Params: status - the exit status of the last command
 * Return: void
 */
static void print_shell_prompt(int status) {
  char *cwd = (char *)malloc(100);
  for (int i = 100; getcwd(cwd, i) == NULL; i += 100)
    cwd = (char *)malloc(i);

  if (status == 0) {
    printf("%s%s%s$ ", "\e[32m", cwd, "\e[39m");
  } else {
    printf("%s%d %s%s%s$ ", "\e[31m", status, "\e[32m", cwd, "\e[39m");
  }
}

/*
 * Function for splitting a c string into an array of c strings
 * Params: a_str - the string to split
 *  	   a_delim - the delimiter to split on
 *	  	   size - pointer to an int to store the size of the array
 * Author: hmjd on stackoverflow
 * source:
 * https://stackoverflow.com/questions/9210528/split-string-with-delimiters-in-c)
 * Return: char ** - the array of c strings
 */
static char **str_split(char *a_str, const char a_delim, int *size) {
  char **result = 0;
  int count = 0;
  char *tmp = a_str;
  char *last_comma = 0;
  char delim[2];
  delim[0] = a_delim;
  delim[1] = 0;
  if (*tmp == a_delim)
    tmp++;
  while (*tmp) {
    if (a_delim == *tmp) {
      count++;
      last_comma = tmp;
    }
    tmp++;
  }
  count += last_comma < (a_str + strlen(a_str) - 1);
  (*size) = count;
  count++;
  result = malloc(sizeof(char *) * count);
  if (result) {
    size_t idx = 0;
    char *token = strtok(a_str, delim);
    while (token) {
      if (!(idx < count))
        return NULL;
      *(result + idx++) = strdup(token);
      token = strtok(0, delim);
    }
    if (idx != count - 1)
      return NULL;
    *(result + idx) = 0;
  }
  return result;
}

/*
 * Function for parsing a command as a Cmd_t struct
 * Params: input - the command to parse
 * Return: Cmd_t - the parsed command
 */
static Cmd_t *parse_command(char *input) {
  Cmd_t *cmd = (Cmd_t *)malloc(sizeof(Cmd_t));
  cmd->numParts = 0;
  cmd->parts = str_split(input, ' ', &cmd->numParts);
  if (cmd->parts == NULL) {
    fprintf(stderr, "Error parsing command\n");
    return NULL;
  }
  cmd->parts[cmd->numParts] = NULL;
  if (pipe(cmd->pipe) != 0) {
    perror("Pipe creation error");
  }
  return cmd;
}

/*
 * Function for parsing a series of commands as a Exec_t struct
 * Params: input - the commands to parse
 * Return: Exec_t - the expression structure
 */
static Expr_t parse_expression(char *input) {
  Expr_t expr;

  char **cmdStrings = 0;
  cmdStrings = str_split(input, '|', &expr.numCmds);
  if (cmdStrings == NULL) {
    fprintf(stderr, "Error parsing expression\n");
    return expr;
  }
  expr.cmd = parse_command(cmdStrings[0]);
  if (expr.cmd == NULL)
    return expr;
  expr.valid = true;
  expr.cmd->parent_expr = &expr;
  expr.cmd->head = true;

  if (expr.cmd->numParts >= 2) {
    if (strcmp(expr.cmd->parts[expr.cmd->numParts - 1], "&") == 0) {
      expr.background = true;
      expr.cmd->parts[expr.cmd->numParts - 1] = NULL;
      expr.cmd->numParts--;
    }
  }

  if (expr.cmd->numParts > 2) {
    if (strcmp(expr.cmd->parts[expr.cmd->numParts - 2], "<") == 0) {
      expr.inputFromFile =
          fix_filename(expr.cmd->parts[expr.cmd->numParts - 1]);
      expr.cmd->parts[expr.cmd->numParts - 2] = NULL;
      expr.cmd->parts[expr.cmd->numParts - 1] = NULL;
      expr.cmd->numParts -= 2;
    } else if (strcmp(expr.cmd->parts[expr.cmd->numParts - 2], ">") == 0) {
      expr.outputToFile = fix_filename(expr.cmd->parts[expr.cmd->numParts - 1]);
      expr.cmd->parts[expr.cmd->numParts - 2] = NULL;
      expr.cmd->parts[expr.cmd->numParts - 1] = NULL;
      expr.cmd->numParts -= 2;
    }
  }
  if (expr.numCmds > 1) {
    Cmd_t *curr = expr.cmd;
    for (int i = 1; i < expr.numCmds; i++) {
      curr->next = parse_command(cmdStrings[i]);
      curr->next->parent_expr = &expr;
      curr = curr->next;
    }

    for (curr = expr.cmd; curr->next != NULL; curr = curr->next)
      ;
    if (curr->numParts >= 2) {
      if (strcmp(curr->parts[curr->numParts - 1], "&") == 0) {
        expr.background = true;
        curr->parts[curr->numParts - 1] = NULL;
        curr->numParts--;
      }
    }
    if (curr->numParts > 2) {
      if (strcmp(curr->parts[curr->numParts - 2], ">") == 0) {
        expr.outputToFile = fix_filename(curr->parts[curr->numParts - 1]);
        curr->parts[curr->numParts - 2] = NULL;
        curr->parts[curr->numParts - 1] = NULL;
        curr->numParts -= 2;
      }
    }
  }

  return expr;
}

/*
 * Function for executing a command
 * Params: cmd - the command to execute
 * Return: void
 */
static void execute_command(Cmd_t *cmd) {
  if (cmd == NULL || cmd->parts[0] == NULL)
    return;

  pid_t pid = fork();
  if (pid < 0) {
    perror("Forking error");
    exit(1);
  }
  if (pid == 0) {
    if (cmd->next != NULL) {
      dup2(cmd->next->pipe[1], STDOUT_FILENO);
      close(cmd->next->pipe[0]);
    }
    if (cmd->head == false) {
      if (cmd->parent_expr->inputFromFile != NULL) {
        int fd = open(cmd->parent_expr->inputFromFile, O_RDONLY);
        dup2(fd, STDIN_FILENO);
        close(fd);
      } else {
        dup2(cmd->pipe[0], STDIN_FILENO);
      }
      close(cmd->pipe[1]);
      if (cmd->parent_expr->outputToFile != NULL) {
        int fd = open(cmd->parent_expr->outputToFile, O_WRONLY | O_CREAT, 0666);
        dup2(fd, STDOUT_FILENO);
        close(fd);
      }
    }
    execvp(cmd->parts[0], cmd->parts);
    perror(cmd->parts[0]);
  } else {
    close(cmd->pipe[0]);
    close(cmd->pipe[1]);
  }
}

/*
 * Function for executing a series of commands
 * Params: expr - the expression to execute
 * Return: status of the last command
 */
static int execute_expression(Expr_t *expr) {
  if (expr->valid != true)
    return EINVAL;
  if (expr->cmd == NULL)
    return EINVAL;

  int status;
  if (expr->numCmds == 1 &&
      handle_internal(expr->cmd, &status) == NOT_INTERNAL) {
    if (fork() == 0) {
      if (expr->inputFromFile != NULL) {
        int fd = open(expr->inputFromFile, O_RDONLY);
        dup2(fd, STDIN_FILENO);
        close(fd);
      }
      if (expr->outputToFile != NULL) {
        int fd = open(expr->outputToFile, O_WRONLY | O_CREAT, 0666);
        dup2(fd, STDOUT_FILENO);
        close(fd);
      }
      execvp(expr->cmd->parts[0], expr->cmd->parts);
      exit(1);
    }
    wait(&status);
  }

  if (expr->numCmds > 1) {
    Cmd_t *curr;
    for (curr = expr->cmd; curr != NULL; curr = curr->next) {
      execute_command(curr);
    }
    pid_t pid;
    if (!expr->background) {
      while ((pid = wait(&status)) != -1) {
        if (VERBOSE) {
          fprintf(stderr, "Process %d exits with %d.\n", pid,
                  WEXITSTATUS(status));
        }
      }
    }
  }

  return status;
}

/*
 * Main function for the shell
 * Params: show_prompt - whether to show the prompt
 * Return: int - acts as exit status of progam. (returned in main)
 */
int shell_loop(bool show_prompt) {
  char *line = (char *)malloc(sizeof(char) * 512);
  int status = 0;
  print_shell_prompt(status);
  while (fgets(line, 512, stdin)) {
    line[strcspn(line, "\n")] = 0;
    if (line[0] == 0) {
      if (show_prompt) {
        print_shell_prompt(status);
      }
      continue;
    }
    Expr_t expr = parse_expression(line);
    status = execute_expression(&expr);
    if (show_prompt) {
      print_shell_prompt(status);
    }
  }

  return 0;
}
