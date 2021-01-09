#include "csx.h"

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>


static void init();


typedef enum csx_type {
    type_null,
    type_pair,
    type_name,
    type_base,
    type_int,
    type_real,
    type_fn,
    type_sx,
    type_str
} csx_type;

static void setmark(void *p)   { ((int *)p)[-2] = 1; }
static int mark(void *p)       { return ((int *)p)[-2]; }
static csx_type type(void *p)  { return ((int *)p)[-1]; }

static void **objs = 0;
static int objslen = 0;
static int objssize;

static void pushobj(void *p)
{
    if (!objs) {
        objs = malloc(sizeof(void *));
        objslen = 0;
        objssize = 1;
    }
    if (objslen == objssize) {
        objs = realloc(objs, (objssize *= 2) * sizeof(void *));
    }
    objs[objslen++] = p;
}

static void **stack = 0;
static int stacklen = 0;
static int stacksize;

static void push(void *p)
{
    if (!stack) {
        stack = malloc(sizeof(void *));
        stacklen = 0;
        stacksize = 1;
    }
    if (stacklen == stacksize) {
        stack = realloc(stack, (stacksize *= 2) * sizeof(void *));
    }
    stack[stacklen++] = p;
}

static void *pop()
{
    return stack[--stacklen];
}

static void restore(int len)
{
    stacklen = len;
    if (stacklen <= stacksize / 4) {
        stacksize /= 2;
        stack = realloc(stack, stacksize * sizeof(void *));
    }
}

static void *new(csx_type type, int data_size)
{
    int *res = malloc(sizeof(int) * 2 + data_size);
    if (!res) exit(1);
    res[0] = 0;
    res[1] = type;
    pushobj(res);
    return res + 2;
}


typedef struct pair_data {
    void *head;
    void *tail;
} pair_data;

static void *head(pair_data *pair)  { return pair->head; }
static void *tail(pair_data *pair)  { return pair->tail; }

static pair_data *new_pair(void *head, void *tail)
{
    pair_data *res = new(type_pair, sizeof(pair_data));
    res->head = head;
    res->tail = tail;
    return res;
}


typedef struct fn_data {
    pair_data *params;
    pair_data *body;
    pair_data *context;
} fn_data;


static int initiated = 0;
static void *null;
static void *one;
static pair_data *context;


static void deepmark(void *p)
{
    if (mark(p)) return;
    setmark(p);
    if (type(p) == type_pair) {
        deepmark(head(p));
        deepmark(tail(p));
    } else if (type(p) == type_fn || type(p) == type_sx) {
        fn_data *fn = p;
        deepmark(fn->params);
        deepmark(fn->body);
        deepmark(fn->context);
    }
}

static void sweep()
{
    void **old = objs;
    int len = objslen;
    int i;
    objs = 0;
    for (i = 0; i != len; ++i) {
        if (*(int *)(old[i])) {
            *(int *)(old[i]) = 0;
            pushobj(old[i]);
        } else {
            free(old[i]);
        }
    }
    free(old);
}

static void gc()
{
    int i;
    static int lastlen = 0;
    if (objslen < lastlen * 2) return;
    setmark(null);
    setmark(one);
    deepmark(context);
    for (i = 0; i != stacklen; ++i) {
        deepmark(stack[i]);
    }
    sweep();
    lastlen = objslen;
}


char *csx_name(const char *name)
{
    if (!initiated) init();
    int namesize = strlen(name) + 1;
    int i;
    for (i = 0; i != objslen; ++i) {
        void *obj = (int *)(objs[i]) + 2;
        if (type(obj) != type_name) continue;
        if (!strncmp(obj, name, namesize)) return obj;
    }
    void *res = new(type_name, namesize);
    memcpy(res, name, namesize);
    return res;
}

csx_base_data *csx_base(csx_base_data base)
{
    if (!initiated) init();
    csx_base_data *res = new(type_base, sizeof(csx_base_data));
    *res = base;
    return res;
}

int *csx_int(int num)
{
    if (!initiated) init();
    int i;
    for (i = 0; i != objslen; ++i) {
        int *obj = (int *)(objs[i]) + 2;
        if (type(obj) != type_int) continue;
        if (*obj == num) return obj;
    }
    int *res = new(type_int, sizeof(int));
    *res = num;
    return res;
}

double *csx_float(double num)
{
    if (!initiated) init();
    int i;
    for (i = 0; i != objslen; ++i) {
        double *obj = (void *)((int *)(objs[i]) + 2);
        if (type(obj) != type_real) continue;
        if (*obj == num) return obj;
    }
    double *res = new(type_real, sizeof(double));
    *res = num;
    return res;
}


static void *run_each(void *l)
{
    if (type(l) != type_pair) return null;
    push(l);
    int ret = stacklen;
    pair_data *res = new_pair(csx_run(head(l)), null);
    pair_data **p = (pair_data **)&res->tail;
    l = tail(l);
    while (type(l) == type_pair) {
        push(res);
        int resret = stacklen;
        *p = new_pair(csx_run(head(l)), null);
        p = (pair_data **)&(*p)->tail;
        restore(resret);
        pop();
        l = tail(l);
    }
    restore(ret);
    pop();
    return res;
}


static void *lookup_frame(const char *name)
{
    pair_data *frame = context->head;
    while (type(frame) == type_pair) {
        if (!strcmp(head(frame->head), name)) return frame->head;
        frame = frame->tail;
    }
    return null;
}

static void *lookup(const char *name)
{
    void *saved = context;
    while (type(context) == type_pair) {
        pair_data *res = lookup_frame(name);
        if (type(res) != type_null) {
            context = saved;
            return res->tail;
        }
        context = context->tail;
    }
    context = saved;
    return null;
}

static void *base_set(void *arg)
{
    pair_data *res;
    push(arg);
    int ret = stacklen;
    void *name = head(arg);
    void *value = csx_run(head(tail(arg)));
    if (type(context) == type_null) {
        void *nameval = new_pair(name, value);
        context = new_pair(new_pair(nameval, null), null);
        restore(ret);
        pop();
        return null;
    }
    res = lookup_frame(name);
    if (type(res) != type_null) {
        res->tail = value;
    } else {
        void **names = &context->head;
        *names = new_pair(new_pair(name, value), *names);
    }
    restore(ret);
    pop();
    return null;
}

static void *base_is_set(void *arg)
{
    arg = run_each(arg);
    return lookup(head(arg)) ? one : null;
}

static void *base_sethead(void *arg)
{
    arg = run_each(arg);
    pair_data *p = head(arg);
    p->head = head(tail(arg));
    return null;
}

static void *base_settail(void *arg)
{
    arg = run_each(arg);
    pair_data *p = head(arg);
    p->tail = head(tail(arg));
    return null;
}

static void *base_pair(void *arg)
{
    arg = run_each(arg);
    return new_pair(head(arg), head(tail(arg)));
}

static void *base_head(void *arg)
{
    arg = run_each(arg);
    return head(head(arg));
}

static void *base_tail(void *arg)
{
    arg = run_each(arg);
    return tail(head(arg));
}

static void *base_quote(void *arg)
{
    return head(arg);
}

static void *base_same(void *arg)
{
    arg = run_each(arg);
    return head(arg) == head(tail(arg)) ? one : null;
}

static void *base_type(void *arg)
{
    arg = run_each(arg);
    switch (type(head(arg))) {
    case type_null: return null;
    case type_pair: return csx_name("pair");
    case type_name: return csx_name("name");
    case type_base: return csx_name("base");
    case type_int:  return csx_name("int");
    case type_real: return csx_name("real");
    case type_fn:   return csx_name("fn");
    case type_sx:   return csx_name("sx");
    case type_str:  return csx_name("str");
    }
    return 0;
}

static void *base_do(void *arg)
{
    void *res = null;
    arg = run_each(arg);
    while (type(arg) == type_pair) {
        res = head(arg);
        arg = tail(arg);
    }
    return res;
}

static void *base_fn(void *arg)
{
    fn_data *res = new(type_fn, sizeof(fn_data));
    res->params = head(arg);
    res->body = tail(arg);
    res->context = context;
    return res;
}

static void *base_sx(void *arg)
{
    fn_data *res = new(type_sx, sizeof(fn_data));
    res->params = head(arg);
    res->body = tail(arg);
    res->context = context;
    return res;
}

static void *base_if(void *arg)
{
    if (type(arg) != type_pair) return null;
    if (type(tail(arg)) != type_pair) return csx_run(head(arg));
    return type(csx_run(head(arg))) != type_null ?
        csx_run(head(tail(arg))) :
        base_if(tail(tail(arg)));
}


static void *base_sum(void *arg)
{
    arg = run_each(arg);
    int res = 0;
    while (type(arg) == type_pair) {
        int *num = head(arg);
        if (type(num) != type_int) exit(1);
        res += *num;
        arg = tail(arg);
    }
    return csx_int(res);
}

static void *base_prod(void *arg)
{
    arg = run_each(arg);
    int res = 1;
    while (type(arg) == type_pair) {
        int *num = head(arg);
        if (type(num) != type_int) exit(1);
        res *= *num;
        arg = tail(arg);
    }
    return csx_int(res);
}

static void *base_neg(void *arg)
{
    arg = run_each(arg);
    return csx_int(-*(int *)head(arg));
}

static void *base_inv(void *arg)
{  /* todo */
    arg = run_each(arg);
    return csx_int(0);
}

static void *base_div(void *arg)
{
    arg = run_each(arg);
    return csx_int(*(int *)head(arg) / *(int *)head(tail(arg)));
}

static void *base_mod(void *arg)
{
    arg = run_each(arg);
    return csx_int(*(int *)head(arg) % *(int *)head(tail(arg)));
}

static void *base_inc(void *arg)
{
    arg = run_each(arg);
    if (type(arg) != type_pair || type(head(arg)) != type_int) return null;
    int num = *(int *)head(arg);
    arg = tail(arg);
    while (type(arg) == type_pair) {
        int *another = head(arg);
        if (type(another) != type_int || *another <= num) return null;
        num = *another;
        arg = tail(arg);
    }
    return one;
}

static void *base_dec(void *arg)
{
    arg = run_each(arg);
    if (type(arg) != type_pair || type(head(arg)) != type_int) return null;
    int num = *(int *)head(arg);
    arg = tail(arg);
    while (type(arg) == type_pair) {
        int *another = head(arg);
        if (type(another) != type_int || *another >= num) return null;
        num = *another;
        arg = tail(arg);
    }
    return one;
}


static void *base_out(void *arg)
{
    arg = run_each(arg);
    int res = putchar(*(int *)head(arg));
    return res != EOF ? one : null;
}

static void *base_in(void *arg)
{
    arg = run_each(arg);
    int res = getchar();
    return res != EOF ? csx_int(res) : null;
}

static void *base_name(void *arg)
{
    arg = run_each(arg);
    if (type(head(arg)) != type_str) exit(1);
    return csx_name(head(arg));
}

static void *base_str(void *arg)
{
    arg = run_each(arg);
    if (type(head(arg)) == type_name) return csx_str(head(arg));
    if (type(head(arg)) == type_null) return csx_str("");
    if (type(head(arg)) != type_pair) exit(1);
    int reslen = 0;
    char *res = malloc(1);
    arg = head(arg);
    while (type(arg) == type_pair) {
        res[reslen] = *(char *)head(arg);
        res = realloc(res, ++reslen + 1);
        arg = tail(arg);
    }
    res[reslen] = 0;
    return csx_str(res);
}

static void *base_len(void *arg)
{
    arg = run_each(arg);
    if (type(head(arg)) == type_str) return csx_int(strlen(head(arg)));
    if (type(head(arg)) != type_pair) exit(1);
    int len = 0;
    arg = head(arg);
    while (type(arg) == type_pair) {
        arg = tail(arg);
        ++len;
    }
    return csx_int(len);
}

static void *base_run(void *arg)
{
    arg = run_each(arg);
    if (type(tail(arg)) == type_null) return csx_run(head(arg));
    push(context);
    int ret = stacklen;
    context = head(tail(arg));
    void *res = csx_run(head(arg));
    void *rescontext = context;
    restore(ret);
    context = pop();
    return new_pair(res, rescontext);
}


static void *zip(void *params, void *values)
{
    void *res = null;
    while (type(params) == type_pair) {
        res = new_pair(new_pair(head(params), head(values)), res);
        params = tail(params);
        values = tail(values);
    }
    if (type(params) == type_name)
        res = new_pair(new_pair(params, values), res);
    return res;
}

void *csx_run(void *arg)
{
    if (!initiated) init();
    push(arg);
    int ret = stacklen;
tailcall:
    gc();
    if (type(arg) == type_name) {
        restore(ret);
        pop();
        return lookup(arg);
    } else if (type(arg) == type_pair) {
        fn_data *fn = csx_run(head(arg));
        void *ops = tail(arg);
        push(fn);
        if (type(fn) == type_base) {
            csx_base_data *base = (void *)fn;
            if (*base == csx_run) {
                ops = run_each(ops);
                arg = head(ops);
                push(arg);
                goto tailcall;
            } else {
                void *res = (*base)(ops);
                restore(ret);
                pop();
                return res;
            }
        } else if (type(fn) == type_fn) {
            void *res;
            push(context);
            int fnret = stacklen;
            ops = run_each(ops);
            context = new_pair(zip(fn->params, ops), fn->context);
            res = base_do(fn->body);
            restore(fnret);
            context = pop();
            restore(ret);
            pop();
            return res;
        } else if (type(fn) == type_sx) {
            push(context);
            int sxret = stacklen;
            context = new_pair(zip(fn->params, ops), fn->context);
            arg = base_do(fn->body);
            restore(sxret);
            context = pop();
            goto tailcall;
        } else if (type(fn) == type_pair) {
            pair_data *res = (void *)fn;
            int pos = *(int *)csx_run(head(ops));
            while (pos--) res = res->tail;
            restore(ret);
            pop();
            return res->head;
        } else if (type(fn) == type_str) {
            char *res = (void *)fn;
            int pos = *(int *)csx_run(head(ops));
            restore(ret);
            pop();
            return csx_int(res[pos]);
        } else if (type(fn) == type_null) {
            restore(ret);
            pop();
            return null;
        } else {
            exit(1);
        }
    }
    restore(ret);
    pop();
    return arg;
}


static void *base_context(void *args)
{
    return context;
}

static void new_context();
static void *base_newcontext(void *args)
{
    push(context);
    new_context();
    void *res = context;
    context = pop();
    return res;
}

static void *base_exit(void *args)
{
    csx_free();
    exit(0);
}

static void new_context()
{
    context = new_pair(null, null);
    base_set(csx_list(csx_name("set"), csx_base(base_set), 0));
    base_set(csx_list(csx_name("set?"), csx_base(base_is_set), 0));
    base_set(csx_list(csx_name("sethead"), csx_base(base_sethead), 0));
    base_set(csx_list(csx_name("settail"), csx_base(base_settail), 0));
    base_set(csx_list(csx_name("pair"), csx_base(base_pair), 0));
    base_set(csx_list(csx_name("head"), csx_base(base_head), 0));
    base_set(csx_list(csx_name("tail"), csx_base(base_tail), 0));
    base_set(csx_list(csx_name("quote"), csx_base(base_quote), 0));
    base_set(csx_list(csx_name("same"), csx_base(base_same), 0));
    base_set(csx_list(csx_name("type"), csx_base(base_type), 0));
    base_set(csx_list(csx_name("do"), csx_base(base_do), 0));
    base_set(csx_list(csx_name("fn"), csx_base(base_fn), 0));
    base_set(csx_list(csx_name("sx"), csx_base(base_sx), 0));
    base_set(csx_list(csx_name("if"), csx_base(base_if), 0));
    base_set(csx_list(csx_name("+"), csx_base(base_sum), 0));
    base_set(csx_list(csx_name("*"), csx_base(base_prod), 0));
    base_set(csx_list(csx_name("neg"), csx_base(base_neg), 0));
    base_set(csx_list(csx_name("inv"), csx_base(base_inv), 0));
    base_set(csx_list(csx_name("div"), csx_base(base_div), 0));
    base_set(csx_list(csx_name("mod"), csx_base(base_mod), 0));
    base_set(csx_list(csx_name("<"), csx_base(base_inc), 0));
    base_set(csx_list(csx_name(">"), csx_base(base_dec), 0));
    base_set(csx_list(csx_name("out"), csx_base(base_out), 0));
    base_set(csx_list(csx_name("in"), csx_base(base_in), 0));
    base_set(csx_list(csx_name("name"), csx_base(base_name), 0));
    base_set(csx_list(csx_name("str"), csx_base(base_str), 0));
    base_set(csx_list(csx_name("len"), csx_base(base_len), 0));
    base_set(csx_list(csx_name("run"), csx_base(base_run), 0));
    base_set(csx_list(csx_name("context"), csx_base(base_context), 0));
    base_set(csx_list(csx_name("newcontext"), csx_base(base_newcontext), 0));
    base_set(csx_list(csx_name("exit"), csx_base(base_exit), 0));
}

static void init()
{
    initiated = 1;
    setbuf(stdin, 0);
    setbuf(stdout, 0);
    null = new(type_null, 0);
    one = csx_int(1);
    new_context();
}

void csx_free()
{
    initiated = 0;
    while (objslen) {
        free(objs[--objslen]);
    }
    free(objs);
    free(stack);
    objs = 0;
    stack = 0;
}


void *csx_list(void *head, ...)
{
    va_list args;
    pair_data *res;
    pair_data **p;
    if (!initiated) init();
    if (!head) return null;
    res = new_pair(head, null);
    p = (pair_data **)&res->tail;
    va_start(args, head);
    for (head = va_arg(args, void *); head; head = va_arg(args, void *)) {
        *p = new_pair(head, null);
        p = (pair_data **)&(*p)->tail;
    }
    va_end(args);
    return res;
}

void *csx_pair(void *a, void *b, void *c, ...)
{
    va_list args;
    pair_data *res;
    pair_data **p;
    if (!initiated) init();
    if (!c) return new_pair(a, b);
    res = new_pair(a, b);
    p = (pair_data **)&res->tail;
    *p = new_pair(*p, c);
    p = (pair_data **)&(*p)->tail;
    va_start(args, c);
    for (c = va_arg(args, void *); c; c = va_arg(args, void *)) {
        *p = new_pair(*p, c);
        p = (pair_data **)&(*p)->tail;
    }
    va_end(args);
    return res;
}

char *csx_str(const char *str)
{
    if (!initiated) init();
    int strsize = strlen(str) + 1;
    char *res = new(type_str, strsize);
    memcpy(res, str, strsize);
    return res;
}
