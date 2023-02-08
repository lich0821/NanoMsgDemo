// This is a port of the nng demo to nngpp
// See https://github.com/nanomsg/nng/tree/master/demo/reqrep
/*
g++ nngpp_test.cpp -o nngpp_test -lnng -lpthread -std=c++11 -Wno-deprecated-declarations

./nngpp_test server "tcp://127.0.0.1:5555"
./nngpp_test client "tcp://127.0.0.1:5555"
*/
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <nngpp/nngpp.h>
#include <nngpp/protocol/rep0.h>
#include <nngpp/protocol/req0.h>

#define CLIENT  "client"
#define SERVER  "server"
#define DATECMD 1

#define PUT64(ptr, u)                                                                                                  \
    do {                                                                                                               \
        (ptr)[0] = (uint8_t)(((uint64_t)(u)) >> 56);                                                                   \
        (ptr)[1] = (uint8_t)(((uint64_t)(u)) >> 48);                                                                   \
        (ptr)[2] = (uint8_t)(((uint64_t)(u)) >> 40);                                                                   \
        (ptr)[3] = (uint8_t)(((uint64_t)(u)) >> 32);                                                                   \
        (ptr)[4] = (uint8_t)(((uint64_t)(u)) >> 24);                                                                   \
        (ptr)[5] = (uint8_t)(((uint64_t)(u)) >> 16);                                                                   \
        (ptr)[6] = (uint8_t)(((uint64_t)(u)) >> 8);                                                                    \
        (ptr)[7] = (uint8_t)((uint64_t)(u));                                                                           \
    } while (0)

#define GET64(ptr, v)                                                                                                  \
    v = (((uint64_t)((uint8_t)(ptr)[0])) << 56) + (((uint64_t)((uint8_t)(ptr)[1])) << 48)                              \
        + (((uint64_t)((uint8_t)(ptr)[2])) << 40) + (((uint64_t)((uint8_t)(ptr)[3])) << 32)                            \
        + (((uint64_t)((uint8_t)(ptr)[4])) << 24) + (((uint64_t)((uint8_t)(ptr)[5])) << 16)                            \
        + (((uint64_t)((uint8_t)(ptr)[6])) << 8) + (((uint64_t)(uint8_t)(ptr)[7]))

#define FUNC_IS_LOGIN        0x01
#define FUNC_ENABLE_RECV_MSG 0x02
#define FUNC_GET_DATE        0x03

#pragma pack(push, 1)
typedef struct RPC {
    uint8_t func;
    uint8_t* args;
} RPC_t;
#pragma pack(pop)

using nng::buffer;

void showdate(time_t now) { printf("%s", asctime(localtime(&now))); }

buffer func_is_login()
{
    buffer ret(1);
    ret.data<uint8_t>()[0] = 0x00;

    return ret;
}

buffer func_enable_recv_msg(void)
{
    buffer ret(1);
    ret.data<uint8_t>()[0] = 0x00;

    return ret;
}

buffer func_get_date(buffer msg)
{
    buffer ret(sizeof(uint64_t));
    auto now = time(nullptr);
    showdate(now);

    PUT64(ret.data<char>(), (uint64_t)now);
    return ret;
}

buffer dispatcher(buffer msg)
{
    buffer ret(1);
    ret.data<uint8_t>()[0] = (uint8_t)-1;
    switch (msg.data<uint8_t>()[0]) {
    case FUNC_IS_LOGIN:
        printf("FUNC_IS_LOGIN\n");
        return func_is_login();

    case FUNC_ENABLE_RECV_MSG:
        printf("FUNC_ENABLE_RECV_MSG\n");
        return func_enable_recv_msg();

    case FUNC_GET_DATE:
        printf("FUNC_GET_DATE\n");
        return func_get_date(msg);

    default:
        printf("UNKNOW FUNCTION: %02X\n", msg.data<uint8_t>()[0]);
    }
    return ret;
}

void server(const char* url)
{
    auto sock = nng::rep::open();
    sock.listen(url);
    while (true) {
        auto msg = sock.recv();
        printf("msg size: %ld\n", msg.size());

        auto rsp = dispatcher(msg);
        printf("rsp size: %ld\n", rsp.size());

        sock.send(rsp);
    }
}

void client(const char* url, uint8_t func)
{
    auto sock = nng::req::open();
    sock.dial(url);

    printf("CLIENT: SENDING REQUEST\n");

    buffer cmd(1);
    cmd.data<uint8_t>()[0] = func;

    sock.send(cmd);

    auto buf = sock.recv();
    printf("CLIENT: RECEIVED: ");
    if (buf.size() == sizeof(uint64_t)) {
        uint64_t now;
        GET64(buf.data<char>(), now);
        showdate((time_t)now);
    }
    else {
        uint8_t* p = buf.data<uint8_t>();
        for (size_t i = 0; i < buf.size(); i++) {
            printf("%02X", p[i]);
        }
        printf("\n");
    }
}

int main(int argc, char** argv)
{
    try {
        if (argc > 1 && strcmp(CLIENT, argv[1]) == 0) {
            uint8_t func = 0;
            if (argc == 4) {
                func = atoi(argv[3]);
            }
            client(argv[2], func);
            return 0;
        }

        if (argc > 1 && strcmp(SERVER, argv[1]) == 0) {
            server(argv[2]);
            return 0;
        }

        fprintf(stderr, "Usage: reqrep %s|%s <URL> ...\n", CLIENT, SERVER);
        return 1;
    }
    catch (const nng::exception& e) {
        fprintf(stderr, "%s: %s\n", e.who(), e.what());
        return 1;
    }
}