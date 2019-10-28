//********************************************************************
//
// Eric McCullough
// Computer Networks
// Homework 3: FTP server and client 
// October 28, 2019
// Instructor: Dr. Ajay K. Katangur
//
//********************************************************************
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
#include <stdbool.h>

#define BUFFER_SIZE 256
#define MAX_MSG_SIZE 255

//********************************************************************
//
// error function
// --------------
// This function displays errors to the user when encountered and 
// gracefully shuts down the program
//
// Return Value
// ------------
// None
//
// Value paramaters
// ----------------
// msg          string      The error message to be displayed
// 
//********************************************************************
void error(char *msg)
{
    perror(msg);
    exit(0);
}

//********************************************************************
//
// Main Function
// -------------
// This function handles the main execution loop of the ftp client 
//
// Return Value
// ------------
// 0 if sucess other if not 
//
// Value paramaters
// ----------------
// argc         int         the number of command line arguments (should be 3)
// argv         string array    the command line arguments (hostname and port of server)
//
// Local Variables
// ---------------
// sockfd       int         the file descriptor for the host socket
// portno       int         port number for the server
// n            int         the number of characters written/read to/from a socket
// fsize        int         the size of the file in bytes to be uploaded/downloaded
// findex       int         the index of the file to be uploaded/downloaded
// dircount     int         iterator tracker for finding a file marked by findex 
//
// d_fname_temp string      temporary string to hold the name of the file to be uploaded/downloaded
// d_fname      string      permanent copy of d_fname_temp
// s_fsize      string      string representation of download file size 
// s_findex     string      string represenation of findex
// file_buffer  string      resizable buffer for downloaded/uploaded file 
// 
// s_fsize_u    character array     buffer to hold uploaded file size
// buffer       character array     buffer for standard communication with server
// fout_name    character array     buffer for file output name (including directory)
// file_name    character array     buffer for file output name (only file) 
// 
// serv_addr    sockaddr_in         socket address structure for server
// server       hostent pointer     Host entry for server
// 
// dir          DIR pointer         directory for files
// fname        dirent pointer      an entry in dir
// 
// fout         file pointer        file output
// fin          file pointer        file input 
//
// found_file   bool                flag to see if file was found or not
// 
//********************************************************************
int main(int argc, char *argv[])
{
    int sockfd, portno, n, fsize, findex, dir_count = 0;
    char *d_fname_temp;
    char *d_fname; 
    char *s_fsize;
    char *s_findex;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    DIR *dir = opendir("./files");
    struct dirent *fname;
    char buffer[BUFFER_SIZE];
    char s_fsize_u[BUFFER_SIZE];
    char *file_buffer; 
    char fout_name[BUFFER_SIZE];
    char file_name[BUFFER_SIZE];
    FILE *fout;
    FILE *fin;
    bool found_file = false;

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

        // Code for local ls 
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
        // Code for exit command
        else if (strcmp(buffer, "bye\n") == 0)
        {
            n = write(sockfd, buffer, strlen(buffer));
            if (n < 0)
                error("ERROR writing to socket");
            break; 
        }
        // Code for download command 
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

            // strtok returns a pointer to a space within a buffer.
            // To prevent the name of the file from being lost with flushing 
            // the buffer, we need to make a deep copy of the string.
            d_fname = (char *)calloc(strlen(d_fname_temp), sizeof(char)); 
            for (int i = 0; i < strlen(d_fname_temp); ++i)
            { 
                d_fname[i] = d_fname_temp[i];
            }
            fsize = atoi(s_fsize);

            bzero(buffer, BUFFER_SIZE);

            strcpy(buffer, "ready for file");

            n = write(sockfd, buffer, strlen(buffer));
            if (n < 0)
                error("ERROR writing to socket");

            file_buffer = (char *)calloc(fsize, sizeof(char));

            n = read(sockfd, file_buffer, fsize);
            if (n < 0)
                error("ERROR reading file from socket");
              
            bzero(fout_name, BUFFER_SIZE); 
            strcpy(fout_name, "files//");
            strcat(fout_name, d_fname);

            fout = fopen(fout_name, "w"); 
            if (fout) { fputs(file_buffer, fout); }
            else { error("ERROR writing to file"); }

            fclose(fout); 
            bzero(buffer, BUFFER_SIZE);
            bzero(fout_name, BUFFER_SIZE); 
            free(file_buffer);
            free(d_fname); 

            continue; 
        }
        // Code for upload command
        else if (buffer[0] == 'u')
        {
            // Extract file index from message
            s_findex = strtok(buffer, " ");
            s_findex = strtok(NULL, " ");
            findex = atoi(s_findex);

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

                            sprintf(s_fsize_u, "%d", fsize);

                            strcpy(buffer, "file_size:");
                            strcat(buffer, s_fsize_u);
                            strcat(buffer, ",");
                            strcat(buffer, "file_name:");
                            strcat(buffer, fname->d_name);

                            // Send client file name and file size
                            n = write(sockfd, buffer, strlen(buffer));
                            if (n < 0)
                                error("ERROR writing to socket");

                            bzero(buffer, BUFFER_SIZE);
                            n = read(sockfd, buffer, MAX_MSG_SIZE);
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
                                n = write(sockfd, file_buffer, fsize);
                                if (n < 0)
                                    error("ERROR writing file to socket");

                                // Free memory for file_buffer
                                free(file_buffer);
                                bzero(buffer, BUFFER_SIZE);
                            }
                        }
                        break;
                    }
                }
            }
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