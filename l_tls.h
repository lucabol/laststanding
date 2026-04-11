// l_tls.h — Freestanding TLS client for laststanding
//
// Usage: #define L_MAINFILE
//        #include "l_tls.h"   // pulls in l_os.h automatically
//
// Windows: SChannel (zero external dependencies — uses built-in Windows TLS)
// Linux:   Stub (returns -1; future: BearSSL integration)
// WASI:    Stub (no sockets in WASI Preview 1)
//
// API:
//   l_tls_init()              → 0 on success, -1 on failure
//   l_tls_connect(host, port) → handle (0-7), or -1 on failure
//   l_tls_send(h, data, len)  → bytes sent, or -1 on failure
//   l_tls_recv(h, buf, len)   → bytes read, 0 on close, -1 on failure
//   l_tls_recv_byte(h)        → byte (0-255), or -1 on failure/close
//   l_tls_close(h)            → void
//   l_tls_cleanup()           → void

#ifndef L_TLSH
#define L_TLSH

// TLS requires sockets
#ifndef L_WITHSOCKETS
#define L_WITHSOCKETS
#endif

#include "l_os.h"

#define L_TLS_MAX_CONNECTIONS 8

// Platform availability flag
#ifdef _WIN32
#define L_TLS_AVAILABLE 1
#else
#define L_TLS_AVAILABLE 0
#endif

// ── Windows: SChannel implementation ─────────────────────────────────────────
#ifdef _WIN32

#define SECURITY_WIN32
#include <security.h>
#include <schannel.h>

#if defined(_MSC_VER) || defined(__clang__)
#pragma comment(lib, "secur32.lib")
#pragma comment(lib, "crypt32.lib")
#pragma comment(lib, "ws2_32.lib")
#endif

// TLS 1.3 may not be defined on older SDKs
#ifndef SP_PROT_TLS1_3_CLIENT
#define SP_PROT_TLS1_3_CLIENT 0x00002000
#endif

#ifndef UNISP_NAME_A
#define UNISP_NAME_A "Microsoft Unified Security Protocol Provider"
#endif

typedef struct {
    SOCKET      sock;
    CredHandle  cred;
    CtxtHandle  ctx;
    int         in_use;
    int         has_cred;
    int         has_ctx;
    // Decrypted data buffer for partial reads
    char        decrypted[65536];
    int         dec_len;
    int         dec_pos;
    // Raw recv buffer for TLS records
    char        raw_buf[65536];
    int         raw_len;
} L_TlsConn;

static L_TlsConn l_tls_conns[L_TLS_MAX_CONNECTIONS];
static int l_tls_initialized = 0;

static inline int l_tls_find_free_slot(void) {
    for (int i = 0; i < L_TLS_MAX_CONNECTIONS; i++) {
        if (!l_tls_conns[i].in_use) return i;
    }
    return -1;
}

// Perform the SChannel TLS handshake on a connected socket
static inline int l_tls_do_handshake(int slot, const char *hostname) {
    L_TlsConn *c = &l_tls_conns[slot];
    SECURITY_STATUS ss;

    // Acquire credentials
    SCHANNEL_CRED sc_cred;
    l_memset(&sc_cred, 0, sizeof(sc_cred));
    sc_cred.dwVersion = SCHANNEL_CRED_VERSION;
    sc_cred.dwFlags = SCH_CRED_AUTO_CRED_VALIDATION |
                      SCH_CRED_NO_DEFAULT_CREDS |
                      SCH_USE_STRONG_CRYPTO;
    sc_cred.grbitEnabledProtocols = SP_PROT_TLS1_2_CLIENT | SP_PROT_TLS1_3_CLIENT;

    ss = AcquireCredentialsHandleA(NULL, (SEC_CHAR *)UNISP_NAME_A,
                                   SECPKG_CRED_OUTBOUND, NULL, &sc_cred,
                                   NULL, NULL, &c->cred, NULL);
    if (ss != SEC_E_OK) return -1;
    c->has_cred = 1;

    // Initial handshake token
    SecBuffer out_buf = { 0, SECBUFFER_TOKEN, NULL };
    SecBufferDesc out_desc = { SECBUFFER_VERSION, 1, &out_buf };
    DWORD ctx_flags = ISC_REQ_ALLOCATE_MEMORY | ISC_REQ_CONFIDENTIALITY |
                      ISC_REQ_REPLAY_DETECT | ISC_REQ_SEQUENCE_DETECT |
                      ISC_REQ_STREAM;
    DWORD out_flags = 0;

    ss = InitializeSecurityContextA(&c->cred, NULL, (SEC_CHAR *)hostname,
                                    ctx_flags, 0, 0, NULL, 0,
                                    &c->ctx, &out_desc, &out_flags, NULL);
    c->has_ctx = 1;

    if (ss != SEC_I_CONTINUE_NEEDED && ss != SEC_E_OK) return -1;

    // Send initial token
    if (out_buf.cbBuffer > 0 && out_buf.pvBuffer) {
        int sent = send(c->sock, (char *)out_buf.pvBuffer, (int)out_buf.cbBuffer, 0);
        FreeContextBuffer(out_buf.pvBuffer);
        if (sent <= 0) return -1;
    }

    // Handshake loop
    char hs_buf[65536];
    int hs_len = 0;

    while (ss == SEC_I_CONTINUE_NEEDED || ss == SEC_E_INCOMPLETE_MESSAGE) {
        if (ss != SEC_E_INCOMPLETE_MESSAGE) {
            int r = recv(c->sock, hs_buf + hs_len, (int)(sizeof(hs_buf) - (size_t)hs_len), 0);
            if (r <= 0) return -1;
            hs_len += r;
        }

        SecBuffer in_bufs[2];
        in_bufs[0].BufferType = SECBUFFER_TOKEN;
        in_bufs[0].cbBuffer = (unsigned long)hs_len;
        in_bufs[0].pvBuffer = hs_buf;
        in_bufs[1].BufferType = SECBUFFER_EMPTY;
        in_bufs[1].cbBuffer = 0;
        in_bufs[1].pvBuffer = NULL;
        SecBufferDesc in_desc = { SECBUFFER_VERSION, 2, in_bufs };

        SecBuffer out_buf2 = { 0, SECBUFFER_TOKEN, NULL };
        SecBufferDesc out_desc2 = { SECBUFFER_VERSION, 1, &out_buf2 };

        ss = InitializeSecurityContextA(&c->cred, &c->ctx, (SEC_CHAR *)hostname,
                                        ctx_flags, 0, 0, &in_desc, 0,
                                        NULL, &out_desc2, &out_flags, NULL);

        if (ss == SEC_E_OK || ss == SEC_I_CONTINUE_NEEDED) {
            if (out_buf2.cbBuffer > 0 && out_buf2.pvBuffer) {
                send(c->sock, (char *)out_buf2.pvBuffer, (int)out_buf2.cbBuffer, 0);
                FreeContextBuffer(out_buf2.pvBuffer);
            }
            // Handle extra data
            if (in_bufs[1].BufferType == SECBUFFER_EXTRA && in_bufs[1].cbBuffer > 0) {
                l_memmove(hs_buf, hs_buf + (hs_len - (int)in_bufs[1].cbBuffer),
                          (size_t)in_bufs[1].cbBuffer);
                hs_len = (int)in_bufs[1].cbBuffer;
            } else {
                hs_len = 0;
            }
        } else if (ss == SEC_E_INCOMPLETE_MESSAGE) {
            // Need more data, continue reading
        } else {
            if (out_buf2.pvBuffer) FreeContextBuffer(out_buf2.pvBuffer);
            return -1;
        }
    }

    // Save any leftover data after handshake
    if (hs_len > 0) {
        l_memcpy(c->raw_buf, hs_buf, (size_t)hs_len);
        c->raw_len = hs_len;
    }

    return 0;
}

/// Initialize the TLS subsystem. Call once before any other l_tls_* function. Returns 0 on success.
static inline int l_tls_init(void) {
    if (l_tls_initialized) return 0;
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return -1;
    l_memset(l_tls_conns, 0, sizeof(l_tls_conns));
    for (int i = 0; i < L_TLS_MAX_CONNECTIONS; i++) {
        l_tls_conns[i].sock = INVALID_SOCKET;
    }
    l_tls_initialized = 1;
    return 0;
}

/// Connect to a host over TLS. Returns a handle (0-7) on success, -1 on failure.
static inline int l_tls_connect(const char *hostname, int port) {
    if (!l_tls_initialized) { if (l_tls_init() != 0) return -1; }

    int slot = l_tls_find_free_slot();
    if (slot < 0) return -1;

    L_TlsConn *c = &l_tls_conns[slot];
    l_memset(c, 0, sizeof(*c));
    c->sock = INVALID_SOCKET;

    // Resolve hostname to IP
    char ip[16];
    if (l_resolve(hostname, ip) != 0) return -1;

    // Parse IP to binary for sockaddr_in
    unsigned int addr;
    if (!l_parse_ipv4(ip, &addr)) return -1;

    // Manual port-to-string for port validation
    if (port <= 0 || port > 65535) return -1;

    // Create socket and connect
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) return -1;

    struct sockaddr_in sa;
    l_memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = l_htons((unsigned short)port);
    sa.sin_addr.s_addr = addr;

    if (connect(sock, (const struct sockaddr *)&sa, (int)sizeof(sa)) != 0) {
        closesocket(sock);
        return -1;
    }

    c->sock = sock;

    // TLS handshake
    if (l_tls_do_handshake(slot, hostname) != 0) {
        closesocket(c->sock);
        if (c->has_ctx)  DeleteSecurityContext(&c->ctx);
        if (c->has_cred) FreeCredentialsHandle(&c->cred);
        l_memset(c, 0, sizeof(*c));
        c->sock = INVALID_SOCKET;
        return -1;
    }

    c->in_use = 1;
    return slot;
}

/// Send data over a TLS connection. Returns bytes sent or -1 on failure.
static inline int l_tls_send(int handle, const void *data, int data_len) {
    if (handle < 0 || handle >= L_TLS_MAX_CONNECTIONS || !l_tls_conns[handle].in_use)
        return -1;
    L_TlsConn *c = &l_tls_conns[handle];

    SecPkgContext_StreamSizes sizes;
    if (QueryContextAttributes(&c->ctx, SECPKG_ATTR_STREAM_SIZES, &sizes) != SEC_E_OK)
        return -1;

    const char *src = (const char *)data;
    int total_sent = 0;

    while (total_sent < data_len) {
        int chunk = data_len - total_sent;
        if ((DWORD)chunk > sizes.cbMaximumMessage) chunk = (int)sizes.cbMaximumMessage;

        int buf_size = (int)(sizes.cbHeader + (DWORD)chunk + sizes.cbTrailer);
        char *msg_buf = (char *)HeapAlloc(GetProcessHeap(), 0, (SIZE_T)buf_size);
        if (!msg_buf) return -1;

        l_memcpy(msg_buf + sizes.cbHeader, src + total_sent, (size_t)chunk);

        SecBuffer bufs[4];
        bufs[0].BufferType = SECBUFFER_STREAM_HEADER;
        bufs[0].cbBuffer   = sizes.cbHeader;
        bufs[0].pvBuffer   = msg_buf;
        bufs[1].BufferType = SECBUFFER_DATA;
        bufs[1].cbBuffer   = (unsigned long)chunk;
        bufs[1].pvBuffer   = msg_buf + sizes.cbHeader;
        bufs[2].BufferType = SECBUFFER_STREAM_TRAILER;
        bufs[2].cbBuffer   = sizes.cbTrailer;
        bufs[2].pvBuffer   = msg_buf + sizes.cbHeader + chunk;
        bufs[3].BufferType = SECBUFFER_EMPTY;
        bufs[3].cbBuffer   = 0;
        bufs[3].pvBuffer   = NULL;
        SecBufferDesc desc = { SECBUFFER_VERSION, 4, bufs };

        SECURITY_STATUS ss = EncryptMessage(&c->ctx, 0, &desc, 0);
        if (ss != SEC_E_OK) { HeapFree(GetProcessHeap(), 0, msg_buf); return -1; }

        int enc_len = (int)(bufs[0].cbBuffer + bufs[1].cbBuffer + bufs[2].cbBuffer);
        int s = send(c->sock, msg_buf, enc_len, 0);
        HeapFree(GetProcessHeap(), 0, msg_buf);
        if (s <= 0) return -1;

        total_sent += chunk;
    }
    return total_sent;
}

// Internal: decrypt buffered raw data
static inline int l_tls_decrypt_data(L_TlsConn *c) {
    if (c->raw_len == 0) return 0;

    SecBuffer bufs[4];
    bufs[0].BufferType = SECBUFFER_DATA;
    bufs[0].cbBuffer   = (unsigned long)c->raw_len;
    bufs[0].pvBuffer   = c->raw_buf;
    bufs[1].BufferType = SECBUFFER_EMPTY;
    bufs[1].cbBuffer = 0; bufs[1].pvBuffer = NULL;
    bufs[2].BufferType = SECBUFFER_EMPTY;
    bufs[2].cbBuffer = 0; bufs[2].pvBuffer = NULL;
    bufs[3].BufferType = SECBUFFER_EMPTY;
    bufs[3].cbBuffer = 0; bufs[3].pvBuffer = NULL;
    SecBufferDesc desc = { SECBUFFER_VERSION, 4, bufs };

    SECURITY_STATUS ss = DecryptMessage(&c->ctx, &desc, 0, NULL);

    if (ss == SEC_E_OK) {
        for (int i = 0; i < 4; i++) {
            if (bufs[i].BufferType == SECBUFFER_DATA && bufs[i].cbBuffer > 0) {
                int space = (int)sizeof(c->decrypted) - c->dec_len;
                int copy = (int)bufs[i].cbBuffer;
                if (copy > space) copy = space;
                l_memcpy(c->decrypted + c->dec_len, bufs[i].pvBuffer, (size_t)copy);
                c->dec_len += copy;
            }
        }
        c->raw_len = 0;
        for (int i = 0; i < 4; i++) {
            if (bufs[i].BufferType == SECBUFFER_EXTRA && bufs[i].cbBuffer > 0) {
                l_memmove(c->raw_buf, bufs[i].pvBuffer, (size_t)bufs[i].cbBuffer);
                c->raw_len = (int)bufs[i].cbBuffer;
            }
        }
        return 1;
    } else if (ss == SEC_E_INCOMPLETE_MESSAGE) {
        return 0;
    } else {
        return -1; // Connection closed or error
    }
}

/// Receive data from a TLS connection. Returns bytes read, 0 on close, -1 on error.
static inline int l_tls_recv(int handle, void *buf, int len) {
    if (handle < 0 || handle >= L_TLS_MAX_CONNECTIONS || !l_tls_conns[handle].in_use)
        return -1;
    L_TlsConn *c = &l_tls_conns[handle];

    // Return from decrypted buffer if available
    if (c->dec_pos < c->dec_len) {
        int avail = c->dec_len - c->dec_pos;
        int copy = avail < len ? avail : len;
        l_memcpy(buf, c->decrypted + c->dec_pos, (size_t)copy);
        c->dec_pos += copy;
        if (c->dec_pos >= c->dec_len) {
            c->dec_pos = 0;
            c->dec_len = 0;
        }
        return copy;
    }

    // Reset buffer positions
    c->dec_pos = 0;
    c->dec_len = 0;

    // Try to decrypt existing raw data
    int dr = l_tls_decrypt_data(c);
    if (dr > 0 && c->dec_len > 0) {
        int copy = c->dec_len < len ? c->dec_len : len;
        l_memcpy(buf, c->decrypted, (size_t)copy);
        c->dec_pos = copy;
        if (c->dec_pos >= c->dec_len) { c->dec_pos = 0; c->dec_len = 0; }
        return copy;
    }
    if (dr < 0) return -1;

    // Read more raw data and try to decrypt
    for (int attempts = 0; attempts < 100; attempts++) {
        int space = (int)sizeof(c->raw_buf) - c->raw_len;
        if (space <= 0) return -1;

        int r = recv(c->sock, c->raw_buf + c->raw_len, space, 0);
        if (r <= 0) return r == 0 ? 0 : -1;
        c->raw_len += r;

        dr = l_tls_decrypt_data(c);
        if (dr > 0 && c->dec_len > 0) {
            int copy = c->dec_len < len ? c->dec_len : len;
            l_memcpy(buf, c->decrypted, (size_t)copy);
            c->dec_pos = copy;
            if (c->dec_pos >= c->dec_len) { c->dec_pos = 0; c->dec_len = 0; }
            return copy;
        }
        if (dr < 0) return -1;
    }
    return -1;
}

/// Receive a single byte from a TLS connection. Returns byte (0-255) or -1 on failure/close.
static inline int l_tls_recv_byte(int handle) {
    unsigned char b;
    int n = l_tls_recv(handle, &b, 1);
    return n == 1 ? (int)b : -1;
}

/// Close a TLS connection. Safe to call with invalid handle.
static inline void l_tls_close(int handle) {
    if (handle < 0 || handle >= L_TLS_MAX_CONNECTIONS || !l_tls_conns[handle].in_use)
        return;
    L_TlsConn *c = &l_tls_conns[handle];

    // Send shutdown notification
    DWORD shutdown_token = SCHANNEL_SHUTDOWN;
    SecBuffer shut_buf = { sizeof(shutdown_token), SECBUFFER_TOKEN, &shutdown_token };
    SecBufferDesc shut_desc = { SECBUFFER_VERSION, 1, &shut_buf };
    ApplyControlToken(&c->ctx, &shut_desc);

    SecBuffer out_buf = { 0, SECBUFFER_TOKEN, NULL };
    SecBufferDesc out_desc = { SECBUFFER_VERSION, 1, &out_buf };
    DWORD flags = ISC_REQ_ALLOCATE_MEMORY | ISC_REQ_STREAM;
    DWORD out_flags = 0;
    InitializeSecurityContextA(&c->cred, &c->ctx, NULL, flags, 0, 0,
                               &shut_desc, 0, NULL, &out_desc, &out_flags, NULL);
    if (out_buf.cbBuffer > 0 && out_buf.pvBuffer) {
        send(c->sock, (char *)out_buf.pvBuffer, (int)out_buf.cbBuffer, 0);
        FreeContextBuffer(out_buf.pvBuffer);
    }

    if (c->has_ctx)  DeleteSecurityContext(&c->ctx);
    if (c->has_cred) FreeCredentialsHandle(&c->cred);
    closesocket(c->sock);

    l_memset(c, 0, sizeof(*c));
    c->sock = INVALID_SOCKET;
}

/// Clean up the TLS subsystem. Closes all connections. Call at program exit.
static inline void l_tls_cleanup(void) {
    for (int i = 0; i < L_TLS_MAX_CONNECTIONS; i++) {
        if (l_tls_conns[i].in_use) l_tls_close(i);
    }
    l_tls_initialized = 0;
    WSACleanup();
}

// ── Linux / WASI: stub implementation ────────────────────────────────────────
#else

/// Initialize the TLS subsystem. Returns -1 (TLS not available on this platform).
static inline int l_tls_init(void) {
    return -1;
}

/// Connect to a host over TLS. Returns -1 (TLS not available on this platform).
static inline int l_tls_connect(const char *hostname, int port) {
    (void)hostname; (void)port;
    return -1;
}

/// Send data over a TLS connection. Returns -1 (TLS not available on this platform).
static inline int l_tls_send(int handle, const void *data, int data_len) {
    (void)handle; (void)data; (void)data_len;
    return -1;
}

/// Receive data from a TLS connection. Returns -1 (TLS not available on this platform).
static inline int l_tls_recv(int handle, void *buf, int len) {
    (void)handle; (void)buf; (void)len;
    return -1;
}

/// Receive a single byte. Returns -1 (TLS not available on this platform).
static inline int l_tls_recv_byte(int handle) {
    (void)handle;
    return -1;
}

/// Close a TLS connection. No-op (TLS not available on this platform).
static inline void l_tls_close(int handle) {
    (void)handle;
}

/// Clean up the TLS subsystem. No-op (TLS not available on this platform).
static inline void l_tls_cleanup(void) {
}

#endif // _WIN32

#endif // L_TLSH
