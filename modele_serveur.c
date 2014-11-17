
/******* MODELE SERVEUR TCP  **************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <netdb.h>
#include <signal.h>
#include <unistd.h>

#define PORTS "2058"//replaced by argv[1]

void gameOn(int args[2]);

int main(int argc, char **argv)
{
	int sockfd, new_fd, rv, sin_size, numbytes; 
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr their_adr;
	char buf[100];

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; 	// use my IP
	printf("\nNumero de port : %s\n", argv[1]);
	fflush(stdout);
	rv = getaddrinfo(NULL, argv[1], &hints, &servinfo);

	if (rv != 0) 
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// Création  socket  et  attachement
	for(p = servinfo; p != NULL; p = p->ai_next) 
	{
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
		{
			perror("server: socket");
			continue;
		}
		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
		{
			close(sockfd);
			perror("server: bind");
			continue;
		}
		break;

	}

	if (p == NULL)
	{
		fprintf(stderr, "server: failed to bind\n");
		return 2;
	}

	freeaddrinfo(servinfo); 	// Libère structure

	listen(sockfd, 5);
	signal(SIGCHLD, SIG_IGN);

	while(1)
	{
		sin_size = sizeof(their_adr);
		printf("waiting for a connexion");
		fflush(stdout);
		new_fd = accept(sockfd, &their_adr, &sin_size);
		
		//we create a new processus
		if(!fork())
		{
			//i am your father
		}
		else
		{
			//we stop receiving from the socket of the main server
			close(sockfd);

			int arguments[2] = {atoi(argv[1]), new_fd};
			gameOn(arguments);

			close(new_fd);
			exit(0);
		} 
	}

	exit(0);
}

void gameOn(int args[2])
{


	//a process for the game server (the little one)
	//this processe will only receive infos from the other client
	//this processus will loop infinitely till he recv a shutdown cmd or the client quit the game
	if(!fork())
	{
		//i am your father
		printf("creating the reciving process");
		/*if((numbytes = recv(sockfd, buf, 100-1, 0)) == -1)*/
		/*{*/
		/*perror("recv");*/
		/*exit(1);*/
		/*}*/
		
	}
	//and another to send request to the oponent server via the sockets info send on the first request
	else
	{
		//here we can use the execlp with modele_client
		//we create another socket for sending info to the client
		/*send(new_fd, "Connexion established !", 23, 0);*/
		/*printf("waiting for an answer");*/
		printf("creating the client");
		fflush(stdout);
		
		//we override the processe 
		execlp("./client.o", ""+args[0], ""+args[1], NULL);//TODO change the args[0]
	} 



}
