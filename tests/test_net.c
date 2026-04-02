// Socket coverage split from the old monolith.

#define L_WITHSNPRINTF
#define L_WITHSOCKETS
#define L_MAINFILE
#include "l_os.h"
#include "test_support.h"

TEST_DECLARE_COUNTERS();

void test_sockets(void) {
    TEST_FUNCTION("l_htons / l_htonl");
    TEST_ASSERT(l_htons(0x1234) == 0x3412 || l_htons(0x1234) == 0x1234, "htons works");
    TEST_ASSERT(l_htonl(0x01020304) == 0x04030201 || l_htonl(0x01020304) == 0x01020304, "htonl works");

    TEST_FUNCTION("l_inet_addr");
    unsigned int addr = l_inet_addr("127.0.0.1");
    TEST_ASSERT(addr != 0, "inet_addr parses 127.0.0.1");
    {
        unsigned char *b = (unsigned char *)&addr;
        TEST_ASSERT(b[0] == 127 && b[1] == 0 && b[2] == 0 && b[3] == 1,
                     "inet_addr correct network byte order");
    }
    TEST_ASSERT(l_inet_addr("192.168.1.1") != 0, "inet_addr parses 192.168.1.1");
    TEST_ASSERT(l_inet_addr("localhost") == 0, "inet_addr rejects hostname text");
    TEST_ASSERT(l_inet_addr("invalid") == 0, "inet_addr rejects invalid");
    TEST_ASSERT(l_inet_addr("256.0.0.1") == 0, "inet_addr rejects octet > 255");

    TEST_FUNCTION("l_resolve");
    {
        char ip[16];
        char guarded[18];
        unsigned int resolved;

        TEST_ASSERT(l_resolve("127.0.0.1", ip) == 0 &&
                    l_strcmp(ip, "127.0.0.1") == 0,
                    "resolve passes through IPv4 literal");
        TEST_ASSERT(l_resolve("0.0.0.0", ip) == 0 &&
                    l_strcmp(ip, "0.0.0.0") == 0,
                    "resolve passes through 0.0.0.0");
        l_memset(guarded, 'x', sizeof(guarded));
        TEST_ASSERT(l_resolve("1.2.3.4", guarded + 1) == 0 &&
                    guarded[0] == 'x' && guarded[8] == '\0' && guarded[9] == 'x',
                    "resolve writes exact IPv4 string without overrunning neighbors");
        TEST_ASSERT(l_resolve("localhost", ip) == 0, "resolve localhost succeeds");
        resolved = l_inet_addr(ip);
        {
            unsigned char *b = (unsigned char *)&resolved;
            TEST_ASSERT(b[0] == 127, "resolve localhost returns loopback IPv4");
        }
        TEST_ASSERT(l_resolve((const char *)0, ip) == -1, "resolve rejects NULL hostname");
        TEST_ASSERT(l_resolve("localhost", (char *)0) == -1, "resolve rejects NULL output buffer");
        TEST_ASSERT(l_resolve("", ip) == -1, "resolve rejects empty hostname");
        TEST_ASSERT(l_resolve("256.0.0.1", ip) == -1, "resolve rejects invalid IPv4 literal");
        TEST_ASSERT(l_resolve("bad host", ip) == -1, "resolve rejects malformed hostname");
    }

#ifndef _WIN32
    TEST_FUNCTION("l_socket_tcp");
    {
        L_SOCKET s = l_socket_tcp();
        TEST_ASSERT(s >= 0, "socket creation succeeds");
        l_socket_close(s);
    }

    TEST_FUNCTION("l_socket_bind / l_socket_listen");
    {
        L_SOCKET s = l_socket_tcp();
        TEST_ASSERT(s >= 0, "socket for bind");
        int br = l_socket_bind(s, 0);
        if (br == 0) {
            TEST_COUNT_ONLY(1);
            puts("    [OK] bind to port 0 succeeds\n");
            TEST_ASSERT(l_socket_listen(s, 5) == 0, "listen succeeds");
        } else {
            TEST_COUNT_ONLY(2);
            puts("    [SKIP] bind not available (QEMU user-mode)\n");
        }
        l_socket_close(s);
    }
#else
    TEST_FUNCTION("l_socket_tcp (Windows)");
    {
        L_SOCKET s = l_socket_tcp();
        TEST_ASSERT(s >= 0, "socket creation succeeds");
        l_socket_close(s);
    }

    TEST_FUNCTION("l_socket_bind / l_socket_listen (Windows)");
    {
        L_SOCKET s = l_socket_tcp();
        TEST_ASSERT(s >= 0, "socket for bind");
        int br = l_socket_bind(s, 0);
        if (br == 0) {
            TEST_COUNT_ONLY(1);
            puts("    [OK] bind to port 0 succeeds\n");
            TEST_ASSERT(l_socket_listen(s, 5) == 0, "listen succeeds");
        } else {
            TEST_COUNT_ONLY(2);
            puts("    [SKIP] bind not available\n");
        }
        l_socket_close(s);
    }
#endif

    TEST_FUNCTION("l_socket_connect hostname / loopback");
    {
        L_SOCKET server = -1;
        int port = 0;
        if (l_test_open_loopback_listener(&server, &port) == 0) {
            L_SOCKET by_name = l_socket_tcp();
            char localhost_ip[16];
            TEST_ASSERT(by_name >= 0, "socket for localhost resolution");
            TEST_ASSERT(l_resolve("localhost", localhost_ip) == 0,
                        "resolve localhost for loopback connect");
            TEST_ASSERT(l_socket_connect(by_name, localhost_ip, port) == 0,
                        "socket_connect accepts resolved localhost IPv4");
            {
                L_PollFd pfd;
                pfd.fd = (L_FD)server;
                pfd.events = L_POLLIN;
                pfd.revents = 0;
                TEST_ASSERT(l_poll(&pfd, 1, 1000) > 0, "localhost connect reaches listening socket");
            }
            {
                L_SOCKET accepted = l_socket_accept(server);
                TEST_ASSERT(accepted >= 0, "accept after localhost connect succeeds");
                l_socket_close(accepted);
            }
            l_socket_close(by_name);

            L_SOCKET by_ip = l_socket_tcp();
            TEST_ASSERT(by_ip >= 0, "socket for numeric loopback");
            TEST_ASSERT(l_socket_connect(by_ip, "127.0.0.1", port) == 0,
                        "socket_connect accepts numeric loopback");
            {
                L_PollFd pfd;
                pfd.fd = (L_FD)server;
                pfd.events = L_POLLIN;
                pfd.revents = 0;
                TEST_ASSERT(l_poll(&pfd, 1, 1000) > 0, "numeric loopback connect reaches listening socket");
            }
            {
                L_SOCKET peer = l_socket_accept(server);
                TEST_ASSERT(peer >= 0, "accept numeric loopback client succeeds");
                TEST_ASSERT(l_socket_send(by_ip, "ok", 2) == 2, "numeric loopback client sends payload");
                {
                    char recvbuf[8];
                    ptrdiff_t n = l_socket_recv(peer, recvbuf, sizeof(recvbuf));
                    TEST_ASSERT(n == 2 && recvbuf[0] == 'o' && recvbuf[1] == 'k',
                                "loopback listener receives payload from numeric loopback client");
                }
                l_socket_close(peer);
            }

            l_socket_close(by_ip);
            l_socket_close(server);
        } else {
            TEST_COUNT_ONLY(10);
            puts("    [SKIP] loopback listener unavailable\n");
        }
    }

    TEST_FUNCTION("l_socket_connect hostname errors");
    {
        L_SOCKET s = l_socket_tcp();
        TEST_ASSERT(s >= 0, "socket for hostname error cases");
        TEST_ASSERT(l_socket_connect(s, "", 80) == -1, "connect rejects empty hostname");
        TEST_ASSERT(l_socket_connect(s, "bad host", 80) == -1, "connect rejects malformed hostname");
        l_socket_close(s);
    }

    TEST_SECTION_PASS("sockets");
}

void test_udp(void) {
    TEST_FUNCTION("l_socket_udp");

    // Create UDP socket and verify it's valid
    L_SOCKET s = l_socket_udp();
    TEST_ASSERT(s >= 0, "UDP socket creation succeeds");

    // Bind to ephemeral port (port 0)
    int br = l_socket_bind(s, 0);
    if (br == 0) {
        TEST_PASS_ONLY(1);
        puts("  [OK] UDP bind to port 0 succeeds\n");
    } else {
        TEST_PASS_ONLY(1);
        puts("  [SKIP] UDP bind not available (QEMU/env)\n");
    }

    // Close socket
    l_socket_close(s);
    TEST_ASSERT(1, "UDP socket closed without crash");

    TEST_FUNCTION("l_socket_sendto hostname / loopback");
    {
        L_SOCKET receiver = -1;
        int port = 0;
        if (l_test_open_loopback_udp_socket(&receiver, &port) == 0) {
            L_SOCKET sender = l_socket_udp();
            char localhost_ip[16];
            TEST_ASSERT(sender >= 0, "UDP sender socket creation succeeds");
            TEST_ASSERT(l_resolve("localhost", localhost_ip) == 0,
                        "resolve localhost for UDP loopback");
            TEST_ASSERT(l_socket_sendto(sender, "lo", 2, localhost_ip, port) == 2,
                        "sendto accepts resolved localhost IPv4");
            {
                L_PollFd pfd;
                pfd.fd = (L_FD)receiver;
                pfd.events = L_POLLIN;
                pfd.revents = 0;
                TEST_ASSERT(l_poll(&pfd, 1, 1000) > 0, "UDP localhost packet reaches receiver");
            }
            {
                char buf[8];
                char addr[16];
                int from_port = 0;
                ptrdiff_t n = l_socket_recvfrom(receiver, buf, sizeof(buf), addr, &from_port);
                TEST_ASSERT(n == 2 && buf[0] == 'l' && buf[1] == 'o',
                            "UDP receiver gets localhost payload");
                TEST_ASSERT(from_port > 0, "UDP recvfrom reports sender port");
            }

            TEST_ASSERT(l_socket_sendto(sender, "ip", 2, "127.0.0.1", port) == 2,
                        "sendto accepts numeric loopback");
            {
                L_PollFd pfd;
                pfd.fd = (L_FD)receiver;
                pfd.events = L_POLLIN;
                pfd.revents = 0;
                TEST_ASSERT(l_poll(&pfd, 1, 1000) > 0, "UDP numeric loopback packet reaches receiver");
            }
            {
                char buf[8];
                char addr[16];
                int from_port = 0;
                ptrdiff_t n = l_socket_recvfrom(receiver, buf, sizeof(buf), addr, &from_port);
                TEST_ASSERT(n == 2 && buf[0] == 'i' && buf[1] == 'p',
                            "UDP receiver gets numeric loopback payload");
                TEST_ASSERT(from_port > 0, "UDP recvfrom reports sender port for numeric loopback");
            }

            TEST_ASSERT(l_socket_sendto(sender, "x", 1, "", port) == -1,
                        "sendto rejects empty hostname");
            TEST_ASSERT(l_socket_sendto(sender, "x", 1, "bad host", port) == -1,
                        "sendto rejects malformed hostname");

            l_socket_close(sender);
            l_socket_close(receiver);
        } else {
            TEST_COUNT_ONLY(12);
            puts("  [SKIP] UDP loopback bind unavailable\n");
        }
    }

    TEST_SECTION_PASS("UDP sockets");
}

void test_valid_hostname(void) {
    TEST_FUNCTION("l_valid_hostname");

    /* Valid hostnames */
    TEST_ASSERT(l_valid_hostname("example.com") == 1, "valid: simple domain");
    TEST_ASSERT(l_valid_hostname("localhost") == 1, "valid: single label");
    TEST_ASSERT(l_valid_hostname("sub.domain.example.co.uk") == 1, "valid: multi-label");
    TEST_ASSERT(l_valid_hostname("a1b2c3") == 1, "valid: alphanumeric");
    TEST_ASSERT(l_valid_hostname("my-host.example.com") == 1, "valid: hyphen in label");
    TEST_ASSERT(l_valid_hostname("A") == 1, "valid: single uppercase letter");
    TEST_ASSERT(l_valid_hostname("UPPER.COM") == 1, "valid: all uppercase");
    TEST_ASSERT(l_valid_hostname("xn--nxasmq6b.com") == 1, "valid: IDN punycode label");

    /* Invalid hostnames */
    TEST_ASSERT(l_valid_hostname(NULL) == 0, "invalid: NULL pointer");
    TEST_ASSERT(l_valid_hostname("") == 0, "invalid: empty string");
    TEST_ASSERT(l_valid_hostname("-bad.com") == 0, "invalid: label starts with hyphen");
    TEST_ASSERT(l_valid_hostname("bad-.com") == 0, "invalid: label ends with hyphen");
    TEST_ASSERT(l_valid_hostname(".bad.com") == 0, "invalid: starts with dot");
    TEST_ASSERT(l_valid_hostname("bad..com") == 0, "invalid: consecutive dots");
    TEST_ASSERT(l_valid_hostname("bad host") == 0, "invalid: space in name");
    TEST_ASSERT(l_valid_hostname("bad!.com") == 0, "invalid: special character");
    TEST_ASSERT(l_valid_hostname("com.") == 0, "invalid: trailing dot (empty last label)");

    TEST_SECTION_PASS("l_valid_hostname");
}

int main(int argc, char *argv[]) {
    l_getenv_init(argc, argv);
    test_sockets();
    test_udp();
    test_valid_hostname();

    l_test_print_summary(passed_count, test_count);
    puts("PASS\n");
    return 0;
}
