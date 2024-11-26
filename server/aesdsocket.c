#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <netdb.h>
#include <string.h>
#include <syslog.h>
#include <fcntl.h>

#define PORT "9000"
#define BACKLOG 10
#define BUFFER_SIZE 1024
#define WRITE_PATH "/var/tmp/aesdsocketdata"

//Global variables
int socketfd = -1;
int clientfd = -1;
int filefd = -1;

void daemonize()
{
	pid_t pid;
	pid = fork();
	if(pid < 0){
		perror("Error: fork failed");
		syslog(LOG_DEBUG, "Error: fork failed");
		exit(EXIT_FAILURE);
	}

	if(pid > 0){
		//Parent exits
		exit(EXIT_SUCCESS);
	}
	
	if(setsid() < 0){
		perror("Error: setsid failed");
		syslog(LOG_DEBUG, "Error: setsid failed");
		exit(EXIT_FAILURE);
	}

	//Redirect standard I/O to dev/null
	int fd = open("/dev/null", O_RDWR);
	if (fd < 0){
		perror("Error: failed to open /dev/null");
		syslog(LOG_DEBUG, "Error: failed to open /dev/null");
		exit(EXIT_FAILURE);
	}

	dup2(fd, STDIN_FILENO);
	dup2(fd, STDOUT_FILENO);
	dup2(fd, STDERR_FILENO);
	close(fd);

	// Set file permissions and working directory
	umask(0);

	// Change the daemon working dir to root 
	if(chdir("/") < 0){
		perror("chdir failed");
		exit(EXIT_FAILURE);
	}
}
		
		
/*
*/
void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

/*
*/
static void signal_handler(int sig_num)
{
	if(sig_num == SIGINT || sig_num == SIGTERM){
		syslog(LOG_DEBUG, "Caugt signal, exiting...");
		if (socketfd != -1){
			close(socketfd);
			printf("Server socket closed\n");
			syslog(LOG_DEBUG, "Server socket closed");
		}

	
		if (clientfd != -1){
			close(socketfd);
			printf("Client socket closed\n");
			syslog(LOG_DEBUG, "Client socket closed");
		}

		remove(WRITE_PATH);
	}
	exit(EXIT_SUCCESS);
}
	

int main(int argc, char *argv[])
{
	int run_as_daemon = 0;
	//Parse arguments
	for (int i = 0; i < argc; i++){
		if (strcmp(argv[i], "-d") == 0){
			run_as_daemon = 1;
			break;
		}
	}

	int rc;
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage client_addr;
	socklen_t addr_size;
	char s[INET_ADDRSTRLEN];
	
	struct sigaction sa;
	memset(&sa, 0, sizeof(struct sigaction));
	sa.sa_handler = signal_handler;
	if((sigaction(SIGINT, &sa, NULL) != 0) || (sigaction(SIGTERM, &sa, NULL) != 0)){
		fprintf(stderr, "Error %d (%s) registering for SIGINT or SIGTEMR", errno, strerror(errno));
		return 0;
	}

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	
	rc = getaddrinfo(NULL, PORT, &hints, &servinfo);
	if (rc != 0){
		fprintf(stderr, "Error: getaddrinfo(): %s\n", gai_strerror(rc));
		syslog(LOG_CRIT, "Error: getaddrinfo(): %s\n", gai_strerror(rc));
		return -1;
	}

	//loop through all results returned from getaddrinfo and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next){
		socketfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (socketfd == -1)
			continue;
		else
			break;
	}

	//Check if cannot create socket file descriptor
	if (socketfd == -1){
		perror("Cannot create socket file descriptor\n");
		syslog(LOG_CRIT, "Cannot create socket file descriptor\n");
		return -1;
	}

	//Bind
	rc = bind(socketfd, servinfo->ai_addr, servinfo->ai_addrlen );
	if (rc == -1){
		fprintf(stderr, "Error: bind %s\n", gai_strerror(rc));
		syslog(LOG_CRIT, "Error: bind %s\n", gai_strerror(rc));
		close(socketfd);
	}
	
	//free servinfo to prevent memory leak	
	freeaddrinfo(servinfo);

	/**Check if run as daemon is enabled, if it is then fork() to create child process**/
	if (run_as_daemon){
		syslog(LOG_DEBUG, "Running as daemon");
		daemonize();
	}

	//Listen
	rc = listen(socketfd, BACKLOG);
	if (rc == -1){
		perror("Error: listen\n");
		return -1;
	}
	printf("Server is listening on port %s...\n", PORT);
	syslog(LOG_CRIT, "Server is listening on port %s...\n", PORT);
	
	//Open a file to write incoming stream input and to read it back.
	filefd = open(WRITE_PATH, O_CREAT | O_RDWR, 0644);

	/*********Main loop*******/
	/*Wait to accept to establish a connection, then
	 * receive data from
	*/

	while(1){
		clientfd = accept(socketfd, (struct sockaddr *)&client_addr, &addr_size);
		if (clientfd == -1){
			perror("Error: accept\n");
			syslog(LOG_CRIT, "Error: accept\n");
			continue;
		}
	
		inet_ntop(client_addr.ss_family, get_in_addr((struct sockaddr *)&client_addr), s, sizeof(s));
		printf("Accepted connection from %s\n", s);
		syslog(LOG_CRIT, "Accepted connection from %s\n", s);

		//Communicate with the client
		char recv_buffer[BUFFER_SIZE];
		ssize_t bytes_received;
		char *send_buffer;
		char *start = recv_buffer;
		char *newline;	
		off_t file_size;
		
		while((bytes_received = recv(clientfd, recv_buffer, sizeof(recv_buffer) - 1, 0)) > 0){
			//Check for newline to detect a complete packet
			//for(size_t i = 0; i < bytes_received; i++){
			while((newline = strchr(start, '\n') ) != NULL){
				ssize_t pktlen = newline - start + 1;
				write(filefd, start, pktlen);
			
				file_size = lseek(filefd, 0, SEEK_END);
				lseek(filefd, 0, SEEK_SET);
				send_buffer = malloc(file_size);
					
			
				if(read(filefd, send_buffer, file_size) != file_size){
					perror("Error reading file");
					break;
				}

				if(send(clientfd, send_buffer, file_size, 0) == -1){
					perror("Error sending to client");
					break;
				}
				free(send_buffer);
				start = newline + 1;
			}

		}


		
	}
	close(filefd);
	close(socketfd);
	close(clientfd);
	return 0;
}
