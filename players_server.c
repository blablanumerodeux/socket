#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <netdb.h>
#include <signal.h>


#define PORTS "2058"

int main(int argc, char **argv)
{
	int sockfd, new_fd, rv, sin_size;
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr their_adr;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; 	// use my IP
	rv = getaddrinfo(NULL, PORTS, &hints, &servinfo);

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
		
		//waiting for a new connection 
		printf("Waiting for a new connection\n");
		fflush(stdout);
		
		new_fd = accept(sockfd, &their_adr, &sin_size);

		if(!fork())
		{
			close(sockfd);

			int numbytes;
			char buf[100];
			if((numbytes = recv(new_fd, buf, 100-1, 0)) == -1)  
			{    
				perror("recv");
				exit(1);
			}
			buf[numbytes] = '\0';

			printf("%s\n",buf);
			fflush(stdout);

			//send him the full stack
			char message[30];
			strcpy(message, "c,ip,port,");
			strcat(message, "login");
			send(new_fd, message, strlen(message), 0);
			
			//and add him to the stack
			char* token = strtok (buf,","); 
			char* entete = token;
			token = strtok(NULL, ",");
			char* ip = token;
			token = strtok(NULL, ",");
			char* port = token;
			token = strtok(NULL, ",");
			char* login = token;
			token = strtok(NULL, ",");

			close(new_fd);
			exit(0);
		} 
	}
	exit(0);
}

