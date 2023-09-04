#ifndef CTTP_LIB_H
#define CTTP_LIB_H

#include <stddef.h>

#define CTTP_MAX_REQUEST_SIZE 1153433
#define CTTP_MAX_BUFFER_SIZE  1048576
#define CTTP_HEADER_LIMIT     100
#define CTTP_MAX_HEADERS_SIZE 8192
#define CTTP_PARAM_LIMIT      250
#define CTTP_MAX_PARAMS_SIZE  8192

enum CTTP_ERROR 
{
    CTTP_ERROR_NIL                     =  1, 
    CTTP_ERROR_INTERNAL_SERVER_ERROR   =  0, /* Occurs when an internal error arises */
    CTTP_ERROR_ROUTE_NOT_FOUND         = -1, /* Occurs when the route does not exist */
    CTTP_ERROR_METHOD_NOT_ALLOWED      = -2, /* Occurs when the route exists but the method is incorrect */
    CTTP_ERROR_HEADER_FIELDS_TOO_LARGE = -3, /* Occurs when headers from a raw request are too large */
    CTTP_ERROR_CONTENT_TOO_LARGE       = -4, /* Occurs when parameters or body from a raw request are too large */
    CTTP_ERROR_SERVER_SOCKET           = -5, /* Occurs when server_fd is invalid */
    CTTP_ERROR_SERVER_BIND             = -6, /* Occurs when the server cannot bind */
    CTTP_ERROR_SERVER_LISTEN           = -7, /* Occurs when the server cannot listen */
    CTTP_ERROR_RAW_INITIAL_LINE        = -8, /* Occurs when a raw request doesn't have the initial line according to the HTTP standard */
    CTTP_ERROR_RAW_REQUEST_HEADERS     = -9, /* Occurs when a raw request doesn't have any headers */
};
 
// <------------------------>
//        CTTP_Header
// <------------------------>

typedef struct
{
    char *key;
    char *val;
}
CTTP_Header;

// <-------------------------->
//        CTTP_Parameter
// <-------------------------->
 
typedef struct
{
    char *key;
    char *val;
}
CTTP_Parameter;

// <------------------------>
//        CTTP_Request
// <------------------------>

typedef struct
{
    char           method[255]; // TODO: more method handling
    char           uri[512]; // TODO: larger uri
    CTTP_Header    headers[CTTP_HEADER_LIMIT];
    size_t         hsize;
    CTTP_Parameter params[CTTP_PARAM_LIMIT];
    size_t         params_count;
    char           http_version[255];
    char           body[1024];
    size_t         bsize;
}
CTTP_Request;

/**
 * Reads a header with key `k` from request `r`. Returns `NULL` if the header does not exists.
 */
char *CTTP_read_request_header(CTTP_Request *r, char *k);

/**
 * Reads a parameter with key `k` from request `r`. Returns `NULL` if the parameter does not exists.
 */
char *CTTP_read_request_param(CTTP_Request *r, char *k);

// <------------------------>
//        CTTP_Writer
// <------------------------>

/**
 *
 */
typedef struct
{
    const char  *status; /* HTTP response status */
    char        *body; /* Response body content */
    size_t       bsize; /* Size in bytes of the response body */
    CTTP_Header  headers[CTTP_HEADER_LIMIT]; /* Array of response headers */
    size_t       hsize; /* Number of headers set */
}
CTTP_Writer;

/**
 * Writes a header with key `k` and value `v` to Writer `w`. Returns 0 if an error occurs.
 */
int CTTP_write_header(CTTP_Writer *w, char *k, char *v);

/**
 * Writes a status to Writer `w`. Returns 0 if an error occurs.
 */
int CTTP_write_status(CTTP_Writer *w, const char *STATUS);

/**
 * Writes the body `b` to `w->body` and sets `Content-Length` to `w->bsize` for the provide Writer. 
 * Returns 0 if an error occurs.
 */
int CTTP_write_body(CTTP_Writer *w, const char *b, size_t _blen);

/**
 * Reads a header with key `k` from writer `w`. Returns `NULL` if the header does not exists.
 */
char *CTTP_read_writer_header(CTTP_Writer *w, char *k);

// <---------------------->
//        CTTP_Route
// <---------------------->

/* Set the upper limit for the number of RouteNode children based on ASCII size */
#define ALPHABET_SIZE 128

/**
 * Type definition for a function handling CTTP routes.
 * The function should return an integer and accept pointers to `CTTP_Writer` and `CTTP_Request`.
 */
typedef int (*CTTP_RouteHandler)(CTTP_Writer *w, CTTP_Request *r);

/**
 * Represents a node in a trie, each corresponding to an ASCII character.
 * Nodes can have child nodes and, if they represent the end of a path, an associated HTTP handler.
 */
typedef struct CTTP_RouteNode CTTP_RouteNode;
struct CTTP_RouteNode
{
    CTTP_RouteNode    *children[ALPHABET_SIZE]; /* Children nodes array, indexed by ASCII codes */
    CTTP_RouteHandler  handler; /* HTTP handler function for the endpoint, if this node is a terminal/leaf node */
    char              *method; /* HTTP method associated with the node */
};

// <----------------------->
//        CTTP_Server
// <----------------------->

/**
 * Represents the core structure of the CTTP server.
 */
typedef struct
{
    /**
     * Root node for routing.
     * Acts as the primary entry point for all route nodes. It's where the server starts
     * its search for matching routes based on incoming requests.
     */
    CTTP_RouteNode* routes;

    /**
     * Port number on which the server listens.
     * This specifies the communication endpoint where the server binds and listens for incoming client requests.
     */
    int port;
    
    /**
     * Logging verbosity level for server operations.
     * Determines the amount and detail of logs that will be generated during server operations.
     */
    int log_level;

    /**
     * Buffer size limit for request or response body.
     * Specifies the maximum allowable size of data in the body section of either incoming requests or outgoing responses.
     * Default: 1MB (1,048,576 bytes).
     */
    size_t bsize;

    /**
     * Header size limit for request or response.
     * Denotes the maximum allowable length of headers for both incoming requests and outgoing responses.
     * Default: 8KB (8,192 bytes).
     */
    size_t hsize;

    /**
     * Parameter size limit for incoming requests.
     * Indicates the maximum allowable length for query parameters in the request URI.
     * Default: 8KB (8,192 bytes).
     */
    size_t psize;

    /**
     * Overall request size limit.
     * Sets an upper bound on the total size of the client's HTTP request, which includes headers, body, method, etc.
     * Default: 1.1MB (1,153,433 bytes).
     */
    size_t rsize;
} CTTP_Server;

/**
 * Initialize a new CTTP server on port `p` with default configurations.
 * To override default settings, redefine the desired attribute (e.g., `server.rsize = 10485760` for 10MB max request size).
 * Start the server using `CTTP_start_server(&server)`.
 */
CTTP_Server CTTP_new_server(int p);

/**
 * Add a route to server `cs` with method `m`, path `p`, and handler `h`.
 * Multiple methods can be separated with `,`. The handler should be a `CTTP_RouteHandler` function pointer.
 */
void CTTP_add_route(CTTP_Server *cs, char *m, char *p, CTTP_RouteHandler h);

/**
 * Retrieve a route from server `cs` using path `p` and method `m`.
 * Returns `NULL` if the route isn't found.
 */
int CTTP_read_route(CTTP_Server *cs, CTTP_RouteNode **r, char *p, char *m);

/**
 * Start the CTTP server. This enters an infinite loop to process requests.
 * Place this call at the end of your code.
 */
int CTTP_start_server(CTTP_Server *cs);


#endif
