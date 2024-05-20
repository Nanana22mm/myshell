
#define MAX_LEN 50
#define LEN 30

enum Type
{
    TKN_STR = 1,
    TKN_REDIR_IN,
    TKN_REDIR_OUT,
    TKN_REDIR_APPEND,
    TKN_PIPE,
    TKN_BG,
    TKN_EOL,
    TKN_EOF,
    TKN_NOCOMMAND,
};

enum State
{
    Start = 1,
    Word,
    Make,
    Exit,
};

extern void gettoken(char *line);

extern int ac;
extern char *av[LEN];
extern int av_type[LEN];
extern enum Type type;
