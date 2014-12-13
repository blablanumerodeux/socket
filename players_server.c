#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <netdb.h>
#include <signal.h>
#include <pthread.h>


#define PORTS "2058"

void* client_thread(void* args);

struct player 
{
	char  ip[50];
	char port[50];
	char  login[50];
	int   status;
};    
struct player players_list[10];
int next_player_number =0;

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

		pthread_t thread_client;
		int desc_thread_client= pthread_create(&thread_client, NULL, client_thread, &new_fd);
	}
	exit(0);
}

void * client_thread(void* args)
{

	/*int* new_fd_pointer = args;*/
	/*int new_fd = *new_fd_pointer;*/
	int new_fd = *(int*)args;

	int numbytes;
	char buf[100];
	if((numbytes = recv(new_fd, buf, 100-1, 0)) == -1)  
	{    
		perror("recv");
		exit(1);
	}
	buf[numbytes] = '\0';

	printf("new player : %s\n",buf);
	fflush(stdout);

	//send him the full stack
	char message[100];
	strcpy(message, "c,");
	int i = 0;
	for (i=0;i<next_player_number;i++){
		strcat(message, players_list[i].ip);
		strcat(message, ",");
		strcat(message, players_list[i].port);
		strcat(message, ",");
		strcat(message, players_list[i].login);
		strcat(message, ",");
	}
	strcat(message, "c");
	if (strlen(message)>4)
	{
		printf("sent message : %s\n",message);
		fflush(stdout);
		send(new_fd, message, strlen(message), 0);
	}

	//and add him to the stack
	char* token = strtok (buf,","); 
	token = strtok(NULL, ",");
	char* entete = token;
	char* ip = token;
	token = strtok(NULL, ",");
	char* port = token;
	token = strtok(NULL, ",");
	char* login = token;
	token = strtok(NULL, ",");

	if (ip==NULL)
	{
		/*strcpy(ip, "");*/
		ip = (char*)malloc(sizeof(char*));
	}
	if (port==NULL)
	{
		/*strcpy(port, "");*/
		port = (char*)malloc(sizeof(char*));
	}
	if (login==NULL)
	{
		/*strcpy(login, "");*/
		login = (char*)malloc(sizeof(char*));
	}
	/*printf("ip : %s\n",ip);*/
	/*fflush(stdout);*/
	strcpy(players_list[next_player_number].ip, ip);
	/*printf("port : %s\n",port);*/
	/*fflush(stdout);*/
	strcpy(players_list[next_player_number].port, port);
	/*printf("login : %s\n",login);*/
	/*fflush(stdout);*/
	strcpy(players_list[next_player_number].login, login);
	players_list[next_player_number].status = 1;

	next_player_number++;
}
	/*printf("ip : %s\n",ip);*/
	/*printf("ip : %s\n",ip);*/
	/*fflush(stdout);*/
	/*fflush(stdout);*/
