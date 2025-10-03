#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>

/* Poor man's httpd */

const char *host = "0.0.0.0";
const char *port = "8080"; /* The functions expect the port to be a string. */

const char *sample_msg = "HTTP/1.1 404 Not found\r\n"
"Server: HyperboreaTTP\r\n"
"Content-Type: text/html\r\n"
"Content-Length: 29\r\n"
"\r\n"
"<h1>Hello, networking!</h1>\r\n";

void addr_to_str(struct sockaddr_storage *addr, socklen_t addr_len, char *addr_str);

int main() {
	/* We will ask the OS for a list of addresses we
	 * can use. These are our search criteria.
	 */
	struct addrinfo hints = {0};
	hints.ai_family = AF_UNSPEC;  /* use either ipv4 or ipv6 addresses */
	hints.ai_socktype = SOCK_STREAM; /* use stream sockets (for TCP) */
	hints.ai_protocol = 0; /* Infer the protocol from the socktype (it will be TCP) */

	/* Store the list of addresses the OS will give us here. */
	struct addrinfo *addrs;

	int error_code;
	if ((error_code = getaddrinfo(host, port, &hints, &addrs)) != 0) {
		fprintf(stderr, "GAI FAILED!: %s\n", gai_strerror(error_code));
		exit(1);
	}

	/* The book I'm reading (Beej's Guide to Networking Programming) warns me that 
	 * I should not trust the first result and that I should verirfy if the address
	 * is usable. I don't know what makes an address unfit for usage, so that
	 * verification will come at a later time.
	 * TODO: ^
	 */

	/* Let there be a socket */
	int sock = socket(addrs->ai_family, addrs->ai_socktype, addrs->ai_protocol);
	if (sock == -1) {
		fprintf(stderr, "Failed to create server socket.\n");
		exit(1);
	}

	/* The socket will be bound by the address we got from the OS. */
	if (bind(sock, addrs->ai_addr, addrs->ai_addrlen) != 0) {
		fprintf(stderr, "Failed to bind socket.\n");
		exit(1);
	}

	/* We no longer need this. */
	freeaddrinfo(addrs);

	/* Mark the socket as accepting connections. */
	if (listen(sock, 1) != 0) {
		fprintf(stderr, "Failed to listen.\n");
		exit(1);
	}

	/* TODO: This loop will freeze execution until a client connects and, thus, sets a
	 * limit of one concurrent user for the server. I need to fix this.
	 */
	do {
		const size_t max_buff_len = 1024;
		struct sockaddr_storage client_addr_storage;
		/* client_addr_len enters accept as the maximul length our client_addr_storage can
		 * hold, after accept() runs, it overwrites with the actual size of the address.
		 */
		socklen_t client_addr_len = sizeof(struct sockaddr_storage);

		char ip_str[INET6_ADDRSTRLEN];

		/* Wait until a client opens a connection */
		int client_sock = accept(sock, (struct sockaddr*)&client_addr_storage, &client_addr_len);

		addr_to_str(&client_addr_storage, client_addr_len, ip_str);
		printf("------------------------\n");
		printf("Got connection from %s\n", ip_str);

		/* For now, we'll discard the request. TODO: Read and interpret the request. */
		char buffer[max_buff_len + 1];
		int buff_len = recv(client_sock, buffer, max_buff_len, 0);
		buffer[buff_len] = '\0';
		printf("Got data:\n%s\n", buffer);

		send(client_sock, sample_msg, strlen(sample_msg) * sizeof(char), 0);

		close(client_sock);
	} while(1);

	close(sock);

	
	
	return 0;
}

/* Converts a sockaddr into a presentation ip string. */
void addr_to_str(struct sockaddr_storage *addr, socklen_t addr_len, char *addr_str) {
	void *raw_addr;

	if (addr->ss_family == AF_INET) {
		/* This is my greatest act of pointer magic to this date. */
		raw_addr = &((struct sockaddr_in *)addr)->sin_addr;
	} else {
		raw_addr = &((struct sockaddr_in6 *)addr)->sin6_addr;
	}

	inet_ntop(addr->ss_family, raw_addr, addr_str, addr_len);
}
