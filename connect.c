#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>

#define VERSION "0.0.1"

// connect -t protocol [-u user] [-p pass] [-h host] [-P port] [-v volumn]
// connect protocol://[use:[pass]@]host[:port]/[:volumn]

#define DEFAULT_HOST "localhost"
#define USAGE "Usage:\n  %s -t protocol [-u user] [-p pass] [-h host] [-P port] [-v volumn]\n  %s protocol://[user:[pass]@]host[:port]/[:volumn]\n"

#define SCRIPT_PREFIX "tell app \"Finder\" to open location \""
#define SCRIPT_SUFFIX "\""
#define SCRIPT_LENGTH 2048
#define URL_LENGTH (SCRIPT_LENGTH - sizeof(SCRIPT_PREFIX) - sizeof(SCRIPT_SUFFIX) + 1)

static int show_usage = 0, show_version = 0;

static char osascript[SCRIPT_LENGTH] = SCRIPT_PREFIX;
static char *url = osascript + sizeof(SCRIPT_PREFIX) - 1;
static struct option options[] = {
    {"protocol", required_argument, NULL, 't'},
    {"user", optional_argument, NULL, 'u'},
    {"pass", optional_argument, NULL, 'p'},
    {"host", optional_argument, NULL, 'h'},
    {"port", optional_argument, NULL, 'P'},
    {"volumn", optional_argument, NULL, 'v'},
    {"help", no_argument, &show_usage, 1},
    {"version", no_argument, &show_version, 1},
    {NULL, 0, NULL, 0},
};

static void handle_opts(int argc, char *argv[]);
static void exec_script();
static void opts_err(char *arg);
static void unknown_err();
static void print_usage(char *arg);
static void print_version();

int main(int argc, char *argv[]) {
    handle_opts(argc, argv);
    strncat(osascript, SCRIPT_SUFFIX, sizeof(SCRIPT_SUFFIX));
    exec_script();
    return 0;
}

static void handle_opts(int argc, char *argv[]) {
    char c, *protocol = NULL, *user = NULL, *pass = NULL, *host = NULL, *port = NULL, *volumn = NULL;
    int err = 0, index = 0;
    while((c = getopt_long(argc, argv, "t:u:p:h:P:v:", options, &index)) != -1) {
        switch (c) {
            case 't': protocol = optarg; break;
            case 'u': user = optarg; break;
            case 'p': pass = optarg; break;
            case 'h': host = optarg; break;
            case 'P': port = optarg; break;
            case 'v': volumn = optarg; break;
            case '?': err = 1; break;
        }
    }

    if (show_usage) print_usage(argv[0]);
    if (show_version) print_version();

    if (err > 0 || optind < argc || argc == 1) opts_err(argv[0]);
    else if (optind == 1) {
        if (protocol == NULL && user == NULL && pass == NULL && host == NULL && port == NULL && volumn == NULL)
            strncpy(url, (const char *)argv[optind], strlen(argv[optind]));
        else
            opts_err(argv[0]);
    } else {
        if (protocol == NULL || (user == NULL && pass != NULL)) opts_err(argv[0]);
        else {
            int protocol_len, user_len, pass_len, host_len, port_len, volumn_len, url_len;

            if (host == NULL) host = DEFAULT_HOST;

            protocol_len = strlen(protocol);
            if (user != NULL) user_len = strlen(user);
            if (pass != NULL) pass_len = strlen(pass);
            host_len = strlen(host);
            if (port != NULL) port_len = strlen(port);
            if (volumn != NULL) volumn_len = strlen(volumn);

            url_len = protocol_len + sizeof("://");
            if (user != NULL) {
                url_len += user_len + sizeof(":");
                if (pass != NULL) url_len += pass_len;
                url_len += sizeof("@");
            }
            url_len += host_len;
            if (port != NULL) url_len += sizeof(":") + port_len;
            if (volumn != NULL) url_len += sizeof("/") + volumn_len;

            if (url_len > URL_LENGTH) opts_err(argv[0]);

            strncat(url, protocol, protocol_len);
            strncat(url, "://", sizeof("://"));
            if (user != NULL) {
                strncat(url, user, user_len);
                strncat(url, ":", sizeof(":"));
                if (pass != NULL) strncat(url, pass, pass_len);
                strncat(url, "@", sizeof("@"));
            }
            strncat(url, host, host_len);
            if (port != NULL) {
                strncat(url, ":", sizeof(":"));
                strncat(url, port, port_len);
            }
            if (volumn != NULL) {
                strncat(url, "/", sizeof("/"));
                strncat(url, volumn, volumn_len);
            }
        }
    }
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
    char message[256];
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
                if (read(fds[0], message, sizeof(message)) == -1) unknown_err();
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
