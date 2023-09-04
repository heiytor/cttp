#ifndef CTTP_INTERNAL_H
#define CTTP_INTERNAL_H

#include "cttp.h"
#include <netinet/in.h>

#define DATE_LEN 30

/**
 * (Internal function) Writes a valid response to buffer `buf` based on provided Writer `w`.
 */
void INCTTP_write_response(char *buf, size_t buf_len, CTTP_Writer *w, CTTP_Server *cs);

/**
 * (Internal function)
 */
CTTP_RouteNode* INCTTP_new_route_node();

/**
 * (Internal function)
 */
void INCTTP_free_route_node(CTTP_RouteNode *root);

/**
 * (Internal function) Parses a raw HTTP request `raw_r` and populates the Request `r`
 * with the parsed data. Returns `n =< 0` if an error arise.
 */
int INCTTP_parse_raw_request(const char *request_str, CTTP_Request *request, CTTP_Server *cs);

/**
 * (Internal function) Writes the current date and time in the format `%a, %d %b %Y %H:%M:%S GMT` to a buffer.
 * The buffer size needs at least `DATE_LEN` characters to store the formatted date and time.
 */
void INCTTP_current_date(char *buf);

/**
 * (Internal function)
 */
void INCTTP_print_start(struct sockaddr_in client_addr, size_t request_count);

/**
 * (Internal function)
 */
void INCTTP_print_end(struct sockaddr_in client_addr, size_t request_count);

#endif
