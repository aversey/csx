#include "csx.h"

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>


typedef enum csx_type {
    type_null,
    type_pair,
    type_name,
    type_base,
    type_num,
    type_fn
} csx_type;


/* typedef void null_data; */

typedef struct pair_data {
    void *head;
    void *tail;
} pair_data;

/* typedef char name_data; */

/* typedef csx_base_data base_data; */

/* typedef int num_data; */

typedef struct fn_data {
    pair_data *params;
    pair_data *body;
    pair_data *context;
} fn_data;


static csx_type type(void *p)
{
    return *((int *)p - 1);
}

static void *new(csx_type type, int data_size)
{
    int *res = malloc(sizeof(int) + data_size);
    if (!res) exit(1);
    *res = type;
    return res + 1;
}


static void *new_null()
{
    return new(type_null, 0);
}

static pair_data *new_pair(void *head, void *tail)
{
    pair_data *res = new(type_pair, sizeof(pair_data));
    res->head = head;
    res->tail = tail;
    return res;
}

static char *new_name(const char *name)
{
    int namesize = strlen(name) + 1;
    void *res = new(type_name, namesize);
    memcpy(res, name, namesize);
    return res;
}

static csx_base_data *new_base(csx_base_data base)
{
    csx_base_data *res = new(type_base, sizeof(csx_base_data));
    *res = base;
    return res;
}

static int *new_num(int num)
{
    int *res = new(type_num, sizeof(int));
    *res = num;
    return res;
}

static fn_data *new_fn(void *params, void *body, void *context)
{
    fn_data *res = new(type_fn, sizeof(fn_data));
    res->params = params;
    res->body = body;
    res->context = context;
    return res;
}


static void *head(pair_data *pair)  { return pair->head; }
static void *tail(pair_data *pair)  { return pair->tail; }


static int initiated = 0;
static void *null;
static void *one;
static pair_data *context;


static void *lookup_frame(const char *name)
{
    void *frame = head(context);
    while (type(frame) == type_pair) {
        if (!strcmp(head(head(frame)), name)) return head(frame);
        frame = tail(frame);
    }
    return null;
}

static void *lookup(const char *name)
{
    void *saved = context;
    while (type(context) == type_pair) {
        void *res = lookup_frame(name);
        if (type(res) != type_null) {
            context = saved;
            return res;
        }
        context = tail(context);
    }
    context = saved;
    return null;
}

static void *base_define(void *arg)
{
    pair_data *res;
    void *name = head(arg);
    void *value = csx_eval(head(tail(arg)));
    if (type(context) == type_null) {
        void *nameval = new_pair(name, value);
        context = new_pair(new_pair(nameval, null), null);
        return null;
    }
    res = lookup_frame(name);
    if (type(res) != type_null) {
        res->tail = value;
    } else {
        void **names = &context->head;
        *names = new_pair(new_pair(name, value), *names);
    }
    return null;
}


static void *eval_each(void *l);


static void *base_not(void *arg)
{
    arg = eval_each(arg);
    return type(head(arg)) == type_null ? one : null;
}

static void *base_is_pair(void *arg)
{
    arg = eval_each(arg);
    return type(head(arg)) == type_pair ? one : null;
}

static void *base_is_name(void *arg)
{
    arg = eval_each(arg);
    return type(head(arg)) == type_name ? one : null;
}

static void *base_is_num(void *arg)
{
    arg = eval_each(arg);
    return type(head(arg)) == type_num ? one : null;
}

static void *base_sum(void *arg)
{
    arg = eval_each(arg);
    int res = 0;
    while (type(arg) == type_pair) {
        void *num = head(arg);
        if (type(num) != type_num) exit(1);
        res += *(int *)num;
        arg = tail(arg);
    }
    return new_num(res);
}

static void *base_diff(void *arg)
{
    arg = eval_each(arg);
    return new_num(*(int *)head(arg) - *(int *)head(tail(arg)));
}

static void *base_prod(void *arg)
{
    arg = eval_each(arg);
    int res = 1;
    while (type(arg) == type_pair) {
        void *num = head(arg);
        if (type(num) != type_num) exit(1);
        res *= *(int *)num;
        arg = tail(arg);
    }
    return new_num(res);
}

static void *base_div(void *arg)
{
    arg = eval_each(arg);
    return new_num(*(int *)head(arg) / *(int *)head(tail(arg)));
}

static void *base_mod(void *arg)
{
    arg = eval_each(arg);
    return new_num(*(int *)head(arg) % *(int *)head(tail(arg)));
}

static void *base_id(void *arg)
{
    arg = eval_each(arg);
    return arg;
}

static void *base_and(void *arg)
{
    arg = eval_each(arg);
    while (type(arg) == type_pair) {
        if (type(head(arg)) == type_null) return null;
        arg = tail(arg);
    }
    return one;
}

static void *base_or(void *arg)
{
    arg = eval_each(arg);
    while (type(arg) == type_pair) {
        if (type(head(arg)) != type_null) return one;
        arg = tail(arg);
    }
    return null;
}


static void *base_eq(void *arg);

static void *eqpair(void *a, void *rest)
{
    while (type(rest) == type_pair) {
        void *res;
        void *another = head(rest);
        if (type(another) != type_pair) return null;
        res = base_eq(new_pair(head(a), new_pair(head(another), null)));
        if (type(res) == type_null) return null;
        res = base_eq(new_pair(tail(a), new_pair(tail(another), null)));
        if (type(res) == type_null) return null;
        rest = tail(rest);
    }
    return one;
}

static void *eqname(char *a, void *rest)
{
    while (type(rest) == type_pair) {
        char *another = head(rest);
        if (type(another) != type_name || strcmp(another, a)) return null;
        rest = tail(rest);
    }
    return one;
}

static void *eqnum(int *a, void *rest)
{
    int num = *a;
    while (type(rest) == type_pair) {
        int *another = head(rest);
        if (type(another) != type_num || *another != num) return null;
        rest = tail(rest);
    }
    return one;
}

static void *base_eq(void *arg)
{
    arg = eval_each(arg);
    void *a;
    void *rest;
    if (type(arg) != type_pair) return null;
    a = head(arg);
    rest = tail(arg);
    if (type(a) == type_pair) return eqpair(a, rest);
    if (type(a) == type_name) return eqname(a, rest);
    if (type(a) == type_num) return eqnum(a, rest);
    while (type(rest) == type_pair) {
        void *another = head(rest);
        if (another != a) return null;
        rest = tail(rest);
    }
    return one;
}


static void *base_inc(void *arg)
{
    arg = eval_each(arg);
    if (type(arg) != type_pair || type(head(arg)) != type_num) return null;
    int num = *(int *)head(arg);
    arg = tail(arg);
    while (type(arg) == type_pair) {
        int *another = head(arg);
        if (type(another) != type_num || *another <= num) return null;
        num = *another;
        arg = tail(arg);
    }
    return one;
}

static void *base_dec(void *arg)
{
    arg = eval_each(arg);
    if (type(arg) != type_pair || type(head(arg)) != type_num) return null;
    int num = *(int *)head(arg);
    arg = tail(arg);
    while (type(arg) == type_pair) {
        int *another = head(arg);
        if (type(another) != type_num || *another >= num) return null;
        num = *another;
        arg = tail(arg);
    }
    return one;
}

static void *base_pair(void *arg)
{
    arg = eval_each(arg);
    return new_pair(head(arg), head(tail(arg)));
}

static void *base_head(void *arg)
{
    arg = eval_each(arg);
    return head(head(arg));
}

static void *base_tail(void *arg)
{
    arg = eval_each(arg);
    return tail(head(arg));
}

static void *base_do(void *arg)
{
    void *res = null;
    arg = eval_each(arg);
    while (type(arg) == type_pair) {
        res = head(arg);
        arg = tail(arg);
    }
    return res;
}

static void *base_is_callable(void *arg)
{
    arg = eval_each(arg);
    return type(head(arg)) == type_fn ||
        type(head(arg)) == type_base ?
        one : null;
}

static void *base_context(void *arg)
{
    return context;
}

static void *base_quote(void *arg)
{
    return head(arg);
}

static void *base_input(void *arg)
{
    int res = getchar();
    return res != EOF ? new_num(res) : null;
}

static void *base_output(void *arg)
{
    arg = eval_each(arg);
    return putchar(*(int *)head(arg)) != EOF ? one : null;
}

static void *base_outname(void *arg)
{
    arg = eval_each(arg);
    return fputs(head(arg), stdout) != EOF ? one : null;
}

static void *base_outnum(void *arg)
{
    arg = eval_each(arg);
    int num = *(int *)head(arg);
    printf("%d", num);
    return one;
}

static void *base_fn(void *arg)
{
    return new_fn(head(arg), tail(arg), context);
}

static void *base_eval(void *arg);
static void *base_if(void *arg)
{
    void *saved = context;
    void *res = base_eval(head(arg));
    context = saved;
    return type(res) != type_null ?
        base_eval(head(tail(arg))) :
        base_eval(head(tail(tail(arg))));
}


static void *base_apply(void *arg)
{  /* shouldn't be called */
    return null;
}

static void *setparams(void *params, void *values)
{
    void *res = null;
    while (type(params) == type_pair) {
        res = new_pair(new_pair(head(params), head(values)), res);
        params = tail(params);
        values = tail(values);
    }
    return res;
}

static void *base_eval(void *arg) {
tailcall:
    if (type(arg) == type_name) return tail(lookup(arg));
    else if (type(arg) == type_pair) {
        fn_data *fn = base_eval(head(arg));
        void *ops = tail(arg);
applycall:
        if (type(fn) == type_base) {
            csx_base_data *base = (void *)fn;
            if (*base == base_eval) {
                ops = eval_each(ops);
                arg = head(ops);
                goto tailcall;
            } else if (*base == base_apply) {
                ops = eval_each(ops);
                fn = head(ops);
                ops = head(tail(ops));
                goto applycall;
            } else {
                return (*base)(ops);
            }
        } else if (type(fn) == type_fn) {
            void *saved;
            void *res;
            ops = eval_each(ops);
            saved = context;
            context = new_pair(setparams(fn->params, ops), fn->context);
            res = base_do(fn->body);
            context = saved;
            return res;
        } else {
            exit(1);
        }
    }
    return arg;
}

static void *eval_each(void *l)
{
    if (type(l) != type_pair) return null;
    pair_data *res = new_pair(base_eval(head(l)), null);
    pair_data **p = (pair_data **)&res->tail;
    l = tail(l);
    while (type(l) == type_pair) {
        void *saved = context;
        *p = new_pair(base_eval(head(l)), null);
        p = (pair_data **)&(*p)->tail;
        context = saved;
        l = tail(l);
    }
    return res;
}


static void new_context()
{
    context = new_pair(null, null);
    base_define(csx_list(new_name("not"), new_base(base_not), 0));
    base_define(csx_list(new_name("pair?"), new_base(base_is_pair), 0));
    base_define(csx_list(new_name("name?"), new_base(base_is_name), 0));
    base_define(csx_list(new_name("num?"), new_base(base_is_num), 0));
    base_define(csx_list(new_name("fn?"), new_base(base_is_callable), 0));
    base_define(csx_list(new_name("sum"), new_base(base_sum), 0));
    base_define(csx_list(new_name("diff"), new_base(base_diff), 0));
    base_define(csx_list(new_name("prod"), new_base(base_prod), 0));
    base_define(csx_list(new_name("div"), new_base(base_div), 0));
    base_define(csx_list(new_name("mod"), new_base(base_mod), 0));
    base_define(csx_list(new_name("eq"), new_base(base_eq), 0));
    base_define(csx_list(new_name("inc"), new_base(base_inc), 0));
    base_define(csx_list(new_name("dec"), new_base(base_dec), 0));
    base_define(csx_list(new_name("pair"), new_base(base_pair), 0));
    base_define(csx_list(new_name("head"), new_base(base_head), 0));
    base_define(csx_list(new_name("tail"), new_base(base_tail), 0));
    base_define(csx_list(new_name("id"), new_base(base_id), 0));
    base_define(csx_list(new_name("do"), new_base(base_do), 0));
    base_define(csx_list(new_name("apply"), new_base(base_apply), 0));
    base_define(csx_list(new_name("context"), new_base(base_context), 0));
    base_define(csx_list(new_name("eval"), new_base(base_eval), 0));
    base_define(csx_list(new_name("quote"), new_base(base_quote), 0));
    base_define(csx_list(new_name("define"), new_base(base_define), 0));
    base_define(csx_list(new_name("fn"), new_base(base_fn), 0));
    base_define(csx_list(new_name("if"), new_base(base_if), 0));
    base_define(csx_list(new_name("and"), new_base(base_and), 0));
    base_define(csx_list(new_name("or"), new_base(base_or), 0));
    base_define(csx_list(new_name("input"), new_base(base_input), 0));
    base_define(csx_list(new_name("output"), new_base(base_output), 0));
    base_define(csx_list(new_name("outname"), new_base(base_outname), 0));
    base_define(csx_list(new_name("outnum"), new_base(base_outnum), 0));
}


static void init()
{
    initiated = 1;
    null = new_null();
    one = new_num(1);
    new_context();
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

int *csx_num(int num)
{
    return new_num(num);
}

void *csx_eval(void *expression)
{
    return base_eval(expression);
}

void *csx_base(csx_base_data base)
{
    return new_base(base);
}

char *csx_name(const char *name)
{
    return new_name(name);
}
