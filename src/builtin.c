#include "builtin.h"
#include "globals.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int Exit(char **);
static int Cd(char **);
static int Pwd(char **);

typedef struct {
  const char *id;
  int (*fn)(char **);
} builtin;

static builtin builtins[] = {{"exit", Exit}, {"cd", Cd}, {"pwd", Pwd}};

int execBuiltin(char **argv) {
  size_t size = sizeof(builtins) / sizeof(builtins[0]);

  for (size_t i = 0; i < size; i++) {
    if (!strcmp(argv[0], builtins[i].id)) {
      builtins[i].fn(argv);
      return SUCCESS;
    }
  }

  return EXEC_FAILURE;
}

static int Exit(char **argv) {
  int ret = SUCCESS;
  if (argv[1]) {
    ret = atoi(argv[1]);
  }

  _exit(ret);
  perror("pish: exit");
  return EXEC_FAILURE;
}

static int Cd(char **argv) {
  char *path = NULL;

  if (!argv[1]) {
    char *tmp = NULL;
    if ((tmp = getenv("HOME"))) {
      path = tmp;
    }
  } else
    path = argv[1];

  if (chdir(path) == -1) {
    perror("pish: cd");
    return EXEC_FAILURE;
  }

  return SUCCESS;
}

static int Pwd(char **argv) {
  char *buf;

  if (!(buf = getcwd(NULL, 0))) {
    perror("pish: pwd");
    return EXEC_FAILURE;
  }

  write(STDOUT_FILENO, buf, strlen(buf));
  write(STDOUT_FILENO, "\n", 1);

  free(buf);

  return SUCCESS;
}
