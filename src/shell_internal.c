#include "shell_internal.h"

/*
 * Function for checking for internal commands, if found, it calls the
 * appropriate function Params: cmd - the command to check Returns: int - the
 * return value of the internal command Macro NOT_INTERNAL if the command is not
 * an internal command
 */
int handle_internal(Cmd_t *cmd, int *status) {
  if (strcmp(cmd->parts[0], "cd") == 0) {
    *status = builtin_cd(cmd);
  } else if (strcmp(cmd->parts[0], "mkdir") == 0) {
    *status = builtin_mkdir(cmd);
  } else if (strcmp(cmd->parts[0], "exit") == 0) {
    builtin_exit();
  } else {
	  return NOT_INTERNAL;
  }
  return 0;
}

/*
 * My version of mkdir. Checks if the directory already exists using stat if
 * not, it creates it Params: cmd - the command Returns: int - -1 if no path
 * supplied, 0 if successful
 */
static int builtin_mkdir(Cmd_t *cmd) {
  if (cmd->numParts == 1) {
    fprintf(stderr, "mkdi: No path supplied");
    return -1;
  }
  for (int i = 1; i < cmd->numParts; i++) {
    struct stat st = {0};
    if (stat(fix_filename(cmd->parts[i]), &st) == -1) {
      mkdir(cmd->parts[i], 0700);
    }
  }
  return 0;
}

/*
 * My version of cd. Checks if the command was supplied the correct amount of
 * arguments If so, it uses chdir() to change the current working directory
 * Params: cmd - the command
 * Returns: int - -1 if no path supplied, -2 if too many arguments, 0 if
 * successful
 */
static int builtin_cd(Cmd_t *cmd) {
  if (cmd->numParts == 1) {
    fprintf(stderr, "cd: No path supplied\n");
    return -1;
  }
  if (cmd->numParts > 2) {
    fprintf(stderr, "cd: Too many arguments\n");
    return -2;
  }
  char *cwd = (char *)malloc(100);
  int i;
  for (i = 100; getcwd(cwd, i) == NULL; i += 100)
    cwd = (char *)malloc(i);
  char *newpath =
      (char *)malloc(sizeof(char) * strlen(cmd->parts[0]) + sizeof(char) * i);
  strcpy(newpath, cwd);
  if (cmd->parts[1][0] == '/') {
    strcat(newpath, cmd->parts[1]);
  } else {
    strcat(newpath, "/");
    strcat(newpath, cmd->parts[1]);
  }
  if (chdir(newpath) != 0)
    perror("Cd err");
  return 0;
}

/*
 * My version of exit. simply exits the process.
 * Params: none
 * Returns: void
 */
static void builtin_exit() { exit(0); }
