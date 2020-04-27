#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pthread.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sstream>
#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>
using namespace std;
#define BUFFER_SIZE 512

int Socket(int ,int,int);
void Bind(int ,const struct sockaddr*sa,socklen_t salen);
void Listen(int ,int);
int Accept(int,struct sockaddr*,socklen_t*);
void handleAccept(int);
void handleHttp(int);
int getRequest(int);
int connecting_socket;

 string tostring( const int& n )
    {
        std::ostringstream stm ;
        stm << n ;
        return stm.str() ;
    }
    
int sendString(char *message, int socket)
{
	int length, bytes_sent;
	length = strlen(message);

	bytes_sent = send(socket, message, length, 0);

	return bytes_sent;
}

 sighandler_t Signal(int signum, sighandler_t handler)
 {
   if (signal(signum, handler)==SIG_ERR)
   {
     err_sys("signal error");
     exit(-1);
   }
   
 }
 
void sig_chld(int signum)
{
    pid_t pid;
     int stat;

    while(pid = waitpid(-1, &stat, WNOHANG) >0 )
        printf("chlid %d terminated!\n",pid);
     return ;
 }
int main(int argc,char **argv)
{
    const int port = 8888;//listen port
    int listenfd=Socket(AF_INET,SOCK_STREAM,0);
    pid_t childpid;
    struct sockaddr_in serverAddr;
    serverAddr.sin_family=AF_INET;
    serverAddr.sin_addr.s_addr=INADDR_ANY;
    serverAddr.sin_port=htons(port);
    Bind(listenfd,(struct sockaddr*)&serverAddr,sizeof(serverAddr));
    Listen(listenfd,5);
     Signal(SIGCHLD, sig_chld); 
    while(true)
    {   
           sockaddr_in clientAddr;
           socklen_t clientLen=sizeof(clientAddr);
    //while(1){ 
           int connfd=Accept(listenfd,(sockaddr *)&clientAddr,&clientLen);//客户机传来的连接 
    
       if((childpid=Fork())==0){
        handleAccept(connfd);//
        close(listenfd)
        exit(0);
    }
    else{
        close(connfd);
    }
    }

}

int Socket(int family , int type,int protocol)
{
    int n;
    if ( (n = socket(family, type, protocol)) < 0)
    {
        printf("socket error\r\n");//创建socket失败退出 
        return -1;
    }
    return(n);

}
void Bind(int fd, const struct sockaddr *sa, socklen_t salen)
{
    if (bind(fd, sa, salen) < 0)//连接中断退出 
    {
        printf("bind error\r\n");
        exit(-1);
    }
}
void Listen(int fd, int backlog)
{
    char    *ptr;

    
    if ( (ptr = getenv("LISTENQ")) != NULL)
        backlog = atoi(ptr);

    if (listen(fd, backlog) < 0)
    {
        printf("listen error\r\n");
        return ;
    }
}
int Accept(int fd, struct sockaddr *sa, socklen_t *salenptr)
{
    int n;

again:
    if ( (n = accept(fd, sa, salenptr)) < 0) {//接受不会一直处于阻塞状态，检测到中断退出 
#ifdef  EPROTO
        if (errno == EPROTO || errno == ECONNABORTED)//当客户机调用accept之前中断时 
#else
        if (errno == ECONNABORTED)
#endif
            goto again;
        else
        {
            printf("accept error\r\n");
            return -1;
        }
    }
    return(n);
}

void handleAccept(int listenfd)
{
//    sockaddr_in clientAddr;
//    socklen_t clientLen=sizeof(clientAddr);
//    //while(1){ 
//    int connfd=Accept(listenfd,(sockaddr *)&clientAddr,&clientLen);//客户机传来的连接 
//    handleHttp(connfd);
      handleHttp(connfd);
      //close(listenfd);
    //} 
    //close(connfd);
}

int getRequestType(char *input)//用来查看请求报文的头部是什么方法如果是除get post之外的 
{
	// IF NOT VALID REQUEST 
	// RETURN -1
	// IF VALID REQUEST
	// RETURN 1 IF GET
	// RETURN 2 IF HEAD
	// RETURN 0 IF NOT YET IMPLEMENTED

	int type = -1;

	if ( strlen ( input ) > 0 )
	{
		type = 1;
	}

	char *requestType = (char*)malloc(6);
    
//    stringstream ss;
//    ss<<buffer;
//    string method;
//    ss>>method;
//    string uri;
//    ss>>uri;
    return 1;
    
//	scan(input, requestType, 0, 6);

	if ( type == 1 && strcmp("GET", requestType) == 0)
	{
		type = 1;
	}
	else if (type == 1 && (strcmp("HEAD", requestType) == 0||strcmp("DELETE", requestType) == 0||strcmp("PUT", requestType) == 0))
	{
		type = 2;
	}
	else if (strlen(input) > 4 && strcmp("POST", requestType) == 0 )
	{
		type = 0;
	}
	else
	{
		type = -1;
	}
	return type;
}


int receive(int socket)
{
	int msgLen = 0;
	char buffer[BUFFER_SIZE];

	memset (buffer,'\0', BUFFER_SIZE);

	if ((msgLen = recv(socket, buffer, BUFFER_SIZE, 0)) == -1)//接收用户请求结果 
	{
		printf("Error handling incoming request");
		return -1;
	}
	
    stringstream ss;
    ss<<buffer;
    string method;//处理请求方法 
    ss>>method;
    string url;//处理url 
    ss>>url;
    
	int request = getRequestType(buffer);//通过 检查报文头方法结果输出相应response 

	if ( request == 1 )				// GET
	{
		getRequest(socket);
	}
	else if ( request == 2 )		// HEAD和delete  
	{
		// SendHeader();
	}
	else if ( request == 0 )		// POST
	{
		sendString("501 Not Implemented\n", connecting_socket);
	}
	else							// GARBAGE
	{
		sendString("400 Bad Request\n", connecting_socket);
	}

	return 1;
}

void handleHttp(int connfd)
{
    if(receive(connfd)<0)
    {
        perror("http request get error");
        exit(-1);
    }
}

int getRequest(int socket)
{
    
    string statusCode("200 OK");
    string content("<html><head><title>my simple httpserver</title></head><h1>hello world</h1></body></html>");
    string contentSize(tostring(content.size()));
    string head("\r\nHTTP/1.1 ");
    string ContentType("\r\nContent-Type: ");
    string contentType("text/html");
    string ServerHead("\r\nServer: localhost");
    string ContentLength("\r\nContent-Length: ");
    string Date("\r\nDate: ");
    string Newline("\r\n");
    time_t rawtime;
    time(&rawtime);
    string message;
    message+=head;
    message+=statusCode;
    message+=ContentType;//文件类型 
    message+=contentType;
    message+=ServerHead;
    message+=ContentLength;
    message+=contentSize;//文件 
    message+=Date;
    message+=(string)ctime(&rawtime);//时间戳 
    message+=Newline;

    int messageLength=message.size();
    int n;
    n=send(socket,message.c_str(),messageLength,0);
    n=send(socket,content.c_str(),content.size(),0);   
    return n;
}
