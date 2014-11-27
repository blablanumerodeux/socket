#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>

#define SERVEUR "127.0.0.1"
#define PORTS "2058" //replaced by argv[1]

int port;

int main(int argc, char **argv)
{
	int sockfd, new_fd, rv, sin_size, numbytes;
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr their_adr;
	char buf[100];

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	rv = getaddrinfo(SERVEUR, argv[1], &hints, &servinfo);

	/*printf("Client\n");*/
	/*fflush(stdout);*/
	
	port = atoi(argv[2]);

	if(rv != 0) 
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}


	// Création  socket  et  attachement
	for(p = servinfo; p != NULL; p = p->ai_next) 
	{
		if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
		{
			perror("client: socket");
			continue;
		}
		if((connect(sockfd, p->ai_addr, p->ai_addrlen) == -1))
		{
			close(sockfd);
			perror("client: connect");
			//TODO here we send an command to the interface pipe to tell the connection failed
			continue;
		}

		break;
	}

	if(p == NULL)
	{
		fprintf(stderr, "client: failed to bind\n");
		return 2;
	}

	freeaddrinfo(servinfo); 	// Libère structure
	
	/*if((numbytes = recv(sockfd, buf, 100-1, 0)) == -1)*/
	/*{*/
	/*perror("recv");*/
	/*exit(1);*/
	/*}*/

	/*printf("\nClient : Message reçu : %s\n",buf);*/
	/*printf("Client : Envoie d'un message au serveur\n");*/
	/*fflush(stdout);*/

	char portInChar[6]; 
	sprintf(portInChar, "%d", port);
	
	char message[30];
	strcpy(message, "demande,");
	strcat(message, portInChar);
	
	//if its a demande send this 
	if (strcmp(argv[3], "0")==0)
	{
		send(sockfd, message, strlen(message), 0);
	}
	//else send this 
	else if (strcmp(argv[3], "1")==0)
	{
		send(sockfd, "ack", 3, 0);
	}

	int descGuiToClient;        
	char guiToClient[] = "guiToClient.fifo";

	if((descGuiToClient= open(guiToClient, O_RDONLY)) == -1) 
	{   
		fprintf(stderr, "Impossible d'ouvrir la sortie du tube nommé.\n");
		exit(EXIT_FAILURE);
	}   

	//we now obey to the pipe messages
	char chaineALire[7];
	int nbBRead;
	while(1)
	{
		if((nbBRead = read(descGuiToClient, chaineALire, 7-1)) == -1)
		{
			perror("read error : ");
			exit(EXIT_FAILURE);
		}else if(nbBRead > 0)
		{
			chaineALire[nbBRead] = '\0'; 
			printf("Client : cmd recved from pipe : %s : %d Bytes\n", chaineALire, (int) strlen(chaineALire));
			fflush(stdout);

			send(sockfd, "des coordonees", 14, 0);
		}

	}

	close(sockfd);
	exit(0);
}
