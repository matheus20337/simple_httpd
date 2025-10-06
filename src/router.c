#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

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
	/* If a path is a directory, append an index.html */
	const char *index_file = "/index.html";

	struct stat file_stat;

	char path[1500];

	sprintf(path, "%s/%s", server->server_root, rel_path);

	if (stat(path, &file_stat) == -1) {
		perror("stat");
		send(client_sock, internal_error_header, strlen(internal_error_header), 0);
		close(client_sock);
		return;
	}

	if (S_ISDIR(file_stat.st_mode)) {
		strcat(path, index_file);
	}

	if (!server_send_file(client_sock, path)) {
		/* file not found. */
		send(client_sock, not_found_header, strlen(not_found_header), 0);
	}
}
