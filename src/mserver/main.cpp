#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <strings.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#include <vector>
#include <algorithm>
#include "network_protocol.h"

struct thread_args {
	int client_sd;
	sockaddr_in client_addr;
};

std::vector<thread_args*> clients;
pthread_mutex_t clients_mutex;

void *handle_client(void *carg) {
	thread_args *args = (thread_args*)carg;
	printf("client connection %d\n", args->client_sd);

	packet hello_packet;
	hello_packet.cmd = HELLO;
	hello_packet.length = 0;
	hello_packet.oid = 0;
	hello_packet.cid = args->client_sd;

	if(send(args->client_sd, (void*)&hello_packet, sizeof(hello_packet), 0) == -1) {
		printf("couldnt hello msg to client\n");
	} else {
		printf("sent hello packet to %d\n", args->client_sd);
	}

	packet in_packet;
	while(true) {
		if(recv(args->client_sd, &in_packet, sizeof(in_packet), 0) == 0) {
			printf("client has exited %d\n", args->client_sd);
			goto close_connection;
		}

		switch(in_packet.cmd) {
			//TODO send all other users positions for hello
			case HELLO:
				if(in_packet.cid == args->client_sd) {
					printf("got back hello packet from %d confirming they are %d\n", in_packet.cid, args->client_sd);
				} else {
					printf("got back hello packet, but they did not reply right\n");
					goto close_connection;
				}
				break;
			case NEW_PPOS:
			case UPD_PPOS:
				printf("got new position packet %d %d %f %f %f\n", args->client_sd, 
									in_packet.oid, in_packet.x, in_packet.y, in_packet.z);
				pthread_mutex_lock(&clients_mutex);
				for(std::vector<thread_args*>::iterator it = clients.begin(); it != clients.end(); ++it) {
					if((*it)->client_sd != args->client_sd) {
						if(send((*it)->client_sd, &in_packet, sizeof(in_packet), 0) > 0) {
							printf("sent new packet to %d\n", (*it)->client_sd);
						}
					}
				}
				pthread_mutex_unlock(&clients_mutex);
				break;
			default:
				printf("unknown packet with %d size from %d\n", in_packet.length, args->client_sd);
				break;
		}
	}

	close_connection:
	printf("cleaning up connection for %d\n", args->client_sd);
	pthread_mutex_lock(&clients_mutex);
	clients.erase(std::remove(clients.begin(), clients.end(), args), clients.end());
	pthread_mutex_unlock(&clients_mutex);
	close(args->client_sd);
	delete args;
	//TODO also errase thread entry prob.
	pthread_exit(NULL);
}

int main(int argc, char **argv) {
	int sockfd;
	struct sockaddr_in servaddr;
	std::vector<pthread_t> threads;

	pthread_mutex_init(&clients_mutex, NULL);

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		printf("could not make socket fd\n");
		exit(1);
	}

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(32000);

	bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
	listen(sockfd, 1024);

	while(true) {
		struct sockaddr_in client_addr;
		socklen_t client_addr_length;
		int client = accept(sockfd, (struct sockaddr*)&client_addr, &client_addr_length);;

		thread_args *arg = new thread_args();
		arg->client_sd = client;
		arg->client_addr = client_addr;

		pthread_t thread;
		int rc = pthread_create(&thread, NULL, handle_client, (void*)arg);
		threads.push_back(thread);

		pthread_mutex_lock(&clients_mutex);
		clients.push_back(arg);
		pthread_mutex_unlock(&clients_mutex);

		if(rc) {
			printf("could not make thread\n");
			exit(1);
		}
	}
	pthread_mutex_destroy(&clients_mutex);
	pthread_exit(NULL);
}
