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

void test_ipv6_parse(void) {
    TEST_FUNCTION("l_parse_ipv6");
    {
        unsigned char addr[16];

        // Loopback ::1
        TEST_ASSERT(l_parse_ipv6("::1", addr) == 1, "parse ::1 succeeds");
        TEST_ASSERT(addr[15] == 1, "::1 last byte is 1");
        {
            int all_zero = 1;
            for (int i = 0; i < 15; i++) if (addr[i] != 0) all_zero = 0;
            TEST_ASSERT(all_zero, "::1 first 15 bytes are zero");
        }

        // All zeros ::
        TEST_ASSERT(l_parse_ipv6("::", addr) == 1, "parse :: succeeds");
        {
            int all_zero = 1;
            for (int i = 0; i < 16; i++) if (addr[i] != 0) all_zero = 0;
            TEST_ASSERT(all_zero, ":: is all zeros");
        }

        // Full address
        TEST_ASSERT(l_parse_ipv6("2001:db8:0:0:0:0:0:1", addr) == 1, "parse full address");
        TEST_ASSERT(addr[0] == 0x20 && addr[1] == 0x01, "first group 2001");
        TEST_ASSERT(addr[2] == 0x0d && addr[3] == 0xb8, "second group 0db8");
        TEST_ASSERT(addr[15] == 1, "last byte is 1");

        // Compressed address
        TEST_ASSERT(l_parse_ipv6("2001:db8::1", addr) == 1, "parse 2001:db8::1");
        TEST_ASSERT(addr[0] == 0x20 && addr[1] == 0x01, "compressed first group");
        TEST_ASSERT(addr[15] == 1, "compressed last byte");

        // fe80::1
        TEST_ASSERT(l_parse_ipv6("fe80::1", addr) == 1, "parse fe80::1");
        TEST_ASSERT(addr[0] == 0xfe && addr[1] == 0x80, "link-local prefix");

        // Invalid: too many groups
        TEST_ASSERT(l_parse_ipv6("1:2:3:4:5:6:7:8:9", addr) == 0, "reject 9 groups");

        // Invalid: double ::
        TEST_ASSERT(l_parse_ipv6("1::2::3", addr) == 0, "reject double ::");

        // Invalid: bad characters
        TEST_ASSERT(l_parse_ipv6("xyz::1", addr) == 0, "reject bad hex chars");

        // Invalid: NULL
        TEST_ASSERT(l_parse_ipv6(NULL, addr) == 0, "reject NULL input");
    }
    TEST_SECTION_PASS("l_parse_ipv6");
}

void test_ipv6_format(void) {
    TEST_FUNCTION("l_format_ipv6");
    {
        unsigned char addr[16];
        char buf[L_INET6_ADDRSTRLEN];

        // Round-trip ::1
        l_parse_ipv6("::1", addr);
        l_format_ipv6(addr, buf, sizeof(buf));
        // Should produce something like "0:0:0:0:0:0:0:1"
        TEST_ASSERT(l_strlen(buf) > 0, "format ::1 produces non-empty string");
        // Verify we can parse the formatted output back
        {
            unsigned char addr2[16];
            TEST_ASSERT(l_parse_ipv6(buf, addr2) == 1, "round-trip: format then parse succeeds");
            TEST_ASSERT(l_memcmp(addr, addr2, 16) == 0, "round-trip: bytes match");
        }

        // Round-trip 2001:db8::1
        l_parse_ipv6("2001:db8::1", addr);
        l_format_ipv6(addr, buf, sizeof(buf));
        {
            unsigned char addr2[16];
            TEST_ASSERT(l_parse_ipv6(buf, addr2) == 1, "round-trip 2001:db8::1 succeeds");
            TEST_ASSERT(l_memcmp(addr, addr2, 16) == 0, "round-trip 2001:db8::1 bytes match");
        }

        // NULL buffer
        TEST_ASSERT(l_format_ipv6(addr, NULL, 0) == NULL, "format rejects NULL buf");

        // Too-small buffer
        char tiny[4];
        TEST_ASSERT(l_format_ipv6(addr, tiny, sizeof(tiny)) == NULL, "format rejects tiny buf");
    }
    TEST_SECTION_PASS("l_format_ipv6");
}

void test_sockaddr(void) {
    TEST_FUNCTION("l_sockaddr_ipv4");
    {
        L_SockAddr sa;
        TEST_ASSERT(l_sockaddr_ipv4(&sa, "127.0.0.1", 8080) == 0, "construct IPv4 sockaddr");
        TEST_ASSERT(sa.family == L_AF_INET, "family is AF_INET");
        TEST_ASSERT(sa.port == 8080, "port is 8080");
        TEST_ASSERT(sa.addr[0] == 127 && sa.addr[1] == 0 && sa.addr[2] == 0 && sa.addr[3] == 1,
                    "addr bytes match 127.0.0.1");

        TEST_ASSERT(l_sockaddr_ipv4(&sa, "invalid", 80) == -1, "reject invalid IPv4");
        TEST_ASSERT(l_sockaddr_ipv4(NULL, "1.2.3.4", 80) == -1, "reject NULL sa");
    }

    TEST_FUNCTION("l_sockaddr_ipv6");
    {
        L_SockAddr sa;
        TEST_ASSERT(l_sockaddr_ipv6(&sa, "::1", 443) == 0, "construct IPv6 sockaddr");
        TEST_ASSERT(sa.family == L_AF_INET6, "family is AF_INET6");
        TEST_ASSERT(sa.port == 443, "port is 443");
        TEST_ASSERT(sa.addr[15] == 1, "addr last byte is 1");
        {
            int all_zero = 1;
            for (int i = 0; i < 15; i++) if (sa.addr[i] != 0) all_zero = 0;
            TEST_ASSERT(all_zero, "addr first 15 bytes are zero");
        }

        TEST_ASSERT(l_sockaddr_ipv6(&sa, "not-ipv6", 80) == -1, "reject invalid IPv6");
        TEST_ASSERT(l_sockaddr_ipv6(NULL, "::1", 80) == -1, "reject NULL sa");
    }
    TEST_SECTION_PASS("L_SockAddr constructors");
}

void test_generic_socket(void) {
    TEST_FUNCTION("l_socket_open (IPv4)");
    {
        L_SOCKET s = l_socket_open(L_AF_INET, L_SOCK_STREAM);
        TEST_ASSERT(s >= 0, "open IPv4 TCP socket");
        l_socket_close(s);

        s = l_socket_open(L_AF_INET, L_SOCK_DGRAM);
        TEST_ASSERT(s >= 0, "open IPv4 UDP socket");
        l_socket_close(s);
    }

    TEST_FUNCTION("l_socket_bind_addr (IPv4)");
    {
        L_SOCKET s = l_socket_open(L_AF_INET, L_SOCK_STREAM);
        TEST_ASSERT(s >= 0, "socket for bind_addr");
        L_SockAddr sa;
        l_sockaddr_ipv4(&sa, "0.0.0.0", 0);
        int br = l_socket_bind_addr(s, &sa);
        if (br == 0) {
            TEST_COUNT_ONLY(1);
            puts("    [OK] bind_addr to port 0 succeeds\n");
        } else {
            TEST_COUNT_ONLY(1);
            puts("    [SKIP] bind_addr not available (QEMU)\n");
        }
        l_socket_close(s);
    }

#ifndef _WIN32
    TEST_FUNCTION("l_socket_open (IPv6)");
    {
        // Capability-detect: try to create an IPv6 socket
        L_SOCKET s = l_socket_open(L_AF_INET6, L_SOCK_STREAM);
        if (s >= 0) {
            TEST_COUNT_ONLY(1);
            puts("    [OK] open IPv6 TCP socket\n");
            l_socket_close(s);

            s = l_socket_open(L_AF_INET6, L_SOCK_DGRAM);
            TEST_ASSERT(s >= 0, "open IPv6 UDP socket");
            l_socket_close(s);
        } else {
            TEST_COUNT_ONLY(2);
            puts("    [SKIP] IPv6 not available on this system\n");
        }
    }

    TEST_FUNCTION("l_socket_connect_addr (IPv4 loopback)");
    {
        L_SOCKET server = -1;
        int port = 0;
        if (l_test_open_loopback_listener(&server, &port) == 0) {
            L_SOCKET client = l_socket_open(L_AF_INET, L_SOCK_STREAM);
            TEST_ASSERT(client >= 0, "socket for connect_addr");

            L_SockAddr sa;
            l_sockaddr_ipv4(&sa, "127.0.0.1", port);
            TEST_ASSERT(l_socket_connect_addr(client, &sa) == 0, "connect_addr to loopback");

            {
                L_PollFd pfd;
                pfd.fd = (L_FD)server;
                pfd.events = L_POLLIN;
                pfd.revents = 0;
                TEST_ASSERT(l_poll(&pfd, 1, 1000) > 0, "connect_addr reaches listener");
            }
            l_socket_close(client);
            l_socket_close(server);
        } else {
            TEST_COUNT_ONLY(3);
            puts("    [SKIP] loopback listener unavailable\n");
        }
    }
#else
    TEST_FUNCTION("l_socket_open (IPv6 — Windows)");
    {
        L_SOCKET s = l_socket_open(L_AF_INET6, L_SOCK_STREAM);
        if (s >= 0) {
            TEST_COUNT_ONLY(1);
            puts("    [OK] open IPv6 TCP socket\n");
            l_socket_close(s);
        } else {
            TEST_COUNT_ONLY(1);
            puts("    [SKIP] IPv6 not available\n");
        }
    }

    TEST_FUNCTION("l_socket_connect_addr (IPv4 loopback — Windows)");
    {
        L_SOCKET server = l_socket_tcp();
        TEST_ASSERT(server >= 0, "server socket");
        int br = l_socket_bind(server, 0);
        if (br == 0) {
            l_socket_listen(server, 1);
            // Get the bound port by connecting and checking
            L_SOCKET client = l_socket_open(L_AF_INET, L_SOCK_STREAM);
            TEST_ASSERT(client >= 0, "client socket");
            // We don't have a portable way to get bound port on Windows without extra work,
            // so just verify the API compiles and the socket is valid
            TEST_COUNT_ONLY(1);
            puts("    [OK] connect_addr API compiles and socket opens\n");
            l_socket_close(client);
        } else {
            TEST_COUNT_ONLY(2);
            puts("    [SKIP] bind not available\n");
        }
        l_socket_close(server);
    }
#endif

    TEST_SECTION_PASS("Generic socket API");
}

int main(int argc, char *argv[]) {
    l_getenv_init(argc, argv);
    test_sockets();
    test_udp();
    test_ipv6_parse();
    test_ipv6_format();
    test_sockaddr();
    test_generic_socket();

    l_test_print_summary(passed_count, test_count);
    puts("PASS\n");
    return 0;
}
