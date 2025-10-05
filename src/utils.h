#ifndef UTILS_H
#define UTILS_H
#include <sys/socket.h>

void addr_to_str(struct sockaddr *addr, socklen_t addr_len, char *addr_str);
#endif /* UTILS_H */
