#include <stdio.h>
#include <ctype.h>


void readlist()
{
    int first = 1;
    int c = getchar();
    while (c != EOF && c != ']') {
        if (c == '[') {
            if (first) first = 0;
            else putchar(',');
            printf("csx_list(");
            readlist();
            printf("0)");
            c = getchar();
        } else if (isdigit(c) || c == '-') {
            if (first) first = 0;
            else putchar(',');
            printf("csx_num(");
            putchar(c);
            c = getchar();
            while (c != EOF && isdigit(c)) {
                putchar(c);
                c = getchar();
            }
            printf(")");
        } else if (isspace(c)) {
            while (isspace(c = getchar()));
        } else {
            if (first) first = 0;
            else putchar(',');
            printf("csx_name(\"");
            while (c != EOF && !isspace(c) && c != '[' && c != ']') {
                if (c == '"' || c == '\\') putchar('\\');
                putchar(c);
                c = getchar();
            }
            printf("\")");
        }
    }
    if (!first) putchar(',');
}

int main()
{
    puts("#include <csx.h>");
    puts("int main()");
    puts("{");
    printf("csx_eval(csx_list(csx_name(\"do\"),");
    readlist();
    puts("0));");
    puts("return 0;");
    puts("}");
    return 0;
}
