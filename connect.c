#include <getopt.h>
#include "common.h"

#define VERSION "0.0.1"

// connect -t protocol [-u user] [-p pass] [-h host] [-P port] [-v volumn]
// connect protocol://[use:[pass]@]host[:port]/[:volumn]

#define USAGE "Usage:\n  %s -t protocol [-u user] [-p pass] [-h host] [-P port] [-v volumn]\n  %s protocol://[user:[pass]@]host[:port]/[:volumn]\n"

#define SCRIPT_PREFIX "tell app \"Finder\" to open location \""
#define SCRIPT_SUFFIX "\""
#define SCRIPT_LENGTH 2048
#define BUFLEN 256

static int show_usage = 0, show_version = 0, show_command = 0;

static char osascript[SCRIPT_LENGTH] = SCRIPT_PREFIX;
static char *location = osascript + strlen(SCRIPT_PREFIX);
static struct option options[] = {
    {"protocol", required_argument, NULL, 't'},
    {"user", optional_argument, NULL, 'u'},
    {"pass", optional_argument, NULL, 'p'},
    {"host", optional_argument, NULL, 'h'},
    {"port", optional_argument, NULL, 'P'},
    {"volumn", optional_argument, NULL, 'v'},
    {"help", no_argument, &show_usage, 1},
    {"version", no_argument, &show_version, 1},
    {"dry-run", no_argument, &show_command, 1},
    {NULL, 0, NULL, 0},
};

extern struct config *configs;
void parse(FILE *config_file);
FILE* find_config();

static struct config* find_by(char *name);
static void handle_opts(int argc, char *argv[]);
static int validate(struct config_entry *entry);
static void exec_script();
static void opts_err(char *arg);
static void unknown_err();
static void print_usage(char *arg);
static void print_version();

int main(int argc, char *argv[]) {
    parse(find_config());
    handle_opts(argc, argv);
    strlcat(osascript, SCRIPT_SUFFIX, SCRIPT_LENGTH);
    if (show_command)
        printf("%s\n", osascript);
    else
        exec_script();
    return 0;
}

static struct config* find_by(char *name) {
    struct config* ptr;
    for (ptr = configs; ptr != NULL; ptr = ptr->next)
        if (ENTRYCMP(name, ptr->name, NAME_LENGTH) == 0) return ptr;
    return NULL;
}

static void handle_opts(int argc, char *argv[]) {
    char c;
    int err = 0, index = 0;
    struct config_entry entry = {0};
    while((c = getopt_long(argc, argv, "t:u:p:h:P:v:", options, &index)) != -1) {
        switch (c) {
            case 't': entry.protocol = optarg; break;
            case 'u': entry.user = optarg; break;
            case 'p': entry.pass = optarg; break;
            case 'h': entry.host = optarg; break;
            case 'P': entry.port = optarg; break;
            case 'v': entry.volumn = optarg; break;
            case '?': err = 1; break;
        }
    }

    if (show_usage) print_usage(argv[0]);
    if (show_version) print_version();

    if (err > 0 || (argc > 2 && optind < argc) || argc == 1) opts_err(argv[0]);
    else if (optind == 1) {
        if (entry_is_null(&entry)) {
            struct config *find = find_by(argv[optind]);
            if (find == NULL)
                strlcpy(location, (const char *)argv[optind], URL_LENGTH);
            else
                strlcpy(location, find->url, URL_LENGTH);
        }
        else
            opts_err(argv[0]);
    } else {
        if (validate(&entry) == 0)
            opts_err(argv[0]);
        else
            to_url(location, &entry);
    }
}

static int validate(struct config_entry *entry) {
    return !(entry->protocol == NULL || (entry->user == NULL && entry->pass != NULL));
}

static void opts_err(char *arg) {
    fprintf(stderr, USAGE, arg, arg);
    exit(EXIT_FAILURE);
}

static void print_usage(char *arg) {
    printf(USAGE, arg, arg);
    exit(EXIT_SUCCESS);
}

static void print_version() {
    printf("%s\n", VERSION);
    exit(EXIT_SUCCESS);
}

static void exec_script() {
    int status, fds[2];
    char message[BUFLEN];
    pid_t pid;

    if (pipe(fds) == -1) unknown_err();

    switch (pid = fork()) {
        case -1:
            unknown_err();
        case 0:
            if (close(fds[0]) == -1) unknown_err();
            if (dup2(fds[1], STDERR_FILENO) == -1) unknown_err();
            execlp("osascript", "osascript", "-e", osascript, NULL);
            unknown_err();
        default:
            if (close(fds[1]) == -1) unknown_err();
            waitpid(pid, &status, 0);
            if (WEXITSTATUS(status) != 0) {
                if (read(fds[0], message, BUFLEN) == -1) unknown_err();
                fprintf(stderr, "Error in apple script: %s\nMessage: %s", osascript, message);
                exit(EXIT_FAILURE);
            } else
                exit(EXIT_SUCCESS);
    }
}

void unknown_err() {
    perror("Unknown error");
    exit(EXIT_FAILURE);
}
