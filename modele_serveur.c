#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#define PORTS "2058"//replaced by argv[1]

int port; 

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

	/*printf("Server : Serveur globale\n");*/
	/*printf("Server : Numero de port : %s\n", argv[1]);*/
	/*fflush(stdout);*/
	port=atoi(argv[1]);

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

		printf("Server : Waiting for a connexion\n");
		fflush(stdout);

		sin_size = sizeof(their_adr);
		new_fd = accept(sockfd, &their_adr, &sin_size);
		
		/*printf("\nServer : Connection incomming\n");*/
		/*fflush(stdout);*/

		//we need a variable to refuse the conexion in case I'm already in game (means a if statement) 
		//we create a new processus
		if(!fork())
		{
			//i am your father
			//for now we only accept one connection
			//we should use the select methode of the socket
			//we quit the while
			break;
		}
		else
		{
			//we stop receiving from the socket of the main server
			//because the son don't need to access the main socket
			close(sockfd);

			int arguments[2] = {atoi(argv[1]), new_fd};

			/*printf("\nServer : new_fd = \"%d\"\n",new_fd);*/
			/*fflush(stdout);*/

			//we respond to the oponnent and launch the game !!
			gameOn(arguments);

			close(new_fd);
			exit(0);
		} 
	}

	/*printf("\nServer : I do not wait for a connexion anymore\n");*/
	/*fflush(stdout);*/

	exit(0);
}

void gameOn(int args[2])
{

	int numbytes; 
	char buf[100];

	/*printf("\nServer : new_fd = \"%d\"\n",args[1]);*/
	/*fflush(stdout);*/

	/*printf("Server : Creating the reciving process \n");*/
	/*fflush(stdout);*/
	
	/*printf("Server : waiting for info from the oponent\n");*/
	/*fflush(stdout);*/

	//We recv the first message from the oponent
	if((numbytes = recv(args[1], buf, 100-1, 0)) == -1)
	{
		perror("recv");
		exit(1);
	}
	
	/*printf("\nServer : Message recved : %s\n", buf);*/
	/*fflush(stdout);*/

	//the first packet received can be a acknoledgment OR a demande of connexion
	//we create the client ONLY IF it's a demande of connexion
	buf[numbytes] = '\0';	
	/*printf("\nServer : messageReceived : %s\n", buf);*/

	char* token = strtok (buf,",");	
	char* cmd = token;
	token = strtok(NULL, buf);
	char* portOponent = token;
	token = strtok(NULL, buf);

	printf("Server : cmd : %s, port : %s\n", cmd, portOponent);
	fflush(stdout);
	
	if (strcmp(cmd, "demande")==0)
	{
		printf("Server : Received a demande of connection\n");
		/*printf("Server : Creating the client\n");*/
		fflush(stdout);

		char portInChar[6]; 
		sprintf(portInChar, "%d", port);
		//we send a ack
		if (execlp("./client.o", "client.o", portOponent, portInChar, "1", NULL))
                {
                        printf("Server : Execlp didn't work\n");
			fflush(stdout);
                        strerror(errno);
                }
	}
	//else it's an acknoledgement 
	else if (strcmp(cmd, "ack")==0)
	{
		printf("Server : Received an ack of connection\n");
		/*printf("Server : The connexion is established\n");*/
		fflush(stdout);

		//we open a pipe to send commands to the gui
		int descServerToGui;
		char serverToGui[] = "serverToGui.fifo";
		if((descServerToGui = open(serverToGui, O_WRONLY)) == -1) 
		{
			fprintf(stderr, "Impossible d'ouvrir l'entrée du tube nommé.\n");
			perror("open");
			exit(EXIT_FAILURE);
		}

		//with this we can notify the gui that the connection is established 
		/*char chaineAEcrire[7] = "Bonjour";*/
		/*write(descServerToGui, chaineAEcrire, 7);*/

		//now we can receive all the moves of the oponent
		while (1)
		{
			/*printf("\nServer : waiting for a message\n");*/
			/*fflush(stdout);*/

			//we flush the buffer before refill it
			memset(buf, 0, sizeof(buf));
			if((numbytes = recv(args[1], buf, 100-1, 0)) == -1)
			{
				perror("recv");
				exit(1);
			}
			buf[numbytes] = '\0';	

			if (numbytes>0)
			{
				printf("Server : message received : %s, Bytes : %d\n", buf, numbytes);
				fflush(stdout);
				
				//here we'll notify the gui about all the moves of the oponent
				//and a lot more too...
				//we can manipulate all the gui from here 
				/*char chaineAEcrire[7] = "ReBonjo";*/
				/*write(descServerToGui, chaineAEcrire, 7);*/
			}
		}
	}
	else 
	{
		printf("Server : Wrong message : %s \n", buf);
		fflush(stdout);
	}
}
