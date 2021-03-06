#ifndef __COMMON_H
#define __COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define DEFAULT_HOST "localhost"

#define NAME_LENGTH 48
#define URL_LENGTH 2000

#define MIN(a, b) ((a > b) ? b : a)
#define ENTRYCMP(name, start, size) strncasecmp(start, name, MIN(strlen(name), size))

struct config {
    char name[NAME_LENGTH];
    char url[URL_LENGTH];
    struct config *next;
};

struct config_entry {
    char *protocol, *user, *pass, *host, *port, *volumn;
};

void to_url(char *url, struct config_entry *entry);
int entry_is_null(struct config_entry *entry);

#endif
