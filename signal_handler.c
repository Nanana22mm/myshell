#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include "token.h"
#include "parser.h"
#include "process.h"
#include "signal_handler.h"

void sigint_handler(int sig);
void init_signal_handler();

void sigint_handler(int sig)
{
    kill_fg_processes();
    tcsetpgrp(0, getshpgid());
    write(1, "\n", 1);
}

void sigtstp_handler(int sig)
{
    char *msg = "SIGTSTP is detected.\n";
    write(1, msg, strlen(msg));
}

void sigchild_handler(int sig)
{
    while (waitpid((pid_t)-1, NULL, WNOHANG) > 0)
        ;
}

void init_signal_handler()
{
    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_handler = sigint_handler;
    sigaction(SIGINT, &act, NULL);

    memset(&act, 0, sizeof(act));
    act.sa_handler = sigtstp_handler;
    sigaction(SIGTSTP, &act, NULL);
}