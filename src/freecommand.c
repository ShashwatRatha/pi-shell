#include "command.h"
#include <stddef.h>
#include <stdlib.h>

void freeCommands(Commands commands) {
  Command *cmds = commands.cmds;
  size_t size = commands.count;

  for (size_t i = 0; i < size; i++) {
    Command cmd = cmds[i];
    for (size_t j = 0; cmd.argv[j]; j++) {
      free(cmd.argv[j]);
    }
    free(cmd.argv);
  }

  free(cmds);
}
