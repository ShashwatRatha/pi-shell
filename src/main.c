#include <linux/limits.h>
#include <signal.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include "../include/token.h"
#include "../include/builtins.h"

#define MAX_ARG_LEN 21

void handle_int(int sig) {
    write(STDIN_FILENO, "\n", 2); // have to fix handler to reprint prompt by itself
}

int main(){
    signal(SIGINT, handle_int);

    while (1) {
        char* buf = getcwd(NULL, 0);
        dprintf(STDOUT_FILENO, "\033[1;93mfishy@pish\033[0m:\033[1;34m[%s]\033[0m:%% ", buf);
        free(buf);

        char* cmd = NULL;
        size_t n = 0;
        if (getline(&cmd, &n, stdin) == -1) {
            perror("input failure");
            exit(INP_FAILURE);
        }

        size_t j = 0;
        while (cmd[j] != 0 && cmd[j] != '\n')
            j++;
        cmd[j] = 0;

        if (cmd[0] == 0) 
            continue;
        
        char* argv[MAX_ARG_LEN];
        size_t num;
        tokenise(cmd, argv, &num);

        if (execBuiltin(argv) == -1) { // IF NOT A BUILTIN
            pid_t pid = fork();

            if (pid == 0) {
                if(execvp(argv[0], argv) == -1){
                    write(STDERR_FILENO, "pish: ", 7);
                    perror(argv[0]);
                    exit(EXEC_FAILURE);
                }
                exit(EXEC_SUCCESS);
            }

            else {
                waitpid(pid, NULL, 0);
            }
        }

        free(cmd);
    }
}
