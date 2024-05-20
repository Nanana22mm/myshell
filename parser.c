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

#define EXIT "exit"
#define PWD "pwd"
#define CD "cd"
#define JOBS "jobs"
#define PATHNAME_SIZE 512

void parser(int ac, char *av[], int type);
void exec_com(char *command[], int bg_flag, int fd_in, int fd_out);
void exec_pipe(int ac, char *av[], int pipe_all, int bg_flag);
int check_file(char *file_name, int type);
void redirect_in(char *command[], int fd, int bg_flag);
void redirect_out(char *command[], int fd, int bg_flag);

void exec_com(char *command[], int bg_flag, int fd_in, int fd_out)
{
    int pid = 0;
    int status;
    int i = 0;
    char *p = getenv("PATH");
    char fullpath[100];
    memset(fullpath, 0, sizeof fullpath);

    if ((pid = fork()) > 0)
    {
        if (bg_flag)
        {
            create_job(pid);
            setpgid(pid, pid);
            show_jobs();
            return;
        }
        else
        {
            setpgid(getpid(), getfgpgid());

            if (fd_in != 0)
                close(fd_in);
            if (fd_out != 1)
                close(fd_out);

            wait(&status);
            if (WIFEXITED(status))
                return;
            return;
        }
    }
    else if (pid == 0)
    {
        if (fd_in != 0)
        {
            if (dup2(fd_in, 0) < 0)
            {
                perror("dup2_in");
                close(fd_in);
                exit(1);
            }    
            close(fd_in);
        }
        if (fd_out != 1)
        {
            if (dup2(fd_out, 1) < 0)
            {
                perror("dup2_out");
                close(fd_out);
                exit(1);
            }
            close(fd_out);
        }
        // execve
        if (!access(command[0], F_OK))
        {
            if (execve(command[0], command, NULL) == -1)
            {
                perror(command[0]);
                exit(1);
            }
        }

        if (!strtok(p, ":"))
        {
            fprintf(stderr, "command not found\n");
            exit(1);
        }
        sprintf(fullpath, "%s/%s", p, command[0]);

        if (!access(fullpath, F_OK))
        {
            if (execve(fullpath, command, NULL) == -1)
            {
                perror(command[0]);
                exit(1);
            }
        }

        while ((p = strtok(NULL, ":")) != NULL)
        {
            memset(fullpath, 0, sizeof fullpath);
            sprintf(fullpath, "%s/%s", p, command[0]);

            if (access(fullpath, F_OK))
            {
                continue;
            }
            if (execve(fullpath, command, NULL) == -1)
            {
                perror(command[0]);
                exit(1);
            }
        }
        fprintf(stderr, "command not found\n");
        exit(1);
    }
    else
    {
        perror("fork");
        return;
    }
}

void exec_pipe(int ac, char *av[], int pipe_all, int bg_flag)
{
    int i = 0;
    int count = 0;
    int pipe_count = 0;
    char *av_cp[LEN];
    int pid = 0;
    int status1;
    int pipefds[2];
    int fd_in = 0;
    int fd_out = 1;
    int fd = -1; //for file
    char *file_name;
    memset(av_cp, 0, sizeof(av_cp));

    for (i = 0; i < ac; i++)
    {
        if (pipe_count > pipe_all)
        {
            return;
        }
        switch (av_type[i])
        {
        case TKN_PIPE:
        case TKN_EOL:
            if (pipe(pipefds))
            {
                perror("pipe");
                return;
            }

            // if the command is after all pipes, make fd_out 1.
            if (fd_out != fd)
            {
                switch (av_type[i])
                {
                case TKN_PIPE:
                    fd_out = pipefds[1];
                    break;
                case TKN_EOL:
                    fd_out = 1;
                    break;
                }
            }
            
            pipe_count++;
            //fprintf(stderr, "%s: in = %d, out = %d, fd = %d\n", av_cp[0], fd_in, fd_out, fd);

            exec_com(av_cp, bg_flag, fd_in, fd_out);
            
            fd_in = pipefds[0];

            memset(av_cp, 0, sizeof(av_cp));
            count = 0;    
            break;

        case TKN_REDIR_APPEND:
        case TKN_REDIR_OUT:
        case TKN_REDIR_IN:
            file_name = av[i + 1];

            switch (av_type[i])
            {
                case TKN_REDIR_APPEND:
                    fd = check_file(file_name, TKN_REDIR_APPEND);
                    fd_out = fd;
                    break;
                case TKN_REDIR_OUT:
                    fd = check_file(file_name, TKN_REDIR_OUT);
                    fd_out = fd;
                    break;
                case TKN_REDIR_IN:
                    fd = check_file(file_name, TKN_REDIR_IN);  
                    fd_in = fd;
                    break;
            }

            //fprintf(stderr, "%s: in = %d, out = %d, fd = %d\n", av_cp[0], fd_in, fd_out, fd);
            i++;
            break;

        default:
            av_cp[count] = av[i];
            count++;
        }
    }
}

int check_file(char *file_name, int type)
{
    int fd = 0;

    switch (type)
    {
    case TKN_REDIR_APPEND:
        if ((fd = open(file_name, O_WRONLY | O_APPEND | O_CREAT, 0644)) < 0)
        {
            perror(file_name);
            return -1;
        }
        break;
    case TKN_REDIR_OUT:
        if ((fd = open(file_name, O_WRONLY | O_TRUNC | O_CREAT, 0644)) < 0)
        {
            perror(file_name);
            return -1;
        }
        break;

    case TKN_REDIR_IN:
        if ((fd = open(file_name, O_RDONLY)) < 0)
        {
            perror(file_name);
            return -1;
        }
        break;
    }
}

void redirect_in(char *command[], int fd, int bg_flag)
{
    int fd_in = fd;
    int fd_out = 1; 

    exec_com(command, bg_flag, fd_in, fd_out);
    close(fd);
}

void redirect_out(char *command[], int fd, int bg_flag)
{
    int fd_in = 0;
    int fd_out = fd;    

    exec_com(command, bg_flag, fd_in, fd_out);
    close(fd);
}

void parser(int ac, char *av[], int type)
{
    int i = 0;
    int j = 0;
    int count = 0;
    int pipe_count = 0;
    int pipe_all = 0;
    int bg_flag = 0;
    int fd = 0;
    int fd_in = 0;
    int fd_out = 1;
    int fd_err = 2;
    int backup = 0;
    int backup_in = 0;
    int backup_out = 1;
    backup_in = dup(0);
    backup_out = dup(1);
    int jobid = 0;
    int pid;
    int status;
    char *file_name;
    char *av_cp[LEN];
    char *command[LEN];
    char pathname[PATHNAME_SIZE];
    memset(pathname, 0, PATHNAME_SIZE);
    memset(command, 0, sizeof(command));
    memset(av_cp, 0, sizeof(av_cp));

    // if pipe, jump exec_pipe
    for (i = 0; i < ac; i++)
    {
        if (av_type[i] == TKN_PIPE)
            pipe_all++;
        else if (av_type[i] == TKN_BG)
        {
            bg_flag = 1;

            struct sigaction act;
            memset(&act, 0, sizeof(act));
            act.sa_handler = sigint_handler;
            sigaction(SIGINT, &act, NULL);

            memset(&act, 0, sizeof(act));
            act.sa_handler = sigchild_handler;
            sigaction(SIGINT, &act, NULL);
        }
    }
    if (pipe_all > 0)
    {
        exec_pipe(ac, av, pipe_all, bg_flag);
        dup2(backup_in, 0);
        close(backup_in);
        dup2(backup_out, 1);
        close(backup_out);
        return;
    }

    // else, execve
    for (i = 0; i < ac; i++)
    {
        switch (av_type[i])
        {
        case TKN_NOCOMMAND:
            fprintf(stderr, "there is no such command.\n");
            return;

        case TKN_REDIR_APPEND:
            //fprintf(stderr, "TKN_REDIR_APPEND\n");
            if (i == 0)
            {
                fprintf(stderr, "Usage is wrong.\n");
                return;
            }
            else if (i == ac - 1)
            {
                fprintf(stderr, "Usage is wrong.\n");
                return;
            }

            backup = dup(1);
            command[0] = av_cp[0];
            for (j = 0; j < count; j++)
            {
                command[j] = av_cp[j];
            }
            command[j] = NULL;

            file_name = av[i + 1];
            i++;
            fd = check_file(file_name, TKN_REDIR_APPEND);
            redirect_out(command, fd, bg_flag);
            close(fd);
            dup2(backup, 1);
            close(backup);

            memset(av_cp, 0, sizeof(av_cp));
            count = 0;
            break;

        case TKN_REDIR_IN:
            //fprintf(stderr, "TKN_REDIR_IN\n");
            if (i == 0)
            {
                fprintf(stderr, "Usage is wrong.\n");
                return;
            }
            else if (i == ac - 1)
            {
                fprintf(stderr, "Usage is wrong.\n");
                return;
            }

            backup = dup(0);
            command[0] = av_cp[0];
            for (j = 0; j < count; j++)
            {
                command[j] = av_cp[j];
            }
            command[j] = NULL;

            file_name = av[i + 1];
            i++;
            fd = check_file(file_name, TKN_REDIR_IN);
            redirect_in(command, fd, bg_flag);
            close(fd);
            dup2(backup, 0);
            close(backup);

            memset(av_cp, 0, sizeof(av_cp));
            count = 0;
            break;

        case TKN_REDIR_OUT:
            //fprintf(stderr, "TKN_REDIR_OUT\n");
            if (i == 0)
            {
                fprintf(stderr, "Usage is wrong.\n");
                return;
            }
            else if (i == ac - 1)
            {
                fprintf(stderr, "Usage is wrong.\n");
                return;
            }

            backup = dup(1);
            command[0] = av_cp[0];
            for (j = 0; j < count; j++)
            {
                command[j] = av_cp[j];
            }
            command[j] = NULL;

            file_name = av[i + 1];
            i++;
            fd = check_file(file_name, TKN_REDIR_OUT);
            redirect_out(command, fd, bg_flag);
            close(fd);
            dup2(backup, 1);
            close(backup);

            memset(av_cp, 0, sizeof(av_cp));
            count = 0;
            break;

        case TKN_STR:
            //fprintf(stderr, "TKN_STR\n");
            if (strcmp(av[i], EXIT) == 0)
            {
                exit(0);
            }
            else if (strcmp(av[i], CD) == 0)
            {
                if ((chdir(av[i + 1])) < 0)
                {
                    perror("chdir");
                }
                return;
            }
            else if (strcmp(av[i], PWD) == 0)
            {
                getcwd(pathname, PATHNAME_SIZE);
                fprintf(stderr, "%s\n", pathname);
                return;
            }
            else if (strcmp(av[i], JOBS) == 0)
            {
                show_jobs();
                return;
            }
            else
            {
                // stock an ordinal str such as commands and file names in av_cp
                av_cp[count] = av[i];
                count++;

                av_cp[count] = NULL;
                if ((i < ac - 1) & (av_type[i + 1] == TKN_EOL) | (av_type[i + 1] == TKN_BG))
                {
                    exec_com(av_cp, bg_flag, fd_in, fd_out);
                    memset(av_cp, 0, sizeof(av_cp));
                    count = 0;

                    if (av_type[i + 1] == TKN_BG || av_type[i + 1] == TKN_EOL)
                        return;

                }
            }
        }
    }
}