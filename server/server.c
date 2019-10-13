/* A simple server in the internet domain using TCP 
   takes the port number as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h> 
#include <strings.h>
#include <dirent.h>

#define BUFFER_SIZE 256
#define MAX_MSG_SIZE 255
#define NUM_CONNECTIONS 5

void error(char *msg)
{
    perror(msg);
    exit(1);
}

void on_connect(int socket_id)
{
    int n, dir_count = 0;
    char buffer[BUFFER_SIZE];
    char sdir_count[5]; 
    DIR *dir = opendir("./files");
    struct dirent *fname;

    while (1)
    {
        bzero(buffer, MAX_MSG_SIZE);
        n = read(socket_id, buffer, MAX_MSG_SIZE);
        if (n < 0)
            error("ERROR reading from socket");\
        if (strcmp(buffer, "ls server\n") == 0)
        {
            bzero(buffer, BUFFER_SIZE);
            while ((fname = readdir(dir)) != NULL)
            {
                if (strcmp(fname->d_name, ".") == 0 || strcmp(fname->d_name, "..") == 0) 
                {
                    continue;
                }
                else if (dir_count == 0)
                {
                    dir_count++; 
                    strcpy(buffer, "1. ");
                    strcat(buffer, fname->d_name);
                    strcat(buffer, "\n");
                }
                else
                {
                    dir_count++;
                    sprintf(sdir_count, "%d", dir_count);
                    strcat(buffer, strcat(sdir_count, ". "));
                    strcat(buffer, fname->d_name);
                    strcat(buffer, "\n");
                } 
            }   
        }
        n = write(socket_id, buffer, strlen(buffer));
        if (n < 0)
            error("ERROR writing to socket");

        dir_count = 0;
        rewinddir(dir);
    }
}

int main(int argc, char *argv[])
{
    int sockfd, newsockfd, pid;
    const int portno = 12000;
    socklen_t clilen; 
    struct sockaddr_in serv_addr, cli_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    { // if opening the socket fails
        error("ERROR opening socket");
    }

    bzero((char *)&serv_addr, sizeof(serv_addr)); // intilializes to zeros
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno); // htons converts portno to network byte order
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        error("ERROR on binding");
    }
    listen(sockfd, NUM_CONNECTIONS);
    clilen = sizeof(cli_addr);
    while (1)
    {
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
        if (newsockfd < 0)
            error("ERROR on accept");
        pid = fork();
        if (pid < 0)
            error("ERROR on fork");
        if (pid == 0)
        {
            printf("pid: %i\n", pid); 
            close(sockfd);
            on_connect(newsockfd);
            exit(0);
        }
        else
            close(newsockfd);
    } // end while
    return 0;
}
