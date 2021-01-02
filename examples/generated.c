#include <csx.h>
int main()
{
csx_eval(csx_list(csx_name("do"),csx_list(csx_name("define"),csx_name("map"),csx_list(csx_name("fn"),csx_list(csx_name("f"),csx_name("l"),0),csx_list(csx_name("if"),csx_list(csx_name("not"),csx_name("l"),0),csx_list(0),csx_list(csx_name("pair"),csx_list(csx_name("f"),csx_list(csx_name("head"),csx_name("l"),0),0),csx_list(csx_name("map"),csx_name("f"),csx_list(csx_name("tail"),csx_name("l"),0),0),0),0),0),0),csx_list(csx_name("map"),csx_list(csx_name("fn"),csx_list(csx_name("n"),0),csx_list(csx_name("outnum"),csx_name("n"),0),csx_list(csx_name("output"),csx_num(10),0),0),csx_list(csx_name("map"),csx_list(csx_name("fn"),csx_list(csx_name("n"),0),csx_list(csx_name("sum"),csx_name("n"),csx_num(10),0),0),csx_list(csx_name("quote"),csx_list(csx_num(1),csx_num(2),csx_num(3),csx_num(4),csx_num(5),0),0),0),0),0));
return 0;
}
