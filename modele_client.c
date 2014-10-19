
/*********** MODELE CLIENT TCP  **************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <netdb.h>


#define SERVEUR "127.0.0.1"
#define PORTS "2058" //replaced by argv[1]

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
      continue;
    }
    
    break;
  }

  if(p == NULL)
  {
    fprintf(stderr, "server: failed to bind\n");
    return 2;
  }

  freeaddrinfo(servinfo); 	// Libère structure

  if((numbytes = recv(sockfd, buf, 100-1, 0)) == -1)
  {
    perror("recv");
    exit(1);
  }

  printf("Message reçu : %s\n",buf);

  close(sockfd);

  exit(0);
}
