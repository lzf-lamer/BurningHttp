#include "string_p.h"

//str1是被查找的字符串，str2是需要查找的字符串
int strstr_p(char * str1,const char * str2)
{
    char * p;
    int k=-1;
    p=strstr(str1,str2);
    if(p==NULL) return -1;
    k=strlen(str1)-strlen(p);
    return k;
}
