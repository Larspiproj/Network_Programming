/*
** listener.c -- a datagram socket "server" demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
// #include <sys/wait.h>
// #include <signal.h>

#define MYPORT "4950"

#define MAXBUFLEN 100

// #define BACKLOG 10		// How many pending connections queue will hold

// void sigchld_handler(int s) {
	// waitpid() might overwrite errno, so we save and restore it
//	int saved_errno = errno;

//	while(waitpid(-1, NULL, WNOHANG) > 0);

//	errno = saved_errno;
// }

// get sockaddr, IPv4, IPv6
void *get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void) {
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;
	struct sockaddr_storage their_addr;		// connector's address information
	char buf[MAXBUFLEN];
	socklen_t addr_len;
	// struct sigaction sa;
	// int yes=1;
	char s[INET6_ADDRSTRLEN];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;	// use my IP

	if ((rv = getaddrinfo(NULL, MYPORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, \
				p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		// if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, \
		// 		sizeof(int)) == -1) {
		// 	perror("setsockopt");
		// 	exit(1);
		// }

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "server: failed to bind\n");
		return 2;
	}

	freeaddrinfo(servinfo);	// all done with this structure

	// if (listen(sockfd, BACKLOG) == -1) {
	// 	perror("listen");
	// 	exit(1);
	//}

	//sa.sa_handler = sigchld_handler;	// reap all dead processes
	//sigemptyset(&sa.sa_mask);
	//sa.sa_flags = SA_RESTART;
	//if (sigaction(SIGCHLD, &sa, NULL) == -1) {
	//	perror("sigaction");
	//	exit(1);
	//}

	printf("server: waiting for connections....\n");

	addr_len = sizeof their_addr;
	if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1, 0, \
			(struct sockaddr *)&their_addr, &addr_len)) == -1) {
		perror("recvfrom");
		exit(1);
	}

	printf("listener: got packet from %s\n", \
			inet_ntop(their_addr.ss_family,
					get_in_addr((struct sockaddr *)&their_addr), \
					s, sizeof s));
	printf("listener: packet is %d bytes long\n", numbytes);
	buf[numbytes] = '\0';
	printf("listener: packet contains \"%s\"\n", buf);

	close(sockfd);

	return 0;
}
