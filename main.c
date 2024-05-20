// 62008208 齊藤七菜

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <ctype.h>
#include "token.h"
#include "parser.h"
#include "process.h"
#include "signal_handler.h"

int ac = 0;
char *av[LEN];
int av_type[LEN];
enum Type type;
char line[MAX_LEN];

int main()
{
    int i = 0;
    memset(line, 0, sizeof(line));
    memset(av, 0, sizeof(av));
    memset(av_type, 0, sizeof(av_type));

    setshpgid(getpgrp());
    init_job();
    init_signal_handler();

    for (;;)
    {
        i = 0;
        ac = 0;
        memset(line, 0, sizeof(line));
        memset(av, 0, sizeof(av));
        memset(av_type, 0, sizeof(av_type));
        fprintf(stderr, "$ ");

        if (fgets(line, MAX_LEN, stdin) == NULL)
        {
            if (ferror(stdin))
            {
                continue;
            }
            else
            {
                // end of file
                type = TKN_EOF;
            }
        }

        // if line is one character and it is '\n', go to the begin of the loop
        if (*line == '\n')
        {
            continue;
        }

        gettoken(line);

        if (av_type[0] == TKN_NOCOMMAND)
        {
            fprintf(stderr, "there is no such command.\n");
            continue;
        }
        else if (av[0] == NULL)
        {
            // if there is only space or tab, continue
            continue;
        }
        else
        {
            parser(ac, av, type);
        }
    }
}