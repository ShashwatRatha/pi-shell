#ifndef COMMANDS_H_
#define COMMANDS_H_

#include "token.h"
#include <stddef.h>

typedef struct {
  char **argv;
  int instream;
  int outstream;
} Command;

typedef struct {
  Command *cmds;
  size_t count;
} Commands;

typedef struct {
  TokenType *delims;
  size_t count;
} Delims;

void freeCommands(Commands);

#endif
