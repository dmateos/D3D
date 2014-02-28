#include "network.h"
#include "mserver/network_protocol.h"

//
// Connection
//
Connection::Connection(std::string server) {
	int sockfd;
	struct sockaddr_in servaddr;

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		printf("cant make socket connection");
	}

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(server.c_str());
	servaddr.sin_port = htons(32000);

	if(connect(sockfd, (const sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
		printf("could not connect to remote server %s\n", server.c_str());
	}
	else {
		int flags = fcntl(sockfd,F_GETFL,0);
		fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

		this->sock_fd = sockfd;
	}
}

void Connection::get_message() {
	packet in_packet;
	int err;

	int len = recv(this->sock_fd, (void*)&in_packet, sizeof(in_packet), 0);
	err = errno;

	if(len < 0) {
		//TODO handle these.
		if (err == EAGAIN || err == EWOULDBLOCK) {
	//		printf("didnt get all the data!\n");
		} else {
	//		printf("socket is fucked\n");
		}
	} else {
		switch(in_packet.cmd) {
			case HELLO:
				printf("new hello packet from %d\n", in_packet.oid);
				packet hello_packet;
				hello_packet.cmd = HELLO;
				hello_packet.length = 0;
				hello_packet.oid = 69;
				hello_packet.cid = in_packet.cid;

				if(send(this->sock_fd, (void*)&hello_packet, sizeof(hello_packet), 0) == sizeof(hello_packet)) {
					printf("sent hello packet confirming i am %d\n", hello_packet.cid);
				}
				this->cid = in_packet.cid;
				break;
			case NEW_PPOS:
				printf("new position packet %d %d %f %f %f\n",
						in_packet.cid, in_packet.oid, in_packet.x, in_packet.y, in_packet.z);
				new_coords(in_packet.oid, in_packet.x, in_packet.y, in_packet.z); //TODO check if null
				break;
			case UPD_PPOS:
				break;
			default:
				break;
		}
	}
}

void Connection::send_coord_update(int oid, float x, float y, float z) {
	packet out_packet;
	out_packet.oid = oid;
	out_packet.cmd = NEW_PPOS;
	out_packet.length = 0;
	out_packet.cid = this->cid;
	out_packet.x = x;
	out_packet.y = y;
	out_packet.z = z;

	if(send(this->sock_fd, (void*)&out_packet, sizeof(out_packet), 0) == sizeof(out_packet)) {
		printf("sent coord update packet\n");
	}
}
