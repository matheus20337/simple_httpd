#ifndef REQUEST_H
#define REQUEST_H
#include <stddef.h>
#include <limits.h>

typedef enum RequestMethod {
	GET_METHOD,
	UNSUPPORTED_METHOD
} RequestMethod;

#define REQ_PATH_LEN 1024

typedef struct HTTPRequest {
	RequestMethod method;
	char path[REQ_PATH_LEN];
} HTTPRequest;
#endif

void free_request(HTTPRequest *request);
HTTPRequest *parse_request(const char *buffer, size_t buffer_len);
