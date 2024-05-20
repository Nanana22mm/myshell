typedef struct job_s
{
    int pgid;
    int jobid;
    struct job_s *prev;
    struct job_s *next;
} Job;

extern int getfgpgid();
extern int getshpgid();
extern void setfgpgid(int id);
extern void setshpgid(int id);
extern void kill_fg_processes();

extern void init_job();
extern void create_job(int pgid);
extern void show_jobs();