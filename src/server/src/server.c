#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#define address     "127.0.0.1"
#define port        1234

int main(int argc , char** agrv)   
{
    
    int listen_sock_fd = socket(AF_INET , SOCK_STREAM , 0);
    if(listen_sock_fd == -1)
    {
        perror("try get listen_sock_fd but fail!");
        exit(-1);   
    }
    struct sockaddr_in listen_sock_addr;
    listen_sock_addr.sin_family = AF_INET;
    listen_sock_addr.sin_addr.s_addr = inet_addr(address);
    listen_sock_addr.sin_port =  htons(port);
    if(bind(listen_sock_fd , (struct sockaddr*)&listen_sock_addr , sizeof(listen_sock_addr)) == -1)
    {
        perror("bind fail!");
        close(listen_sock_fd);
        exit(-1);
    }
    if(listen(listen_sock_fd , 5) == -1)
    {
        perror("listen fail!");
        close(listen_sock_fd);
        exit(-1);
    }
    
    for(;;)
    {   
        printf("----------------------------------------------\n");
        printf("waiting for connect~\n");

        struct sockaddr_in client_sock_addr;
        socklen_t client_sock_addr_len = sizeof(client_sock_addr);
        int accept_sock_fd = accept(listen_sock_fd , (struct sockaddr*)&client_sock_addr , &client_sock_addr_len);
        if(accept_sock_fd == -1)
        {
            perror("accept fail!");
            close(listen_sock_fd);
            exit(-1);
        }
        printf("connect success!  client:%s , %d \n" , inet_ntoa(client_sock_addr.sin_addr) , ntohs(client_sock_addr.sin_port));

        char buffer[1024];
        char res_buffer_suffix[80] = ": i have received";
        while (1)
        {
            /* code */
            int msg_len = recv(accept_sock_fd , buffer , sizeof(buffer) , 0);
            if(msg_len == -1)
            {
                perror("recv fail!");
                close(listen_sock_fd);
                close(accept_sock_fd);
                exit(-1);
            }
            else if(msg_len == 0)
            {
                printf("client:%s , %d     disconnect!\n" , inet_ntoa(client_sock_addr.sin_addr) , ntohs(client_sock_addr.sin_port));
                close(accept_sock_fd);
                break;
            }
            printf("received message: %s \n" , buffer);
            
            strcat(buffer , res_buffer_suffix); //不要把strcat的返回值直接作为send的参数
            if(send(accept_sock_fd , buffer , strlen(buffer) , 0 ) == -1)
            {
                perror("send fail!");
                close(listen_sock_fd);
                close(accept_sock_fd);
                exit(-1);     
            }
            memset(buffer , 0 , sizeof(buffer));
            printf("ok!");
        }

    }
    
    printf("server offline!\n");
    close(listen_sock_fd);
    
   
    return 0;
    
}
/*
select()的注意事项：
        在fd_set被set的fd，在select中如果没有准备好，则会被clear，只有准备好的才会保留被set的状态
*/