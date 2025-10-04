#include <stdio.h>
#include <sys/select.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
int main(int argc , char** argv)
{
    char buffer[20] = "hello\n";
    printf("%s , %ld" , buffer , strlen(buffer));
    return 0;
}

