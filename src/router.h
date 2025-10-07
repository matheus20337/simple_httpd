#ifndef ROUTER_H
#define ROUTER_H
#include "server.h"
#include "request.h"

void route(Server *server, int client_sock, HTTPRequest *request);

#endif /* ROUTER_H */
