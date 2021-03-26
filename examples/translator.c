#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>


int c;
int indent = 0;


int items();

void prindent()
{
    int i;
    printf("\n    ");
    for (i = 0; i != indent; ++i) {
        printf("  ");
    }
}

void name(char a)
{
    prindent();
    printf("N(\"");
    if (a) putchar(a);
    while (c != EOF && !isspace(c) && c != '[' && c != ']') {
        if (c == '"' || c == '\\') putchar('\\');
        putchar(c);
        c = getchar();
    }
    printf("\")");
}

void num(char a, char b)
{
    prindent();
    printf("I(");
    if (a) putchar(a);
    if (b) putchar(b);
    c = getchar();
    while (c != EOF && isdigit(c)) {
        putchar(c);
        c = getchar();
    }
    printf(")");
}

void list()
{
    prindent();
    printf("L(");
    if (items()) prindent();
    printf("0)");
    c = getchar();
}

void dot()
{
    if ((c = getchar()) == '[') {
        prindent();
        printf("D(");
        items();
        prindent();
        printf("0)");
        c = getchar();
    } else name('=');
}

void quote()
{
    prindent();
    printf("L(qt,\n");
    c = getchar();
    if (c == '[') list();
    else if (c == '=') dot();
    else if (c == '\'') quote();
    else name(0);
    printf(",\n");
    prindent();
    printf("0)");
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

int items()
{
    int first = 1;
    ++indent;
    c = getchar();
    skip();
    while (c != EOF && c != ']') {
        if (first) first = 0; else putchar(',');
        if (c == '[') list();
        else if (c == '-')
            if (isdigit(c = getchar())) num('-', c);
            else name('-');
        else if (isdigit(c)) num(c, 0);
        else if (c == '=') dot();
        else if (c == '"') {
            prindent();
            printf("S(\"");
            c = getchar();
            while (c != EOF && c != '"') {
                if (c == '\\') c = getchar();
                if (c == '"' || c == '\\') putchar('\\');
                putchar(c);
                c = getchar();
            }
            printf("\")");
            c = getchar();
        } else if (c == '\'') quote();
        else name(0);
        skip();
    }
    if (!first) putchar(',');
    --indent;
    return !first;
}

int main()
{
    printf("#include <csxbind.h>\n\nint main()\n{\n");
    printf("    init();\n    R(L(_do,");
    items();
    printf("\n    0));\n    return 0;\n}\n");
    return 0;
}
