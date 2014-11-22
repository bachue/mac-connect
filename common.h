#ifndef __COMMON_H
#define __COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define DEFAULT_HOST "localhost"

#define NAME_LENGTH (48 - 1)
#define URL_LENGTH 2000

struct config {
    char name[NAME_LENGTH + 1];
    char url[URL_LENGTH + 1];
    struct config *next;
};

struct config_entry {
    char *protocol, *user, *pass, *host, *port, *volumn;
};

void to_url(char *url, struct config_entry *entry);
void parse(FILE *config_file);
FILE* find_config();

#endif
