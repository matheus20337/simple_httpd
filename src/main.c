#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>

/* Poor man's httpd */

const char *port = "8080"; /* The functions expect the port to be a string. */

/* Base headers for the 404 page. If the server cannot find a 
 * 404 html page in the server root, only this header will be sent.
 */
const char *not_found_header = "HTTP/1.1 404 Not found\r\n"
"Server: HyperboreaTTP\r\n";

/* The server only supports the GET method. If another method is requested, the
 * server will show the usual 404 page but will send a 405 code. If the server
 * cannot find a 404 page, only this header will be sent.
 */
const char *unsupported_method_header = "HTTP/1.1 405 Unsupported method\r\n"
"Server: HyperboreaTTP\r\n";

const char *internal_error_header = "HTTP/1.1 500 Internal error\r\n"
"Server: HyperboreaTTP\r\n";

void addr_to_str(struct sockaddr *addr, socklen_t addr_len, char *addr_str);
int init_server(void);

/* Sends a static file to the client.
 * Will send an error page if the file cannot
 * be found in the server root.*/
void send_file(int cl_sock, char *server_root, char *rel_path) {
	char *fullpath = malloc(strlen(server_root) + strlen(rel_path) + 2);

	if (fullpath == NULL) {
		perror("malloc");
		send(cl_sock, internal_error_header, strlen(internal_error_header), 0);
		return;
	}

	sprintf(fullpath, "%s/%s", server_root, rel_path);

	FILE *file = fopen(fullpath, "r");
	free(fullpath);

	if (file == NULL) {
		perror("fopen");
		send(cl_sock, not_found_header, strlen(not_found_header), 0);
		return;
	}

	const size_t buff_len = 1024;
	char file_buffer[buff_len];
	size_t buffer_pos = 0;

	/* We need the size of the file. */
	fseek(file, 0, SEEK_END);
	long int file_size = ftell(file);
	rewind(file);

	sprintf(file_buffer, "HTTP/1.1 200 OK\r\n"
		"Server: HyperboreaTTP\r\n"
		"Content-Type: text/html\r\n"
		"Content-Length: %ld\r\n\r\n", file_size);

	for (buffer_pos = strlen(file_buffer); buffer_pos - 1 < buff_len && !feof(file); buffer_pos++) {
		file_buffer[buffer_pos] = fgetc(file);
	}
	file_buffer[buffer_pos++] = '\0';

	do {
		send(cl_sock, file_buffer, buff_len, 0);

		for (buffer_pos = 0; buffer_pos < buff_len - 1 && !feof(file); buffer_pos++) {
			file_buffer[buffer_pos] = fgetc(file);
		}
		file_buffer[buffer_pos++] = '\0';
	} while(!feof(file));
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		fprintf(stderr, "Usage: %s <root_directory>\n", argv[0]);
		exit(1);
	}

	char ip_str[INET6_ADDRSTRLEN];

	int sock = init_server();

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

		/* Wait until a client opens a connection */
		int client_sock = accept(sock, (struct sockaddr*)&client_addr_storage, &client_addr_len);

		addr_to_str((struct sockaddr *)&client_addr_storage, client_addr_len, ip_str);
		printf("------------------------\n");
		printf("Got connection from %s\n", ip_str);

		char buffer[max_buff_len + 1];
		int buff_len = recv(client_sock, buffer, max_buff_len, 0);
		buffer[buff_len] = '\0';
		printf("Got data:\n%s\n", buffer);

		send_file(client_sock, argv[1], "index.html");

		close(client_sock);
	} while(1);

	/* TODO: The code never reaches this place. I need to find a way to gracefully shutdown the server.
	 */

	close(sock);
	printf("Server closed gracefully...\n");

	return 0;
}

/* Converts a sockaddr into a presentation ip string. */
void addr_to_str(struct sockaddr *addr, socklen_t addr_len, char *addr_str) {
	void *raw_addr;

	if (addr->sa_family == AF_INET) {
		/* This is my greatest act of pointer magic to this date. */
		raw_addr = &((struct sockaddr_in *)addr)->sin_addr;
	} else {
		raw_addr = &((struct sockaddr_in6 *)addr)->sin6_addr;
	}

	inet_ntop(addr->sa_family, raw_addr, addr_str, addr_len);
}

/* Asks the OS for an address, creates a socket,
 * bind it and listen. Returns the socket.
 */
int init_server(void) {
	char ip_str[INET6_ADDRSTRLEN];
	/* We will ask the OS for a list of addresses we
	 * can use. These are our search criteria.
	 */
	struct addrinfo hints = {0};
	hints.ai_family = AF_UNSPEC;  /* use either ipv4 or ipv6 addresses */
	hints.ai_socktype = SOCK_STREAM; /* use stream sockets (for TCP) */
	hints.ai_flags = AI_PASSIVE; /* Let the OS decide the IP address for us. */

	int sock;


	/* Store the list of addresses the OS will give us here. */
	struct addrinfo *addrs;

	int error_code;
	if ((error_code = getaddrinfo(NULL, port, &hints, &addrs)) != 0) {
		fprintf(stderr, "GAI FAILED!: %s\n", gai_strerror(error_code));
		exit(1);
	}

	struct addrinfo *p;
	for (p = addrs; p != NULL; p = p->ai_next) {
		if ((sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			fprintf(stderr, "Failed to create server socket.\nRetrying...");
			continue;
		}

		/* The socket will be bound by the address we got from the OS. */
		if (bind(sock, addrs->ai_addr, addrs->ai_addrlen) != 0) {
			fprintf(stderr, "Failed to bind socket.\n");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "Failed to bind to address.\n");
		exit(1);
	}

	/* Mark the socket as accepting connections. */
	if (listen(sock, 1) != 0) {
		fprintf(stderr, "Failed to listen.\n");
		exit(1);
	}

	addr_to_str(addrs->ai_addr, addrs->ai_addrlen, ip_str);
	printf("Listening on %s:%s\n", ip_str, port);
	freeaddrinfo(addrs);

	return sock;
}
