#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cttp-internal.h"
#include "cttp.h"

char *CTTP_read_request_header(CTTP_Request *r, char *k)
{
    for (int i = 0; i < r->hsize; i++)
    {
        if (strcmp(r->headers[i].key, k) == 0)
        {
            return r->headers[i].val;
        }
    }

    return NULL;
}

char *CTTP_read_request_param(CTTP_Request *r, char *k)
{
    for (int i = 0; i < r->params_count; i++)
    {
        if (strcmp(r->params[i].key, k) == 0)
        {
            return r->params[i].val;
        }
    }

    return NULL;
}

/**
 * For debugging only
 */
static void print_request(CTTP_Request *r)
{
    printf("\n\n");
    printf("method: %s\n", r->method);
    printf("uri: %s\n", r->uri);
    printf("http_version: %s\n", r->http_version);

    printf("headers:\n");
    for (int i = 0; i < r->hsize; i++)
    {
        printf("\tkey: %s | val: %s\n", r->headers[i].key, r->headers[i].val);
    }

    printf("parameters:\n");
    for (int i = 0; i < r->params_count; i++)
    {
        printf("\tkey: %s | val: %s\n", r->params[i].key, r->params[i].val);
    }

    printf("body:\n%s\n", r->body);
    printf("\n\n");
}

static int set_raw_request_header(CTTP_Request *r, char *k, char *v)
{
    if (r->hsize > CTTP_HEADER_LIMIT) {
        return 0;
    }

    CTTP_Header *header = &r->headers[r->hsize];
    header->key = strdup(k);
    header->val = strdup(v);
    r->hsize++;

    return 1;
}

static int parse_raw_headers(CTTP_Server *cs, CTTP_Request *r, const char *raw_r)
{
    // Move to headers section. Headers begin on the second line.
    const char *restrict raw_h = strchr(raw_r, '\n');
    // Headers are required. If none are provided, return an error.
    if (raw_h == NULL) return CTTP_ERROR_RAW_REQUEST_HEADERS;

    // Check the length of headers for security reasons (e.g., potential DoS attacks). 
    if (strlen(raw_h) >= cs->hsize)
    {
        return CTTP_ERROR_HEADER_FIELDS_TOO_LARGE;
    }

    char k[512], v[512];
    for (raw_h++; raw_h && strncmp(raw_h, "\r\n", 2) != 0; raw_h++)
    {
        if (sscanf(raw_h, "%511[^:]: %511[^\r\n]", k, v) == 2)
        {
            set_raw_request_header(r, k, v);
        }

        if ((raw_h = strchr(raw_h, '\n')) == NULL) break;
    }

    return 1;
}

static int set_raw_request_param(CTTP_Request *r, char *k, char *v)
{
    if (r->params_count > CTTP_PARAM_LIMIT)
    {
        return 0;
    }

    CTTP_Parameter *parameter = &r->params[r->params_count];
    parameter->key = strdup(k);
    parameter->val = strdup(v);
    r->params_count++;

    return 1;
}

static int parse_raw_params(CTTP_Server *cs, CTTP_Request *r, char *uri)
{
    // Parse query parameters.
    // Unlike headers, it's not an error if no parameters are found.
    char *restrict raw_p = strchr(uri, '?');
    if (raw_p != NULL)
    {
        // For security, limit the length of parameters to prevent potential attacks, such as SQL injection.
        if (strlen(raw_p) >= cs->psize)
        {
            return CTTP_ERROR_CONTENT_TOO_LARGE;
        }

        *raw_p = '\0'; // Set termination character at the '?' position, truncating r->uri here.
                       // This cleans up the uri and let us to match patterns later
        raw_p++;

        char k[512], v[512];
        for (int i = 0; raw_p[i]; i++, raw_p++)
        {
            if (sscanf(raw_p, "%511[^=]=%511[^&]", k, v) == 2)
            {
                set_raw_request_param(r, k, v);
            }
            
            if ((raw_p = strchr(raw_p, '&')) == NULL)
            {
                break;
            }
        }
    }
    return 1;
}

static int parse_raw_body(CTTP_Server *cs, CTTP_Request *r, const char *restrict raw_r)
{
    const char *body_start = strstr(raw_r, "\r\n\r\n");
    if (!body_start)
    {
        r->body[0] = '\0';
        return 1;
    }
    
    body_start += 4;

    size_t body_len = strlen(body_start);
    if (body_len >= cs->bsize)
    {
        return CTTP_ERROR_CONTENT_TOO_LARGE;
    }

    strcpy(r->body, body_start);
    return 1;
}

int INCTTP_parse_raw_request(const char *restrict raw_r, CTTP_Request *r, CTTP_Server *cs)
{
    // Parse initial request line
    if (sscanf(raw_r, "%244s %511s %244s", r->method, r->uri, r->http_version) != 3)
    {
        return CTTP_ERROR_RAW_INITIAL_LINE;
    }

    int headers = parse_raw_headers(cs, r, raw_r);
    if (headers < 1) return headers;

    int parameters = parse_raw_params(cs, r, r->uri);
    if (parameters< 1) return parameters;

    int body = parse_raw_body(cs, r, raw_r);
    if (body < 1) return body;

    // print_request(r);
    return 1;
}
