/*
 * Shell written in c for Radboud University operating systems course
 * Author: Midas van Veen (unless specified otherwise)
 * Date: 2020-03-01
 * Language: c
 */

#include "../include/shell.h"

int main(int argc, char **argv) {
  bool show_prompt = true;
  if (argv[1] != NULL)
    if (strcmp(argv[1], "false") == 0)
      show_prompt = false;
  return shell_loop(show_prompt);
}
