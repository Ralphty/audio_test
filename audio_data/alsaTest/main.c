#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <alsaCapture.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#define NUM 320
#define SERVER_PORT 8000

static int count = 0;
void udpc_initSocket(int * sockfd,struct sockaddr_in * addr,char * strIpaddr,int port)
{
	(*sockfd) = socket(AF_INET,SOCK_DGRAM,0);
	
	if((*sockfd)<0)
	{
		fprintf(stderr,"Socket Error:%s\n",strerror(errno));
		exit(1);
	}
	bzero(addr,sizeof(struct sockaddr_in));
	addr->sin_family=AF_INET;
	addr->sin_port = htons(port);
	if(inet_aton(strIpaddr,&(addr->sin_addr))<0)
	{
		fprintf(stderr,"Ip error:%s\n",strerror(errno));
		exit(1);
	}
}

void udpc_send(int sockfd,const struct sockaddr_in * addr,short * buf,int len)
{
//	printf("Send To Len %d - %d\n",len,count++);
	sendto(sockfd,buf,len,0,(struct sockaddr *)addr,sizeof(struct sockaddr_in));
}
void udpc_close(int sockfd)
{
	close(sockfd);
}
void signal_alrm(int sig)
{
	printf("\n");
	alarm(1);
	count = 0;
}
void main (int argc, char *argv[])
{
	short buf[NUM+1];
	int iResult = 0;
	int iReadNum = NUM;
	int sockfd;
	struct sockaddr_in addr;
	
	
	//signal(SIGALRM,signal_alrm);	
	//alarm(1);	
	if(argc < 3 )
	{
		fprintf(stderr,"Usage:%s ipaddr portnumber \a\n",argv[0]);
		exit(1);
	}
	
	udpc_initSocket(&sockfd,&addr,argv[1],atoi(argv[2]));
	
	iResult = As_Init(16,8000,1);
	if(iResult != 0)
	{
		fprintf(stderr,"init alsaCapture error \n");
		return;
	}
	while(iResult = As_Read(buf,iReadNum/2))
	{
		if(iResult >0)
			udpc_send(sockfd,&addr,buf,iResult);
	}	
	udpc_close(sockfd);
	As_Close();
		
}
