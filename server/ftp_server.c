/* A simple server in the internet domain using TCP 
   takes the port number as an argument */
#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h> 
#include <strings.h>
#include <dirent.h>
#include <stdbool.h>

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
    int n, findex, fsize, dir_count = 0;
    char buffer[BUFFER_SIZE];
    char sdir_count[5];
    char *s_findex;
    char s_fsize[BUFFER_SIZE];
    char file_name[BUFFER_SIZE];
    char *file_buffer; 
    bool found_file = false;
    FILE *fin; 
    DIR *dir = opendir("./files");
    struct dirent *fname;

    while (1)
    {
        bzero(buffer, MAX_MSG_SIZE);
        n = read(socket_id, buffer, MAX_MSG_SIZE);
        if (n < 0)
            error("ERROR reading from socket");
        
        // Code for ls command
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

        // Code for download command
        else if (buffer[0] == 'd')
        {
            // Extract file index from message 
            s_findex = strtok(buffer, " "); 
            s_findex = strtok(NULL, " ");
            findex = atoi(s_findex);

            printf("Looking for file: %d\n", findex);

            bzero(buffer, BUFFER_SIZE);

            // Search for file matching index
            while ((fname = readdir(dir)) != NULL)
            {
                if (strcmp(fname->d_name, ".") != 0 && strcmp(fname->d_name, "..") != 0) 
                {
                    dir_count++;
                    if (dir_count == findex)
                    {
                        found_file = true;
                        strcpy(file_name, "files/");
                        strcat(file_name, fname->d_name);
                
                        fin = fopen(file_name, "r"); 
                        if (fin == NULL)
                            strcpy(buffer, "ERROR retrieving file");

                        else
                        {
                            // Find size of file in bytes
                            fseek(fin, 0, SEEK_END);
                            fsize = ftell(fin) + 1;
                            rewind(fin); 
                        
                            printf("File size %d\n", fsize);
                            sprintf(s_fsize, "%d", fsize);

                            strcpy(buffer, "file_size:");
                            strcat(buffer, s_fsize);
                            strcat(buffer, ",");
                            strcat(buffer, "file_name:");
                            strcat(buffer, fname->d_name); 

                            // Send client file name and file size 
                            n = write(socket_id, buffer, strlen(buffer));
                            if (n < 0)
                                error("ERROR writing to socket");

                            bzero(buffer, BUFFER_SIZE);
                            n = read(socket_id, buffer, MAX_MSG_SIZE);
                            if (n < 0)
                                error("ERROR reading from socket");

                            // If client indicates it is ready for the file
                            if (strcmp(buffer, "ready for file") == 0)
                            {
                                // Realloc file_buffer to be proper size 
                                file_buffer = calloc(fsize, sizeof(char)); 
                                n = fread(file_buffer, sizeof(char), fsize, fin);
                                if (n < 0)
                                    error("ERROR reading from file");
                                fclose(fin); 

                                // Send file 
                                printf("About to upload file\n");
                                n = write(socket_id, file_buffer, fsize);
                                if (n < 0)
                                    error("ERROR writing file to socket");

                                // Free memory for file_buffer
                                free(file_buffer);
                            }
                        }
                        break;
                    }
                }
            }

            if (!found_file)
            {
                strcpy(buffer, "ERROR retrieving file");
            }
            else
            {
                found_file = false;
            }
            

        } else
        {
            printf("First char: %c\n", buffer[0]);
        }
        printf("Sending message: %s\n", buffer);
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
    { 
        error("ERROR opening socket");
    }

    bzero((char *)&serv_addr, sizeof(serv_addr)); 
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno); 
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
    } 
    return 0;
}