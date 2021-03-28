#ifndef CSX_INCLUDED_BIND
#define CSX_INCLUDED_BIND


#include "csx.h"


extern char *name;
extern char *pair;
extern char *str;
extern char *_int;
extern char *base;
extern char *fn;
extern char *sx;
extern char *set;
extern char *_isset;
extern char *sethead;
extern char *settail;
extern char *head;
extern char *tail;
extern char *qt;
extern char *same;
extern char *type;
extern char *_do;
extern char *_if;
extern char *_plus;
extern char *_star;
extern char *neg;
extern char *div;
extern char *mod;
extern char *_less;
extern char *_more;
extern char *out;
extern char *in;
extern char *len;
extern char *run;
extern char *context;
extern char *newcontext;
extern char *_exit;

void csxbind_init();
void csxbind_free();

csxpair *L(void *head, ...);
csxpair *D(void *a, void *b, void *c, ...);
csxpair *P(void *head, void *tail);
char    *N(const char *name);
char    *S(const char *str);
int     *I(int num);

void *R(void *expression);

csxbase *newbase(csxbase base);


#endif
