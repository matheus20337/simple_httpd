#ifndef ROUTER_H
#define ROUTER_H
#include "server.h"

void route(Server *server, int client_sock, char *rel_path);

#endif /* ROUTER_H */
