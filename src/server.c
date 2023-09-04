#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "cttp-internal.h"
#include "cttp-status.h"
#include "cttp.h"

CTTP_Server CTTP_new_server(int port)
{
    CTTP_Server cs;
    cs.log_level = 1;
    cs.port = port;

    cs.routes = INCTTP_new_route_node();

    cs.bsize = CTTP_MAX_BUFFER_SIZE;
    cs.hsize = CTTP_MAX_HEADERS_SIZE;
    cs.psize = CTTP_MAX_PARAMS_SIZE;
    cs.rsize = CTTP_MAX_REQUEST_SIZE;

    return cs;
}

/**
 * This function serves as a request handler for `CTTP_start_server`, reading the request and writing the response.
 * It simplifies error handling by consolidating various error codes into a single switch-case.
 */
static int handle_request(CTTP_Server *server, char *raw_r, CTTP_Writer *w, CTTP_Request *r)
{
    // Parse raw input and populate request structure
    int parse_result = INCTTP_parse_raw_request(raw_r, r, server);
    if (parse_result < 1) return parse_result;
    
    // Identify route based on parsed request's URI
    CTTP_RouteNode *route;
    int route_result = CTTP_read_route(server, &route, r->uri, r->method);
    if (route_result < 1) return route_result;

    // Execute route's handler using request and writer
    return route->handler(w, r);
}

int CTTP_start_server(CTTP_Server *cs)
{
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        // TODO: print error when cannot start fd
        return CTTP_ERROR_SERVER_SOCKET;
    }

    int val = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

    struct sockaddr_in server_addr = {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(cs->port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        // TODO: print error when cannot bind
        return CTTP_ERROR_SERVER_BIND;
    }

    if (listen(server_fd, 5) < 0)
    {
        // TODO: print error when cannot listen
        return CTTP_ERROR_SERVER_LISTEN;
    }

    while (1)
    {
        struct sockaddr_in client_addr = {};
        socklen_t socklen = sizeof(client_addr);
        int connfd = accept(server_fd, (struct sockaddr *)&client_addr, &socklen);
        if (connfd < 0) {
            continue;
        }
        
        char raw_request[cs->rsize];
        memset(raw_request, 0, sizeof(raw_request));

        ssize_t bytes_received = read(connfd, raw_request, sizeof(raw_request));
        if (bytes_received < 1)
        {
            close(connfd);
            continue;
        }

        CTTP_Writer writer;   // writer will be sent as a pointer to the route handler
        CTTP_Request request; // request will be sent as a pointer to the route handler

        // To avoid memory access errors, we ensure that all bytes in the writer and request are filled
        memset(&writer, 0, sizeof(CTTP_Writer));
        memset(&request, 0, sizeof(CTTP_Request));

        switch (handle_request(cs, raw_request, &writer, &request))
        {
        case CTTP_ERROR_HEADER_FIELDS_TOO_LARGE:
            // Handle the scenario when the request header fields are too large.
            CTTP_write_status(&writer, CTTP_STATUS_REQUEST_HEADER_FIELDS_TOO_LARGE);
            CTTP_write_body(&writer, CTTP_MESSAGE_REQUEST_HEADER_FIELDS_TOO_LARGE, strlen(CTTP_MESSAGE_REQUEST_HEADER_FIELDS_TOO_LARGE));
            break;
        case CTTP_ERROR_CONTENT_TOO_LARGE:
            // Handle the scenario when the content (e.g., parameters or body) is too large.
            CTTP_write_status(&writer, CTTP_STATUS_CONTENT_TOO_LARGE);
            CTTP_write_body(&writer, CTTP_MESSAGE_CONTENT_TOO_LARGE, strlen(CTTP_MESSAGE_CONTENT_TOO_LARGE));
            break;
        case CTTP_ERROR_ROUTE_NOT_FOUND:
            // Handle the scenario when the requested route is not found.
            CTTP_write_status(&writer, CTTP_STATUS_NOT_FOUND);
            CTTP_write_body(&writer, CTTP_MESSAGE_NOT_FOUND, strlen(CTTP_MESSAGE_NOT_FOUND));
            break;
        case CTTP_ERROR_METHOD_NOT_ALLOWED:
            // Handle the scenario when the HTTP method used is not allowed for the given route.
            CTTP_write_header(&writer, "Allow", request.method);
            CTTP_write_status(&writer, CTTP_STATUS_METHOD_NOT_ALLOWED);
            CTTP_write_body(&writer, CTTP_MESSAGE_METHOD_NOT_ALLOWED, strlen(CTTP_MESSAGE_METHOD_NOT_ALLOWED));
            break;
        case CTTP_ERROR_RAW_INITIAL_LINE:
        case CTTP_ERROR_RAW_REQUEST_HEADERS:
        case CTTP_ERROR_INTERNAL_SERVER_ERROR:
            // Handle internal server errors.
            CTTP_write_status(&writer, CTTP_STATUS_INTERNAL_SERVER_ERROR);
            CTTP_write_body(&writer, CTTP_MESSAGE_INTERNAL_SERVER_ERROR, strlen(CTTP_MESSAGE_INTERNAL_SERVER_ERROR));
            break;
        default:
            // No errors occurred during request processing.
            // Successful responses are handled directly in the respective route handlers.
            break;
        }

        char buf[cs->bsize];
        INCTTP_write_response(buf, cs->bsize, &writer, cs);

        if (send(connfd, buf, strlen(buf), 0) == -1)
        {
            // TODO
        }

        close(connfd);
    }

    INCTTP_free_route_node(cs->routes);
    return 1;
}
