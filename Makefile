output: client/client.c server/server.c 
	gcc client/client.c -o client/client.app 
	gcc server/server.c -o server/server.app

clean:
	rm client/client.app server/server.app