/*
 * lookup9 : does no looking up locally, but instead asks
 * a server for the answer. Communication is by Internet UDP Sockets
 * The name of the server is passed as resource. PORT is defined in dict.h
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include "dict.h"

int lookup(Dictrec * sought, const char * resource) {
	static int sockfd;
	static struct sockaddr_in server;
	struct hostent *host;
	static int first_time = 1;

	if (first_time) {  /* Set up server address & create local UDP socket */
		first_time = 0;

		/* Set up destination address. */
		memset((char *) &server, '\0', sizeof(server));
		server.sin_family = AF_INET;
		/* Fill in code. */
		server.sin_port = htons(PORT);
		// if(inet_pton(AF_INET, resource, &server.sin_addr) < 0){
		// 	perror("inet_pton");
		// 	exit(1);
		// }
		host = gethostbyname(resource);
		if (host == NULL) {
			herror("gethostbyname");
			exit(1);
    	}
		memcpy(&server.sin_addr, host->h_addr_list[0], host->h_length);

		/* Allocate socket.
		 * Fill in code. */
		if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
			perror("socket");
			exit(1);
		}
	}

	/* Send a datagram & await reply
	 * Fill in code. */
	socklen_t addr_len = sizeof(server);
	sendto(sockfd, sought, sizeof(Dictrec), 0, (const struct sockaddr *)&server, addr_len);
	int n = recvfrom(sockfd, sought, sizeof(Dictrec), 0, (struct sockaddr *)&server, &addr_len);

	if(strcmp(sought->text,"XXXX") != 0){
		return FOUND;
	}

	return NOTFOUND;
}
