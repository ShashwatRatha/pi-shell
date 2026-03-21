#include "builtins.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#define GRP_INIT 8

static int execPipeline(CommandGrp);

int execute(char* cmdstr) {
    Commands commands; 
    Delims dlms;

    if (parse(cmdstr, &commands, &dlms) != SUCCESS)
        return EXEC_FAILURE;

    Command* cmds = commands.cmds;

    for (size_t dlm_idx = 0; dlm_idx < dlms.count; dlm_idx++) {    
        // create command groups
        Command* group = malloc(GRP_INIT * sizeof(*group));
        if (!group) 
            return ALLOC_FAILURE;
        size_t grp_idx = 0, grp_size = GRP_INIT;

        group[grp_idx++] = cmds[dlm_idx];
        for (;(dlm_idx < dlms.count) && (dlms.delims[dlm_idx] == PIPE); dlm_idx++, grp_idx++) {
            if (grp_idx >= grp_size - 1) {
                grp_size <<= 1;
                Command* tmp = realloc(group, grp_size * sizeof(*tmp));
                if (!tmp) {
                    free(group);
                    return ALLOC_FAILURE;
                }
                group = tmp;
            }

            group[grp_idx] = cmds[dlm_idx+1];
        }

        CommandGrp cmdgrp = (CommandGrp) {
            .group = group,
            .size = grp_idx,
            .bgproc = 0
        };

        if (dlms.delims[dlm_idx] == BG_PROC)
            cmdgrp.bgproc = 1;
        
        int ret = execPipeline(cmdgrp);
        if (dlm_idx >= dlms.count)
            return SUCCESS;
        else if (dlms.delims[dlm_idx] == AND_OP && ret != SUCCESS)
            return EXEC_FAILURE;
        else if (dlms.delims[dlm_idx] == OR_OP && ret == SUCCESS)
            return EXEC_FAILURE;

        free(group);
    }

    return SUCCESS;
}

static int execPipeline(CommandGrp cmdgrp) {
    Command* group = cmdgrp.group;
    int num = cmdgrp.size;
    bool bg = cmdgrp.bgproc;

    int pipefds[num-1][2];
    for (int i = 0; i < num-1; i++)
        if (pipe(pipefds[i]) == -1) {
            perror("pipe");
            return PIPE_FAILURE; 
        }

    pid_t pids[num];
    for (int i = 0; i < num; i++)
        pids[i] = -1;

    for (int i = 0; i < num; i++) {
        if (execBuiltin(group[i].argv) != -1)
            continue;

        pid_t pid = fork();

        // child
        if (pid == 0) {
            if (i > 0)
                dup2(pipefds[i-1][0], STDIN_FILENO);
            if (i < num - 1)
                dup2(pipefds[i][1], STDOUT_FILENO);

            if (group[i].instream != STDIN_FILENO) {
                dup2(group[i].instream, STDIN_FILENO);
                close(group[i].instream);
            }
            if (group[i].outstream != STDOUT_FILENO) {
                dup2(group[i].outstream, STDOUT_FILENO);
                close(group[i].outstream);
            }

            for (int j = 0; j < num - 1; j++){
                close(pipefds[j][0]);
                close(pipefds[j][1]);
            }

            if (execvp(group[i].argv[0], group[i].argv) == -1) {
                perror("exec");
                exit(EXEC_FAILURE);
            }
        } 

        else if (pid < 0) {
            perror("fork");
            return FORK_FAILURE;
        }

        pids[i] = pid;
    }

    for (int i = 0; i < num - 1; i++) {
        close(pipefds[i][0]);
        close(pipefds[i][1]);
    }

    int status = SUCCESS;
    if (!bg) {
        for (int i = 0; i < num - 1; i++)
            if (pids[i] != -1)
            waitpid(pids[i], NULL, 0);
        waitpid(pids[num - 1], &status, 0);
    }

    return status;
}
