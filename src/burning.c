#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/time.h>
#include "string_p.h"
#include "w_algorithm.h"

#define AGENT "burning/1.0"

struct resp_header
{
    int status_code;//HTTP/1.1 '200' OK
    char content_type[128];//Content-Type: application/gzip
    long content_length;//Content-Length: 11683079
    char file_name[256];
};
struct resp_header resp;

void parse_thunder(char const * argv,char * url);
void parse_url(char const *url,char *domain,int *port,char *file_name);
void get_ip_addr(char const *domain,char *ip_addr);
void send_request(char const *url,char const *domain,int *re_socket,int port);
void parse_response(int client_socket);
struct resp_header get_resp_header(char const * response);
void progressBar(long cur_size,long tot_size);
void download(void *socket_d);


void parse_thunder(char const * argv,char * url)
{
    int p=10;
    char src[2048],dest[1024];
    memset(src,0,sizeof(src));
    strncpy(src,argv+p,strlen(argv)-p); //一直复制到字符串尾，已经复制了'\0'
    
    base64_decode(src,dest);
    strncpy(url,dest+2,strlen(dest)-4);
    url[strlen(dest)-4]='\0';
    
    //printf("%s\n",url);
}
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
void send_request(char const *url,char const *domain,int *re_socket,int port)
{
    char header[2048];
    char ip_addr[16]={0};
    
    sprintf(header,"GET %s HTTP/1.1\r\n" \
            "Accept:text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n"\
            "User-Agent:%s\r\n" \
            "Host:%s\r\n" \
            "Connection:close\r\n" \
            "\r\n",url,AGENT,domain);
    
    get_ip_addr(domain, ip_addr);
    printf("IP:%s\n ",ip_addr);
    int client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    *re_socket=client_socket;
    
    if(client_socket < 0)
    {
        printf("invalid socket descriptor: %d\n", client_socket);
        exit(-1);
    }
    struct sockaddr_in addr;
    memset(&addr,0,sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip_addr);
    addr.sin_port = htons(port);
    
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
        i=strlen(response) - 1;
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
void progressBar(long cur_size, long total_size)
{
    float percent = (float) cur_size / total_size;
    const int numTotal = 50;
    int numShow = (int)(numTotal * percent);
    
    if (numShow == 0)
        numShow = 1;
    
    if (numShow > numTotal)
        numShow = numTotal;
    
    char sign[51] = {0};
    memset(sign, '*', numTotal);
    
    printf("\r%.2f%%\t[%-*.*s] %.2f/%.2fMB", percent * 100, numTotal, numShow, sign, cur_size / 1024.0 / 1024.0, total_size / 1024.0 / 1024.0);
    fflush(stdout);
    
    if (numShow == numTotal)
        printf("\n");
}
void download(void *socket_d)
{
    int client_socket = *(int *)socket_d;
    int length = 0;
    int MAX_SIZE = 2048;
    int buf_len = MAX_SIZE;
    int len;
    
    int fd = open(resp.file_name, O_CREAT | O_WRONLY, S_IRWXG | S_IRWXO | S_IRWXU);
    if(fd < 0)
    {
        printf("Create file failed.\n");
        exit(0);
    }
    char * buf = (char *)malloc(buf_len * sizeof(char));
    while((len=read(client_socket,buf,buf_len))!=0 && length < resp.content_length)
    {
        write(fd,buf,len);
        length+=len;
        progressBar(length,resp.content_length);
    }
    if(length == resp.content_length)
    {
        printf("Download Successfull!\n");
    }
}
int main(int argc,char const *argv[])
{
    char url[2048]="127.0.0.1";
    char domain[64]={0};
    int port=80,client_socket;
    char file_name[256]={0};
    
    if(argc==1)
    {
        printf("Please input URL.\n");
        return 0;
    }else if(strstr(argv[1],"thunder") != NULL) parse_thunder(argv[1],url);
    else strcpy(url,argv[1]);
    
    
    parse_url(url,domain,&port,file_name);
    send_request(url,domain,&client_socket,port);
    parse_response(client_socket);
    strcpy(resp.file_name,file_name);

    
    printf("URL: %s\n", url);
    printf("DOMAIN: %s\n", domain);
    printf("PORT: %d\n", port);
    printf("FILENAME: %s\n\n", file_name);
    
    pthread_t download_thread;
    pthread_create(&download_thread, NULL, (void *)download, (void *) &client_socket);
    pthread_join(download_thread, NULL);
    
    return 0;
}
