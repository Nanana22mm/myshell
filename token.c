#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include <ctype.h>
#include "token.h"

enum State state;

void gettoken(char *line)
{
    int i = 0;
    int fin = 0;
    int first = 0;
    char *cp = line;
    char *p = line;

    state = Start;

    while (!fin)
    {
        switch (state)
        {
        case Start:
            if (first == 0)
            {
                switch (*cp)
                {
                case ' ':
                case '\t':
                    cp++;
                    p++;
                    break;

                case '\n':
                case '|':
                case '>':
                case '<':
                case '&':
                    av_type[ac] = TKN_NOCOMMAND;
                    ac ++;
                    state = Exit;
                    break;

                default:
                    state = Word;
                    break;
                }
                first++;
            }
            else
            {
                switch (*cp)
                {
                case ' ':
                case '\t':
                    cp++;
                    p++;
                    break;

                case '\n':
                    av_type[ac] = TKN_EOL;
                    ac ++;
                    *cp = '\0';
                    state = Exit;
                    break;

                case '|':
                    av_type[ac] = TKN_PIPE;
                    cp++;
                    p++;
                    ac++;
                    state = Start;
                    break;

                case '>':
                    if (*(cp + 1) == '>')
                    {
                        cp++;
                        p++;
                        av_type[ac] = TKN_REDIR_APPEND;
                    }
                    else
                    {
                        av_type[ac] = TKN_REDIR_OUT;
                    }
                    ac++;
                    cp++;
                    p++;
                    state = Start;
                    break;

                case '<':
                    av_type[ac] = TKN_REDIR_IN;
                    ac++;
                    cp++;
                    p++;
                    state = Start;
                    break;

                case '&':
                    av_type[ac] = TKN_BG;
                    ac++;
                    cp++;
                    p++;
                    state = Start;
                    break;

                default:
                    switch (*(cp + 1))
                    {
                    case '|':
                    case '>':
                    case '<':
                    case '&':
                        state = Make;
                        break;
                    default:
                        state = Word;
                        break;
                    }
                    break;
                }
            }
            break;

        case Word:
            switch (*cp)
            {
            case ' ':
            case '\t':
            case '\n':
                state = Make;
                break;

            default:
                switch (*(cp + 1))
                {
                case '|':
                case '>':
                case '<':
                case '&':
                    state = Make;
                    break;

                default:
                    cp++;
                    break;
                }
            }
            break;

        case Make:
            switch (*cp)
            {
            case '\n':
                av[ac] = p;
                av_type[ac] = TKN_STR;
                *cp = '\0';
                ac++;
                av[ac] = NULL;
                av_type[ac] = TKN_EOL;
                ac++;
                state = Exit;
                break;

            case ' ':
            case '\t':
                av[ac] = p;
                av_type[ac] = TKN_STR;
                *cp = '\0';
                cp++;
                p += strlen(av[ac]) + sizeof(*cp);
                ac++;
                state = Start;
                break;

            default:
                switch (*(cp + 1))
                {
                case '|':
                case '>':
                case '<':
                case '&':
                    av[ac] = p;
                    av_type[ac] = TKN_STR;
                    cp++;
                    p += strlen(av[ac]);

                    ac++;
                    switch (*(cp))
                    {
                    case '|':
                        av_type[ac] = TKN_PIPE;
                        break;
                    case '>':
                        if (*(cp + 1) == '>')
                        {
                            *cp = '\0';
                            cp++;
                            p++;
                            av_type[ac] = TKN_REDIR_APPEND;
                        }
                        else
                        {
                            av_type[ac] = TKN_REDIR_OUT;
                        }
                        break;
                    case '<':
                        av_type[ac] = TKN_REDIR_IN;
                        break;
                    case '&':
                        av_type[ac] = TKN_BG;
                        break;
                    }
                    *cp = '\0';
                    av[ac] = '\0';
                    p++;
                    ac++;
                    cp++;
                    p = cp;
                    state = Start;
                    break;
                }
            }

            break;

        case Exit:
            fin++;
            break;
        }
    }
}