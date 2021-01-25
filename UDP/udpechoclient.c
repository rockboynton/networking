// Simple UDP echo server
// CE4961, Dr. Rothe
//
// Build with gcc -o udpechoserver udpecchoserver.c

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// Max message to echo
#define MAX_MESSAGE 1000

/* server main routine */
int main(int argc, char **argv)
{
    // locals
    unsigned short port = 22222; // default port
    int sock;                    // socket descriptor

    // Was help requested?  Print usage statement
    if (argc > 1 && ((!strcmp(argv[1], "-?")) || (!strcmp(argv[1], "-h"))))
    {
        printf("\nUsage: udpechoserver [-p port] where port is the requested \
			port that the server monitors.  If no port is provided, the server \
			listens on port %d.\n\n",
               port);
        exit(1);
    }

    // get the port from ARGV
    if (argc > 1 && !strcmp(argv[1], "-p"))
    {
        if (sscanf(argv[2], "%hu", &port) != 1)
        {
            perror("Error parsing port option");
            exit(1);
        }
    }

    // ready to go
    printf("UDP Echo Server configuring on port: %d\n", port);

    // for UDP, we want IP protocol domain (AF_INET)
    // and UDP transport type (SOCK_DGRAM)
    // no alternate protocol - 0, since we have already specified IP

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("Error on socket creation");
        exit(1);
    }

    // establish address - this is the server and will
    // only be listening on the specified port
    struct sockaddr_in sock_address;

    // address family is AF_INET
    // fill in INADDR_ANY for address (any of our IP addresses)
    // for a client, this would be the desitation address
    // the port number is per default or option above
    // note that address and port must be in memory in network order

    sock_address.sin_family = AF_INET;
    sock_address.sin_addr.s_addr = htonl(INADDR_ANY);
    sock_address.sin_port = htons(port);

    // go into forever loop and echo whatever message is received
    // to console and back to source
    char *message_buffer = calloc(MAX_MESSAGE, sizeof(char));
    char *ip_buffer = calloc(MAX_MESSAGE, sizeof(char));

    int bytes_read;
    int bytes_echoed;
    struct sockaddr_in from;
    socklen_t from_len;

    while (1)
    {
        from_len = sizeof(from);

        // get message from console
        printf("Message: \n");
        scanf("%s", message_buffer);

        // get IP address from console
        printf("IP address to send to: \n");
        scanf("%s", ip_buffer);

        uint32_t servaddr = htonl(inet_pton(AF_INET, ip_buffer))

        sendto(sock, (const char *)message_buffer, strlen(message_buffer),
               MSG_CONFIRM, (const struct sockaddr *)&servaddr,
               sizeof(servaddr));
    }

    // minor issue - we will never get here...

    // release buffers
    free(message_buffer);
    free(ip_buffer);
    // close socket
    close(sock);
    // done
    return (0);
}
