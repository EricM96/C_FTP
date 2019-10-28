# C_FTP
A simple socket file transfer server and client written in C 

## Usage
The server and client runs on unix sockets, and therefore will only run on Linux
and MacOS systems.

Run ```make``` in the project root directory.
1. run ```./server.app``` in the *server directory* 
2. run ```./client.app <host name> <port number>``` in the *client directory* 

**NOTES**
* The server and client must be run in their respective directories with their file folders
* The client must be provided with the server hostname, *not* its ip address 

## Commands
* ```ls server``` to see the files available from the server
* ```ls client``` to see the files available on the client 
* ```d <num>``` download file from server located an index <num>
* ```u <num>``` upload file to server located at index <num> 
* ```bye``` close connection to server 

## Known Bugs
* Default buffer size for both components is 256 bytes. If any standard communication
including the contents of a directory exceed this number, a segmentation fault will result.
* The client cannot accept a ip address, it must be given a host name 
* The client and server cannot read or write from any directory other than the file
directories in their respective folders. 
