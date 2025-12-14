/*
 * udp_server : listen on a UDP socket ;reply immediately
 * argv[1] is the name of the local datafile
 * PORT is defined in dict.h
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <errno.h>

#include "dict.h"

int main(int argc, char **argv) {
	static struct sockaddr_in server,client;
	int sockfd,siz;
	Dictrec dr, *tryit = &dr;

	if (argc != 2) {
		fprintf(stderr,"Usage : %s <datafile>\n",argv[0]);
		exit(errno);
	}

	/* Create a UDP socket.
	 * Fill in code. */
	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
		perror("socket");
		exit(1);
	}

	/* Initialize address.
	 * Fill in code. */
	memset((char *) &server, '\0', sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(PORT);
	server.sin_addr.s_addr = htonl(INADDR_ANY);

	/* Name and activate the socket.
	 * Fill in code. */
	if(bind(sockfd, (struct sockaddr*)&server, sizeof(server))){
		perror("bind");
		exit(1);
	}

	for (;;) { /* await client packet; respond immediately */

		siz = sizeof(client); /* siz must be non-zero */

		/* Wait for a request.
		 * Fill in code. */
		int n = -1;

		while((n=recvfrom(sockfd, (void*)tryit, sizeof(Dictrec), 0, (struct sockaddr *)&client, &siz)) != -1){
			printf("[Server] Recv request %s \n", tryit->word);
			fflush(stdout);
			/* Lookup request and respond to user. */
			switch(lookup(tryit,argv[1]) ) {
				/* Write response back to the client. */
				case FOUND:
					/* Fill in code. */
					sendto(sockfd, (void*)tryit, sizeof(Dictrec), 0, (const struct sockaddr *)&client, siz);
					printf("[Server] LookUP success\n");
					fflush(stdout);
					break;
				case NOTFOUND : 
					/* Send response.
					 * Fill in code. */
					strcpy(tryit->text, "XXXX");
					sendto(sockfd, (void*)tryit, sizeof(Dictrec), 0, (const struct sockaddr *)&client, siz);
					printf("[Server] LookUP Fails\n");
					fflush(stdout);
					break;
				case UNAVAIL:
					DIE(argv[1]);
			} /* end lookup switch */
			siz = sizeof(client);
		} /* end while */
	} /* end forever loop */
} /* end main */
