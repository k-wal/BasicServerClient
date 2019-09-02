BASIC SERVER AND CLIENTS USING SOCKETS
=======================================

- to run :
	- `gcc -o client.c client`
	- `gcc -o server.c client`

- multiple clients supported
- acknowledgements (from client to server)
- requests
	- `send *filename*` : downloads file from server's directory to client directory (only for text files)
	- `listall` : lists all the files in server's directory
	- `end` : to end the connection
- appropriate error messages for wrong requests and lack of acknowledgements
