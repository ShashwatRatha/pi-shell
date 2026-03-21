#ifndef _BUILTINS_H_
#define _BUILTINS_H_

#include "token.h"
#include <stddef.h>

#define bool char

enum return_vals{
    SUCCESS, ALLOC_FAILURE, EXEC_FAILURE, INP_FAILURE, 
    TOK_FAILURE, PIPE_FAILURE, FORK_FAILURE
};

typedef struct {
    const char* id;
    int (*fn) (char**);
} builtin;

typedef struct {
    char** argv;
    int instream;
    int outstream;
} Command;

typedef struct {
    Command* cmds;
    size_t count;
} Commands;

typedef struct {
    Command* group;
    int size;
    bool bgproc;
} CommandGrp;

typedef struct {
    TokenType* delims;
    size_t count;
} Delims;

char execBuiltin(char**);
int parse(char*, Commands*, Delims*);
int execute(char*);

#endif // INCLUDE_BUILTINS_H_ 
