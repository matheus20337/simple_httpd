#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "server.h"

/* Poor man's httpd */

const char *port = "8080"; /* The functions expect the port to be a string. */

int main(int argc, char *argv[]) {
	if (argc < 2) {
		fprintf(stderr, "Usage: %s <root_directory>\n", argv[0]);
		exit(1);
	}

	Server server = init_server(port, argv[1]);

	/* TODO: This loop will freeze execution until a client connects and, thus, sets a
	 * limit of one concurrent user for the server. I need to fix this.
	 */
	do {
		server_handle_client_connection(&server);
	} while(1);

	/* TODO: The code never reaches this place. I need to find a way to gracefully shutdown the server.
	 */
	stop_server(&server);

	return 0;
}

