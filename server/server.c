/* A simple server in the internet domain using TCP 
   takes the port number as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

// helpful macros
#define BUFFER_SIZE 256
#define MAX_MSG_SIZE 255
#define NUM_CONNECTIONS 1

void error(char *msg)
{
    perror(msg);
    exit(1);
}

void on_connect(int socket_id)
{
    int n;
    char buffer[BUFFER_SIZE];

    while (1)
    {
        bzero(buffer, MAX_MSG_SIZE);
        n = read(socket_id, buffer, MAX_MSG_SIZE);
        if (n < 0)
            error("ERROR reading from socket");
        printf("Message from %i: %s", socket_id, buffer);

        printf("Please enter the message: ");
        bzero(buffer, BUFFER_SIZE);
        fgets(buffer, MAX_MSG_SIZE, stdin);
        n = write(socket_id, buffer, strlen(buffer));
        if (n < 0)
            error("ERROR writing to socket");
    }
}

int main(int argc, char *argv[])
{
    int sockfd, newsockfd, portno, clilen, pid;
    struct sockaddr_in serv_addr, cli_addr;

    if (argc < 2)
    {
        fprintf(stderr, "ERROR, no port provided\n");
        exit(1);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    { // if opening the socket fails
        error("ERROR opening socket");
    }

    bzero((char *)&serv_addr, sizeof(serv_addr)); // intilializes to zeros
    portno = atoi(argv[1]);
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
            close(sockfd);
            on_connect(newsockfd);
            exit(0);
        }
        else
            close(newsockfd);
    } // end while
    return 0;
}
