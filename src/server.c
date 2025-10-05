#include "server.h"
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "utils.h"
#include "request.h"

/* Base headers for the 404 page. If the server cannot find a 
 * 404 html page in the server root, only this header will be sent.
 */
static const char *not_found_header = "HTTP/1.1 404 Not found\r\n"
"Server: HyperboreaTTP\r\n";

/* The server only supports the GET method. If another method is requested, the
 * server will show the usual 404 page but will send a 405 code. If the server
 * cannot find a 404 page, only this header will be sent.
 */
//static const char *unsupported_method_header = "HTTP/1.1 405 Unsupported method\r\n"
//"Server: HyperboreaTTP\r\n";

static const char *internal_error_header = "HTTP/1.1 500 Internal error\r\n"
"Server: HyperboreaTTP\r\n";

void server_send_file(const Server *server, int cl_sock, char *rel_path) {
	char *fullpath = malloc(server->server_root_len + strlen(rel_path) + 2);

	if (fullpath == NULL) {
		perror("malloc");
		send(cl_sock, internal_error_header, strlen(internal_error_header), 0);
		return;
	}

	sprintf(fullpath, "%s/%s", server->server_root, rel_path);

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

	buffer_pos = strlen(file_buffer);

	fread(file_buffer + buffer_pos, sizeof(char), buff_len - buffer_pos, file);

	do {
		send(cl_sock, file_buffer, buff_len, 0);

		fread(file_buffer, sizeof(char), buff_len, file);
	} while(!feof(file));

	fclose(file);
}

Server init_server(const char *port, const char *server_root) {
	Server server;

	strcpy(server.server_root, server_root);
	server.server_root_len = strlen(server.server_root);

	char ip_str[INET6_ADDRSTRLEN];
	/* We will ask the OS for a list of addresses we
	 * can use. These are our search criteria.
	 */
	struct addrinfo hints = {0};
	hints.ai_family = AF_UNSPEC;  /* use either ipv4 or ipv6 addresses */
	hints.ai_socktype = SOCK_STREAM; /* use stream sockets (for TCP) */
	hints.ai_flags = AI_PASSIVE; /* Let the OS decide the IP address for us. */

	/* Store the list of addresses the OS will give us here. */
	struct addrinfo *addrs;

	int error_code;
	if ((error_code = getaddrinfo(NULL, port, &hints, &addrs)) != 0) {
		fprintf(stderr, "GAI FAILED!: %s\n", gai_strerror(error_code));
		exit(1);
	}

	struct addrinfo *p;
	for (p = addrs; p != NULL; p = p->ai_next) {
		if ((server.sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			fprintf(stderr, "Failed to create server socket.\nRetrying...");
			continue;
		}

		/* The socket will be bound by the address we got from the OS. */
		if (bind(server.sock, addrs->ai_addr, addrs->ai_addrlen) != 0) {
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
	if (listen(server.sock, 1) != 0) {
		fprintf(stderr, "Failed to listen.\n");
		exit(1);
	}

	addr_to_str(addrs->ai_addr, addrs->ai_addrlen, ip_str);
	printf("Listening on %s:%s\n", ip_str, port);
	freeaddrinfo(addrs);

	return server;
}

void stop_server(Server *server) {
	close(server->sock);
	printf("Server closed gracefully...\n");
}

void server_handle_client_connection(Server *server) {
	char ip_str[INET6_ADDRSTRLEN];

	const size_t max_buff_len = 1024;
	struct sockaddr_storage client_addr_storage;
	/* client_addr_len enters accept as the maximul length our client_addr_storage can
	 * hold, after accept() runs, it overwrites with the actual size of the address.
	 */
	socklen_t client_addr_len = sizeof(struct sockaddr_storage);

	/* Wait until a client opens a connection */
	int client_sock = accept(server->sock, (struct sockaddr*)&client_addr_storage, &client_addr_len);

	addr_to_str((struct sockaddr *)&client_addr_storage, client_addr_len, ip_str);
	printf("------------------------\n");
	printf("Got connection from %s\n", ip_str);

	char buffer[max_buff_len + 1];
	int buff_len = recv(client_sock, buffer, max_buff_len, 0);
	buffer[buff_len] = '\0';
	HTTPRequest *request= parse_request(buffer, buff_len);

	printf("Method: %s\n", (request->method == GET_METHOD) ? "GET" : "NOT GET");
	printf("Path: %s\n", request->path);

	server_send_file(server, client_sock, "index.html");

	free_request(request);
	close(client_sock);
}

