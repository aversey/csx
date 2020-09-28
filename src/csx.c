#include "csx.h"

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>


enum { type_nil, type_pair, type_string, type_atom, type_fn };
static int  *type(char *p)         { return (int *) (p - sizeof(int)); }
static void  type_shift(void *p)  { *(char **)p += sizeof(int); }
static void *type_alloc(int t, int content_size)
{
    int *res = malloc(sizeof(int) + content_size);
    *res     = t;
    type_shift(&res);
    return res;
}


struct deflist {
    char *atom;
    void *data;
    struct deflist *next;
};

static struct state {
    struct deflist *atoms;
} csx;
static int csx_to_init = 0;

static void *nil;


static void adddef(struct deflist **l, const char *atom, void *data)
{
    struct deflist *old = *l;
    *l = malloc(sizeof(**l));
    (*l)->next = old;
    (*l)->atom = scopy(atom);
    (*l)->data = data;
}

static void *csx_fn_new(void *fn, void *userdata, int size)
{
    void *res = malloc(sizeof(fn) + sizeof(int) + size);
    *(void **)res = fn;
    (char *)res += sizeof(fn);
    *(int *)res = type_fn;
    typeshift(&res);
    if (userdata && size) memcpy(res, userdata, size);
    return res;
}

static void *define(void *userdata, struct csx_pair *data)
{  /* TODO! */
    int t = *type(data);
    if (t != type_pair) return nil;
    t = *type(data->car);
    if (t == type_atom) {
        struct csx_pair *cdr = data->cdr;
        if (*type(cdr) != type_pair || *type(cdr->cdr) != type_nil)
            return nil;
        return define_constant(data->car, cdr->cdr);
    }
    return define_function(data->car, data->cdr);
}

/* TODO append, cdr, ... */

static void csx_init()
{
    if (!csx_to_init) csx_free();
    csx.atoms = 0;

    nil = type_alloc(type_nil, 0);
    adddef(csx.atoms, "append", csx_fn_new(append, 0, 0));
    adddef(csx.atoms, "define", csx_fn_new(define, 0, 0));
    adddef(csx.atoms, "cdr",    csx_fn_new(cdr,    0, 0));
    adddef(csx.atoms, "ifx",    csx_fn_new(ifx,    0, 0));
    adddef(csx.atoms, "not",    csx_fn_new(not,    0, 0));
}

void csx_free()
{
    free(nil);
    /* free everything allocated */
    csx_to_init = 0;
}


struct csx_pair *csx_pair_new(void *car, void *cdr)
{
    struct csx_pair *res =type_alloc(type_pair, sizeof(*res));
    res->car = car;
    res->cdr = cdr;
    return res;
}


struct csx_pair *csx_list(void *head, ...)
{
    struct csx_pair *res;
    va_list args;
    if (csx_to_init) csx_init();
    if (!head) return nil;

    res = csx_pair_new(head, nil);
    va_start(args, head);
    for (head = va_arg(args, void *); head; head = va_arg(args, void *)) {
        res->cdr = csx_pair_new(head, nil);
    }
    va_end(args);
    return res;
}

char *csx_string(const char *string)
{
    int   strsize;
    void *res;
    if (csx_to_init) csx_init();
    strsize = strlen(string) + 1;
    res     = type_alloc(type_string, strsize);
    memcpy(res, string, strsize);
    return res;
}

char *csx_atom(const char *name)
{
    int   namesize;
    void *res;
    if (csx_to_init) csx_init();
    namesize = strlen(name) + 1;
    res      = type_alloc(type_atom, namesize);
    memcpy(res, name, namesize);
    return res;
}


static void *lookup(struct deflist *d, char *atom)
{
    if (!d) return nil;
    if (!strcmp(d->atom, atom)) return d->data;
    return lookup(d->next, atom);
}

void *eval_fn(void *fn, void *args)
{  /* TODO! */
    /* function : type :> userdata */
    return 0;
}

static void *eval_pair(struct csx_pair *pair)
{
    void *fn;
    int t = *type(pair->car);
    if (t == type_fn) fn = pair->car;
    else              fn = csx_evaluate(pair->car);
    return eval_fn(fn, pair->cdr);
}

void *csx_evaluate(void *expression)
{
    int t = *type(expression);
    if (t == type_atom) return lookup(csx.atoms, expression);
    if (t != type_pair) return nil;
    return eval_pair(expression);
}
