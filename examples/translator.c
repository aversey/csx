#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>


int c;


void items();

void name(char a)
{
    printf("csx_name(\"");
    if (a) putchar(a);
    while (c != EOF && !isspace(c) && c != '[' && c != ']') {
        if (c == '"' || c == '\\') putchar('\\');
        putchar(c);
        c = getchar();
    }
    printf("\")");
}

void num(char a)
{
    printf("csx_int(");
    if (a) putchar(a);
    c = getchar();
    while (c != EOF && isdigit(c)) {
        putchar(c);
        c = getchar();
    }
    printf(")");
}

void pair()
{
    if ((c = getchar()) == '[') {
        printf("csx_pair(");
        items();
        printf("0)");
        c = getchar();
    } else name('=');
}

void list()
{
    printf("csx_list(");
    items();
    printf("0)");
    c = getchar();
}

void skip()
{
    while (isspace(c) || c == '{') {
        if (isspace(c)) while (isspace(c = getchar()));
        if (c == '{') {
            int level = 1;
            while (level && c != EOF) {
                while ((c = getchar()) != '{' && c != '}' && c != EOF);
                if (c == '{') ++level;
                if (c == '}') --level;
            }
            if (c != EOF) c = getchar();
        }
    }
}

void items()
{
    int first = 1;
    c = getchar();
    skip();
    while (c != EOF && c != ']') {
        if (first) first = 0; else putchar(',');
        if (c == '[') list();
        else if (c == '-')
            if (isdigit(c = getchar())) num('-');
            else name('-');
        else if (isdigit(c)) num(c);
        else if (c == '=') pair();
        else if (c == '"') {
            printf("csx_str(\"");
            c = getchar();
            while (c != EOF && c != '"') {
                if (c == '\\') c = getchar();
                if (c == '"' || c == '\\') putchar('\\');
                putchar(c);
                c = getchar();
            }
            printf("\")");
            c = getchar();
        } else if (c == '\'') {
            printf("csx_list(csx_name(\"quote\"),");
            c = getchar();
            if (c == '[') list();
            else if (c == '=') pair();
            else name(0);
            printf(",0)");
        } else name(0);
        skip();
    }
    if (!first) putchar(',');
}

int main()
{
    printf("#include <csx.h>\nint main(){csx_run(csx_list(csx_name(\"do\"),");
    items();
    printf("0));return 0;}\n");
    return 0;
}
