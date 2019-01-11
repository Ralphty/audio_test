#include <QtCore/QCoreApplication>

#include <soundJNI_c.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
void udpc_initSocket(int*sockfd,struct sockaddr_in* addr,int port)
{
    (*sockfd) = socket(AF_INET,SOCK_DGRAM,0);
    fcntl(*sockfd,F_SETFL,O_NONBLOCK);
    if((*sockfd)<0)
    {
        fprintf(stderr,"SOCKET Error:%s\n",strerror(errno));
        exit(1);
    }
    bzero(addr,sizeof(struct sockaddr_in));
    addr->sin_family=AF_INET;
    addr->sin_addr.s_addr=htonl(INADDR_ANY);
    addr->sin_port=htons(port);
    if(bind(*sockfd,(struct sockaddr*)addr,sizeof(struct sockaddr_in))<0)
    {
        fprintf(stderr,"Bind Error:%s\n",strerror(errno));
        exit(1);
    }
}
int udpc_recvfrom(int sockfd,char * buf,int len)
{
    struct sockaddr_in addr;
    int addrlen = sizeof(struct sockaddr_in);
    int recLen = 0;

    struct timeval tv;
    fd_set readfds;
    tv.tv_sec =0;
    tv.tv_usec=5000;

    FD_ZERO(&readfds);
    FD_SET(sockfd,&readfds);
    select(sockfd+1,&readfds,NULL,NULL,&tv);
    if(FD_ISSET(sockfd,&readfds))
    {
        socklen_t length = sizeof(addrlen);
        recLen = recvfrom(sockfd,buf,len,0,(struct sockaddr * )&addr,&length);
        printf("recvfrom len %d \n",recLen);
    }
    return recLen;
}
void udpc_close(int sockfd)
{
    close(sockfd);
}

int runFlag = 1;
void sig_stop(int sig)
{
        printf("quit program \n");
        runFlag = 0;
}


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    int k;
        FILE * fp;
        int hfp;
        int iresult;
        char buf[2048];
        int value = 0;
        int iResult =1;
        int sockfd;
        struct sockaddr_in addr;


        signal(SIGTERM,sig_stop);
            signal(SIGHUP,sig_stop);
            signal(SIGQUIT,sig_stop);
            signal(SIGKILL,sig_stop);
            signal(SIGINT,sig_stop);


        if(argc < 2)
        {
            fprintf(stderr,"Usge:%s ipaddr portNumber \a\n",argv[0]);
            exit(1);
        }
        udpc_initSocket(&sockfd,&addr,atoi(argv[1]));

        iresult = init_manager(5,10);

        if(iresult == 0 )
        {
            printf("init error \n");
            return 0;
        }
        k = new_link(1,8000,1,16,0,640);
        printf("begin play id is %d\n",k);
        do
        {
            iResult = udpc_recvfrom(sockfd,buf,1024);
            if(iResult != 0)
                write_link_by_cache(k,(unsigned char*)buf,iResult);
        }while((iResult>=0) &&(runFlag ==1) );
        printf("while end \n");
        destory_link(k);
        printf("destory link ok\n");
        destory_manager();
        printf("destory manager ok\n");
        udpc_close(sockfd);
    
    return a.exec();
}
