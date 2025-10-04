#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <sys/select.h>

#define default_address     "127.0.0.1"
#define default_port        1234
#define MAX_CLIENT_SIZE     10
#define server_res_surffix  ":server have received" 

int create_listen_socket(char* address , int port)
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
        perror("bind fail:");
        close(listen_sock_fd);
        exit(-1);
    }
    if(listen(listen_sock_fd , 5) == -1)
    {
        perror("listen fail:");
        close(listen_sock_fd);
        exit(-1);
    }
    printf("waitting for connect! \n");

    return listen_sock_fd;
}


typedef struct
{
    int client_fd;
    struct sockaddr_in client_socket_addr;
}client_socket;

client_socket create_accept_socket(int listen_fd)
{
    struct sockaddr_in client_sock_addr;
    socklen_t client_sock_addr_len = sizeof(client_sock_addr);
    int accept_sock_fd = accept(listen_fd , (struct sockaddr*)&client_sock_addr , &client_sock_addr_len);
    if(accept_sock_fd == -1)
    {
        perror("accept fail:");
        printf("\n");
        client_socket accept_socket = {
            .client_fd = -1 ,
        };
        return accept_socket;
    }
    printf("connect success!  client:%s , %d \n" , inet_ntoa(client_sock_addr.sin_addr) , ntohs(client_sock_addr.sin_port));
    client_socket accept_socket = {
        .client_fd = accept_sock_fd , 
        .client_socket_addr = client_sock_addr
    };
    return accept_socket;
}


//-------------------------------------------------------------
/*以下是OOP实现一个客户端的socket集合 */

struct Client_set
{
    client_socket set[MAX_CLIENT_SIZE];
    int now_size;
    int (*add)(struct Client_set * , client_socket);
    int (*get_max_fd)(struct Client_set  , int);
    void (*delete)(struct Client_set *  , int);
    void (*distory)(struct Client_set *);
};

int Client_set__add(struct Client_set *self , client_socket new_client_socket)
{
    if(self->now_size < MAX_CLIENT_SIZE)
    {
        self->set[self->now_size] = new_client_socket;
        self->now_size++;
        return 1;
    }
    else
    {
        /*达到最大连接数量*/
        return -1;
    }
}


void Client_set__delete(struct Client_set *self , int index)
{
    /*index之后的元素整体前移*/
    if(index < self->now_size && self->now_size > 0)
    {
        close(self->set[index].client_fd);
        self->now_size --;
        for(int i = index ; i < self->now_size ; i++)
        {
            self->set[i] = self->set[i+1];
        }
        self->set[self->now_size].client_fd = -1;
    }
}

int Client_set__get_max_fd(struct Client_set self , int listen_fd)
{
    int max = listen_fd;
    for(int i = 0 ; i < self.now_size ; i++)
    {
        if(self.set[i].client_fd > max) 
            max = self.set[i].client_fd;
    }
    return max;
}
void Client_set__distory(struct Client_set* self)
{
    for(int i = 0 ; i <self->now_size ; i++)
    {
        close(self->set[i].client_fd);
        self->set[i].client_fd = -1;
    }
    self->now_size = 0;
}

struct Client_set create_Client_set()
{
    struct Client_set client_set;
    for(int  i = 0 ; i <MAX_CLIENT_SIZE ; i++)
    {
        client_set.set[i].client_fd = -1;
    }
    client_set.now_size = 0;
    client_set.add = Client_set__add;
    client_set.get_max_fd = Client_set__get_max_fd;
    client_set.delete = Client_set__delete;
    return client_set;
}


//-------------------------------------------------------------


int main(int argc , char** agrv)
{
    char* address;
    int port;
    if(argc > 3)
    {
        address = agrv[1];
        port = atoi(agrv[2]);   //把字符转换成int型
    }
    else
    {
        address = default_address;
        port = default_port;
    }

    int listen_sock_fd = create_listen_socket(address , port);


    struct Client_set client_set = create_Client_set();

    fd_set readset;

    char rec_buffer[1024];

    
    for(;;)
    {   
        /*主循环里执行的函数里不可以有exit()函数 ， 便于错误处理*/
        FD_ZERO(&readset);
        FD_SET(listen_sock_fd , &readset);
        for(int i = 0 ; i < client_set.now_size ;i++)
        {
            FD_SET(client_set.set[i].client_fd , &readset);
        }

        int maxfd = client_set.get_max_fd(client_set  , listen_sock_fd);
        if(-1 == select(maxfd+1 , &readset , NULL , NULL , NULL))
        {
            perror("select:");
            exit(-1);
        }


        if(FD_ISSET(listen_sock_fd , &readset))
        {
            /*已经有客户端完成了与服务端的tcp的三次握手,可以被accept了*/
            client_socket accept_socket = create_accept_socket(listen_sock_fd);
            if(accept_socket.client_fd == -1)
            {
                /*create_accept_socket()函数里accept函数出问题了*/
                close(listen_sock_fd);
                client_set.distory(&client_set);
                exit(-1);
            }
            if(client_set.add(&client_set , accept_socket) == -1)
            {
                printf("connect fail! reach the maximum client quantity!\n");
            }
            else
            {
                /*向新连接的客户端发送欢迎*/
                char welcome[1024] = "welcome!";
                if(-1 == send(accept_socket.client_fd , welcome , sizeof(welcome) , 0))
                {
                    perror("send welcome");
                    close(listen_sock_fd);
                    client_set.distory(&client_set);
                    exit(-1);
                }
                
            }
        }

        for(int i = 0 ; i < client_set.now_size ;i++)
        {
            if(FD_ISSET(client_set.set[i].client_fd , &readset))
            {
                int msg_len = recv(client_set.set[i].client_fd , rec_buffer , sizeof(rec_buffer) , 0);
                if(msg_len == -1)
                {
                    /*错误处理*/
                    perror("recv fail:");
                    printf("\n");
                    client_set.distory(&client_set);
                    close(listen_sock_fd);
                    exit(-1);
                }
                else if(msg_len == 0)
                {
                    /*客户端正常断开连接，处理*/
                    printf("%s , %d disconnect! \n" , inet_ntoa(client_set.set[i].client_socket_addr.sin_addr) , ntohs(client_set.set[i].client_socket_addr.sin_port));
                    client_set.delete(&client_set , i);
                }
                else
                {
                    /*正常收到消息*/
                    printf("%s , %d: %s \n" , inet_ntoa(client_set.set[i].client_socket_addr.sin_addr) , ntohs(client_set.set[i].client_socket_addr.sin_port) , rec_buffer);
                    char* surffix = server_res_surffix;
                    strcat(rec_buffer , surffix);
                    if(-1 == send(client_set.set[i].client_fd , rec_buffer , sizeof(rec_buffer) , 0)) //出现乱码的原因：客户端读的字节数量多于服务端发送的字节数量
                    {
                        perror("send fail:");
                        printf("\n");
                        client_set.distory(&client_set);
                        close(listen_sock_fd);
                        exit(-1);
                    }

                }
                memset(rec_buffer , 0 , sizeof(rec_buffer));
            }
        }



    }
    
    printf("server offline!\n");
    client_set.distory(&client_set);
    close(listen_sock_fd);
    
   
    return 0;
    
}
/*
select()的注意事项：
        在fd_set被set的fd，在select中如果没有准备好，则会被clear，只有准备好的才会保留被set的状态
*/