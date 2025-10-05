#include "request.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

void free_request(HTTPRequest *request) {
	free(request);
}

/* My Http server only serves static files. The only
 * info we care about in the request is the method
 * (so that we can inform the client that only the GET
 * method is supported) and the path (excluding query parameters).
 * Everything else about the request can be discarded.
 */
HTTPRequest *parse_request(const char *buffer, size_t buffer_len) {
	HTTPRequest *request = malloc(sizeof(HTTPRequest));
	request->path[0] = '\0';

	size_t buff_index = 0;

	/* The first line of a request has the following:
	 * <Method> <URI> <Protocol version>\r\n
	 */

	char method[20];
	/*
	 * We start by parsing the method
	 */
	for (buff_index = 0; buff_index < 20 && buff_index < buffer_len; buff_index++) {
		if (isspace(buffer[buff_index])) {
			break;
		}
		method[buff_index] = buffer[buff_index];
	}
	method[buff_index] = '\0';

	if (strcmp("GET", method) != 0) {
		request->method = NOT_GET_METHOD;
		return request;
	} else {
		request->method = GET_METHOD;
	}

	/* Skip to the next non-whitespace char */
	while (buff_index < buffer_len) {
		if (isspace(buffer[buff_index])) {
			buff_index++;
		} else {
			break;
		}
	}

	/*
	 * Now we need to parse the path.
	 * 
	 * The path can optionally have query parameteres. We need to ignore them.
	 * /path/to/resource?value=true
	 *                  ^- everything after this must be ignored.
	 */
	size_t i;
	for (i = 0; buff_index < buffer_len && buff_index < sizeof request->path; i++, buff_index++) {
		if (isspace(buffer[buff_index]) || buffer[buff_index] == '?') {
			break;
		}

		request->path[i] = buffer[buff_index];
	}
	request->path[i] = '\0';

	return request;
}

