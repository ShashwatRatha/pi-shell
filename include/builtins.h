#ifndef _BUILTINS_H_
#define _BUILTINS_H_

enum return_vals{
    EXEC_FAILURE, EXEC_SUCCESS,
    INP_FAILURE, TOK_FAILURE
};

typedef struct {
    const char* id;
    int (*fn) (char**);
} builtin;

char execBuiltin(char**);

#endif // builtins.h included
