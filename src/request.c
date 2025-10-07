#include "request.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

void free_request(HTTPRequest *request) {
	free(request);
}
/* Extracts a path from an HTTP request line.
 * Returns if a valid path could be found.
 */
bool extract_path(char *path, const char *line) {
	path[0] = '\0';

	/* <method> <path> <protocol version>\r\n 
	 *          ^- start
	 */
	const char *start = strchr(line, ' ');
	if (start == NULL) {
		return false;
	}
	start++;

	/* <method> <path> <protocol version>\r\n 
	 *                ^- end
	 */
	/* We ignore query parameters. */
	const char *end = strchr(start, '?');
	if (end == NULL) {
		end = strchr(start, ' ');
	}

	if (end == NULL || (end - start >= REQ_PATH_LEN)) {
		return false;
	}

	strncpy(path, start, end - start);
	path[end - start] = '\0';

	return true;
}

void url_decode(char *dst, const char *src) {
	while (*src) {
		if (*src == '%' && isxdigit(*(src + 1)) && isxdigit(*(src + 2))) {
			char hex[3] = { *(src + 1), *(src + 2), '\0' };
			*dst++ = (char)strtol(hex, 0, 16);
			src += 3;
		} else if (*src == '+') {
			*dst++ = ' ';
			src++;
		} else {
			*dst++ = *src++;
		}
	}
	*dst = '\0';
}

void extract_method(char *method_str, size_t method_str_len, const char *buffer, size_t buffer_len) {
	/* <method> <path> <protocol version>\r\n
	 * ^- Start at te beggining
	 */
	size_t i;
	for (i = 0; i < method_str_len && i < buffer_len; i++) {
		if (isspace(buffer[i])) {
			break;
		}
		method_str[i] = buffer[i];
	}
	method_str[i] = '\0';
}

/* My Http server only serves static files. The only
 * info we care about in the request is the method
 * (so that we can inform the client that only the GET
 * method is supported) and the path (excluding query parameters).
 * Everything else about the request can be discarded.
 */
HTTPRequest *parse_request(const char *buffer, size_t buffer_len) {
	HTTPRequest *request = malloc(sizeof(HTTPRequest));
	if (request == NULL) {
		perror("Could not allocate request");
		return NULL;
	}

	char path[REQ_PATH_LEN];

	char line[REQ_PATH_LEN];

	for (int i = 0; i < REQ_PATH_LEN; i++) {
		if (buffer[i] == '\r' || buffer[i] == '\n') {
			line[i] = '\0';
			break;
		} else {
			line[i] = buffer[i];
		}
	}

	if (!extract_path(path, line)) {
		perror("Could not resolve path string");
		free_request(request);
		return NULL;
	}

	url_decode(request->path, path);

	char method[20];
	extract_method(method, 20, buffer, buffer_len);

	if (strcmp("GET", method) == 0) {
		request->method = GET_METHOD;
	} else {
		request->method = UNSUPPORTED_METHOD;
	}

	return request;
}

