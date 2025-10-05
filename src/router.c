#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <errno.h>

#include "server.h"
/* TODO: The router allows the user to fetch files outside of the server root. Stop that. */

/* Base headers for the 404 page. If the server cannot find a 
 * 404 html page in the server root, only this header will be sent.
 */
static const char *not_found_header = "HTTP/1.1 404 Not found\r\n"
"Server: HyperboreaTTP\r\n";

static const char *internal_error_header = "HTTP/1.1 500 Internal error\r\n"
"Server: HyperboreaTTP\r\n";

void route(Server *server, int client_sock, char *rel_path) {
	/* If a path ends with /, append an index.html */
	const char *index_file = "index.html";

	char *path = NULL;
	int path_err;
	if (rel_path[strlen(rel_path)-1] == '/') {
		path = calloc(server->server_root_len + 2 + strlen(rel_path) + strlen(index_file), sizeof(char));
		path_err = errno;
		sprintf(path, "%s/%s%s", server->server_root, rel_path, index_file);
	} else {
		path = calloc(server->server_root_len + 2 + strlen(rel_path), sizeof(char));
		path_err = errno;
		sprintf(path, "%s/%s", server->server_root, rel_path);
	}

	if (path == NULL) {
		errno = path_err;
		perror("path");
		send(client_sock, internal_error_header, strlen(internal_error_header), 0);
	}

	if (!server_send_file(client_sock, path)) {
		/* file not found. */
		send(client_sock, not_found_header, strlen(not_found_header), 0);
	}
	free(path);
}
