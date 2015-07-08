#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/types.h>

#define MAXBUF 1024

/*********************************************************************
* filename: select-server.c
* 演示网络异步通讯、select用法，这是服务器端程序
* ./server 7838 1
*********************************************************************/

int main(int argc, char **argv)
{
    int sockfd, new_fd;
    socklen_t len;
    struct sockaddr_in my_addr, their_addr;
    
    unsigned int myport, lisnum;
    char buf[MAXBUF + 1];
    
    fd_set rfds;
    struct timeval tv;
    
    int retval, maxfd = -1;

    if (argv[1])
        myport = atoi(argv[1]);
    else
        myport = 7838;

    if (argv[2])
        lisnum = atoi(argv[2]);
    else
        lisnum = 2;

    // create an new socket
    if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) 
    {
        perror("socket");
        exit(1);
    }

    bzero(&my_addr, sizeof(my_addr));
    my_addr.sin_family = PF_INET;
    my_addr.sin_port = htons(myport);
    
    if (argv[3])
        my_addr.sin_addr.s_addr = inet_addr(argv[3]);
    else
        my_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr)) == -1) 
    {
        perror("bind");
        exit(1);
    }

    if (listen(sockfd, lisnum) == -1) 
    {
        perror("listen");
        exit(1);
    }

    while (1) 
    {
        printf("\n----等待新的连接到来开始新一轮聊天……\n");
	
        len = sizeof(struct sockaddr);
	
        if ((new_fd = accept(sockfd, (struct sockaddr *) &their_addr, &len)) == -1) 
	{
            perror("accept");
            exit(errno);
        } 
        else
	{
            printf("server: got connection from %s, port %d, socket %d\n", inet_ntoa(their_addr.sin_addr), ntohs(their_addr.sin_port), new_fd);
	}
	
        // 开始处理每个新连接上的数据收发
        printf("\n准备就绪，可以开始聊天了……直接输入消息回车即可发信息给对方\n");
	
        while (1) 
	{
            // 把集合清空 
            FD_ZERO(&rfds);
	    
            // 把标准输入(stdin)句柄0加入到集合中
            FD_SET(0, &rfds);
    
            // 把当前连接(socket)句柄new_fd加入到集合中 
            FD_SET(new_fd, &rfds);
	    
	    maxfd = 0;
            if (new_fd > maxfd)
	    {
                maxfd = new_fd;
	    }
	    
            // 设置最大等待时间 
            tv.tv_sec = 5;
            tv.tv_usec = 0;
	    
            // 开始等待 
            retval = select(maxfd + 1, &rfds, NULL, NULL, &tv);
	    
            if (retval == -1) 
	    {
                printf("将退出，select出错！ %s", strerror(errno));
                break;
            } 
            else if (retval == 0) 
	    {
                printf("没有任何消息到来，用户也没有按键，继续等待……\n");
                continue;
            } 
            else 
	    {
		 // 判断当前IO是否是stdin
                if (FD_ISSET(0, &rfds))  // 用户按键了，则读取用户输入的内容发送出去
		{                  
                    bzero(buf, MAXBUF + 1);
                    fgets(buf, MAXBUF, stdin);
                    if (!strncasecmp(buf, "quit", 4)) 
		    {
                        printf("自己请求终止聊天！\n");
                        break;
                    }
                    len = send(new_fd, buf, strlen(buf) - 1, 0);
                    if (len > 0)
                        printf("消息:%s\t发送成功，共发送了%d个字节！\n", buf, len);
                    else 
		    {
                        printf("消息'%s'发送失败！错误代码是%d，错误信息是'%s'\n", buf, errno, strerror(errno));
                        break;
                    }
                }
                
		 // 判断当前IO是否是来自socket
                if (FD_ISSET(new_fd, &rfds)) // 当前连接的socket上有消息到来则接收对方发过来的消息并显示 
		{                 
                    bzero(buf, MAXBUF + 1);
                    // 接收客户端的消息 
                    len = recv(new_fd, buf, MAXBUF, 0);
                    if (len > 0)
		    {
                        printf("接收消息成功:'%s'，共%d个字节的数据\n", buf, len);
		    }
                    else 
		    {
                        if (len < 0)
                            printf("消息接收失败！错误代码是%d，错误信息是'%s'\n", errno, strerror(errno));
                        else
                            printf("对方退出了，聊天终止\n");
                        break;
                    }
                }
            }
        }
        
        close(new_fd);
        // 处理每个新连接上的数据收发结束
	
        printf("还要和其它连接聊天吗？(no->退出)");
        fflush(stdout);
	
        bzero(buf, MAXBUF + 1);
        fgets(buf, MAXBUF, stdin);
        if (!strncasecmp(buf, "no", 2))
	{
            printf("终止聊天！\n");
            break;
        }
    }

    close(sockfd);
    return 0;
}