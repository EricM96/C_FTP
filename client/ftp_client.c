#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <strings.h>
#include <dirent.h>

#define BUFFER_SIZE 256
#define MAX_MSG_SIZE 255

void error(char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int sockfd, portno, n, fsize, dir_count = 0;
    char *d_fname_temp;
    char *d_fname; 
    char *s_fsize;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    DIR *dir = opendir("./files");
    struct dirent *fname;
    char buffer[BUFFER_SIZE];
    char *file_buffer; 
    char fout_name[BUFFER_SIZE];
    FILE *fout; 

    if (argc < 3)
    {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }

    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        error("ERROR opening socket");
    }
    server = gethostbyname(argv[1]);
    if (server == NULL)
    {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }
    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr,
          server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        error("ERROR connecting");
    }
    while (1)
    {
        printf(">>");
        bzero(buffer, BUFFER_SIZE);
        fgets(buffer, MAX_MSG_SIZE, stdin);

        if (strcmp(buffer, "ls client\n") == 0)
        {
            while ((fname = readdir(dir)) != NULL)
            {
                if (strcmp(fname->d_name, ".") == 0 || strcmp(fname->d_name, "..") == 0)
                {
                    continue;
                }
                else
                {
                    dir_count++;
                    printf("%i. %s\n", dir_count, fname->d_name);
                }
            }
            dir_count = 0;
            rewinddir(dir);
        }
        else if (buffer[0] == 'd')
        {
            n = write(sockfd, buffer, strlen(buffer));
            if (n < 0)
                error("ERROR writing to socket");
            bzero(buffer, BUFFER_SIZE);
            n = read(sockfd, buffer, MAX_MSG_SIZE);
            if (n < 0)
                error("ERROR reading from socket");

            // If server failed to find file, force the next iteration
            if (strcmp(buffer, "ERROR retrieving file") == 0)
            {
                printf("%s\n", buffer);
                continue;
            }

            s_fsize = strtok(buffer, ",");
            d_fname = strtok(NULL, ",");
            
            s_fsize = strtok(s_fsize, ":"); 
            s_fsize = strtok(NULL, ":");

            d_fname_temp = strtok(d_fname, ":");
            d_fname_temp = strtok(NULL, ":");

            printf("file size: %s\nfile name: %s\n", s_fsize, d_fname);

            // strtok returns a pointer to a space within a buffer.
            // To prevent the name of the file from being lost with flushing 
            // the buffer, we need to make a deep copy of the string.
            d_fname = (char *)calloc(strlen(d_fname_temp), sizeof(char)); 
            for (int i = 0; i < strlen(d_fname_temp); ++i)
            { 
                d_fname[i] = d_fname_temp[i];
            }

            bzero(buffer, BUFFER_SIZE);

            printf("d_fname after flushing buffer: %s\n", d_fname);
            strcpy(buffer, "ready for file");

            n = write(sockfd, buffer, strlen(buffer));
            if (n < 0)
                error("ERROR writing to socket");

            fsize = atoi(s_fsize);
            file_buffer = (char *)calloc(fsize, sizeof(char));

            n = read(sockfd, file_buffer, MAX_MSG_SIZE);
            if (n < 0)
                error("ERROR reading file from socket");

            printf("\n%s\n", file_buffer);
            
            /* TODO get the file name and path figured out */ 
            bzero(fout_name, BUFFER_SIZE); 
            strcpy(fout_name, "files//");
            printf("d_fname: %s\n", d_fname);
            strcat(fout_name, d_fname);

            printf("writing to file: %s\n", d_fname);

            fout = fopen(fout_name, "w"); 
            if (fout) { fputs(file_buffer, fout); }
            else { error("ERROR writing to file"); }

            fclose(fout); 
            bzero(fout_name, BUFFER_SIZE); 
            free(file_buffer);
            free(d_fname); 

            continue; 
        }
        else
        {
            n = write(sockfd, buffer, strlen(buffer));
            if (n < 0)
                error("ERROR writing to socket");
            bzero(buffer, BUFFER_SIZE);
            n = read(sockfd, buffer, MAX_MSG_SIZE);
            if (n < 0)
                error("ERROR reading from socket");
            printf("%s\n", buffer);
        }
    }
    return 0;
}