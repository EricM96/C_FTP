output: client/ftp_client.c server/ftp_server.c 
	gcc client/ftp_client.c -o client/client.app 
	gcc server/ftp_server.c -o server/server.app

clean:
	rm client/client.app server/server.app