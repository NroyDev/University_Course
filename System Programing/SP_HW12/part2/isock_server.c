/*
 * isock_server : listen on an internet socket ; fork ;
 *                child does lookup ; replies down same socket
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
	static struct sockaddr_in server;
	int sd,cd,n;
	Dictrec tryit;

	if (argc != 2) {
		fprintf(stderr,"Usage : %s <datafile>\n",argv[0]);
		exit(1);
	}

	/* Create the socket.
	 * Fill in code. */
	if((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
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
	if(bind(sd, (struct sockaddr*)&server, sizeof(server))){
		perror("bind");
		exit(1);
	}
	if(listen(sd, 128)){
		perror("listen");
		exit(1);
	}

	printf("[Server] Start Accep Client Connection\n");
	fflush(stdout);
	/* main loop : accept connection; fork a child to have dialogue */
	for (;;) {
		/* Wait for a connection.
		 * Fill in code. */
		static struct sockaddr_in client;
		socklen_t clientlen = sizeof(client);
		if((cd = accept(sd, (struct sockaddr *)&client, &clientlen)) == -1){
			perror("accept");;
			exit(1);
		}
		printf("[Server] New Client\n");
		fflush(stdout);

		/* Handle new client in a subprocess. */
		switch (fork()) {
			case -1 : 
				DIE("fork");
			case 0 :
				close (sd);	/* Rendezvous socket is for parent only. */
				/* Get next request.
				 * Fill in code. */
				while (read(cd, &tryit, sizeof(Dictrec))>0) {
					printf("[Server] Recv request %s \n", tryit.word);
					fflush(stdout);
					/* Lookup the word , handling the different cases appropriately */
					switch(lookup(&tryit,argv[1]) ) {
						/* Write response back to the client. */
						case FOUND:
							/* Fill in code. */
							write(cd, &tryit, sizeof(Dictrec));
							printf("[Server] LookUP success\n");
							fflush(stdout);
							break;
						case NOTFOUND:
							/* Fill in code. */
							strcpy(tryit.text, "XXXX");
							write(cd, &tryit, sizeof(Dictrec));
							printf("[Server] LookUP Fails\n");
							fflush(stdout);
							 break;
						case UNAVAIL:
							DIE(argv[1]);
					} /* end lookup switch */
				} /* end of client dialog */
				exit(0); /* child does not want to be a parent */

			default :
				close(cd);
				break;
		} /* end fork switch */
	} /* end forever loop */
} /* end main */
