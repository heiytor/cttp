#include <stdio.h>
#include <string.h>

#include "cttp-internal.h"
#include "cttp-status.h"
#include "cttp.h"

char *CTTP_read_writer_header(CTTP_Writer *w, char *k)
{
    for (int i = 0; i < w->hsize; i++)
    {
        if (strcmp(w->headers[i].key, k) == 0)
        {
            return w->headers[i].val;
        }
    }

    return NULL;
}

int CTTP_write_header(CTTP_Writer *w, char *k, char *v)
{
    CTTP_Header *header = &w->headers[w->hsize];
    header->key = strdup(k);
    header->val = strdup(v);
    w->hsize++;

    return header->key != NULL && header->val != NULL ? 1 : 0;
}

int CTTP_write_status(CTTP_Writer *w, const char *STATUS)
{
    w->status = strdup(STATUS);
    return w->status != NULL ? 1 : 0;
}

int CTTP_write_body(CTTP_Writer *w, const char *b, size_t bsize)
{
    w->body = strdup(b);
    w->bsize = bsize;
    if (w->body == NULL) return 0;

    char contentLength[1024];
    snprintf(contentLength, 1024, "%lu", bsize);
    if (CTTP_write_header(w, "Content-Length", contentLength) == 0) return 0;

    return 1;
}

void INCTTP_write_response(char *buf, size_t buf_len, CTTP_Writer *w, CTTP_Server *cs)
{
    // When `CTTP_write_body` is not sent, `w->body` is `NULL`, to avoid a body (null)
    // response, we ensure that `w-body` is always a string.
    if (w->body == NULL) w->body = "";

    // Write the response headers
    char hsize[cs->hsize]; // represents all key/value pairs in headers
    char hline[400]; // represents a single key/valur header
    hsize[0] = '\0';
    for (size_t i = 0; i < w->hsize; i++) {
        snprintf(hline, sizeof(hline), "%s: %s\r\n", w->headers[i].key, w->headers[i].val);
        strncat(hsize, hline, sizeof(hsize) - strlen(hsize) - 1);
    }

    char date[DATE_LEN]; 
    INCTTP_current_date(date);
    // Write the response body
    snprintf(
            buf,
            buf_len,
            "HTTP/1.1 %s\r\nDate: %s\r\nServer: 0.0.0.0\r\n%s\r\n%s",
            w->status,
            date,
            hsize,
            w->body
            );
}

