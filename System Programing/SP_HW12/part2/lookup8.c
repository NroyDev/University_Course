/*
 * lookup8 : does no looking up locally, but instead asks
 * a server for the answer. Communication is by Internet TCP Sockets
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

	if (first_time) {        /* connect to socket ; resource is server name */
		first_time = 0;

		/* Set up destination address. */
		memset((char *) &server, '\0', sizeof(server));
		server.sin_family = AF_INET;
		/* Fill in code. */
		server.sin_port = htons(PORT);
		host = gethostbyname(resource);
		if (host == NULL) {
			herror("gethostbyname");
			exit(1);
    	}
		memcpy(&server.sin_addr, host->h_addr_list[0], host->h_length);
		// if(inet_pton(AF_INET, resource, &server.sin_addr) < 0){		// 網路上是推薦說用這個 上面那個是簡報上的寫法 原本的code 也有 hostent
		// 	perror("inet_pton");
		// 	exit(1);
		// }


		/* Allocate socket.
		 * Fill in code. */
		if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
			perror("socket");
			exit(1);
		}

		/* Connect to the server.
		 * Fill in code. */
		if(connect(sockfd, (struct sockaddr *)&server, sizeof(server))){
			perror("connect");
			exit(1);
		}
		printf("[Client] Connected\n");
	}

	/* write query on socket ; await reply
	 * Fill in code. */
	write(sockfd ,sought, sizeof(Dictrec));
	read(sockfd, sought, sizeof(Dictrec));

	if (strcmp(sought->text,"XXXX") != 0) {
		return FOUND;
	}

	return NOTFOUND;
}
