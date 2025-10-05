#ifndef SERVER_H
#define SERVER_H

#include <stddef.h>
#include <stdbool.h>

/* HTTP Status codes supported by the server. */
typedef enum StatusCode {
	STATUS_OK = 200,
	STATUS_NOT_FOUND = 404,
	STATUS_UNSUPORTED_METHOD = 405,
	STATUS_SERVER_ERROR = 500
} StatusCode;

typedef struct Server {
	int sock;
	char server_root[512];
	size_t server_root_len; /* There is no reason to call strlen more than once on the server root */
} Server;

/* Asks the OS for an address, creates a socket,
 * bind it and listen. Returns the socket.
 */
Server init_server(const char *port, const char *server_root);

/* Properly destroys a server */
void stop_server(Server *server);

/* Sends a static file to the client.
 * returns if the file has been sent.
 */
bool server_send_file(int cl_sock, char *path);

/* accept()s a client connection and handles the request. */
void server_handle_client_connection(Server *server);

#endif /* SERVER_H */
