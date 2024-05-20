#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include "token.h"
#include "parser.h"
#include "process.h"
#define MAX_LINE_LENGTH 256

int fgpgid;
int shpgid;
int cpgid = 1;
unsigned jobid = 1;
Job *head;

int generate_jobid();

int getfgpgid()
{
    return fgpgid;
}

int getshpgid()
{
    return shpgid;
}

void setfgpgid(int id)
{
    fgpgid = id;
}

void setshpgid(int id)
{
    shpgid = id;
    fgpgid = id;
}

void kill_fg_processes()
{
    if (fgpgid != shpgid)
    {
        killpg(fgpgid, SIGINT);
    }
}

void init_job()
{
    head = (Job *)malloc(sizeof(Job));
    head->jobid = -1;
    head->pgid = -1;
    head->next = head;
    head->prev = head;
}

void create_job(int pgid)
{
    Job *j = (Job *)malloc(sizeof(Job));
    j->jobid = generate_jobid();
    j->pgid = pgid;

    // insert new job
    j->next = head;
    j->prev = head->prev;
    head->prev->next = j;
    head->prev = j;
}

void show_jobs()
{
    Job *j;
    for (j = head->next; j != head; j = j->next)
    {
        printf("[%d] %d\n", j->jobid, j->pgid);
    }
}

int generate_jobid()
{
    return jobid++;
}