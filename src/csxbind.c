#include "csxbind.h"

#include <stdarg.h>


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


static csxi csx;
static int initialised = 0;


void init()
{
    initialised = 1;
    csx_init(&csx);
    csxbasenames *n = &csx.basenames;
    name       = n->name;
    pair       = n->pair;
    str        = n->str;
    _int       = n->_int;
    base       = n->base;
    fn         = n->fn;
    sx         = n->sx;
    set        = n->set;
    _isset     = n->_isset;
    sethead    = n->sethead;
    settail    = n->settail;
    head       = n->tail;
    tail       = n->tail;
    qt         = n->qt;
    same       = n->same;
    type       = n->type;
    _do        = n->_do;
    _if        = n->_if;
    _plus      = n->_plus;
    _star      = n->_star;
    neg        = n->neg;
    div        = n->div;
    mod        = n->mod;
    _less      = n->_less;
    _more      = n->_more;
    out        = n->out;
    in         = n->in;
    len        = n->len;
    run        = n->run;
    context    = n->context;
    newcontext = n->newcontext;
    _exit      = n->_exit;
}


csxpair *L(void *head, ...)
{
    va_list args;
    csxpair *res;
    csxpair **p;
    if (!head) return csx.null;
    res = csx_pair(&csx, head, csx.null);
    p = (csxpair **)&res->tail;
    va_start(args, head);
    for (head = va_arg(args, void *); head; head = va_arg(args, void *)) {
        *p = csx_pair(&csx, head, csx.null);
        p = (csxpair **)&(*p)->tail;
    }
    va_end(args);
    return res;
}
csxpair *D(void *a, void *b, void *c, ...)
{
    va_list args;
    csxpair *res;
    csxpair **p;
    if (!c) return csx_pair(&csx, a, b);
    res = csx_pair(&csx, a, b);
    p = (csxpair **)&res->tail;
    *p = csx_pair(&csx, *p, c);
    p = (csxpair **)&(*p)->tail;
    va_start(args, c);
    for (c = va_arg(args, void *); c; c = va_arg(args, void *)) {
        *p = csx_pair(&csx, *p, c);
        p = (csxpair **)&(*p)->tail;
    }
    va_end(args);
    return res;
}
csxpair *P(void *head, void *tail)
{
    return csx_pair(&csx, head, tail);
}
char *N(const char *name)
{
    return csx_name(&csx, name);
}
char *S(const char *str)
{
    return csx_str(&csx, str);
}
int *I(int num)
{
    return csx_int(&csx, num);
}

void *R(void *expression)
{
    return csx_run(&csx, expression);
}

csxbase *newbase(csxbase base)
{
    return csx_base(&csx, base);
}
