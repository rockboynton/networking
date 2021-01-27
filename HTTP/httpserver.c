// Simple TCP echo server

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// Max message to echo
#define MAX_MESSAGE 100000

void parse_request(int connection, char *request)
{
	char *verb = strtok(request, " ");
	if (strcmp(verb, "GET") != 0)
	{
		printf("Cannot parse commandf verb %s", verb);
	}
	char *uri = strtok(NULL, " ");
	if (strcmp(uri, "/") == 0)
	{
		uri = "index.html";
	}
	else
	{
		uri++; // remove `/`
	}

	FILE *fptr;
	int response_code = 200;
	if ((fptr = fopen(uri, "rb")) == NULL)
	{
		response_code = 404;
		fptr = fopen("404.html", "rb");
	}
	char *response = calloc(MAX_MESSAGE, sizeof(char));
	strcpy(response, "HTTP/1.1 ");
	char response_code_str[3];
	sprintf(response_code_str, "%d", response_code);
	strcat(response, response_code_str);
	strcat(response, response_code == 200 ? " OK" : " Not Found");
	strcat(response, "\r\n\r\n");
	size_t bytes_written = strlen(response);
	bytes_written += fread(response + bytes_written, 1, MAX_MESSAGE, fptr);
	int bytes_sent;
	if ((bytes_sent = write(connection, response, bytes_written)) < 0)
	{
		perror("Error sending response");
		exit(-1);
	}
	else
		printf("Bytes sent: %d\n", bytes_sent);
	free(response);
	close(connection);
}

	/* server main routine */
int main(int argc, char **argv)
{
	// locals
	unsigned short port = 22222; // default port
	int sock;					 // socket descriptor

	// Was help requested?  Print usage statement
	if (argc > 1 && ((!strcmp(argv[1], "-?")) || (!strcmp(argv[1], "-h"))))
	{
		printf("\nUsage: httpserver [-p port] port is the requested \
 port that the server monitors.  If no port is provided, the server \
 listens on port 22222.\n\n");
		exit(0);
	}

	// get the port from ARGV
	if (argc > 1 && !strcmp(argv[1], "-p"))
	{
		if (sscanf(argv[2], "%hu", &port) != 1)
		{
			perror("Error parsing port option");
			exit(0);
		}
	}

	// ready to go
	printf("HTTP server configuring on port: %d\n", port);

	// for TCP, we want IP protocol domain (PF_INET)
	// and TCP transport type (SOCK_STREAM)
	// no alternate protocol - 0, since we have already specified IP
	if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("Error on socket creation");
		exit(1);
	}

	// lose the pesky "Address already in use" error message
	int yes = 1;

	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
	{
		perror("setsockopt");
		exit(1);
	}

	// establish address - this is the server and will
	// only be listening on the specified port
	struct sockaddr_in sock_address;

	// address family is AF_INET
	// our IP address is INADDR_ANY (any of our IP addresses)
	// the port number is per default or option above

	sock_address.sin_family = AF_INET;
	sock_address.sin_addr.s_addr = htonl(INADDR_ANY);
	sock_address.sin_port = htons(port);

	// we must now bind the socket descriptor to the address info
	if (bind(sock, (struct sockaddr *)&sock_address, sizeof(sock_address)) < 0)
	{
		perror("Problem binding");
		exit(-1);
	}

	// extra step to TCP - listen on the port for a connection
	// willing to queue 5 connection requests
	if (listen(sock, 5) < 0)
	{
		perror("Error calling listen()");
		exit(-1);
	}

	// go into forever loop and echo whatever message is received
	// to console and back to source
	char *request = calloc(MAX_MESSAGE, sizeof(char));
	int request_bytes_read;
	int connection;

	while (1)
	{
		// hang in accept and wait for connection
		printf("====Waiting====\n");
		connection = accept(sock, NULL, NULL);
		if (connection < 0)
		{
			perror("Error calling accept");
			exit(-1);
		}
		pid_t pid = fork();

		if (pid < 0)
			perror("Error calling fork");

		if (pid == 0)
		{
			close(sock);
			// ready to r/w - another loop - it will be broken when the connection is closed
			while (1)
			{
				// read message
				request_bytes_read = read(connection, request, MAX_MESSAGE - 1);

				// print info to console
				printf("Received message\n");

				// put message to console
				printf("Message: \n%s\n", request);

				parse_request(connection, request);
			}
		}
		else
			close(connection);
	} // end of outer loop

	// will never get here
	return (0);
}
