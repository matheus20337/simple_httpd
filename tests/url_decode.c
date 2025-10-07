#include <stdio.h>
#include <assert.h>
#include <string.h>

void url_decode(char *dst, const char *src);

int main() {
	char decoded[1024];

	/* Hello, World! */
	char *example1 = "Hello,%20World%21";
	/* "I am groot" */
	char *example2 = "%22I%20am%20groot%22";
	/* 😊 */
	char *example3 = "%F0%9F%98%8A";
	/* name=John&age=30 */
	char *example4 = "name%3DJohn%26age%3D30";

	url_decode(decoded, example1);
	assert(strcmp(decoded, "Hello, World!") == 0);
	printf("Test 1 passed.\n");

	url_decode(decoded, example2);
	assert(strcmp(decoded, "\"I am groot\"") == 0);
	printf("Test 2 passed.\n");

	url_decode(decoded, example3);
	assert(strcmp(decoded, "😊") == 0);
	printf("Test 3 passed.\n");

	url_decode(decoded, example4);
	assert(strcmp(decoded, "name=John&age=30") == 0);
	printf("Test 4 passed.\n");
	
	return 0;
}
