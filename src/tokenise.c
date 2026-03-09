#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../include/token.h"

// include support for i/o redirection and process manipulation tokens (>, <, >>, |, &, &&)

static char isWhite(char ch) {
        return ((ch == ' ') || (ch == '\n') || (ch == '\t'));
}

static size_t white_trimmer(char* str, size_t i, size_t j) {
    assert(j > i);
    j--;


    while (str[i] != 0 && isWhite(str[i])) 
        i++;
    while (j > i && isWhite(str[j]))
        j--;
    str[j+1] = 0;

    return i;
}

//target is to tokenise based on spaces, and to deal with inverted commas separately 

void tokenise(char* str, char* toks[], size_t* num_toks) {
    size_t idx = 0, next = 0;

    if (str[idx] != ' ' && str[idx] != '"')
        toks[next++] = str + idx;

    while (str[idx]) {         
        if (str[idx] == '"') {
            str[idx] = 0;
            idx++;

            size_t j = idx;
            while (str[j] != 0 && str[j] != '"') 
                j++;
            
            if (str[j] != '"'){
                char err[100];
                int len = snprintf(err, sizeof(err), "toks: missing closing \" for corresponding quote at %lu\n", idx - 1);
                write(STDERR_FILENO, err, len);
                exit(TOK_FAILURE);
            }

            else {
                if (j != idx)          
                    toks[next++] = str + white_trimmer(str, idx, j);
                
                idx = j;
                str[idx] = 0, idx++;

                if (str[idx] != 0 && str[idx] != ' ')
                    toks[next++] = str + idx;
            }
        }

        else if (str[idx] == ' '){
            while (str[idx] != 0 && str[idx] == ' ')
                str[idx] = 0, idx++;
            
            if (str[idx] != 0 && str[idx] != '"')
                toks[next++] = str + idx;
        }
        
        else 
            idx++;
    }

    toks[next++] = NULL;
    
    *num_toks = next;
}
