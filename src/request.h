#ifndef REQUEST_H
#define REQUEST_H
#include <stddef.h>

typedef enum RequestMethod {
	GET_METHOD,
	UNSUPPORTED_METHOD
} RequestMethod;

typedef struct HTTPRequest {
	RequestMethod method;
	char path[1024];
} HTTPRequest;
#endif

void free_request(HTTPRequest *request);
HTTPRequest *parse_request(const char *buffer, size_t buffer_len);
