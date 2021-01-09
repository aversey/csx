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
    printf("csx_name(\"");
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
    printf("csx_int(");
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
    printf("csx_list(");
    if (items()) prindent();
    printf("0)");
    c = getchar();
}

void pair()
{
    if ((c = getchar()) == '[') {
        prindent();
        printf("csx_pair(");
        items();
        prindent();
        printf("0)");
        c = getchar();
    } else name('=');
}

void quote()
{
    prindent();
    printf("csx_list(csx_name(\"quote\"),\n");
    c = getchar();
    if (c == '[') list();
    else if (c == '=') pair();
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
        else if (c == '=') pair();
        else if (c == '"') {
            prindent();
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
    printf("#include <csx.h>\n\nint main()\n{\n");
    printf("    csx_run(csx_list(csx_name(\"do\"),");
    items();
    printf("\n    0));\n    csx_free();\n    return 0;\n}\n");
    return 0;
}
