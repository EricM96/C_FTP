output: client/client.c server/server.c 
	gcc client/client.c -o client/client 
	gcc server/server.c -o server/server 

clean:
	rm client/client server/server