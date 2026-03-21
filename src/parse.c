#include "token.h"
#include "builtins.h"
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define ARGV_INIT 8
#define CMD_INIT 8
#define DLM_INIT 8

static inline char isDelim(TokenType tp);

int parse(char* cmdstr, Commands* commands, Delims* dlms) {
    //extract tokens
    TokenArr toks = emitToks(cmdstr);
    Token* tokens = toks.tokens;

    size_t tok_idx = 0, cmd_idx = 0;

    Command* cmds = (Command*) malloc(CMD_INIT * sizeof(*cmds));
    if (!cmds) {
        freeTokens(tokens);
        return ALLOC_FAILURE;
    }
    size_t cmd_size = CMD_INIT;

    TokenType* delims = malloc(DLM_INIT * sizeof(*delims));
    if (!delims) {
        freeTokens(tokens);
        return ALLOC_FAILURE;
    }
    size_t delim_idx = 0;

    while (tok_idx < toks.count) {
        if (cmd_idx >= cmd_size - 1) {
            cmd_size <<= 1;
            Command* tmp = (Command*)realloc(cmds, cmd_size * sizeof(*cmds));
            if (!tmp) {
                freeTokens(tokens);
                free(cmds); // need to have separate free-commands method
                return ALLOC_FAILURE;
            }
            cmds = tmp;
        }

        cmds[cmd_idx] = (Command) {
            .argv = NULL,
                .instream = STDIN_FILENO,
                .outstream = STDOUT_FILENO,
        };
        // allocate argv
        char** argv = malloc((ARGV_INIT + 1) * sizeof(*argv));
        if (!argv) {
            freeTokens(tokens);
            free(cmds);
            return ALLOC_FAILURE;
        }
        size_t argv_idx = 0, argv_size = ARGV_INIT;

        // construct the command
        while (tokens[tok_idx].type == TEXT) {
            // reallocate argv if needed
            if (argv_idx >= argv_size - 1) {
                argv_size <<= 1;
                char** tmp = realloc(argv, ((argv_size + 1) * sizeof(*argv)));
                if (!tmp) {
                    free(cmds);
                    freeTokens(tokens);
                    return ALLOC_FAILURE;
                }
                argv = tmp;
            }
            // make the command
            argv[argv_idx++] = tokens[tok_idx++].name;
        }

        argv[argv_idx] = NULL; // argv constructed
        cmds[cmd_idx].argv = argv; // errors in here will be dealt with by the exec block


        TokenType t = tokens[tok_idx].type;
        while (t == REDIR_IN || t == REDIR_OUT_OW || t == REDIR_OUT_APP) {
            // deal with i/o redirection 

            tok_idx++;
            if (tok_idx >= toks.count - 1 || tokens[tok_idx].type != TEXT) {
                char* op = t == REDIR_IN ? "<" : t == REDIR_OUT_OW ? ">" : ">>";
                dprintf(STDERR_FILENO, "expected valid filename after %s", op);
                free(cmds);
                freeTokens(tokens);
                return EXIT_FAILURE;
            }

            const char* name = tokens[tok_idx].name;
            if (t == REDIR_IN) {
                if ((cmds[cmd_idx].instream = open(name, O_RDONLY, 0644)) == -1) {
                    perror("pish");
                    free(cmds);
                    freeTokens(tokens);
                    return EXIT_FAILURE;
                }
            }

            else if (t == REDIR_OUT_OW) {
                if ((cmds[cmd_idx].outstream = open(name, O_WRONLY | O_TRUNC | O_CREAT, 0644)) == -1) {
                    perror("pish");
                    free(cmds);
                    freeTokens(tokens);
                    return EXIT_FAILURE;
                }
            }

            else {
                if ((cmds[cmd_idx].outstream = open(name, O_WRONLY | O_CREAT | O_APPEND, 0644)) == -1) {
                    perror("pish");
                    free(cmds);
                    freeTokens(tokens);
                    return EXIT_FAILURE;
                }
            }

            tok_idx++;
            t = tokens[tok_idx].type;
        }

        delims[delim_idx++] = tokens[tok_idx++].type;
        cmd_idx++;
    }

    *commands = (Commands) {
        .cmds = cmds,
            .count = cmd_idx
    };

    *dlms = (Delims) {
        .delims = delims,
            .count = delim_idx
    };

    return SUCCESS;
}

static inline char isDelim(TokenType tp) {
    return ((tp == PIPE) || (tp == OR_OP) || (tp == AND_OP) || (tp == BG_PROC) || (tp == NULL_TYPE));
}
