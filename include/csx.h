#ifndef CSX_INCLUDED
#define CSX_INCLUDED


typedef struct csxobj {
    int mark;
    void *type;
} csxobj;

csxobj *csx_obj(void *x);


typedef struct csxpair {
    void *head;
    void *tail;
} csxpair;


typedef struct csxbasenames {
    char *name;
    char *pair;
    char *str;
    char *_int;
    char *base;
    char *fn;
    char *sx;
    char *set;
    char *_isset;
    char *sethead;
    char *settail;
    char *head;
    char *tail;
    char *qt;
    char *same;
    char *type;
    char *_do;
    char *_if;
    char *_plus;
    char *_star;
    char *neg;
    char *div;
    char *mod;
    char *_less;
    char *_more;
    char *out;
    char *in;
    char *len;
    char *run;
    char *context;
    char *newcontext;
    char *_exit;
} csxbasenames;

typedef struct csxi {
    csxobj **objs;
    int objslen;
    int objssize;
    void **stack;
    int stacklen;
    int stacksize;
    int lastlen;
    void *null;
    int *one;
    csxbasenames basenames;
    csxpair *context;
} csxi;

void csx_init(csxi *csx);
void csx_free(csxi *csx);

csxpair *csx_list(csxi *csx, void *head, ...);
csxpair *csx_dot(csxi *csx, void *a, void *b, void *c, ...);
csxpair *csx_pair(csxi *csx, void *head, void *tail);
char    *csx_name(csxi *csx, const char *name);
char    *csx_str(csxi *csx, const char *str);
int     *csx_int(csxi *csx, int num);

void *csx_run(csxi *csx, void *expression);

typedef void *(*csxbase)(csxi *csx, void *arg);
csxbase *csx_base(csxi *csx, csxbase base);


#endif
