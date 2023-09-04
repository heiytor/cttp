#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>
#include <stdio.h>

#include "cttp-internal.h"

static const char *DAYS[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
static const char *MONTHS[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

void INCTTP_current_date(char *buf) {
    time_t timestamp;
    struct tm *tm_info;
    time(&timestamp);
    tm_info = gmtime(&timestamp);

    snprintf(
            buf,
            DATE_LEN,
            "%s, %02d %s %d %02d:%02d:%02d GMT",
            DAYS[tm_info->tm_wday],
            tm_info->tm_wday,
            MONTHS[tm_info->tm_mon],
            tm_info->tm_year + 1900,
            tm_info->tm_hour,
            tm_info->tm_min,
            tm_info->tm_sec
            );
}

void INCTTP_print_start(struct sockaddr_in client_addr, size_t request_count)
{
    char date[DATE_LEN];
    INCTTP_current_date(date);

    printf(
            "\033[0;32mRequest \033[1;33m#%lu\033[0;32m from %s:%d started at %s\033[0m\n",
            request_count,
            inet_ntoa(client_addr.sin_addr), // client's IP
            ntohs(client_addr.sin_port), // client's port
            date
          );
}


// 35 31 error

void INCTTP_print_end(struct sockaddr_in client_addr, size_t request_count)
{
    char date[DATE_LEN];
    INCTTP_current_date(date);

    printf(
            "\033[0;32mRequest \033[1;33m#%lu\033[0;32m from %s:%d finished at %s\033[0m\n",
            request_count,
            inet_ntoa(client_addr.sin_addr), // client's IP
            ntohs(client_addr.sin_port), // client's port
            date
          );
}
