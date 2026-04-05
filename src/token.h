#ifndef _TOKEN_H_
#define _TOKEN_H_

#include <stddef.h>

typedef enum {
    REDIR_OUT_OW, REDIR_OUT_APP,
    REDIR_IN, HERE_DOC, PIPE, BG_PROC,
    OR_OP, AND_OP, TEXT, NULL_TYPE
} TokenType;

typedef struct {
    TokenType type;
    char* name;
} Token;

typedef struct {
    size_t count;
    Token* tokens;
} TokenArr;

TokenArr emitToks(char* str);
void freeTokens(Token* toks);

#endif  // INCLUDE_TOKEN_H_
