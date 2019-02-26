#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include<dirent.h>
#include<sys/stat.h>
#include<sys/types.h>
#define PORT 8080

// checks if acknowledgment is received from new_socket
int check_ack(int new_socket)
{
    char in[1024] = {0};
    int valread = read( new_socket , in, 1024);  // read infromation received into the buffer
    printf("%s\n",in);
    if(strcmp(in,"ack")!=0)
    {
        printf("Error : correct acknowledgement not received.\n");
        return -1;
    }
    return 0;

}

// save the file names in list[]
void get_list(char list[])
{
    DIR *d;
    struct dirent *dir;
    d = opendir(".");
    if(d==NULL)
    {
        list = "Error opening directory";
        return;
    }
    while((dir=readdir(d))!=NULL)
    {
        if(dir->d_name[0]=='.')
            continue;
        strcat(list,dir->d_name);
        strcat(list,"\n");
    }
        //printf("\n");
    closedir(d);

}

// sends file to new_socket
int send_file(char filename[],int new_socket)
{
    FILE* ptr;
    char buffer[1024] = {0};
    ptr = fopen(filename,"r");
    if(ptr==NULL)
        return -1;
    
    char *initial = "file incoming";
    send(new_socket , initial , strlen(initial) , 0 );  // use sendto() and recvfrom() for DGRAM
    int ackn = check_ack(new_socket);
    send(new_socket , filename , strlen(filename) , 0 );  // use sendto() and recvfrom() for DGRAM
    int ack2 = check_ack(new_socket);
    int block;
    while(block = fread(buffer,sizeof(char),1024,ptr)>0)
    {
        send(new_socket , buffer , strlen(buffer) , block );  // use sendto() and recvfrom() for DGRAM
        int ack = check_ack(new_socket);
        memset(buffer,0,1024);
    }
    char *final = "file transfer done";
    send(new_socket , final , strlen(final) , 0 );  // use sendto() and recvfrom() for DGRAM
    int ack = check_ack(new_socket);
    
    /*if(!feof(ptr))
    {
        printf("-1\n");
        return -1;
    } */

    return 0;
}



int main(int argc, char const *argv[])
{
    int running = 1;
    int server_fd, new_socket, valread,new_socket2;
    struct sockaddr_in address;  // sockaddr_in - references elements of the socket address. "in" for internet
    int opt = 1;
    int addrlen = sizeof(address);
    char *hello = "Hello from server :)";

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)  // creates socket, SOCK_STREAM is for TCP. SOCK_DGRAM for UDP
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // This is to lose the pesky "Address already in use" error message
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                                                  &opt, sizeof(opt))) // SOL_SOCKET is the socket layer itself
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;  // Address family. For IPv6, it's AF_INET6. 29 others exist like AF_UNIX etc.
    address.sin_addr.s_addr = INADDR_ANY;  // Accept connections from any IP address - listens from all interfaces.
    address.sin_port = htons( PORT );    // Server port to open. Htons converts to Big Endian - Left to Right. RTL is Little Endian

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address,
                                 sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Port bind is done. You want to wait for incoming connections and handle them in some way.
    // The process is two step: first you listen(), then you accept()
    while(running)
    {
        if (listen(server_fd, 3) < 0) // 3 is the maximum size of queue - connections you haven't accepted
        {
            perror("listen");
            exit(EXIT_FAILURE);
        }
        
        // listening and accepting any waiting connections
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                       (socklen_t*)&addrlen))<0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        // break from loop if child, stay if parent
        pid_t parent = getpid();
        fork();
        if(getpid()!=parent)
        {
            break;
        }        
    }
    
    // returns a brand new socket file descriptor to use for this single accepted connection. Once done, use send and recv
    while(running == 1)
    {
        char buffer[1024] = {0};
        valread = read( new_socket , buffer, 1024);  // read infromation received into the buffer
        if(strlen(buffer) == 0)
            continue;

        printf("%s\n",buffer );
        
        if(strcmp(buffer,"end")==0)
        {
            char* end = "end connection";
            send(new_socket , end , strlen(end) , 0 );  // use sendto() and recvfrom() for DGRAM
            exit(1);
        }

        // if command is listall
        if(strcmp(buffer,"listall")==0)
        {
            char list[1024] = {0};
            get_list(list);
            send(new_socket , list , strlen(list) , 0 );  // use sendto() and recvfrom() for DGRAM
            int ack = check_ack(new_socket);
            continue;
        }
        
        char* saveptr;
        char* token = strtok_r(buffer," ",&saveptr);
        printf("%s\n",token);
        // if command is send 
        if(strcmp(token,"send")==0)
        {
            token = strtok_r(NULL, " ", &saveptr);
            if(token == NULL)
            {
                char *error = "ERROR : No filename given";
                send(new_socket , error , strlen(error) , 0 );  // use sendto() and recvfrom() for DGRAM
                int ack = check_ack(new_socket);
                //printf("%s\n",error);
                continue;
            }
            char *filename = token;
            token = strtok_r(NULL, " ", &saveptr);
            if(token != NULL)
            {
                char *error = "ERROR : Too many filenames given";
                send(new_socket , error , strlen(error) , 0 );  // use sendto() and recvfrom() for DGRAM
                int ack = check_ack(new_socket);
                continue;
            }
            if(send_file(filename,new_socket)==-1)
            {
                char *error = "ERROR : File does not exist";
                send(new_socket , error , strlen(error) , 0 );  // use sendto() and recvfrom() for DGRAM
                int ack = check_ack(new_socket);
                continue;
            }
            printf("after send over\n");
            continue;
        }
        else
        {
            char *error = "ERROR : Request not recognized.";
            send(new_socket , error , strlen(error) , 0 );  // use sendto() and recvfrom() for DGRAM
            int ack = check_ack(new_socket);
            continue;    
        }    
    }
    return 0;
}
