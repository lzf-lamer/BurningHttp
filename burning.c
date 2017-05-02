#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include"string_p.h"

void parse_url(char const *url,char *domain,int *port,char *file_name)
{
    char *head[]={"http://","https://"};
    int i,j,p,start=-1;
    for(i=0;i<2;i++)
    {
        if((strstr(url,head[i]) != NULL))
        {
            start=i;
            break;
        }
    }
    if(start==0) start=7;
    else start=8;
    //parse_domain
    j=0;
    for(i=start;url[i] != '/';i++) domain[j++]=url[i];
    domain[i]='\0';
//    printf("%s\n",domain);
    if((p=strstr_p(domain,":"))!= -1)
    {
        char pos[16];
        strncpy(pos,domain+p+1,strlen(domain)-p);
        *port=atoi(pos);
    }else *port=80;
    domain[p]='\0';
//    printf("%d\n",*port);
    j=0;
    for(i=start;url[i]!='\0';i++)
    {
        if(url[i]=='/') j=0;
        else{
            file_name[j++]=url[i];
        }
    }
    file_name[j]='\0';
    return;
//    printf("%s\n",file_name);
}
int main(int argc,char const *argv[])
{
    char url[2048]="127.0.0.1";
    char domain[64]={0};
    char ip_addr[16]={0};
    int port=80;
    char file_name[256]={0};
    
    if(argc==1)
    {
        printf("Please input URL.\n");
        return 0;
    }else strcpy(url,argv[1]);
    parse_url(url,domain,&port,file_name);
    return 0;
}
