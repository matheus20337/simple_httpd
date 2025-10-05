#include <arpa/inet.h>
#include <sys/socket.h>

/* Converts a generic sockaddr into a presentation ip string. */
void addr_to_str(struct sockaddr *addr, socklen_t addr_len, char *addr_str) {
	void *raw_addr;

	if (addr->sa_family == AF_INET) {
		/* This is my greatest act of pointer magic to this date. */
		raw_addr = &((struct sockaddr_in *)addr)->sin_addr;
	} else {
		raw_addr = &((struct sockaddr_in6 *)addr)->sin6_addr;
	}

	inet_ntop(addr->sa_family, raw_addr, addr_str, addr_len);
}
