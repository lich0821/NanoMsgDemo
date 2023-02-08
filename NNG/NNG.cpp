/*
https://nanomsg.org/gettingstarted/nng/reqrep.html
./reqrep node0 ipc:///tmp/reqrep.ipc & node0=$! && sleep 1
./reqrep node1 ipc:///tmp/reqrep.ipc
kill $node0

Windows usage(typo in the name):
./reqprep "node0" "ipc:///tmp/reqrep.ipc"
./reqprep "node1" "ipc:///tmp/reqrep.ipc"

*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef __unix__
#include <netinet/in.h> /* For htonl and ntohl */
#include <unistd.h>
#elif defined _WIN32
#include <time.h>
#include <windows.h>
#include <winsock.h>
#define sleep(x) Sleep(1000 * (x))
#define getpid() GetCurrentProcessId()
#endif

#include <nng/nng.h>
#include <nng/protocol/reqrep0/rep.h>
#include <nng/protocol/reqrep0/req.h>

#define SERVER "server"
#define CLIENT "client"
#define DATE  "DATE"

void fatal(const char *func, int rv)
{
    fprintf(stderr, "%s: %s\n", func, nng_strerror(rv));
    exit(1);
}

char *date(void)
{
    time_t now             = time(&now);
    struct tm *info        = localtime(&now);
    char *text             = asctime(info);
    text[strlen(text) - 1] = '\0'; // remove '\n'
    return (text);
}

int node0(const char *url)
{
    nng_socket sock;
    int rv;

    if ((rv = nng_rep0_open(&sock)) != 0) {
        fatal("nng_rep0_open", rv);
    }
    if ((rv = nng_listen(sock, url, NULL, 0)) != 0) {
        fatal("nng_listen", rv);
    }
    for (;;) {
        char *buf = NULL;
        size_t sz;
        if ((rv = nng_recv(sock, &buf, &sz, NNG_FLAG_ALLOC)) != 0) {
            fatal("nng_recv", rv);
        }
        Sleep(5000);
        if ((sz == (strlen(DATE) + 1)) && (strcmp(DATE, buf) == 0)) {
            printf("SERVER: RECEIVED DATE REQUEST\n");
            char *d = date();
            printf("SERVER: SENDING DATE %s\n", d);
            if ((rv = nng_send(sock, d, strlen(d) + 1, 0)) != 0) {
                fatal("nng_send", rv);
            }
        }
        nng_free(buf, sz);
    }
}

int node1(const char *url)
{
    nng_socket sock;
    int rv;
    size_t sz;
    char *buf = NULL;

    if ((rv = nng_req0_open(&sock)) != 0) {
        fatal("nng_socket", rv);
    }
    if ((rv = nng_dial(sock, url, NULL, 0)) != 0) {
        fatal("nng_dial", rv);
    }
    printf("CLIENT: SENDING DATE REQUEST %s\n", DATE);
    if ((rv = nng_send(sock, (void *)DATE, strlen(DATE) + 1, 0)) != 0) {
        fatal("nng_send", rv);
    }
    if ((rv = nng_recv(sock, &buf, &sz, NNG_FLAG_ALLOC)) != 0) {
        fatal("nng_recv", rv);
    }
    printf("CLIENT: RECEIVED DATE %s\n", buf);
    nng_free(buf, sz);
    nng_close(sock);
    return (0);
}

int main(const int argc, const char **argv)
{
    if ((argc > 1) && (strcmp(SERVER, argv[1]) == 0))
        return (node0(argv[2]));

    if ((argc > 1) && (strcmp(CLIENT, argv[1]) == 0))
        return (node1(argv[2]));

    fprintf(stderr, "Usage: reqrep %s|%s <URL> ...\n", SERVER, CLIENT);
    return (1);
}
