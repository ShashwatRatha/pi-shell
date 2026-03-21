#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include "token.h"

#define TOK_INIT 32
#define BUF_INIT 32

typedef enum {
    RST, QTS, WTE, CHR, OPR
} State;

static inline char isWhite(char ch) {
    return ((ch == ' ') || (ch == '\n') || (ch == '\t'));
}

static inline char isOp(char ch) {
    return ((ch == '|') || (ch == '>') || (ch == '<') || (ch == '&'));
}

static inline char isChar(char ch) {
    return ((ch != '"') && !isWhite(ch) && !isOp(ch));
}

TokenArr emitToks(char* str) {
    Token* tokens = (Token*) malloc((TOK_INIT + 1) * sizeof(Token));
    if (!tokens){
        dprintf(STDERR_FILENO, "tokens alloc failed\n");
        return (TokenArr){0, NULL};
    }
    size_t tok_size = TOK_INIT;

    size_t next = 0;
    State st = RST;

    char* curr = str, *buf = NULL;
    int buf_size = 0, buf_idx = -1;

    while (*curr) {
        char ch = *curr;

        if (next >= tok_size-1){
            tok_size *= 2;
            Token* tmp = (Token*) realloc(tokens, (tok_size + 1) * sizeof(Token));
            if (!tmp){
                free(tokens);
                return (TokenArr){0, NULL};
            }
            tokens = tmp;
        }

        switch (st) {
            case RST: {
                          if (ch == '"')
                              st = QTS;
                          else if (isOp(ch))
                              st = OPR;
                          else if (isWhite(ch))
                              st = WTE;
                          else 
                              st = CHR;

                          break;
                      }
            case WTE: {
                          while (isWhite(*curr))
                              *curr = 0, curr++;

                          st = RST;
                          break;
                      }
            case OPR: {
                          char opstr[3];

                          opstr[2] = 0;
                          if (*curr && *(curr + 1) && *(curr+1) == *curr) {
                              opstr[0] = opstr[1] = *curr;
                              *curr = *(curr + 1) = 0;
                              curr += 2;
                          }
                          else if (isOp(*(curr+1))) {
                              return (TokenArr){0, NULL};
                          }

                          else {
                              opstr[0] = *curr, opstr[1] = 0;
                              *curr = 0; curr++;
                          }

                          for (int i = 0; op_ref[i].name != NULL; i++) {
                              if (!strcmp(op_ref[i].name, opstr)){
                                  tokens[next++] = op_ref[i];
                                  break;
                              }
                          }

                          st = RST;
                          break;
                      }
            case CHR: {
                          int idx = (buf_idx == -1 ? 0 : buf_idx);
                          if (buf_idx == -1){
                              buf = (char*) malloc((BUF_INIT + 1) * sizeof(char));
                              if (buf == NULL)
                                  return (TokenArr){0, NULL};

                              buf_size = BUF_INIT;
                          }

                          while (*curr && isChar(*curr)) {
                              if (idx < buf_size)
                                  buf[idx] = *curr, idx++, curr++;
                              else {
                                  buf_size += (buf_size >> 1);
                                  char* tmp = realloc(buf, (buf_size + 1) * sizeof(char));
                                  if (!tmp){
                                      free(buf);
                                      return (TokenArr){0, NULL};
                                  }
                                  buf = tmp;
                              }
                          }

                          if (*curr == '"') {
                              buf_idx = idx;
                          }
                          else {
                              buf[idx] = 0;
                              tokens[next++] = (Token) {TEXT, buf};
                              buf_idx = -1, buf_size = 0;
                          }

                          st = RST;
                          break;
                      }
            case QTS: {
                          curr++;
                          int idx;
                          if (buf_idx == -1){
                              buf = (char*) malloc((BUF_INIT + 1) * sizeof(char));
                              if (!buf)
                                  return (TokenArr){0, NULL};

                              idx = 0, buf_size = BUF_INIT;
                          }
                          else 
                              idx = buf_idx;

                          while (*curr && *curr != '"') {
                              if (idx < buf_size)
                                  buf[idx] = *curr, idx++, curr++;
                              else {
                                  buf_size += (buf_size >> 1);
                                  char* tmp = realloc(buf, (buf_size + 1) * sizeof(char));
                                  if (tmp == NULL){
                                      free(buf);
                                      return (TokenArr){0, NULL};
                                  }
                                  buf = tmp;
                              }
                          }
                          if (*curr != '"') {
                              if (buf_idx != -1)
                                  free(buf);
                              dprintf(STDERR_FILENO, "quotes not closed\n");
                              return (TokenArr){0, NULL};
                          }

                          if (*(curr + 1) && isChar(*(curr+1))) {
                              buf_idx = idx;
                          }
                          else {
                              buf[idx] = 0;
                              tokens[next++] = (Token) {TEXT, buf};
                              buf_idx = -1, buf_size = 0;
                          }

                          curr++;
                          st = RST;
                          break;
                      }
        }
    }

    if (next >= tok_size-1) {
        Token* tmp = realloc(tokens, (next + 1)*sizeof(Token));
        if (!tmp) {
            free(tokens);
            return (TokenArr){0, NULL};
        }
        tokens = tmp;
    }

    tokens[next++] = (Token) {NULL_TYPE, NULL};

    return (TokenArr){next, tokens};
}

void freeTokens(Token *toks) {
    for (size_t idx = 0; toks[idx].name != NULL; idx++) {
        if (toks[idx].type == TEXT) {
            free(toks[idx].name);
            toks[idx].name = NULL;
        }
    }

    free(toks);
}
