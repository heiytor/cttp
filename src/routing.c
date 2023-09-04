#include <stdlib.h>
#include <string.h>

#include "cttp-internal.h"
#include "cttp.h"

CTTP_RouteNode* INCTTP_new_route_node()
{
    CTTP_RouteNode* node = (CTTP_RouteNode *)malloc(sizeof(CTTP_RouteNode));
    if (node == NULL) return NULL;
    
    for (int i = 0; i < ALPHABET_SIZE; i++)
    {
        node->children[i] = NULL;
    }

    node->handler = NULL;
    node->method = NULL;

    return node;
}

void INCTTP_free_route_node(CTTP_RouteNode *root)
{
    for (int i = 0; i < ALPHABET_SIZE; i++)
    {
        if (root->children[i] != NULL)
        {
            INCTTP_free_route_node(root->children[i]);
        }
    }

    free(root);
}

void CTTP_add_route(CTTP_Server *cs, char *m, char *p, CTTP_RouteHandler h)
{
    if (h == NULL) return;
    CTTP_RouteNode *node = cs->routes;

    for (int i = 0; p[i] != '\0'; i++)
    {
        int ascii_code = (int)p[i];
        if (node->children[ascii_code] == NULL)
        {
            node->children[ascii_code] = INCTTP_new_route_node();
        }

        node = node->children[ascii_code];
    }

    node->handler = h;
    node->method = m;
}

int CTTP_read_route(CTTP_Server *cs, CTTP_RouteNode **r, char *p, char *m)
{
    CTTP_RouteNode *node = cs->routes;
    
    for (int i = 0; p[i] != '\0'; i++)
    {
        int ascii_code = (int)p[i];
        if (node->children[ascii_code] == NULL)
        {
            return CTTP_ERROR_ROUTE_NOT_FOUND;
        }

        node = node->children[ascii_code];
    }


    if (node == NULL || node->handler == NULL)
    {
        return CTTP_ERROR_ROUTE_NOT_FOUND;
    }

    if (node != NULL && node->handler != NULL && strcmp(node->method, m) != 0)
    {
        return CTTP_ERROR_METHOD_NOT_ALLOWED;
    }
    
    *r = node;
    return 1;
}
