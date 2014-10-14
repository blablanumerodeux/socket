
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
    new_fd = accept(sockfd, &their_adr, &sin_size);
    
    if(!fork())
    {
      close(sockfd);
      send(new_fd, "Hello!", 6, 0);
      close(new_fd);

      exit(0);
    } 
  }
  
  exit(0);
}
        