#include "../include/builtins.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define FAILURE -1
#define SUCCESS 0

static int Exit(char**);
static int Cd(char**);
static int Pwd(char**);

static builtin builtins[] = {
    {"exit", Exit},
    {"cd", Cd},
    {"pwd", Pwd}
};

char execBuiltin(char** argv){
    size_t size = sizeof(builtins)/sizeof(builtins[0]);

    for (size_t i = 0; i < size; i++) {
        if (!strcmp(argv[0], builtins[i].id)) {
            builtins[i].fn(argv);
            return SUCCESS; 
        }
    }

    return FAILURE;
}

static int Exit(char** argv){
    int ret = SUCCESS;
    if (argv[1]){
        ret = atoi(argv[1]);
    }

    _exit(ret);
    perror("pish: exit"); 
    return FAILURE;
}

static int Cd(char** argv) {
    if (!argv[1]) {
        chdir(getenv("HOME"));
    }
else if (chdir(argv[1]) == -1) {
        perror("pish: cd");
        return FAILURE;
    }

    return SUCCESS;
}

static int Pwd(char** argv){
    
    char* buf;
    
    if (!(buf = getcwd(NULL, 0))) {
        perror("pish: pwd");
        return FAILURE;
    }

    write(STDOUT_FILENO, buf, strlen(buf));
    write(STDOUT_FILENO, "\n", 1);

    return SUCCESS;
}
