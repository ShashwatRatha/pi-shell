#include "exec.h"
#include "globals.h"
#include <errno.h>
#include <linux/limits.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

static inline void printPrompt();

static void handle_int(int sig) {
  write(STDOUT_FILENO, "\n",
        1); // have to fix handler to reprint prompt by itself
}

int main() {
  signal(SIGINT, handle_int);

  while (1) {
    printPrompt();
    char *cmd = NULL;
    size_t n = 0;
    if (getline(&cmd, &n, stdin) == -1) {
      if (errno == EINTR) {
        free(cmd);
        cmd = NULL;
        continue;
      }
      perror("input failure");
      _exit(-1);
    }

    size_t j = 0;
    while (cmd[j] != 0 && cmd[j] != '\n')
      j++;
    cmd[j] = 0;

    if (cmd[0] == 0) {
      free(cmd);
      continue;
    }

    int ret = execute(cmd);
    if (ret == ALLOC_FAILURE) {
      free(cmd);
      return EXIT_FAILURE;
    }

    free(cmd);
  }
}

// modifies the shown working directory to show ~ in place of /home/user
static inline void printPrompt() {
  char *buf = getcwd(NULL, 0);
  size_t dirLen = strlen(buf);

  char check[7];
  memcpy(check, buf, 6);
  check[6] = 0;

  if (!strcmp("/home/", check)) {
    char *ptr = buf + 6;
    while (*ptr && *ptr != '/')
      ptr++;
    size_t skip = ptr - buf;
    char *tmp = malloc((dirLen - skip + 2) * sizeof *tmp);

    tmp[0] = '~';
    memcpy(tmp + 1, buf + skip, dirLen - skip);
    tmp[dirLen - skip + 1] = 0;

    free(buf);
    buf = tmp;
  }

  dprintf(STDOUT_FILENO,
          "\033[1;93mfishy@pish\033[0m:\033[1;34m[%s]\033[0m:%% ", buf);
  free(buf);
}
