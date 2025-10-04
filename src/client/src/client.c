#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <string.h>


#define target_address     "127.0.0.1"
#define target_port        1234

int create_client_socket(char* address , int port)
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
    printf("success connect to %s  %d \n start chatting!\n" , target_address , target_port);
    return client_sock_fd;
}


int main(int argc , char** argv)
{
    int client_sock_fd = create_client_socket(target_address , target_port);

    char send_buffer[1024];
    char rec_buffer[1024];
    int rec_len = 0;
    
    fd_set read_set;    //stdin是只读的

    for(;;)
    {
        FD_ZERO(&read_set);
        FD_SET(client_sock_fd , &read_set);
        FD_SET(STDIN_FILENO , &read_set);
    
        if(-1 == select(client_sock_fd+1 , &read_set , NULL , NULL , NULL))
        {
            perror("select:");
            exit(-1);
        }

        if(FD_ISSET(STDIN_FILENO , &read_set))
        {
            /*已经输入了东西*/
            if(NULL == fgets(send_buffer , sizeof(send_buffer) , stdin)) //从stdin里读出来
            {
                perror("fgets"); 
                close(client_sock_fd);
                exit(-1);                   
            }
            if(send_buffer[0] == 'q' && send_buffer[1] == '\n')
            {
                printf("exit chat!\n");
                close(client_sock_fd);
                exit(0);
            }
            /*从stdin里读出来的字符串是这样的：hello\n\0 , 可以处理一下换行符 。 strlen()返回的长度里包含了\n ,不包含\0  */
            send_buffer[strlen(send_buffer) - 1] = '\0';
            if(send(client_sock_fd , send_buffer , sizeof(send_buffer) , 0) == -1)
            {
                perror("fail to send"); 
                close(client_sock_fd);
                exit(-1);   
            }

        }

        if(FD_ISSET(client_sock_fd , &read_set))
        {
            rec_len = recv(client_sock_fd , rec_buffer , sizeof(rec_buffer) , 0);    //出现乱码的原因：客户端读的字节数量多于服务端发送的字节数量
            if(rec_len == -1)
            {
                perror("fail to recive"); 
                close(client_sock_fd);
                exit(-1);   
            }
            printf("receive from server:  %s \n",rec_buffer);
        }
    }
    return 0;
}