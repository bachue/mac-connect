#include "common.h"

void to_url(char *url, struct config_entry *entry) {
    size_t url_len;

    if (entry->host == NULL) entry->host = DEFAULT_HOST;

    url_len = strlen(entry->protocol) + strlen("://");
    if (entry->user != NULL) {
        url_len += strlen(entry->user) + strlen(":");
        if (entry->pass != NULL) url_len += strlen(entry->pass);
        url_len += strlen("@");
    }
    url_len += strlen(entry->host);
    if (entry->port != NULL) url_len += strlen(":") + strlen(entry->port);
    if (entry->volumn != NULL) url_len += strlen("/") + strlen(entry->volumn);

    if (url_len >= URL_LENGTH) {
        fprintf(stderr, "URL is too long\n");
        exit(EXIT_FAILURE);
    }

    strcat(url, entry->protocol);
    strcat(url, "://");
    if (entry->user != NULL) {
        strcat(url, entry->user);
        strcat(url, ":");
        if (entry->pass != NULL) strcat(url, entry->pass);
        strcat(url, "@");
    }
    strcat(url, entry->host);
    if (entry->port != NULL) {
        strcat(url, ":");
        strcat(url, entry->port);
    }
    if (entry->volumn != NULL) {
        strcat(url, "/");
        strcat(url, entry->volumn);
    }
}

int entry_is_null(struct config_entry *entry) {
    return entry->protocol == NULL && entry->user == NULL && entry->pass == NULL &&
           entry->host == NULL && entry->port == NULL && entry->volumn == NULL;
}
