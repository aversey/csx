#include "csx.h"

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>


typedef struct fn_data {
    csxpair *params;
    csxpair *body;
    csxpair *context;
} fn_data;


csxobj *csx_obj(void *p)  { return (csxobj *)p - 1; }


static void pushobj(csxi *csx, void *p)
{
    if (!csx->objs) {
        csx->objs = malloc(sizeof(void *));
        csx->objslen = 0;
        csx->objssize = 1;
    }
    if (csx->objslen == csx->objssize) {
        csx->objssize *= 2;
        csx->objs = realloc(csx->objs, csx->objssize * sizeof(void *));
    }
    csx->objs[csx->objslen++] = p;
}

static void push(csxi *csx, void *p)
{
    if (!csx->stack) {
        csx->stack = malloc(sizeof(void *));
        csx->stacklen = 0;
        csx->stacksize = 1;
    }
    if (csx->stacklen == csx->stacksize) {
        csx->stacksize *= 2;
        csx->stack = realloc(csx->stack, csx->stacksize * sizeof(void *));
    }
    csx->stack[csx->stacklen++] = p;
}

static void *pop(csxi *csx)  { return csx->stack[--(csx->stacklen)]; }

static void restore(csxi *csx, int len)
{
    csx->stacklen = len;
    if (csx->stacklen <= csx->stacksize / 4) {
        csx->stacksize /= 2;
        csx->stack = realloc(csx->stack, csx->stacksize * sizeof(void *));
    }
}

static void *new(csxi *csx, void *type, int data_size)
{
    csxobj *res = malloc(sizeof(*res) + data_size);
    if (!res) exit(1);
    res->mark = 0;
    res->type = type;
    pushobj(csx, res);
    return res + 1;
}


static void *head(csxpair *pair)  { return pair->head; }
static void *tail(csxpair *pair)  { return pair->tail; }

csxpair *csx_pair(csxi *csx, void *head, void *tail)
{
    csxpair *res = new(csx, csx->basenames.pair, sizeof(*res));
    res->head = head;
    res->tail = tail;
    return res;
}


static void deepmark(csxi *csx, void *p)
{
    if (csx_obj(p)->mark) return;
    csx_obj(p)->mark = 1;
    deepmark(csx, csx_obj(p)->type);
    if (csx_obj(p)->type == csx->basenames.pair) {
        deepmark(csx, head(p));
        deepmark(csx, tail(p));
    } else if (csx_obj(p)->type == csx->basenames.fn ||
               csx_obj(p)->type == csx->basenames.sx) {
        fn_data *fn = p;
        deepmark(csx, fn->params);
        deepmark(csx, fn->body);
        deepmark(csx, fn->context);
    }
}

static void sweep(csxi *csx)
{
    csxobj **old = csx->objs;
    int len = csx->objslen;
    int i;
    csx->objs = 0;
    for (i = 0; i != len; ++i) {
        if (old[i]->mark || old[i]->type == csx->basenames.name) {
            old[i]->mark = 0;
            pushobj(csx, old[i]);
        } else {
            free(old[i]);
        }
    }
    free(old);
}

static void gc(csxi *csx)
{
    int i;
    if (csx->objslen < csx->lastlen * 2) return;
    csx_obj(csx->null)->mark = 1;
    csx_obj(csx->one)->mark  = 1;
    deepmark(csx, csx->context);
    for (i = 0; i != csx->stacklen; ++i) {
        deepmark(csx, csx->stack[i]);
    }
    sweep(csx);
    csx->lastlen = csx->objslen;
}


char *csx_name(csxi *csx, const char *name)
{
    void *res;
    const int namesize = strlen(name) + 1;
    int i;
    for (i = 0; i != csx->objslen; ++i) {
        void *obj = (int *)(csx->objs[i]) + 2;
        if (csx_obj(obj)->type != csx->basenames.name) continue;
        if (!strncmp(obj, name, namesize)) return obj;
    }
    res = new(csx, csx->basenames.name, namesize);
    memcpy(res, name, namesize);
    return res;
}

csxbase *csx_base(csxi *csx, csxbase base)
{
    csxbase *res = new(csx, csx->basenames.base, sizeof(*res));
    *res = base;
    return res;
}

int *csx_int(csxi *csx, int num)
{
    int *res = new(csx, csx->basenames._int, sizeof(*res));
    *res = num;
    return res;
}


static void *run_each(csxi *i, void *l)
{
    int ret;
    csxpair *res;
    csxpair **p;
    if (csx_obj(l)->type != i->basenames.pair) return i->null;
    push(i, l);
    ret = i->stacklen;
    res = csx_pair(i, csx_run(i, head(l)), i->null);
    p = (csxpair **)&res->tail;
    l = tail(l);
    while (csx_obj(l)->type == i->basenames.pair) {
        int resret;
        push(i, res);
        resret = i->stacklen;
        *p = csx_pair(i, csx_run(i, head(l)), i->null);
        p = (csxpair **)&(*p)->tail;
        restore(i, resret);
        pop(i);
        l = tail(l);
    }
    restore(i, ret);
    pop(i);
    return res;
}


static void *lookup_frame(csxi *i, const char *name)
{
    csxpair *frame = i->context->head;
    while (csx_obj(frame)->type == i->basenames.pair) {
        if (head(frame->head) == name) return frame->head;
        frame = frame->tail;
    }
    return i->null;
}

static void *lookup(csxi *i, const char *name)
{
    void *saved = i->context;
    while (csx_obj(i->context)->type == i->basenames.pair) {
        csxpair *res = lookup_frame(i, name);
        if (res != i->null) {
            i->context = saved;
            return res->tail;
        }
        i->context = i->context->tail;
    }
    i->context = saved;
    return i->null;
}

static void *base_set(csxi *i, void *arg)
{
    int ret;
    csxpair *res;
    void *name;
    void *value;
    push(i, arg);
    ret = i->stacklen;
    name = head(arg);
    value = csx_run(i, head(tail(arg)));
    if (i->context == i->null) {
        void *nameval = csx_pair(i, name, value);
        i->context = csx_pair(i, csx_pair(i, nameval, i->null), i->null);
        restore(i, ret);
        pop(i);
        return i->null;
    }
    res = lookup_frame(i, name);
    if (res != i->null) {
        res->tail = value;
    } else {
        void **names = &i->context->head;
        *names = csx_pair(i, csx_pair(i, name, value), *names);
    }
    restore(i, ret);
    pop(i);
    return i->null;
}

static void *base_is_set(csxi *i, void *arg)
{
    arg = run_each(i, arg);
    return lookup(i, head(arg)) ? i->one : i->null;
}

static void *base_sethead(csxi *i, void *arg)
{
    csxpair *p;
    arg = run_each(i, arg);
    p = head(arg);
    p->head = head(tail(arg));
    return i->null;
}

static void *base_settail(csxi *i, void *arg)
{
    csxpair *p;
    arg = run_each(i, arg);
    p = head(arg);
    p->tail = head(tail(arg));
    return i->null;
}

static void *base_pair(csxi *i, void *arg)
{
    arg = run_each(i, arg);
    return csx_pair(i, head(arg), head(tail(arg)));
}

static void *base_head(csxi *i, void *arg)
{
    arg = run_each(i, arg);
    return head(head(arg));
}

static void *base_tail(csxi *i, void *arg)
{
    arg = run_each(i, arg);
    return tail(head(arg));
}

static void *base_qt(csxi *i, void *arg)
{
    return head(arg);
}

static void *base_same(csxi *i, void *arg)
{
    arg = run_each(i, arg);
    if (csx_obj(head(arg))->type == i->basenames._int)
        return *(int *)head(arg) == *(int *)head(tail(arg)) ? i->one : i->null;
    return head(arg) == head(tail(arg)) ? i->one : i->null;
}

static void *base_type(csxi *i, void *arg)
{
    arg = run_each(i, arg);
    return csx_obj(head(arg))->type;
}

static void *base_do(csxi *i, void *arg)
{
    void *res = i->null;
    arg = run_each(i, arg);
    while (csx_obj(arg)->type == i->basenames.pair) {
        res = head(arg);
        arg = tail(arg);
    }
    return res;
}

static void *base_fn(csxi *i, void *arg)
{
    fn_data *res = new(i, i->basenames.fn, sizeof(*res));
    res->params = head(arg);
    res->body = tail(arg);
    res->context = i->context;
    return res;
}

static void *base_sx(csxi *i, void *arg)
{
    fn_data *res = new(i, i->basenames.sx, sizeof(*res));
    res->params = head(arg);
    res->body = tail(arg);
    res->context = i->context;
    return res;
}

static void *base_if(csxi *i, void *arg)
{
    if (csx_obj(arg)->type != i->basenames.pair) return i->null;
    if (tail(arg) == i->null) return csx_run(i, head(arg));
    return csx_run(i, head(arg)) != i->null ?
        csx_run(i, head(tail(arg))) :
        base_if(i, tail(tail(arg)));
}


static void *base_sum(csxi *i, void *arg)
{
    int res = 0;
    arg = run_each(i, arg);
    while (csx_obj(arg)->type == i->basenames.pair) {
        int *num = head(arg);
        if (csx_obj(num)->type != i->basenames._int) exit(1);
        res += *num;
        arg = tail(arg);
    }
    return csx_int(i, res);
}

static void *base_prod(csxi *i, void *arg)
{
    int res = 1;
    arg = run_each(i, arg);
    while (csx_obj(arg)->type == i->basenames.pair) {
        int *num = head(arg);
        if (csx_obj(num)->type != i->basenames._int) exit(1);
        res *= *num;
        arg = tail(arg);
    }
    return csx_int(i, res);
}

static void *base_neg(csxi *i, void *arg)
{
    arg = run_each(i, arg);
    return csx_int(i, -*(int *)head(arg));
}

static void *base_div(csxi *i, void *arg)
{
    arg = run_each(i, arg);
    return csx_int(i, *(int *)head(arg) / *(int *)head(tail(arg)));
}

static void *base_mod(csxi *i, void *arg)
{
    arg = run_each(i, arg);
    return csx_int(i, *(int *)head(arg) % *(int *)head(tail(arg)));
}

static void *base_inc(csxi *i, void *arg)
{
    int num;
    arg = run_each(i, arg);
    if (csx_obj(arg)->type != i->basenames.pair ||
        csx_obj(head(arg))->type != i->basenames._int) return i->null;
    num = *(int *)head(arg);
    arg = tail(arg);
    while (csx_obj(arg)->type == i->basenames.pair) {
        int *another = head(arg);
        if (csx_obj(another)->type != i->basenames._int || *another <= num)
            return i->null;
        num = *another;
        arg = tail(arg);
    }
    return i->one;
}

static void *base_dec(csxi *i, void *arg)
{
    int num;
    arg = run_each(i, arg);
    if (csx_obj(arg)->type != i->basenames.pair ||
        csx_obj(head(arg))->type != i->basenames._int) return i->null;
    num = *(int *)head(arg);
    arg = tail(arg);
    while (csx_obj(arg)->type == i->basenames.pair) {
        int *another = head(arg);
        if (csx_obj(another)->type != i->basenames._int || *another >= num)
            return i->null;
        num = *another;
        arg = tail(arg);
    }
    return i->one;
}


static void *base_out(csxi *i, void *arg)
{
    int res;
    arg = run_each(i, arg);
    res = putchar(*(int *)head(arg));
    return res != EOF ? i->one : i->null;
}

static void *base_in(csxi *i, void *arg)
{
    int res;
    run_each(i, arg);
    res = getchar();
    return res != EOF ? csx_int(i, res) : i->null;
}

static void *base_name(csxi *i, void *arg)
{
    arg = run_each(i, arg);
    if (csx_obj(head(arg))->type != i->basenames.str) exit(1);
    return csx_name(i, head(arg));
}

static void *base_str(csxi *i, void *arg)
{
    void *t;
    int reslen = 0;
    char *res;
    arg = run_each(i, arg);
    t = csx_obj(head(arg))->type;
    if (t == i->basenames.name) return csx_str(i, head(arg));
    if (t == i->null) return csx_str(i, "");
    if (t != i->basenames.pair) exit(1);
    res = malloc(1);
    arg = head(arg);
    while (csx_obj(arg)->type == i->basenames.pair) {
        res[reslen] = *(char *)head(arg);
        res = realloc(res, ++reslen + 1);
        arg = tail(arg);
    }
    res[reslen] = 0;
    return csx_str(i, res);
}

static void *base_len(csxi *i, void *arg)
{
    void *t;
    int len = 0;
    arg = run_each(i, arg);
    t = csx_obj(head(arg))->type;
    if (t == i->basenames.str) return csx_int(i, strlen(head(arg)));
    if (t != i->basenames.pair) exit(1);
    arg = head(arg);
    while (csx_obj(arg)->type == i->basenames.pair) {
        arg = tail(arg);
        ++len;
    }
    return csx_int(i, len);
}

static void *base_run(csxi *i, void *arg)
{
    int ret;
    void *res;
    void *rescontext;
    arg = run_each(i, arg);
    if (tail(arg) == i->null) return csx_run(i, head(arg));
    push(i, i->context);
    ret = i->stacklen;
    i->context = head(tail(arg));
    res = csx_run(i, head(arg));
    rescontext = i->context;
    restore(i, ret);
    i->context = pop(i);
    return csx_pair(i, res, rescontext);
}


static void *zip(csxi *i, void *params, void *values)
{
    void *res = i->null;
    while (csx_obj(params)->type == i->basenames.pair) {
        res = csx_pair(i, csx_pair(i, head(params), head(values)), res);
        params = tail(params);
        values = tail(values);
    }
    if (csx_obj(params)->type == i->basenames.name)
        res = csx_pair(i, csx_pair(i, params, values), res);
    return res;
}

void *csx_run(csxi *i, void *arg)
{
    int ret;
    push(i, arg);
    ret = i->stacklen;
tailcall:
    gc(i);
    if (csx_obj(arg)->type == i->basenames.name) {
        restore(i, ret);
        pop(i);
        return lookup(i, arg);
    } else if (csx_obj(arg)->type == i->basenames.pair) {
        fn_data *fn = csx_run(i, head(arg));
        void *ops = tail(arg);
        push(i, fn);
        if (csx_obj(fn)->type == i->basenames.base) {
            csxbase *base = (void *)fn;
            if (*base == csx_run) {
                ops = run_each(i, ops);
                arg = head(ops);
                push(i, arg);
                goto tailcall;
            } else {
                void *res = (*base)(i, ops);
                restore(i, ret);
                pop(i);
                return res;
            }
        } else if (csx_obj(fn)->type == i->basenames.fn) {
            void *res;
            int fnret;
            push(i, i->context);
            fnret = i->stacklen;
            ops = run_each(i, ops);
            i->context = csx_pair(i, zip(i, fn->params, ops), fn->context);
            res = base_do(i, fn->body);
            restore(i, fnret);
            i->context = pop(i);
            restore(i, ret);
            pop(i);
            return res;
        } else if (csx_obj(fn)->type == i->basenames.sx) {
            int sxret;
            push(i, i->context);
            sxret = i->stacklen;
            i->context = csx_pair(i, zip(i, fn->params, ops), fn->context);
            arg = base_do(i, fn->body);
            restore(i, sxret);
            i->context = pop(i);
            goto tailcall;
        } else if (csx_obj(fn)->type == i->basenames.pair) {
            csxpair *res = (void *)fn;
            int pos = *(int *)csx_run(i, head(ops));
            while (pos--) res = res->tail;
            restore(i, ret);
            pop(i);
            return res->head;
        } else if (csx_obj(fn)->type == i->basenames.str) {
            char *res = (void *)fn;
            int pos = *(int *)csx_run(i, head(ops));
            restore(i, ret);
            pop(i);
            return csx_int(i, res[pos]);
        } else if (csx_obj(fn)->type == i->null) {
            restore(i, ret);
            pop(i);
            return i->null;
        } else {
            exit(1);
        }
    }
    restore(i, ret);
    pop(i);
    return arg;
}


static void *base_context(csxi *i, void *args)  { return i->context; }

static void new_context(csxi *i);
static void *base_newcontext(csxi *i, void *args)
{
    void *res;
    push(i, i->context);
    new_context(i);
    res = i->context;
    i->context = pop(i);
    return res;
}

static void *base_exit(csxi *i, void *args)
{
    csx_free(i);
    exit(0);
}

static void new_context(csxi *i)
{
    csxbasenames *n = &i->basenames;
    i->context = csx_pair(i, i->null, i->null);
    base_set(i, csx_list(i, n->set,        csx_base(i, base_set),        0));
    base_set(i, csx_list(i, n->_isset,     csx_base(i, base_is_set),     0));
    base_set(i, csx_list(i, n->sethead,    csx_base(i, base_sethead),    0));
    base_set(i, csx_list(i, n->settail,    csx_base(i, base_settail),    0));
    base_set(i, csx_list(i, n->pair,       csx_base(i, base_pair),       0));
    base_set(i, csx_list(i, n->head,       csx_base(i, base_head),       0));
    base_set(i, csx_list(i, n->tail,       csx_base(i, base_tail),       0));
    base_set(i, csx_list(i, n->qt,         csx_base(i, base_qt),         0));
    base_set(i, csx_list(i, n->same,       csx_base(i, base_same),       0));
    base_set(i, csx_list(i, n->type,       csx_base(i, base_type),       0));
    base_set(i, csx_list(i, n->_do,        csx_base(i, base_do),         0));
    base_set(i, csx_list(i, n->fn,         csx_base(i, base_fn),         0));
    base_set(i, csx_list(i, n->sx,         csx_base(i, base_sx),         0));
    base_set(i, csx_list(i, n->_if,        csx_base(i, base_if),         0));
    base_set(i, csx_list(i, n->_plus,      csx_base(i, base_sum),        0));
    base_set(i, csx_list(i, n->_star,      csx_base(i, base_prod),       0));
    base_set(i, csx_list(i, n->neg,        csx_base(i, base_neg),        0));
    base_set(i, csx_list(i, n->div,        csx_base(i, base_div),        0));
    base_set(i, csx_list(i, n->mod,        csx_base(i, base_mod),        0));
    base_set(i, csx_list(i, n->_less,      csx_base(i, base_inc),        0));
    base_set(i, csx_list(i, n->_more,      csx_base(i, base_dec),        0));
    base_set(i, csx_list(i, n->out,        csx_base(i, base_out),        0));
    base_set(i, csx_list(i, n->in,         csx_base(i, base_in),         0));
    base_set(i, csx_list(i, n->name,       csx_base(i, base_name),       0));
    base_set(i, csx_list(i, n->str,        csx_base(i, base_str),        0));
    base_set(i, csx_list(i, n->len,        csx_base(i, base_len),        0));
    base_set(i, csx_list(i, n->run,        csx_base(i, base_run),        0));
    base_set(i, csx_list(i, n->context,    csx_base(i, base_context),    0));
    base_set(i, csx_list(i, n->newcontext, csx_base(i, base_newcontext), 0));
    base_set(i, csx_list(i, n->_exit,      csx_base(i, base_exit),       0));
}

static void init_the_name(csxi *csx)
{
    char *name = new(csx, 0, 5);
    csx_obj(name)->type = name;
    memcpy(name, "name", 5);
    csx->basenames.name = name;
}

static char *init_basename(csxi *csx, const char *name)
{
    const int size = strlen(name) + 1;
    char *res = new(csx, csx->basenames.name, size);
    memcpy(res, name, size);
    return res;
}

static void init_basenames(csxi *csx)
{
    csxbasenames *n = &csx->basenames;
    n->pair       = init_basename(csx, "pair");
    n->str        = init_basename(csx, "str");
    n->_int       = init_basename(csx, "int");
    n->base       = init_basename(csx, "base");
    n->fn         = init_basename(csx, "fn");
    n->sx         = init_basename(csx, "sx");
    n->set        = init_basename(csx, "set");
    n->_isset     = init_basename(csx, "set?");
    n->sethead    = init_basename(csx, "sethead");
    n->settail    = init_basename(csx, "settail");
    n->head       = init_basename(csx, "head");
    n->tail       = init_basename(csx, "tail");
    n->qt         = init_basename(csx, "qt");
    n->same       = init_basename(csx, "same");
    n->type       = init_basename(csx, "type");
    n->_do        = init_basename(csx, "do");
    n->_if        = init_basename(csx, "if");
    n->_plus      = init_basename(csx, "+");
    n->_star      = init_basename(csx, "*");
    n->neg        = init_basename(csx, "neg");
    n->div        = init_basename(csx, "div");
    n->mod        = init_basename(csx, "mod");
    n->_less      = init_basename(csx, "<");
    n->_more      = init_basename(csx, ">");
    n->out        = init_basename(csx, "out");
    n->in         = init_basename(csx, "in");
    n->len        = init_basename(csx, "len");
    n->run        = init_basename(csx, "run");
    n->context    = init_basename(csx, "context");
    n->newcontext = init_basename(csx, "newcontext");
    n->_exit      = init_basename(csx, "exit");
}


void csx_init(csxi *csx)
{
    csx->objs = 0;
    csx->objslen = 0;
    csx->objssize = 0;
    csx->stack = 0;
    csx->stacklen = 0;
    csx->stacksize = 0;
    csx->lastlen = 0;
    csx->context = 0;
    init_the_name(csx);
    init_basenames(csx);
    csx->null = new(csx, 0, 0);
    csx_obj(csx->null)->type = csx->null;
    csx->one = csx_int(csx, 1);
    new_context(csx);
}

void csx_free(csxi *csx)
{
    while (csx->objslen) {
        free(csx->objs[--(csx->objslen)]);
    }
    free(csx->objs);
    free(csx->stack);
}


csxpair *csx_list(csxi *csx, void *head, ...)
{
    va_list args;
    csxpair *res;
    csxpair **p;
    if (!head) return csx->null;
    res = csx_pair(csx, head, csx->null);
    p = (csxpair **)&res->tail;
    va_start(args, head);
    for (head = va_arg(args, void *); head; head = va_arg(args, void *)) {
        *p = csx_pair(csx, head, csx->null);
        p = (csxpair **)&(*p)->tail;
    }
    va_end(args);
    return res;
}

csxpair *csx_dot(csxi *csx, void *a, void *b, void *c, ...)
{
    va_list args;
    csxpair *res;
    csxpair **p;
    if (!c) return csx_pair(csx, a, b);
    res = csx_pair(csx, a, b);
    p = (csxpair **)&res->tail;
    *p = csx_pair(csx, *p, c);
    p = (csxpair **)&(*p)->tail;
    va_start(args, c);
    for (c = va_arg(args, void *); c; c = va_arg(args, void *)) {
        *p = csx_pair(csx, *p, c);
        p = (csxpair **)&(*p)->tail;
    }
    va_end(args);
    return res;
}

char *csx_str(csxi *csx, const char *str)
{
    const int strsize = strlen(str) + 1;
    char *res = new(csx, csx->basenames.str, strsize);
    memcpy(res, str, strsize);
    return res;
}
