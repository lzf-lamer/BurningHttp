#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include"string_p.h"

#define AGENT "burning/1.0"

struct resp_header
{
    int status_code;//HTTP/1.1 '200' OK
    char content_type[128];//Content-Type: application/gzip
    long content_length;//Content-Length: 11683079
    char file_name[256];
};
struct resp_header resp;

void parse_url(char const *url,char *domain,int *port,char *file_name);
void get_ip_addr(char const *domain,char *ip_addr);
void send_request(char const *url,char const *domain,int *socket,int *port);
void parse_response(int client_socket);
struct resp_header get_resp_header(char const * response);
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
}
void get_ip_addr(char const *domain,char *ip_addr)
{
    struct hostent *host = gethostbyname(domain);
    if(!host)
    {
        ip_addr=NULL;
        return ;
    }
    for(int i=0;host->h_addr_list[i];i++)
    {
        strcpy(ip_addr,inet_ntoa( * (struct in_addr *) host->h_addr_list[i]));
        break;
    }
}
void send_request(char const *url,char const *domain,int *socket,int * port)
{
    char header[2048];
    char ip_addr[16]={0};
    
    sprintf(header,"GET %s HTTP/1.1\r\n" /
            "Accept:*/*\r\n" /
            "User-Agent:%s\r\n" /
            "Host:%s\r\n" /
            "Connection:close\r\n" /
            "\r\n",url,AGENT,domain);
    
    get_ip_addr(domain, ip_addr);
    
    int client_socket = socket(AF_INET,SOCKET_STREAM,IPPROTO_TCP);
    *socket=client_socket;
    
    if(client_socket < 0)
    {
        printf("invalid socket descriptor: %d\n", client_socket);
        exit(-1);
    }
    struct sockaddr_in addr;
    memset(add,0,sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip_addr);
    addr.sin_port = htons(*port);
    
    int res = connect(client_socket,(struct sockaddr *)&addr,sizeof(addr));
    if(res == -1)
    {
        printf("connect failed,return %d.\n",res);
        exit(-1);
    }
    write(client_socket,header,strlen(header));
}
void parse_response(int client_socket)
{
    int MAX_SIZE = 4096;
    int length = 0;
    int len;
    char * buf = (char *)malloc(MAX_SIZE * sizeof(char));
    char * response = (char *)malloc(MAX_SIZE * sizeof(char));
    
    while((len=read(client_socket,buf,1)) != 0)
    {
        if(length + len > MAX_SIZE)
        {
            MAX_SIZE *=2;
            char * temp = (char *)realloc(response,MAX_SIZE * sizeof(char));
            if(temp == NULL)
            {
                printf("realloc failed.\n");
                exit(-1);
            }
            response = temp;
        }
        buf[len]='\0';
        strcat(response,buf);
        
        int sum=0,i;
        i=strlen(response);
        while(response[i] == '\r' || response[i] == '\n')
        {
            i--;
            sum++;
        }
        if(sum==4) break;
        length += len;
    }
    resp = get_resp_header(response);
    return ;
}
struct resp_header get_resp_header(char const * response)
{
    struct resp_header resp;
    
    char *pos = strstr(response, "HTTP/");
    if (pos)
        sscanf(pos, "%*s %d", &resp.status_code);
    pos = strstr(response, "Content-Type:");
    if (pos)
        sscanf(pos, "%*s %s", resp.content_type);
    pos = strstr(response, "Content-Length:");
    if (pos)
        sscanf(pos, "%*s %ld", &resp.content_length);
    
    return resp;
}
int main(int argc,char const *argv[])
{
    char url[2048]="127.0.0.1";
    char domain[64]={0}
    int port=80,client_socket;
    char file_name[256]={0};
    
    if(argc==1)
    {
        printf("Please input URL.\n");
        return 0;
    }else strcpy(url,argv[1]);
    parse_url(url,domain,&client_socket,&port,file_name);
    send_request();
    parse_response(client_socket);
    strcpy(resp.file_name,file_name);
    
    return 0;
}
