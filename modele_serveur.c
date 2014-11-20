
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
	printf("\nServeur globale\n");
	printf("\nNumero de port : %s\n", argv[1]);
	fflush(stdout);
	rv = getaddrinfo(NULL, argv[1], &hints, &servinfo);

	if (rv != 0) 
	{
		fprintf(stderr, "\ngetaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// Création  socket  et  attachement
	for(p = servinfo; p != NULL; p = p->ai_next) 
	{
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
		{
			perror("\nserver: socket\n");
			continue;
		}
		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
		{
			close(sockfd);
			perror("\nserver: bind\n");
			continue;
		}
		break;

	}

	if (p == NULL)
	{
		fprintf(stderr, "\nserver: failed to bind\n");
		return 2;
	}

	freeaddrinfo(servinfo); 	// Libère structure

	listen(sockfd, 5);
	signal(SIGCHLD, SIG_IGN);

	while(1)
	{
		sin_size = sizeof(their_adr);
		printf("\nwaiting for a connexion\n");
		fflush(stdout);
		new_fd = accept(sockfd, &their_adr, &sin_size);

		//we need a variable to refuse the conexion in case I'm already in game (means a if statement) 
		//we create a new processus
		if(!fork())
		{
			//i am your father
			//for now we only accept one connection
			break;
		}
		else
		{
			//we stop receiving from the socket of the main server
			close(sockfd);

			int arguments[2] = {atoi(argv[1]), new_fd};

			//instead of this we create the client and we send a cmd in the pipe to connect to the oponnent
			gameOn(arguments);

			close(new_fd);
			exit(0);
		} 
	}

	printf("\nI do not wait for a connexion anymore\n");
	fflush(stdout);

	exit(0);
}

//this can be triggered from a pipe
void gameOn(int args[2])
{


	//a process for the game server (the little one)
	//this processe will only receive infos from the other client
	//this processus will loop infinitely till he recv a shutdown cmd or the client quit the game
	if(!fork())
	{
		//i am your father


		if((numbytes = recv(sockfd, buf, 100-1, 0)) == -1)
		{
			perror("recv");
			exit(1);
		}
		//the first packet received can be a acknoledgment OR a demand of connexion
		//we create the client ONLY IF it's a demande of connexion

		if (buf == "demande")
		{
			execlp("./client.o", ""+args[0], ""+args[1], NULL);//TODO change the args[0]
			printf("\ncreating the reciving process\n");
		}

	}

	//TODO put this in a the IF statement in the father processe !!!!!

	//and another to send request to the oponent server via the sockets info send on the first request
	else
	{
		//here we can use the execlp with modele_client
		//we create another socket for sending info to the client
		/*send(new_fd, "Connexion established !", 23, 0);*/
		/*printf("waiting for an answer");*/
		printf("\ncreating the client\n");
		fflush(stdout);

		//we override the processe 
	} 



}
