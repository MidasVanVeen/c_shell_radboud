#ifndef _SHELL_INTERNAL_H
#define _SHELL_INTERNAL_H

#include "shell_common.h"

static int builtin_mkdir(Cmd_t *cmd);

static int builtin_cd(Cmd_t *cmd);

static void builtin_exit();

int handle_internal(Cmd_t *cmd, int *status);

#endif
