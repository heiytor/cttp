#include <string.h>

#include "cttp-internal.h"
#include "cttp-status.h"
#include "cttp.h"

int route_handler(CTTP_Writer *w, CTTP_Request *r)
{
    char *r_header = CTTP_read_request_header(r, "X-User-Id");
    if (r_header == NULL)
    {
        return CTTP_ERROR_INTERNAL_SERVER_ERROR;
    }

    char message[] = "text from /rota";
    CTTP_write_header(w,"Content-Type", "plain/text");
    CTTP_write_status(w, CTTP_STATUS_CREATED);
    CTTP_write_body(w, message, strlen(message));

    return CTTP_ERROR_NIL;
}

int home_handler(CTTP_Writer *w, CTTP_Request *r)
{
    char *r_param = CTTP_read_request_param(r, "id");
    if (r_param == NULL)
    {
        return CTTP_ERROR_INTERNAL_SERVER_ERROR;
    }

    char message[] = "{\"home\": true}";

    CTTP_write_header(w, "Content-Type", "application/json");
    CTTP_write_status(w, CTTP_STATUS_OK);
    CTTP_write_body(w, message, strlen(message));

    return CTTP_ERROR_NIL;
}

int main()
{
    CTTP_Server cs = CTTP_new_server(8080);

    CTTP_add_route(&cs, "GET", "/", home_handler);
    CTTP_add_route(&cs, "GET", "/rota", route_handler);

    switch (CTTP_start_server(&cs))
    {
    case CTTP_ERROR_SERVER_SOCKET:
        break;
    case CTTP_ERROR_SERVER_BIND:
        break;
    case CTTP_ERROR_SERVER_LISTEN:
        break;
    }

    return 0;
}

