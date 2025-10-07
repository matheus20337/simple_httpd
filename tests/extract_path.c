#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* Make sure to compile with src/request.c */
char *extract_path(const char *line);

int main() {
	/* / */
	const char *request_1 = "GET / HTTP/1.1\r\n";
	/* /users/create */
	const char *request_2 = "POST /users/create?name=neo&pass=rosebud HTTP/1.1\r\n";
	/* /I%20am%20groot/ */
	const char *request_3 = "PATCH /I%20am%20groot/ HTTP/1.1\r\n";
	/* Not a valid request */
	const char *request_4 = "PUT /w233\r\n";

	char *path = extract_path(request_1);
	assert(strcmp(path, "/") == 0);
	printf("Passed test 1.\n");
	free(path);

	path = extract_path(request_2);
	assert(strcmp(path, "/users/create") == 0);
	printf("Passed test 2.\n");
	free(path);

	path = extract_path(request_3);
	assert(strcmp(path, "/I%20am%20groot/") == 0);
	printf("Passed test 3.\n");
	free(path);

	path = extract_path(request_4);
	assert(path == NULL);
	printf("Passed test 4.\n");

	return 0;
}
