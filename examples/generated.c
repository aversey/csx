#include <csx.h>
int main()
{
csx_run(csx_list(csx_name("do"),csx_list(csx_name("set"),csx_name("map"),csx_list(csx_name("fn"),csx_list(csx_name("f"),csx_name("l"),0),csx_list(csx_name("if"),csx_list(csx_name("same"),csx_name("l"),csx_list(0),0),csx_list(0),csx_list(csx_name("pair"),csx_list(csx_name("f"),csx_list(csx_name("head"),csx_name("l"),0),0),csx_list(csx_name("map"),csx_name("f"),csx_list(csx_name("tail"),csx_name("l"),0),0),0),0),0),0),csx_list(csx_name("map"),csx_list(csx_name("fn"),csx_list(csx_name("n"),0),csx_list(csx_name("sum"),csx_name("n"),csx_int(10),0),0),csx_list(csx_name("quote"),csx_list(csx_int(1),csx_int(2),csx_int(3),csx_int(4),csx_int(5),0),0),0),0));
return 0;
}
