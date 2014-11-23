#include "common.h"

#define BUFLEN 2048

struct config *configs = NULL;
static struct config *cur = NULL;
static char config_path[BUFLEN] = {0};

void parse(FILE *config_file);
FILE* find_config();
static char* trim(char *line, size_t *n);
static void parse_err(char *message, unsigned line);
static void unknown_entry(unsigned line);
static void create_node(char *name, size_t size);
static void set_curnode_url(char *url, size_t size);
static char* getval(char *name, char *start, size_t *size, char *err, unsigned line);
static void writeback_entry(struct config_entry *entry);
static void clear_entry(struct config_entry *entry);

void parse(FILE *config_file) {
    char line[BUFLEN + 1], buf[BUFLEN + 1] = {0}, *buf_start = buf, *start;
    size_t size;
    unsigned line_num = 0;
    struct config_entry cur_entry = {0};

    if (config_file == NULL) return;

    while ((fgets(line, BUFLEN, config_file)) != NULL) {
        line_num++;
#ifdef TEST
fprintf(stderr, "%s:%d %s", config_path, line_num, line);
#endif
        size = strlen(line);
        start = trim(line, &size);

        if (size == 0) continue;

        if (ENTRYCMP("Server", start, size) == 0) {
            start = getval("Server", start, &size, "Server name is expected to be followed", line_num);
            if(cur != NULL) { 
                writeback_entry(&cur_entry);
                clear_entry(&cur_entry);
                memset(buf, 0, URL_LENGTH);
                buf_start = buf;
            }
            create_node(start, size);
        } else if (ENTRYCMP("Url", start, size) == 0) {
            start = getval("Url", start, &size, "URL is expected to be followed", line_num);
            if (cur == NULL) parse_err("Server must be declared before", line_num);
            set_curnode_url(start, size);
        } else if (ENTRYCMP("Protocol", start, size) == 0) {
            start = getval("Protocol", start, &size, "Protocol is expected to be followed", line_num);
            cur_entry.protocol = buf_start;
            buf_start += size + 1;
            strncpy(cur_entry.protocol, start, size);
        } else if (ENTRYCMP("User", start, size) == 0) {
            start = getval("User", start, &size, "User is expected to be followed", line_num);
            cur_entry.user = buf_start;
            buf_start += size + 1;
            strncpy(cur_entry.user, start, size);
        } else if (ENTRYCMP("Pass", start, size) == 0) {
            start = getval("Pass", start, &size, "Password is expected to be followed", line_num);
            cur_entry.pass = buf_start;
            buf_start += size + 1;
            strncpy(cur_entry.pass, start, size);
        } else if (ENTRYCMP("Host", start, size) == 0) {
            start = getval("Host", start, &size, "Host is expected to be followed", line_num);
            cur_entry.host = buf_start;
            buf_start += size + 1;
            strncpy(cur_entry.host, start, size);
        } else if (ENTRYCMP("Port", start, size) == 0) {
            start = getval("Port", start, &size, "Port is expected to be followed", line_num);
            cur_entry.port = buf_start;
            buf_start += size + 1;
            strncpy(cur_entry.port, start, size);
        } else if (ENTRYCMP("Volumn", start, size) == 0) {
            start = getval("Volumn", start, &size, "Volumn is expected to be followed", line_num);
            cur_entry.volumn = buf_start;
            buf_start += size + 1;
            strncpy(cur_entry.volumn, start, size);
        } else
            unknown_entry(line_num);
    }

    if(cur != NULL) writeback_entry(&cur_entry); // write back last time
}

FILE* find_config() {
    FILE *file;
    char *home = getenv("HOME");
    size_t homelen = strlen(home);
    strlcat(config_path, home, BUFLEN);
    strlcat(config_path, "/.connectrc", BUFLEN - homelen);
    file = fopen(config_path, "r");
    if (file == NULL) {
        strlcpy(config_path, "/etc/connect/config", BUFLEN);
        file = fopen(config_path, "r");
    }
    if (file == NULL) config_path[0] = '\0';
    return file;
}

static void set_curnode_url(char *url, size_t size) {
    strncpy(cur->url, url, size);
}

static void create_node(char *name, size_t size) {
    struct config *node_ptr = (struct config *) malloc(sizeof(struct config));
    memset((void *) node_ptr, 0, sizeof(struct config));
    strncpy(node_ptr->name, name, size);
    if (configs == NULL)
        configs = node_ptr, cur = node_ptr;
    else
        cur->next = node_ptr, cur = node_ptr;
}

static char* getval(char *name, char *start, size_t *size, char *err, unsigned line) {
    start += strlen(name);
    *size -= strlen(name);
    start = trim(start, size);

    if (size == 0) parse_err(err, line);
    return start;
}

static char* trim(char *line, size_t *n) {
    char *start = line, *end;

    while(isspace((int) *start)) start++;

    end = index(line, (int) '#');
    if(end == NULL) end = line + *n;
    while(isspace((int) *(end - 1)) && end > start) end--;

    *n = end - start;
    return start;
}

static void writeback_entry(struct config_entry *entry) {
    struct config_entry null = {0};
    if (memcmp(entry, &null, sizeof(struct config_entry)) != 0)
        to_url(cur->url, entry);
}

static void clear_entry(struct config_entry *entry) {
    memset(entry, 0, sizeof(struct config_entry));
}

static void parse_err(char *message, unsigned line) {
    fprintf(stderr, "[%s:%d] Parse config error: %s\n", config_path, line, message);
    exit(EXIT_FAILURE);
}

static void unknown_entry(unsigned line) {
    parse_err("Unknown entry", line);
}

#ifdef TEST

#include <assert.h>

void refresh();
void testcase(char *path);

int main(int argc, char  *argv[]) {
    struct config *test;

    testcase("test/config.empty");
    assert(configs == NULL && cur == NULL);

    testcase("test/config.blank");
    assert(configs == NULL && cur == NULL);

    testcase("test/config.1");
    test = configs;
    assert(test);
    assert(strcmp(test->name, "example") == 0);
    assert(strcmp(test->url, "protocol://user:pass@example/home") == 0);
    test = test->next;
    assert(test);
    assert(strcmp(test->name, "example2") == 0);
    assert(strcmp(test->url, "protocol://user2:pass2@example2/home2") == 0);
    test = test->next;
    assert(test);
    assert(strcmp(test->name, "example3") == 0);
    assert(strcmp(test->url, "protocol://user3:pass3@example3/home3") == 0);
    test = test->next;
    assert(test);
    assert(strcmp(test->name, "example4") == 0);
    assert(strcmp(test->url, "protocol://user4:pass4@example4/home4") == 0);

    testcase("test/config.2");
    test = configs;
    assert(test);
    assert(strcmp(test->name, "example") == 0);
    assert(strcmp(test->url, "smb://guest:@localhost:1234/public") == 0);
    test = test->next;
    assert(test);
    assert(strcmp(test->name, "example2") == 0);
    assert(strcmp(test->url, "ftp://guest:QAP@SSw0rd@localhost") == 0);
    test = test->next;
    assert(test);
    assert(strcmp(test->name, "example3") == 0);
    assert(strcmp(test->url, "samba://192.168.1.1/public") == 0);
    test = test->next;
    assert(test);
    assert(strcmp(test->name, "example4") == 0);
    assert(strcmp(test->url, "samba://localhost") == 0);

    printf("All test cases pass\n");
    return 0;
}

void refresh() {
    struct config *ptr;
    for (ptr = configs; ptr != NULL; ptr = ptr->next)
        free(ptr);
    configs = cur = NULL;
}

void testcase(char *path) {
    refresh();
    strnlpy(config_path, path, BUFLEN);
    parse(fopen(config_path, "r"));
}

#endif
