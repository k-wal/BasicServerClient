// Client side C/C++ program to demonstrate Socket programming
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#define PORT 8080

// function to send acknowledgments
void send_ack(int sock)
{
    char* ack ="ack";
    send(sock,ack,strlen(ack),0);
}


int main(int argc, char const *argv[])
{
    int running = 1;
    struct sockaddr_in address;
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char *hello = "Hello from client";
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }

    memset(&serv_addr, '0', sizeof(serv_addr)); // to make sure the struct is empty. Essentially sets sin_zero as 0
                                                // which is meant to be, and rest is defined below

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Converts an IP address in numbers-and-dots notation into either a
    // struct in_addr or a struct in6_addr depending on whether you specify AF_INET or AF_INET6.
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)  // connect to the server address
    {
        printf("\nConnection Failed \n");
        return -1;
    }
    while(running==1)
    {
        char input[1024] = {0};
        fgets (input, 100, stdin);
        int len = strlen(input);
        if(len)
        {   
            input[len-1]='\0';
        }
        send(sock , input , strlen(input) , 0 );  // send the message.
        char buffer[1024] = {0};
        valread = read( sock , buffer, 1024);  // receive message back from server, into the buffer
        send_ack(sock);
        if(strcmp(buffer,"file incoming")==0)
        {
            printf("-------------incoming detected-----------\n");
            char filename[1024] = {0};
            valread = read( sock , filename, 1024);  // receive message back from server, into the buffer
            send_ack(sock);
            //printf("%s\n",filename);
            FILE* fptr;
            fptr = fopen(filename,"w");

            valread = read( sock , buffer, 1024);  // receive message back from server, into the buffer
            send_ack(sock);
            while(strcmp(buffer,"file transfer done")!=0)
            {
                fprintf(fptr,"%s",buffer);
                char buf2[1024] = {0};
                //printf("%s\n",buffer );
                buffer[0] = '\0';
                valread = read( sock , buf2, 1024);  // receive message back from server, into the buffer
                send_ack(sock);
                strcpy(buffer,buf2);
            }
            fclose(fptr);
            printf("-------------incoming done-----------\n");
            
        }
        if(strcmp(buffer,"end connection")==0)
        {
            break;
        }
        else
        {
            printf("%s\n",buffer );
        }
    
    }
    //send(sock , hello , strlen(hello) , 0 );  // send the message.
    //printf("Hello message sent\n");
    //valread = read( sock , buffer, 1024);  // receive message back from server, into the buffer
    //printf("%s\n",buffer );
    return 0;
}
