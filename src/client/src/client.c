#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
/*
    解耦发送和接受消息
    实现elf，源码，makefile不在同一个文件夹下
    实现git控制版本
*/
#define address     "127.0.0.1"
#define port        1234

int main(int argc , char** argv)
{
    int client_sock_fd = socket(AF_INET , SOCK_STREAM , 0); 
    if(client_sock_fd == -1)
    {
        perror("try get listen_sock_fd but fail!");
        exit(-1);   
    }
    struct sockaddr_in client_sock_addr = {
        .sin_addr.s_addr = inet_addr(address),
        .sin_family = AF_INET,
        .sin_port = htons(port),
    };
    if(connect(client_sock_fd , (struct sockaddr*)&client_sock_addr ,sizeof(client_sock_addr)) == -1)
    {
        perror("try connect but fail!"); 
        close(client_sock_fd);
        exit(-1);   
    }
    printf("success connect to %s  %d \n start chatting!\n" , address , port);

    char send_buffer[1024];
    char rec_buffer[1024];
    int rec_len = 0;
    while (1)
    {
        /* code */
        scanf("%s" , send_buffer);
        if(send_buffer[0] == 'q' && send_buffer[1] == '\0')
        {
            printf("exit chat!\n");
            close(client_sock_fd);
            exit(0);
        }
        
        if(send(client_sock_fd , send_buffer , sizeof(send_buffer) , 0) == -1)
        {
            perror("fail to send!"); 
            close(client_sock_fd);
            exit(-1);   

        }
        rec_len = recv(client_sock_fd , rec_buffer , sizeof(rec_buffer) , 0);    //出现乱码的原因：客户端读的字节数量多于服务端发送的字节数量
        if(rec_len == -1)
        {
            perror("fail to recive"); 
            close(client_sock_fd);
            exit(-1);   
        }
        printf("receive from server:  %s \n",rec_buffer);
    }
    return 0;
}