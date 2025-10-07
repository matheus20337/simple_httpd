#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include <stdbool.h>

#include "server.h"
#include "request.h"
/* TODO: The router allows the user to fetch files outside of the server root. Stop that. */

/* Base headers for the 404 page. If the server cannot find a 
 * 404 html page in the server root, only this header will be sent.
 */
static const char *not_found_header = "HTTP/1.1 404 Not found\r\n"
"Server: HyperboreaTTP\r\n";

//static const char *internal_error_header = "HTTP/1.1 500 Internal error\r\n"
//"Server: HyperboreaTTP\r\n";

/* Combines the server root and the request path.
 * Returns true if the path is safe, false if the path is unsafe or invalid.
 */
bool resolve_safe_path(const Server *server, const HTTPRequest *request, char *dst, size_t dst_len) {
	/* If a path is a directory, append an index.html */
	const char *index_file = "/index.html";

	char tmp_path[dst_len];
	
	snprintf(tmp_path, dst_len, "%s/%s", server->server_root, request->path);

	char real_path[PATH_MAX];

	/* Get the real path of the request. */
	if (realpath(tmp_path, real_path) == NULL) {
		*dst = '\0';
		return false;
	}

	/* If the real path does not coincide with the server root, the request is
	 * a directory traversal attack.
	 */
	if (strncmp(server->server_root, real_path, server->server_root_len) != 0) {
		*dst = '\0';
		return false;
	}

	struct stat file_stat;

	if (stat(real_path, &file_stat) == -1) {
		perror("stat");
		return false;
	}

	if (S_ISDIR(file_stat.st_mode)) {
		strcat(real_path, index_file);
	}

	strcpy(dst, real_path);
	return true;
}

void route(Server *server, int client_sock, HTTPRequest *request) {

	char path[PATH_MAX];

	if (!resolve_safe_path(server, request, path, PATH_MAX)) {
		/* Unsafe */
		/* Send an error message */
		send(client_sock, not_found_header, strlen(not_found_header), 0);
		close(client_sock);
		return;
	}

	if (!server_send_file(client_sock, path)) {
		/* file not found. */
		send(client_sock, not_found_header, strlen(not_found_header), 0);
	}
}
