/*
 * BearSSL amalgamation for laststanding - minimal TLS 1.2 client
 * Auto-generated from BearSSL v0.6 (Thomas Pornin)
 * License: MIT
 *
 * Single-file amalgamation containing only TLS 1.2 client code.
 * Uses i31 big-integer engine, constant-time AES, ECDHE+RSA.
 * No server code. No dynamic allocation (zero malloc).
 */

#ifndef BEARSSL_AMALG_C
#define BEARSSL_AMALG_C

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wsign-conversion"
#pragma clang diagnostic ignored "-Wconversion"
#pragma clang diagnostic ignored "-Wshorten-64-to-32"
#pragma clang diagnostic ignored "-Wimplicit-fallthrough"
#pragma clang diagnostic ignored "-Wcast-qual"
#pragma clang diagnostic ignored "-Wmissing-prototypes"
#pragma clang diagnostic ignored "-Wmissing-variable-declarations"
#endif
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#pragma GCC diagnostic ignored "-Wcast-qual"
#endif

/* ===== inc/bearssl_hash.h ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef BR_BEARSSL_HASH_H__
#define BR_BEARSSL_HASH_H__

#include <stddef.h>
#include <stdint.h>
#include <string.h>

/** \file bearssl_hash.h
 *
 * # Hash Functions
 *
 * This file documents the API for hash functions.
 *
 *
 * ## Procedural API
 *
 * For each implemented hash function, of name "`xxx`", the following
 * elements are defined:
 *
 *   - `br_xxx_vtable`
 *
 *     An externally defined instance of `br_hash_class`.
 *
 *   - `br_xxx_SIZE`
 *
 *     A macro that evaluates to the output size (in bytes) of the
 *     hash function.
 *
 *   - `br_xxx_ID`
 *
 *     A macro that evaluates to a symbolic identifier for the hash
 *     function. Such identifiers are used with HMAC and signature
 *     algorithm implementations.
 *
 *     NOTE: for the "standard" hash functions defined in [the TLS
 *     standard](https://tools.ietf.org/html/rfc5246#section-7.4.1.4.1),
 *     the symbolic identifiers match the constants used in TLS, i.e.
 *     1 to 6 for MD5, SHA-1, SHA-224, SHA-256, SHA-384 and SHA-512,
 *     respectively.
 *
 *   - `br_xxx_context`
 *
 *     Context for an ongoing computation. It is allocated by the
 *     caller, and a pointer to it is passed to all functions. A
 *     context contains no interior pointer, so it can be moved around
 *     and cloned (with a simple `memcpy()` or equivalent) in order to
 *     capture the function state at some point. Computations that use
 *     distinct context structures are independent of each other. The
 *     first field of `br_xxx_context` is always a pointer to the
 *     `br_xxx_vtable` structure; `br_xxx_init()` sets that pointer.
 *
 *   - `br_xxx_init(br_xxx_context *ctx)`
 *
 *     Initialise the provided context. Previous contents of the structure
 *     are ignored. This calls resets the context to the start of a new
 *     hash computation; it also sets the first field of the context
 *     structure (called `vtable`) to a pointer to the statically
 *     allocated constant `br_xxx_vtable` structure.
 *
 *   - `br_xxx_update(br_xxx_context *ctx, const void *data, size_t len)`
 *
 *     Add some more bytes to the hash computation represented by the
 *     provided context.
 *
 *   - `br_xxx_out(const br_xxx_context *ctx, void *out)`
 *
 *     Complete the hash computation and write the result in the provided
 *     buffer. The output buffer MUST be large enough to accomodate the
 *     result. The context is NOT modified by this operation, so this
 *     function can be used to get a "partial hash" while still keeping
 *     the possibility of adding more bytes to the input.
 *
 *   - `br_xxx_state(const br_xxx_context *ctx, void *out)`
 *
 *     Get a copy of the "current state" for the computation so far. For
 *     MD functions (MD5, SHA-1, SHA-2 family), this is the running state
 *     resulting from the processing of the last complete input block.
 *     Returned value is the current input length (in bytes).
 *
 *   - `br_xxx_set_state(br_xxx_context *ctx, const void *stb, uint64_t count)`
 *
 *     Set the internal state to the provided values. The 'stb' and
 *     'count' values shall match that which was obtained from
 *     `br_xxx_state()`. This restores the hash state only if the state
 *     values were at an appropriate block boundary. This does NOT set
 *     the `vtable` pointer in the context.
 *
 * Context structures can be discarded without any explicit deallocation.
 * Hash function implementations are purely software and don't reserve
 * any resources outside of the context structure itself.
 *
 *
 * ## Object-Oriented API
 *
 * For each hash function that follows the procedural API described
 * above, an object-oriented API is also provided. In that API, function
 * pointers from the vtable (`br_xxx_vtable`) are used. The vtable
 * incarnates object-oriented programming. An introduction on the OOP
 * concept used here can be read on the BearSSL Web site:<br />
 * &nbsp;&nbsp;&nbsp;[https://www.bearssl.org/oop.html](https://www.bearssl.org/oop.html)
 *
 * The vtable offers functions called `init()`, `update()`, `out()`,
 * `set()` and `set_state()`, which are in fact the functions from
 * the procedural API. That vtable also contains two informative fields:
 *
 *   - `context_size`
 *
 *     The size of the context structure (`br_xxx_context`), in bytes.
 *     This can be used by generic implementations to perform dynamic
 *     context allocation.
 *
 *   - `desc`
 *
 *     A "descriptor" field that encodes some information on the hash
 *     function: symbolic identifier, output size, state size,
 *     internal block size, details on the padding.
 *
 * Users of this object-oriented API (in particular generic HMAC
 * implementations) may make the following assumptions:
 *
 *   - Hash output size is no more than 64 bytes.
 *   - Hash internal state size is no more than 64 bytes.
 *   - Internal block size is a power of two, no less than 16 and no more
 *     than 256.
 *
 *
 * ## Implemented Hash Functions
 *
 * Implemented hash functions are:
 *
 * | Function  | Name    | Output length | State length |
 * | :-------- | :------ | :-----------: | :----------: |
 * | MD5       | md5     |     16        |     16       |
 * | SHA-1     | sha1    |     20        |     20       |
 * | SHA-224   | sha224  |     28        |     32       |
 * | SHA-256   | sha256  |     32        |     32       |
 * | SHA-384   | sha384  |     48        |     64       |
 * | SHA-512   | sha512  |     64        |     64       |
 * | MD5+SHA-1 | md5sha1 |     36        |     36       |
 *
 * (MD5+SHA-1 is the concatenation of MD5 and SHA-1 computed over the
 * same input; in the implementation, the internal data buffer is
 * shared, thus making it more memory-efficient than separate MD5 and
 * SHA-1. It can be useful in implementing SSL 3.0, TLS 1.0 and TLS
 * 1.1.)
 *
 *
 * ## Multi-Hasher
 *
 * An aggregate hasher is provided, that can compute several standard
 * hash functions in parallel. It uses `br_multihash_context` and a
 * procedural API. It is configured with the implementations (the vtables)
 * that it should use; it will then compute all these hash functions in
 * parallel, on the same input. It is meant to be used in cases when the
 * hash of an object will be used, but the exact hash function is not
 * known yet (typically, streamed processing on X.509 certificates).
 *
 * Only the standard hash functions (MD5, SHA-1, SHA-224, SHA-256, SHA-384
 * and SHA-512) are supported by the multi-hasher.
 *
 *
 * ## GHASH
 *
 * GHASH is not a generic hash function; it is a _universal_ hash function,
 * which, as the name does not say, means that it CANNOT be used in most
 * places where a hash function is needed. GHASH is used within the GCM
 * encryption mode, to provide the checked integrity functionality.
 *
 * A GHASH implementation is basically a function that uses the type defined
 * in this file under the name `br_ghash`:
 *
 *     typedef void (*br_ghash)(void *y, const void *h, const void *data, size_t len);
 *
 * The `y` pointer refers to a 16-byte value which is used as input, and
 * receives the output of the GHASH invocation. `h` is a 16-byte secret
 * value (that serves as key). `data` and `len` define the input data.
 *
 * Three GHASH implementations are provided, all constant-time, based on
 * the use of integer multiplications with appropriate masking to cancel
 * carry propagation.
 */

/**
 * \brief Class type for hash function implementations.
 *
 * A `br_hash_class` instance references the methods implementing a hash
 * function. Constant instances of this structure are defined for each
 * implemented hash function. Such instances are also called "vtables".
 *
 * Vtables are used to support object-oriented programming, as
 * described on [the BearSSL Web site](https://www.bearssl.org/oop.html).
 */
typedef struct br_hash_class_ br_hash_class;
struct br_hash_class_ {
	/**
	 * \brief Size (in bytes) of the context structure appropriate for
	 * computing this hash function.
	 */
	size_t context_size;

	/**
	 * \brief Descriptor word that contains information about the hash
	 * function.
	 *
	 * For each word `xxx` described below, use `BR_HASHDESC_xxx_OFF`
	 * and `BR_HASHDESC_xxx_MASK` to access the specific value, as
	 * follows:
	 *
	 *     (hf->desc >> BR_HASHDESC_xxx_OFF) & BR_HASHDESC_xxx_MASK
	 *
	 * The defined elements are:
	 *
	 *  - `ID`: the symbolic identifier for the function, as defined
	 *    in [TLS](https://tools.ietf.org/html/rfc5246#section-7.4.1.4.1)
	 *    (MD5 = 1, SHA-1 = 2,...).
	 *
	 *  - `OUT`: hash output size, in bytes.
	 *
	 *  - `STATE`: internal running state size, in bytes.
	 *
	 *  - `LBLEN`: base-2 logarithm for the internal block size, as
	 *    defined for HMAC processing (this is 6 for MD5, SHA-1, SHA-224
	 *    and SHA-256, since these functions use 64-byte blocks; for
	 *    SHA-384 and SHA-512, this is 7, corresponding to their
	 *    128-byte blocks).
	 *
	 * The descriptor may contain a few other flags.
	 */
	uint32_t desc;

	/**
	 * \brief Initialisation method.
	 *
	 * This method takes as parameter a pointer to a context area,
	 * that it initialises. The first field of the context is set
	 * to this vtable; other elements are initialised for a new hash
	 * computation.
	 *
	 * \param ctx   pointer to (the first field of) the context.
	 */
	void (*init)(const br_hash_class **ctx);

	/**
	 * \brief Data injection method.
	 *
	 * The `len` bytes starting at address `data` are injected into
	 * the running hash computation incarnated by the specified
	 * context. The context is updated accordingly. It is allowed
	 * to have `len == 0`, in which case `data` is ignored (and could
	 * be `NULL`), and nothing happens.
	 * on the input data.
	 *
	 * \param ctx    pointer to (the first field of) the context.
	 * \param data   pointer to the first data byte to inject.
	 * \param len    number of bytes to inject.
	 */
	void (*update)(const br_hash_class **ctx, const void *data, size_t len);

	/**
	 * \brief Produce hash output.
	 *
	 * The hash output corresponding to all data bytes injected in the
	 * context since the last `init()` call is computed, and written
	 * in the buffer pointed to by `dst`. The hash output size depends
	 * on the implemented hash function (e.g. 16 bytes for MD5).
	 * The context is _not_ modified by this call, so further bytes
	 * may be afterwards injected to continue the current computation.
	 *
	 * \param ctx   pointer to (the first field of) the context.
	 * \param dst   destination buffer for the hash output.
	 */
	void (*out)(const br_hash_class *const *ctx, void *dst);

	/**
	 * \brief Get running state.
	 *
	 * This method saves the current running state into the `dst`
	 * buffer. What constitutes the "running state" depends on the
	 * hash function; for Merkle-Damgård hash functions (like
	 * MD5 or SHA-1), this is the output obtained after processing
	 * each block. The number of bytes injected so far is returned.
	 * The context is not modified by this call.
	 *
	 * \param ctx   pointer to (the first field of) the context.
	 * \param dst   destination buffer for the state.
	 * \return  the injected total byte length.
	 */
	uint64_t (*state)(const br_hash_class *const *ctx, void *dst);

	/**
	 * \brief Set running state.
	 *
	 * This methods replaces the running state for the function.
	 *
	 * \param ctx     pointer to (the first field of) the context.
	 * \param stb     source buffer for the state.
	 * \param count   injected total byte length.
	 */
	void (*set_state)(const br_hash_class **ctx,
		const void *stb, uint64_t count);
};

#ifndef BR_DOXYGEN_IGNORE
#define BR_HASHDESC_ID(id)           ((uint32_t)(id) << BR_HASHDESC_ID_OFF)
#define BR_HASHDESC_ID_OFF           0
#define BR_HASHDESC_ID_MASK          0xFF

#define BR_HASHDESC_OUT(size)        ((uint32_t)(size) << BR_HASHDESC_OUT_OFF)
#define BR_HASHDESC_OUT_OFF          8
#define BR_HASHDESC_OUT_MASK         0x7F

#define BR_HASHDESC_STATE(size)      ((uint32_t)(size) << BR_HASHDESC_STATE_OFF)
#define BR_HASHDESC_STATE_OFF        15
#define BR_HASHDESC_STATE_MASK       0xFF

#define BR_HASHDESC_LBLEN(ls)        ((uint32_t)(ls) << BR_HASHDESC_LBLEN_OFF)
#define BR_HASHDESC_LBLEN_OFF        23
#define BR_HASHDESC_LBLEN_MASK       0x0F

#define BR_HASHDESC_MD_PADDING       ((uint32_t)1 << 28)
#define BR_HASHDESC_MD_PADDING_128   ((uint32_t)1 << 29)
#define BR_HASHDESC_MD_PADDING_BE    ((uint32_t)1 << 30)
#endif

/*
 * Specific hash functions.
 *
 * Rules for contexts:
 * -- No interior pointer.
 * -- No pointer to external dynamically allocated resources.
 * -- First field is called 'vtable' and is a pointer to a
 *    const-qualified br_hash_class instance (pointer is set by init()).
 * -- SHA-224 and SHA-256 contexts are identical.
 * -- SHA-384 and SHA-512 contexts are identical.
 *
 * Thus, contexts can be moved and cloned to capture the hash function
 * current state; and there is no need for any explicit "release" function.
 */

/**
 * \brief Symbolic identifier for MD5.
 */
#define br_md5_ID     1

/**
 * \brief MD5 output size (in bytes).
 */
#define br_md5_SIZE   16

/**
 * \brief Constant vtable for MD5.
 */
extern const br_hash_class br_md5_vtable;

/**
 * \brief MD5 context.
 *
 * First field is a pointer to the vtable; it is set by the initialisation
 * function. Other fields are not supposed to be accessed by user code.
 */
typedef struct {
	/**
	 * \brief Pointer to vtable for this context.
	 */
	const br_hash_class *vtable;
#ifndef BR_DOXYGEN_IGNORE
	unsigned char buf[64];
	uint64_t count;
	uint32_t val[4];
#endif
} br_md5_context;

/**
 * \brief MD5 context initialisation.
 *
 * This function initialises or resets a context for a new MD5
 * computation. It also sets the vtable pointer.
 *
 * \param ctx   pointer to the context structure.
 */
void br_md5_init(br_md5_context *ctx);

/**
 * \brief Inject some data bytes in a running MD5 computation.
 *
 * The provided context is updated with some data bytes. If the number
 * of bytes (`len`) is zero, then the data pointer (`data`) is ignored
 * and may be `NULL`, and this function does nothing.
 *
 * \param ctx    pointer to the context structure.
 * \param data   pointer to the injected data.
 * \param len    injected data length (in bytes).
 */
void br_md5_update(br_md5_context *ctx, const void *data, size_t len);

/**
 * \brief Compute MD5 output.
 *
 * The MD5 output for the concatenation of all bytes injected in the
 * provided context since the last initialisation or reset call, is
 * computed and written in the buffer pointed to by `out`. The context
 * itself is not modified, so extra bytes may be injected afterwards
 * to continue that computation.
 *
 * \param ctx   pointer to the context structure.
 * \param out   destination buffer for the hash output.
 */
void br_md5_out(const br_md5_context *ctx, void *out);

/**
 * \brief Save MD5 running state.
 *
 * The running state for MD5 (output of the last internal block
 * processing) is written in the buffer pointed to by `out`. The
 * number of bytes injected since the last initialisation or reset
 * call is returned. The context is not modified.
 *
 * \param ctx   pointer to the context structure.
 * \param out   destination buffer for the running state.
 * \return  the injected total byte length.
 */
uint64_t br_md5_state(const br_md5_context *ctx, void *out);

/**
 * \brief Restore MD5 running state.
 *
 * The running state for MD5 is set to the provided values.
 *
 * \param ctx     pointer to the context structure.
 * \param stb     source buffer for the running state.
 * \param count   the injected total byte length.
 */
void br_md5_set_state(br_md5_context *ctx, const void *stb, uint64_t count);

/**
 * \brief Symbolic identifier for SHA-1.
 */
#define br_sha1_ID     2

/**
 * \brief SHA-1 output size (in bytes).
 */
#define br_sha1_SIZE   20

/**
 * \brief Constant vtable for SHA-1.
 */
extern const br_hash_class br_sha1_vtable;

/**
 * \brief SHA-1 context.
 *
 * First field is a pointer to the vtable; it is set by the initialisation
 * function. Other fields are not supposed to be accessed by user code.
 */
typedef struct {
	/**
	 * \brief Pointer to vtable for this context.
	 */
	const br_hash_class *vtable;
#ifndef BR_DOXYGEN_IGNORE
	unsigned char buf[64];
	uint64_t count;
	uint32_t val[5];
#endif
} br_sha1_context;

/**
 * \brief SHA-1 context initialisation.
 *
 * This function initialises or resets a context for a new SHA-1
 * computation. It also sets the vtable pointer.
 *
 * \param ctx   pointer to the context structure.
 */
void br_sha1_init(br_sha1_context *ctx);

/**
 * \brief Inject some data bytes in a running SHA-1 computation.
 *
 * The provided context is updated with some data bytes. If the number
 * of bytes (`len`) is zero, then the data pointer (`data`) is ignored
 * and may be `NULL`, and this function does nothing.
 *
 * \param ctx    pointer to the context structure.
 * \param data   pointer to the injected data.
 * \param len    injected data length (in bytes).
 */
void br_sha1_update(br_sha1_context *ctx, const void *data, size_t len);

/**
 * \brief Compute SHA-1 output.
 *
 * The SHA-1 output for the concatenation of all bytes injected in the
 * provided context since the last initialisation or reset call, is
 * computed and written in the buffer pointed to by `out`. The context
 * itself is not modified, so extra bytes may be injected afterwards
 * to continue that computation.
 *
 * \param ctx   pointer to the context structure.
 * \param out   destination buffer for the hash output.
 */
void br_sha1_out(const br_sha1_context *ctx, void *out);

/**
 * \brief Save SHA-1 running state.
 *
 * The running state for SHA-1 (output of the last internal block
 * processing) is written in the buffer pointed to by `out`. The
 * number of bytes injected since the last initialisation or reset
 * call is returned. The context is not modified.
 *
 * \param ctx   pointer to the context structure.
 * \param out   destination buffer for the running state.
 * \return  the injected total byte length.
 */
uint64_t br_sha1_state(const br_sha1_context *ctx, void *out);

/**
 * \brief Restore SHA-1 running state.
 *
 * The running state for SHA-1 is set to the provided values.
 *
 * \param ctx     pointer to the context structure.
 * \param stb     source buffer for the running state.
 * \param count   the injected total byte length.
 */
void br_sha1_set_state(br_sha1_context *ctx, const void *stb, uint64_t count);

/**
 * \brief Symbolic identifier for SHA-224.
 */
#define br_sha224_ID     3

/**
 * \brief SHA-224 output size (in bytes).
 */
#define br_sha224_SIZE   28

/**
 * \brief Constant vtable for SHA-224.
 */
extern const br_hash_class br_sha224_vtable;

/**
 * \brief SHA-224 context.
 *
 * First field is a pointer to the vtable; it is set by the initialisation
 * function. Other fields are not supposed to be accessed by user code.
 */
typedef struct {
	/**
	 * \brief Pointer to vtable for this context.
	 */
	const br_hash_class *vtable;
#ifndef BR_DOXYGEN_IGNORE
	unsigned char buf[64];
	uint64_t count;
	uint32_t val[8];
#endif
} br_sha224_context;

/**
 * \brief SHA-224 context initialisation.
 *
 * This function initialises or resets a context for a new SHA-224
 * computation. It also sets the vtable pointer.
 *
 * \param ctx   pointer to the context structure.
 */
void br_sha224_init(br_sha224_context *ctx);

/**
 * \brief Inject some data bytes in a running SHA-224 computation.
 *
 * The provided context is updated with some data bytes. If the number
 * of bytes (`len`) is zero, then the data pointer (`data`) is ignored
 * and may be `NULL`, and this function does nothing.
 *
 * \param ctx    pointer to the context structure.
 * \param data   pointer to the injected data.
 * \param len    injected data length (in bytes).
 */
void br_sha224_update(br_sha224_context *ctx, const void *data, size_t len);

/**
 * \brief Compute SHA-224 output.
 *
 * The SHA-224 output for the concatenation of all bytes injected in the
 * provided context since the last initialisation or reset call, is
 * computed and written in the buffer pointed to by `out`. The context
 * itself is not modified, so extra bytes may be injected afterwards
 * to continue that computation.
 *
 * \param ctx   pointer to the context structure.
 * \param out   destination buffer for the hash output.
 */
void br_sha224_out(const br_sha224_context *ctx, void *out);

/**
 * \brief Save SHA-224 running state.
 *
 * The running state for SHA-224 (output of the last internal block
 * processing) is written in the buffer pointed to by `out`. The
 * number of bytes injected since the last initialisation or reset
 * call is returned. The context is not modified.
 *
 * \param ctx   pointer to the context structure.
 * \param out   destination buffer for the running state.
 * \return  the injected total byte length.
 */
uint64_t br_sha224_state(const br_sha224_context *ctx, void *out);

/**
 * \brief Restore SHA-224 running state.
 *
 * The running state for SHA-224 is set to the provided values.
 *
 * \param ctx     pointer to the context structure.
 * \param stb     source buffer for the running state.
 * \param count   the injected total byte length.
 */
void br_sha224_set_state(br_sha224_context *ctx,
	const void *stb, uint64_t count);

/**
 * \brief Symbolic identifier for SHA-256.
 */
#define br_sha256_ID     4

/**
 * \brief SHA-256 output size (in bytes).
 */
#define br_sha256_SIZE   32

/**
 * \brief Constant vtable for SHA-256.
 */
extern const br_hash_class br_sha256_vtable;

#ifdef BR_DOXYGEN_IGNORE
/**
 * \brief SHA-256 context.
 *
 * First field is a pointer to the vtable; it is set by the initialisation
 * function. Other fields are not supposed to be accessed by user code.
 */
typedef struct {
	/**
	 * \brief Pointer to vtable for this context.
	 */
	const br_hash_class *vtable;
} br_sha256_context;
#else
typedef br_sha224_context br_sha256_context;
#endif

/**
 * \brief SHA-256 context initialisation.
 *
 * This function initialises or resets a context for a new SHA-256
 * computation. It also sets the vtable pointer.
 *
 * \param ctx   pointer to the context structure.
 */
void br_sha256_init(br_sha256_context *ctx);

#ifdef BR_DOXYGEN_IGNORE
/**
 * \brief Inject some data bytes in a running SHA-256 computation.
 *
 * The provided context is updated with some data bytes. If the number
 * of bytes (`len`) is zero, then the data pointer (`data`) is ignored
 * and may be `NULL`, and this function does nothing.
 *
 * \param ctx    pointer to the context structure.
 * \param data   pointer to the injected data.
 * \param len    injected data length (in bytes).
 */
void br_sha256_update(br_sha256_context *ctx, const void *data, size_t len);
#else
#define br_sha256_update      br_sha224_update
#endif

/**
 * \brief Compute SHA-256 output.
 *
 * The SHA-256 output for the concatenation of all bytes injected in the
 * provided context since the last initialisation or reset call, is
 * computed and written in the buffer pointed to by `out`. The context
 * itself is not modified, so extra bytes may be injected afterwards
 * to continue that computation.
 *
 * \param ctx   pointer to the context structure.
 * \param out   destination buffer for the hash output.
 */
void br_sha256_out(const br_sha256_context *ctx, void *out);

#if BR_DOXYGEN_IGNORE
/**
 * \brief Save SHA-256 running state.
 *
 * The running state for SHA-256 (output of the last internal block
 * processing) is written in the buffer pointed to by `out`. The
 * number of bytes injected since the last initialisation or reset
 * call is returned. The context is not modified.
 *
 * \param ctx   pointer to the context structure.
 * \param out   destination buffer for the running state.
 * \return  the injected total byte length.
 */
uint64_t br_sha256_state(const br_sha256_context *ctx, void *out);
#else
#define br_sha256_state       br_sha224_state
#endif

#if BR_DOXYGEN_IGNORE
/**
 * \brief Restore SHA-256 running state.
 *
 * The running state for SHA-256 is set to the provided values.
 *
 * \param ctx     pointer to the context structure.
 * \param stb     source buffer for the running state.
 * \param count   the injected total byte length.
 */
void br_sha256_set_state(br_sha256_context *ctx,
	const void *stb, uint64_t count);
#else
#define br_sha256_set_state   br_sha224_set_state
#endif

/**
 * \brief Symbolic identifier for SHA-384.
 */
#define br_sha384_ID     5

/**
 * \brief SHA-384 output size (in bytes).
 */
#define br_sha384_SIZE   48

/**
 * \brief Constant vtable for SHA-384.
 */
extern const br_hash_class br_sha384_vtable;

/**
 * \brief SHA-384 context.
 *
 * First field is a pointer to the vtable; it is set by the initialisation
 * function. Other fields are not supposed to be accessed by user code.
 */
typedef struct {
	/**
	 * \brief Pointer to vtable for this context.
	 */
	const br_hash_class *vtable;
#ifndef BR_DOXYGEN_IGNORE
	unsigned char buf[128];
	uint64_t count;
	uint64_t val[8];
#endif
} br_sha384_context;

/**
 * \brief SHA-384 context initialisation.
 *
 * This function initialises or resets a context for a new SHA-384
 * computation. It also sets the vtable pointer.
 *
 * \param ctx   pointer to the context structure.
 */
void br_sha384_init(br_sha384_context *ctx);

/**
 * \brief Inject some data bytes in a running SHA-384 computation.
 *
 * The provided context is updated with some data bytes. If the number
 * of bytes (`len`) is zero, then the data pointer (`data`) is ignored
 * and may be `NULL`, and this function does nothing.
 *
 * \param ctx    pointer to the context structure.
 * \param data   pointer to the injected data.
 * \param len    injected data length (in bytes).
 */
void br_sha384_update(br_sha384_context *ctx, const void *data, size_t len);

/**
 * \brief Compute SHA-384 output.
 *
 * The SHA-384 output for the concatenation of all bytes injected in the
 * provided context since the last initialisation or reset call, is
 * computed and written in the buffer pointed to by `out`. The context
 * itself is not modified, so extra bytes may be injected afterwards
 * to continue that computation.
 *
 * \param ctx   pointer to the context structure.
 * \param out   destination buffer for the hash output.
 */
void br_sha384_out(const br_sha384_context *ctx, void *out);

/**
 * \brief Save SHA-384 running state.
 *
 * The running state for SHA-384 (output of the last internal block
 * processing) is written in the buffer pointed to by `out`. The
 * number of bytes injected since the last initialisation or reset
 * call is returned. The context is not modified.
 *
 * \param ctx   pointer to the context structure.
 * \param out   destination buffer for the running state.
 * \return  the injected total byte length.
 */
uint64_t br_sha384_state(const br_sha384_context *ctx, void *out);

/**
 * \brief Restore SHA-384 running state.
 *
 * The running state for SHA-384 is set to the provided values.
 *
 * \param ctx     pointer to the context structure.
 * \param stb     source buffer for the running state.
 * \param count   the injected total byte length.
 */
void br_sha384_set_state(br_sha384_context *ctx,
	const void *stb, uint64_t count);

/**
 * \brief Symbolic identifier for SHA-512.
 */
#define br_sha512_ID     6

/**
 * \brief SHA-512 output size (in bytes).
 */
#define br_sha512_SIZE   64

/**
 * \brief Constant vtable for SHA-512.
 */
extern const br_hash_class br_sha512_vtable;

#ifdef BR_DOXYGEN_IGNORE
/**
 * \brief SHA-512 context.
 *
 * First field is a pointer to the vtable; it is set by the initialisation
 * function. Other fields are not supposed to be accessed by user code.
 */
typedef struct {
	/**
	 * \brief Pointer to vtable for this context.
	 */
	const br_hash_class *vtable;
} br_sha512_context;
#else
typedef br_sha384_context br_sha512_context;
#endif

/**
 * \brief SHA-512 context initialisation.
 *
 * This function initialises or resets a context for a new SHA-512
 * computation. It also sets the vtable pointer.
 *
 * \param ctx   pointer to the context structure.
 */
void br_sha512_init(br_sha512_context *ctx);

#ifdef BR_DOXYGEN_IGNORE
/**
 * \brief Inject some data bytes in a running SHA-512 computation.
 *
 * The provided context is updated with some data bytes. If the number
 * of bytes (`len`) is zero, then the data pointer (`data`) is ignored
 * and may be `NULL`, and this function does nothing.
 *
 * \param ctx    pointer to the context structure.
 * \param data   pointer to the injected data.
 * \param len    injected data length (in bytes).
 */
void br_sha512_update(br_sha512_context *ctx, const void *data, size_t len);
#else
#define br_sha512_update   br_sha384_update
#endif

/**
 * \brief Compute SHA-512 output.
 *
 * The SHA-512 output for the concatenation of all bytes injected in the
 * provided context since the last initialisation or reset call, is
 * computed and written in the buffer pointed to by `out`. The context
 * itself is not modified, so extra bytes may be injected afterwards
 * to continue that computation.
 *
 * \param ctx   pointer to the context structure.
 * \param out   destination buffer for the hash output.
 */
void br_sha512_out(const br_sha512_context *ctx, void *out);

#ifdef BR_DOXYGEN_IGNORE
/**
 * \brief Save SHA-512 running state.
 *
 * The running state for SHA-512 (output of the last internal block
 * processing) is written in the buffer pointed to by `out`. The
 * number of bytes injected since the last initialisation or reset
 * call is returned. The context is not modified.
 *
 * \param ctx   pointer to the context structure.
 * \param out   destination buffer for the running state.
 * \return  the injected total byte length.
 */
uint64_t br_sha512_state(const br_sha512_context *ctx, void *out);
#else
#define br_sha512_state   br_sha384_state
#endif

#ifdef BR_DOXYGEN_IGNORE
/**
 * \brief Restore SHA-512 running state.
 *
 * The running state for SHA-512 is set to the provided values.
 *
 * \param ctx     pointer to the context structure.
 * \param stb     source buffer for the running state.
 * \param count   the injected total byte length.
 */
void br_sha512_set_state(br_sha512_context *ctx,
	const void *stb, uint64_t count);
#else
#define br_sha512_set_state   br_sha384_set_state
#endif

/*
 * "md5sha1" is a special hash function that computes both MD5 and SHA-1
 * on the same input, and produces a 36-byte output (MD5 and SHA-1
 * concatenation, in that order). State size is also 36 bytes.
 */

/**
 * \brief Symbolic identifier for MD5+SHA-1.
 *
 * MD5+SHA-1 is the concatenation of MD5 and SHA-1, computed over the
 * same input. It is not one of the functions identified in TLS, so
 * we give it a symbolic identifier of value 0.
 */
#define br_md5sha1_ID     0

/**
 * \brief MD5+SHA-1 output size (in bytes).
 */
#define br_md5sha1_SIZE   36

/**
 * \brief Constant vtable for MD5+SHA-1.
 */
extern const br_hash_class br_md5sha1_vtable;

/**
 * \brief MD5+SHA-1 context.
 *
 * First field is a pointer to the vtable; it is set by the initialisation
 * function. Other fields are not supposed to be accessed by user code.
 */
typedef struct {
	/**
	 * \brief Pointer to vtable for this context.
	 */
	const br_hash_class *vtable;
#ifndef BR_DOXYGEN_IGNORE
	unsigned char buf[64];
	uint64_t count;
	uint32_t val_md5[4];
	uint32_t val_sha1[5];
#endif
} br_md5sha1_context;

/**
 * \brief MD5+SHA-1 context initialisation.
 *
 * This function initialises or resets a context for a new SHA-512
 * computation. It also sets the vtable pointer.
 *
 * \param ctx   pointer to the context structure.
 */
void br_md5sha1_init(br_md5sha1_context *ctx);

/**
 * \brief Inject some data bytes in a running MD5+SHA-1 computation.
 *
 * The provided context is updated with some data bytes. If the number
 * of bytes (`len`) is zero, then the data pointer (`data`) is ignored
 * and may be `NULL`, and this function does nothing.
 *
 * \param ctx    pointer to the context structure.
 * \param data   pointer to the injected data.
 * \param len    injected data length (in bytes).
 */
void br_md5sha1_update(br_md5sha1_context *ctx, const void *data, size_t len);

/**
 * \brief Compute MD5+SHA-1 output.
 *
 * The MD5+SHA-1 output for the concatenation of all bytes injected in the
 * provided context since the last initialisation or reset call, is
 * computed and written in the buffer pointed to by `out`. The context
 * itself is not modified, so extra bytes may be injected afterwards
 * to continue that computation.
 *
 * \param ctx   pointer to the context structure.
 * \param out   destination buffer for the hash output.
 */
void br_md5sha1_out(const br_md5sha1_context *ctx, void *out);

/**
 * \brief Save MD5+SHA-1 running state.
 *
 * The running state for MD5+SHA-1 (output of the last internal block
 * processing) is written in the buffer pointed to by `out`. The
 * number of bytes injected since the last initialisation or reset
 * call is returned. The context is not modified.
 *
 * \param ctx   pointer to the context structure.
 * \param out   destination buffer for the running state.
 * \return  the injected total byte length.
 */
uint64_t br_md5sha1_state(const br_md5sha1_context *ctx, void *out);

/**
 * \brief Restore MD5+SHA-1 running state.
 *
 * The running state for MD5+SHA-1 is set to the provided values.
 *
 * \param ctx     pointer to the context structure.
 * \param stb     source buffer for the running state.
 * \param count   the injected total byte length.
 */
void br_md5sha1_set_state(br_md5sha1_context *ctx,
	const void *stb, uint64_t count);

/**
 * \brief Aggregate context for configurable hash function support.
 *
 * The `br_hash_compat_context` type is a type which is large enough to
 * serve as context for all standard hash functions defined above.
 */
typedef union {
	const br_hash_class *vtable;
	br_md5_context md5;
	br_sha1_context sha1;
	br_sha224_context sha224;
	br_sha256_context sha256;
	br_sha384_context sha384;
	br_sha512_context sha512;
	br_md5sha1_context md5sha1;
} br_hash_compat_context;

/*
 * The multi-hasher is a construct that handles hashing of the same input
 * data with several hash functions, with a single shared input buffer.
 * It can handle MD5, SHA-1, SHA-224, SHA-256, SHA-384 and SHA-512
 * simultaneously, though which functions are activated depends on
 * the set implementation pointers.
 */

/**
 * \brief Multi-hasher context structure.
 *
 * The multi-hasher runs up to six hash functions in the standard TLS list
 * (MD5, SHA-1, SHA-224, SHA-256, SHA-384 and SHA-512) in parallel, over
 * the same input.
 *
 * The multi-hasher does _not_ follow the OOP structure with a vtable.
 * Instead, it is configured with the vtables of the hash functions it
 * should run. Structure fields are not supposed to be accessed directly.
 */
typedef struct {
#ifndef BR_DOXYGEN_IGNORE
	unsigned char buf[128];
	uint64_t count;
	uint32_t val_32[25];
	uint64_t val_64[16];
	const br_hash_class *impl[6];
#endif
} br_multihash_context;

/**
 * \brief Clear a multi-hasher context.
 *
 * This should always be called once on a given context, _before_ setting
 * the implementation pointers.
 *
 * \param ctx   the multi-hasher context.
 */
void br_multihash_zero(br_multihash_context *ctx);

/**
 * \brief Set a hash function implementation.
 *
 * Implementations shall be set _after_ clearing the context (with
 * `br_multihash_zero()`) but _before_ initialising the computation
 * (with `br_multihash_init()`). The hash function implementation
 * MUST be one of the standard hash functions (MD5, SHA-1, SHA-224,
 * SHA-256, SHA-384 or SHA-512); it may also be `NULL` to remove
 * an implementation from the multi-hasher.
 *
 * \param ctx    the multi-hasher context.
 * \param id     the hash function symbolic identifier.
 * \param impl   the hash function vtable, or `NULL`.
 */
static inline void
br_multihash_setimpl(br_multihash_context *ctx,
	int id, const br_hash_class *impl)
{
	/*
	 * This code relies on hash functions ID being values 1 to 6,
	 * in the MD5 to SHA-512 order.
	 */
	ctx->impl[id - 1] = impl;
}

/**
 * \brief Get a hash function implementation.
 *
 * This function returns the currently configured vtable for a given
 * hash function (by symbolic ID). If no such function was configured in
 * the provided multi-hasher context, then this function returns `NULL`.
 *
 * \param ctx    the multi-hasher context.
 * \param id     the hash function symbolic identifier.
 * \return  the hash function vtable, or `NULL`.
 */
static inline const br_hash_class *
br_multihash_getimpl(const br_multihash_context *ctx, int id)
{
	return ctx->impl[id - 1];
}

/**
 * \brief Reset a multi-hasher context.
 *
 * This function prepares the context for a new hashing computation,
 * for all implementations configured at that point.
 *
 * \param ctx    the multi-hasher context.
 */
void br_multihash_init(br_multihash_context *ctx);

/**
 * \brief Inject some data bytes in a running multi-hashing computation.
 *
 * The provided context is updated with some data bytes. If the number
 * of bytes (`len`) is zero, then the data pointer (`data`) is ignored
 * and may be `NULL`, and this function does nothing.
 *
 * \param ctx    pointer to the context structure.
 * \param data   pointer to the injected data.
 * \param len    injected data length (in bytes).
 */
void br_multihash_update(br_multihash_context *ctx,
	const void *data, size_t len);

/**
 * \brief Compute a hash output from a multi-hasher.
 *
 * The hash output for the concatenation of all bytes injected in the
 * provided context since the last initialisation or reset call, is
 * computed and written in the buffer pointed to by `dst`. The hash
 * function to use is identified by `id` and must be one of the standard
 * hash functions. If that hash function was indeed configured in the
 * multi-hasher context, the corresponding hash value is written in
 * `dst` and its length (in bytes) is returned. If the hash function
 * was _not_ configured, then nothing is written in `dst` and 0 is
 * returned.
 *
 * The context itself is not modified, so extra bytes may be injected
 * afterwards to continue the hash computations.
 *
 * \param ctx   pointer to the context structure.
 * \param id    the hash function symbolic identifier.
 * \param dst   destination buffer for the hash output.
 * \return  the hash output length (in bytes), or 0.
 */
size_t br_multihash_out(const br_multihash_context *ctx, int id, void *dst);

/**
 * \brief Type for a GHASH implementation.
 *
 * GHASH is a sort of keyed hash meant to be used to implement GCM in
 * combination with a block cipher (with 16-byte blocks).
 *
 * The `y` array has length 16 bytes and is used for input and output; in
 * a complete GHASH run, it starts with an all-zero value. `h` is a 16-byte
 * value that serves as key (it is derived from the encryption key in GCM,
 * using the block cipher). The data length (`len`) is expressed in bytes.
 * The `y` array is updated.
 *
 * If the data length is not a multiple of 16, then the data is implicitly
 * padded with zeros up to the next multiple of 16. Thus, when using GHASH
 * in GCM, this method may be called twice, for the associated data and
 * for the ciphertext, respectively; the zero-padding implements exactly
 * the GCM rules.
 *
 * \param y      the array to update.
 * \param h      the GHASH key.
 * \param data   the input data (may be `NULL` if `len` is zero).
 * \param len    the input data length (in bytes).
 */
typedef void (*br_ghash)(void *y, const void *h, const void *data, size_t len);

/**
 * \brief GHASH implementation using multiplications (mixed 32-bit).
 *
 * This implementation uses multiplications of 32-bit values, with a
 * 64-bit result. It is constant-time (if multiplications are
 * constant-time).
 *
 * \param y      the array to update.
 * \param h      the GHASH key.
 * \param data   the input data (may be `NULL` if `len` is zero).
 * \param len    the input data length (in bytes).
 */
void br_ghash_ctmul(void *y, const void *h, const void *data, size_t len);

/**
 * \brief GHASH implementation using multiplications (strict 32-bit).
 *
 * This implementation uses multiplications of 32-bit values, with a
 * 32-bit result. It is usually somewhat slower than `br_ghash_ctmul()`,
 * but it is expected to be faster on architectures for which the
 * 32-bit multiplication opcode does not yield the upper 32 bits of the
 * product. It is constant-time (if multiplications are constant-time).
 *
 * \param y      the array to update.
 * \param h      the GHASH key.
 * \param data   the input data (may be `NULL` if `len` is zero).
 * \param len    the input data length (in bytes).
 */
void br_ghash_ctmul32(void *y, const void *h, const void *data, size_t len);

/**
 * \brief GHASH implementation using multiplications (64-bit).
 *
 * This implementation uses multiplications of 64-bit values, with a
 * 64-bit result. It is constant-time (if multiplications are
 * constant-time). It is substantially faster than `br_ghash_ctmul()`
 * and `br_ghash_ctmul32()` on most 64-bit architectures.
 *
 * \param y      the array to update.
 * \param h      the GHASH key.
 * \param data   the input data (may be `NULL` if `len` is zero).
 * \param len    the input data length (in bytes).
 */
void br_ghash_ctmul64(void *y, const void *h, const void *data, size_t len);

#endif


/* ===== inc/bearssl_block.h ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef BR_BEARSSL_BLOCK_H__
#define BR_BEARSSL_BLOCK_H__

#include <stddef.h>
#include <stdint.h>

/** \file bearssl_block.h
 *
 * # Block Ciphers and Symmetric Ciphers
 *
 * This file documents the API for block ciphers and other symmetric
 * ciphers.
 *
 *
 * ## Procedural API
 *
 * For a block cipher implementation, up to three separate sets of
 * functions are provided, for CBC encryption, CBC decryption, and CTR
 * encryption/decryption. Each set has its own context structure,
 * initialised with the encryption key.
 *
 * For CBC encryption and decryption, the data to encrypt or decrypt is
 * referenced as a sequence of blocks. The implementations assume that
 * there is no partial block; no padding is applied or removed. The
 * caller is responsible for handling any kind of padding.
 *
 * Function for CTR encryption are defined only for block ciphers with
 * blocks of 16 bytes or more (i.e. AES, but not DES/3DES).
 *
 * Each implemented block cipher is identified by an "internal name"
 * from which are derived the names of structures and functions that
 * implement the cipher. For the block cipher of internal name "`xxx`",
 * the following are defined:
 *
 *   - `br_xxx_BLOCK_SIZE`
 *
 *     A macro that evaluates to the block size (in bytes) of the
 *     cipher. For all implemented block ciphers, this value is a
 *     power of two.
 *
 *   - `br_xxx_cbcenc_keys`
 *
 *     Context structure that contains the subkeys resulting from the key
 *     expansion. These subkeys are appropriate for CBC encryption. The
 *     structure first field is called `vtable` and points to the
 *     appropriate OOP structure.
 *
 *   - `br_xxx_cbcenc_init(br_xxx_cbcenc_keys *ctx, const void *key, size_t len)`
 *
 *     Perform key expansion: subkeys for CBC encryption are computed and
 *     written in the provided context structure. The key length MUST be
 *     adequate for the implemented block cipher. This function also sets
 *     the `vtable` field.
 *
 *   - `br_xxx_cbcenc_run(const br_xxx_cbcenc_keys *ctx, void *iv, void *data, size_t len)`
 *
 *     Perform CBC encryption of `len` bytes, in place. The encrypted data
 *     replaces the cleartext. `len` MUST be a multiple of the block length
 *     (if it is not, the function may loop forever or overflow a buffer).
 *     The IV is provided with the `iv` pointer; it is also updated with
 *     a copy of the last encrypted block.
 *
 *   - `br_xxx_cbcdec_keys`
 *
 *     Context structure that contains the subkeys resulting from the key
 *     expansion. These subkeys are appropriate for CBC decryption. The
 *     structure first field is called `vtable` and points to the
 *     appropriate OOP structure.
 *
 *   - `br_xxx_cbcdec_init(br_xxx_cbcenc_keys *ctx, const void *key, size_t len)`
 *
 *     Perform key expansion: subkeys for CBC decryption are computed and
 *     written in the provided context structure. The key length MUST be
 *     adequate for the implemented block cipher. This function also sets
 *     the `vtable` field.
 *
 *   - `br_xxx_cbcdec_run(const br_xxx_cbcdec_keys *ctx, void *iv, void *data, size_t num_blocks)`
 *
 *     Perform CBC decryption of `len` bytes, in place. The decrypted data
 *     replaces the ciphertext. `len` MUST be a multiple of the block length
 *     (if it is not, the function may loop forever or overflow a buffer).
 *     The IV is provided with the `iv` pointer; it is also updated with
 *     a copy of the last _encrypted_ block.
 *
 *   - `br_xxx_ctr_keys`
 *
 *     Context structure that contains the subkeys resulting from the key
 *     expansion. These subkeys are appropriate for CTR encryption and
 *     decryption. The structure first field is called `vtable` and
 *     points to the appropriate OOP structure.
 *
 *   - `br_xxx_ctr_init(br_xxx_ctr_keys *ctx, const void *key, size_t len)`
 *
 *     Perform key expansion: subkeys for CTR encryption and decryption
 *     are computed and written in the provided context structure. The
 *     key length MUST be adequate for the implemented block cipher. This
 *     function also sets the `vtable` field.
 *
 *   - `br_xxx_ctr_run(const br_xxx_ctr_keys *ctx, const void *iv, uint32_t cc, void *data, size_t len)` (returns `uint32_t`)
 *
 *     Perform CTR encryption/decryption of some data. Processing is done
 *     "in place" (the output data replaces the input data). This function
 *     implements the "standard incrementing function" from NIST SP800-38A,
 *     annex B: the IV length shall be 4 bytes less than the block size
 *     (i.e. 12 bytes for AES) and the counter is the 32-bit value starting
 *     with `cc`. The data length (`len`) is not necessarily a multiple of
 *     the block size. The new counter value is returned, which supports
 *     chunked processing, provided that each chunk length (except possibly
 *     the last one) is a multiple of the block size.
 *
 *
 * It shall be noted that the key expansion functions return `void`. If
 * the provided key length is not allowed, then there will be no error
 * reporting; implementations need not validate the key length, thus an
 * invalid key length may result in undefined behaviour (e.g. buffer
 * overflow).
 *
 * Subkey structures contain no interior pointer, and no external
 * resources are allocated upon key expansion. They can thus be
 * discarded without any explicit deallocation.
 *
 *
 * ## Object-Oriented API
 *
 * Each context structure begins with a field (called `vtable`) that
 * points to an instance of a structure that references the relevant
 * functions through pointers. Each such structure contains the
 * following:
 *
 *   - `context_size`
 *
 *     The size (in bytes) of the context structure for subkeys.
 *
 *   - `block_size`
 *
 *     The cipher block size (in bytes).
 *
 *   - `log_block_size`
 *
 *     The base-2 logarithm of cipher block size (e.g. 4 for blocks
 *     of 16 bytes).
 *
 *   - `init`
 *
 *     Pointer to the key expansion function.
 *
 *   - `run`
 *
 *     Pointer to the encryption/decryption function.
 *
 *
 * For block cipher "`xxx`", static, constant instances of these
 * structures are defined, under the names:
 *
 *   - `br_xxx_cbcenc_vtable`
 *   - `br_xxx_cbcdec_vtable`
 *   - `br_xxx_ctr_vtable`
 *
 *
 * ## Implemented Block Ciphers
 * 
 * Provided implementations are:
 *
 * | Name      | Function | Block Size (bytes) | Key lengths (bytes) |
 * | :-------- | :------- | :----------------: | :-----------------: |
 * | aes_big   | AES      |        16          | 16, 24 and 32       |
 * | aes_small | AES      |        16          | 16, 24 and 32       |
 * | aes_ct    | AES      |        16          | 16, 24 and 32       |
 * | aes_ct64  | AES      |        16          | 16, 24 and 32       |
 * | des_ct    | DES/3DES |         8          | 8, 16 and 24        |
 * | des_tab   | DES/3DES |         8          | 8, 16 and 24        |
 *
 * **Note:** DES/3DES nominally uses keys of 64, 128 and 192 bits (i.e. 8,
 * 16 and 24 bytes), but some of the bits are ignored by the algorithm, so
 * the _effective_ key lengths, from a security point of view, are 56,
 * 112 and 168 bits, respectively.
 *
 * `aes_big` is a "classical" AES implementation, using tables. It
 * is fast but not constant-time, since it makes data-dependent array
 * accesses.
 *
 * `aes_small` is an AES implementation optimized for code size. It
 * is substantially slower than `aes_big`; it is not constant-time
 * either.
 *
 * `aes_ct` is a constant-time implementation of AES; its code is about
 * as big as that of `aes_big`, while its performance is comparable to
 * that of `aes_small`. However, it is constant-time. This
 * implementation should thus be considered to be the "default" AES in
 * BearSSL, to be used unless the operational context guarantees that a
 * non-constant-time implementation is safe, or an architecture-specific
 * constant-time implementation can be used (e.g. using dedicated
 * hardware opcodes).
 *
 * `aes_ct64` is another constant-time implementation of AES. It is
 * similar to `aes_ct` but uses 64-bit values. On 32-bit machines,
 * `aes_ct64` is not faster than `aes_ct`, often a bit slower, and has
 * a larger footprint; however, on 64-bit architectures, `aes_ct64`
 * is typically twice faster than `aes_ct` for modes that allow parallel
 * operations (i.e. CTR, and CBC decryption, but not CBC encryption).
 *
 * `des_tab` is a classic, table-based implementation of DES/3DES. It
 * is not constant-time.
 *
 * `des_ct` is an constant-time implementation of DES/3DES. It is
 * substantially slower than `des_tab`.
 *
 * ## ChaCha20 and Poly1305
 *
 * ChaCha20 is a stream cipher. Poly1305 is a MAC algorithm. They
 * are described in [RFC 7539](https://tools.ietf.org/html/rfc7539).
 *
 * Two function pointer types are defined:
 *
 *   - `br_chacha20_run` describes a function that implements ChaCha20
 *     only.
 *
 *   - `br_poly1305_run` describes an implementation of Poly1305,
 *     in the AEAD combination with ChaCha20 specified in RFC 7539
 *     (the ChaCha20 implementation is provided as a function pointer).
 *
 * `chacha20_ct` is a straightforward implementation of ChaCha20 in
 * plain C; it is constant-time, small, and reasonably fast.
 *
 * `poly1305_ctmul` is an implementation of the ChaCha20+Poly1305 AEAD
 * construction, where the Poly1305 part is performed with mixed 32-bit
 * multiplications (operands are 32-bit, result is 64-bit).
 */

/**
 * \brief Class type for CBC encryption implementations.
 *
 * A `br_block_cbcenc_class` instance points to the functions implementing
 * a specific block cipher, when used in CBC mode for encrypting data.
 */
typedef struct br_block_cbcenc_class_ br_block_cbcenc_class;
struct br_block_cbcenc_class_ {
	/**
	 * \brief Size (in bytes) of the context structure appropriate
	 * for containing subkeys.
	 */
	size_t context_size;

	/**
	 * \brief Size of individual blocks (in bytes).
	 */
	unsigned block_size;

	/**
	 * \brief Base-2 logarithm of the size of individual blocks,
	 * expressed in bytes.
	 */
	unsigned log_block_size;

	/**
	 * \brief Initialisation function.
	 *
	 * This function sets the `vtable` field in the context structure.
	 * The key length MUST be one of the key lengths supported by
	 * the implementation.
	 *
	 * \param ctx       context structure to initialise.
	 * \param key       secret key.
	 * \param key_len   key length (in bytes).
	 */
	void (*init)(const br_block_cbcenc_class **ctx,
		const void *key, size_t key_len);

	/**
	 * \brief Run the CBC encryption.
	 *
	 * The `iv` parameter points to the IV for this run; it is
	 * updated with a copy of the last encrypted block. The data
	 * is encrypted "in place"; its length (`len`) MUST be a
	 * multiple of the block size.
	 *
	 * \param ctx    context structure (already initialised).
	 * \param iv     IV for CBC encryption (updated).
	 * \param data   data to encrypt.
	 * \param len    data length (in bytes, multiple of block size).
	 */
	void (*run)(const br_block_cbcenc_class *const *ctx,
		void *iv, void *data, size_t len);
};

/**
 * \brief Class type for CBC decryption implementations.
 *
 * A `br_block_cbcdec_class` instance points to the functions implementing
 * a specific block cipher, when used in CBC mode for decrypting data.
 */
typedef struct br_block_cbcdec_class_ br_block_cbcdec_class;
struct br_block_cbcdec_class_ {
	/**
	 * \brief Size (in bytes) of the context structure appropriate
	 * for containing subkeys.
	 */
	size_t context_size;

	/**
	 * \brief Size of individual blocks (in bytes).
	 */
	unsigned block_size;

	/**
	 * \brief Base-2 logarithm of the size of individual blocks,
	 * expressed in bytes.
	 */
	unsigned log_block_size;

	/**
	 * \brief Initialisation function.
	 *
	 * This function sets the `vtable` field in the context structure.
	 * The key length MUST be one of the key lengths supported by
	 * the implementation.
	 *
	 * \param ctx       context structure to initialise.
	 * \param key       secret key.
	 * \param key_len   key length (in bytes).
	 */
	void (*init)(const br_block_cbcdec_class **ctx,
		const void *key, size_t key_len);

	/**
	 * \brief Run the CBC decryption.
	 *
	 * The `iv` parameter points to the IV for this run; it is
	 * updated with a copy of the last encrypted block. The data
	 * is decrypted "in place"; its length (`len`) MUST be a
	 * multiple of the block size.
	 *
	 * \param ctx    context structure (already initialised).
	 * \param iv     IV for CBC decryption (updated).
	 * \param data   data to decrypt.
	 * \param len    data length (in bytes, multiple of block size).
	 */
	void (*run)(const br_block_cbcdec_class *const *ctx,
		void *iv, void *data, size_t len);
};

/**
 * \brief Class type for CTR encryption/decryption implementations.
 *
 * A `br_block_ctr_class` instance points to the functions implementing
 * a specific block cipher, when used in CTR mode for encrypting or
 * decrypting data.
 */
typedef struct br_block_ctr_class_ br_block_ctr_class;
struct br_block_ctr_class_ {
	/**
	 * \brief Size (in bytes) of the context structure appropriate
	 * for containing subkeys.
	 */
	size_t context_size;

	/**
	 * \brief Size of individual blocks (in bytes).
	 */
	unsigned block_size;

	/**
	 * \brief Base-2 logarithm of the size of individual blocks,
	 * expressed in bytes.
	 */
	unsigned log_block_size;

	/**
	 * \brief Initialisation function.
	 *
	 * This function sets the `vtable` field in the context structure.
	 * The key length MUST be one of the key lengths supported by
	 * the implementation.
	 *
	 * \param ctx       context structure to initialise.
	 * \param key       secret key.
	 * \param key_len   key length (in bytes).
	 */
	void (*init)(const br_block_ctr_class **ctx,
		const void *key, size_t key_len);

	/**
	 * \brief Run the CTR encryption or decryption.
	 *
	 * The `iv` parameter points to the IV for this run; its
	 * length is exactly 4 bytes less than the block size (e.g.
	 * 12 bytes for AES/CTR). The IV is combined with a 32-bit
	 * block counter to produce the block value which is processed
	 * with the block cipher.
	 *
	 * The data to encrypt or decrypt is updated "in place". Its
	 * length (`len` bytes) is not required to be a multiple of
	 * the block size; if the final block is partial, then the
	 * corresponding key stream bits are dropped.
	 *
	 * The resulting counter value is returned.
	 *
	 * \param ctx    context structure (already initialised).
	 * \param iv     IV for CTR encryption/decryption.
	 * \param cc     initial value for the block counter.
	 * \param data   data to encrypt or decrypt.
	 * \param len    data length (in bytes).
	 * \return  the new block counter value.
	 */
	uint32_t (*run)(const br_block_ctr_class *const *ctx,
		const void *iv, uint32_t cc, void *data, size_t len);
};

/*
 * Traditional, table-based AES implementation. It is fast, but uses
 * internal tables (in particular a 1 kB table for encryption, another
 * 1 kB table for decryption, and a 256-byte table for key schedule),
 * and it is not constant-time. In contexts where cache-timing attacks
 * apply, this implementation may leak the secret key.
 */

/** \brief AES block size (16 bytes). */
#define br_aes_big_BLOCK_SIZE   16

/**
 * \brief Context for AES subkeys (`aes_big` implementation, CBC encryption).
 *
 * First field is a pointer to the vtable; it is set by the initialisation
 * function. Other fields are not supposed to be accessed by user code.
 */
typedef struct {
	/** \brief Pointer to vtable for this context. */
	const br_block_cbcenc_class *vtable;
#ifndef BR_DOXYGEN_IGNORE
	uint32_t skey[60];
	unsigned num_rounds;
#endif
} br_aes_big_cbcenc_keys;

/**
 * \brief Context for AES subkeys (`aes_big` implementation, CBC decryption).
 *
 * First field is a pointer to the vtable; it is set by the initialisation
 * function. Other fields are not supposed to be accessed by user code.
 */
typedef struct {
	/** \brief Pointer to vtable for this context. */
	const br_block_cbcdec_class *vtable;
#ifndef BR_DOXYGEN_IGNORE
	uint32_t skey[60];
	unsigned num_rounds;
#endif
} br_aes_big_cbcdec_keys;

/**
 * \brief Context for AES subkeys (`aes_big` implementation, CTR encryption
 * and decryption).
 *
 * First field is a pointer to the vtable; it is set by the initialisation
 * function. Other fields are not supposed to be accessed by user code.
 */
typedef struct {
	/** \brief Pointer to vtable for this context. */
	const br_block_ctr_class *vtable;
#ifndef BR_DOXYGEN_IGNORE
	uint32_t skey[60];
	unsigned num_rounds;
#endif
} br_aes_big_ctr_keys;

/**
 * \brief Class instance for AES CBC encryption (`aes_big` implementation).
 */
extern const br_block_cbcenc_class br_aes_big_cbcenc_vtable;

/**
 * \brief Class instance for AES CBC decryption (`aes_big` implementation).
 */
extern const br_block_cbcdec_class br_aes_big_cbcdec_vtable;

/**
 * \brief Class instance for AES CTR encryption and decryption
 * (`aes_big` implementation).
 */
extern const br_block_ctr_class br_aes_big_ctr_vtable;

/**
 * \brief Context initialisation (key schedule) for AES CBC encryption
 * (`aes_big` implementation).
 *
 * \param ctx   context to initialise.
 * \param key   secret key.
 * \param len   secret key length (in bytes).
 */
void br_aes_big_cbcenc_init(br_aes_big_cbcenc_keys *ctx,
	const void *key, size_t len);

/**
 * \brief Context initialisation (key schedule) for AES CBC decryption
 * (`aes_big` implementation).
 *
 * \param ctx   context to initialise.
 * \param key   secret key.
 * \param len   secret key length (in bytes).
 */
void br_aes_big_cbcdec_init(br_aes_big_cbcdec_keys *ctx,
	const void *key, size_t len);

/**
 * \brief Context initialisation (key schedule) for AES CTR encryption
 * and decryption (`aes_big` implementation).
 *
 * \param ctx   context to initialise.
 * \param key   secret key.
 * \param len   secret key length (in bytes).
 */
void br_aes_big_ctr_init(br_aes_big_ctr_keys *ctx,
	const void *key, size_t len);

/**
 * \brief CBC encryption with AES (`aes_big` implementation).
 *
 * \param ctx    context (already initialised).
 * \param iv     IV (updated).
 * \param data   data to encrypt (updated).
 * \param len    data length (in bytes, MUST be multiple of 16).
 */
void br_aes_big_cbcenc_run(const br_aes_big_cbcenc_keys *ctx, void *iv,
	void *data, size_t len);

/**
 * \brief CBC decryption with AES (`aes_big` implementation).
 *
 * \param ctx    context (already initialised).
 * \param iv     IV (updated).
 * \param data   data to decrypt (updated).
 * \param len    data length (in bytes, MUST be multiple of 16).
 */
void br_aes_big_cbcdec_run(const br_aes_big_cbcdec_keys *ctx, void *iv,
	void *data, size_t len);

/**
 * \brief CTR encryption and decryption with AES (`aes_big` implementation).
 *
 * \param ctx    context (already initialised).
 * \param iv     IV (constant, 12 bytes).
 * \param cc     initial block counter value.
 * \param data   data to decrypt (updated).
 * \param len    data length (in bytes).
 * \return  new block counter value.
 */
uint32_t br_aes_big_ctr_run(const br_aes_big_ctr_keys *ctx,
	const void *iv, uint32_t cc, void *data, size_t len);

/*
 * AES implementation optimized for size. It is slower than the
 * traditional table-based AES implementation, but requires much less
 * code. It still uses data-dependent table accesses (albeit within a
 * much smaller 256-byte table), which makes it conceptually vulnerable
 * to cache-timing attacks.
 */

/** \brief AES block size (16 bytes). */
#define br_aes_small_BLOCK_SIZE   16

/**
 * \brief Context for AES subkeys (`aes_small` implementation, CBC encryption).
 *
 * First field is a pointer to the vtable; it is set by the initialisation
 * function. Other fields are not supposed to be accessed by user code.
 */
typedef struct {
	/** \brief Pointer to vtable for this context. */
	const br_block_cbcenc_class *vtable;
#ifndef BR_DOXYGEN_IGNORE
	uint32_t skey[60];
	unsigned num_rounds;
#endif
} br_aes_small_cbcenc_keys;

/**
 * \brief Context for AES subkeys (`aes_small` implementation, CBC decryption).
 *
 * First field is a pointer to the vtable; it is set by the initialisation
 * function. Other fields are not supposed to be accessed by user code.
 */
typedef struct {
	/** \brief Pointer to vtable for this context. */
	const br_block_cbcdec_class *vtable;
#ifndef BR_DOXYGEN_IGNORE
	uint32_t skey[60];
	unsigned num_rounds;
#endif
} br_aes_small_cbcdec_keys;

/**
 * \brief Context for AES subkeys (`aes_small` implementation, CTR encryption
 * and decryption).
 *
 * First field is a pointer to the vtable; it is set by the initialisation
 * function. Other fields are not supposed to be accessed by user code.
 */
typedef struct {
	/** \brief Pointer to vtable for this context. */
	const br_block_ctr_class *vtable;
#ifndef BR_DOXYGEN_IGNORE
	uint32_t skey[60];
	unsigned num_rounds;
#endif
} br_aes_small_ctr_keys;

/**
 * \brief Class instance for AES CBC encryption (`aes_small` implementation).
 */
extern const br_block_cbcenc_class br_aes_small_cbcenc_vtable;

/**
 * \brief Class instance for AES CBC decryption (`aes_small` implementation).
 */
extern const br_block_cbcdec_class br_aes_small_cbcdec_vtable;

/**
 * \brief Class instance for AES CTR encryption and decryption
 * (`aes_small` implementation).
 */
extern const br_block_ctr_class br_aes_small_ctr_vtable;

/**
 * \brief Context initialisation (key schedule) for AES CBC encryption
 * (`aes_small` implementation).
 *
 * \param ctx   context to initialise.
 * \param key   secret key.
 * \param len   secret key length (in bytes).
 */
void br_aes_small_cbcenc_init(br_aes_small_cbcenc_keys *ctx,
	const void *key, size_t len);

/**
 * \brief Context initialisation (key schedule) for AES CBC decryption
 * (`aes_small` implementation).
 *
 * \param ctx   context to initialise.
 * \param key   secret key.
 * \param len   secret key length (in bytes).
 */
void br_aes_small_cbcdec_init(br_aes_small_cbcdec_keys *ctx,
	const void *key, size_t len);

/**
 * \brief Context initialisation (key schedule) for AES CTR encryption
 * and decryption (`aes_small` implementation).
 *
 * \param ctx   context to initialise.
 * \param key   secret key.
 * \param len   secret key length (in bytes).
 */
void br_aes_small_ctr_init(br_aes_small_ctr_keys *ctx,
	const void *key, size_t len);

/**
 * \brief CBC encryption with AES (`aes_small` implementation).
 *
 * \param ctx    context (already initialised).
 * \param iv     IV (updated).
 * \param data   data to encrypt (updated).
 * \param len    data length (in bytes, MUST be multiple of 16).
 */
void br_aes_small_cbcenc_run(const br_aes_small_cbcenc_keys *ctx, void *iv,
	void *data, size_t len);

/**
 * \brief CBC decryption with AES (`aes_small` implementation).
 *
 * \param ctx    context (already initialised).
 * \param iv     IV (updated).
 * \param data   data to decrypt (updated).
 * \param len    data length (in bytes, MUST be multiple of 16).
 */
void br_aes_small_cbcdec_run(const br_aes_small_cbcdec_keys *ctx, void *iv,
	void *data, size_t len);

/**
 * \brief CTR encryption and decryption with AES (`aes_small` implementation).
 *
 * \param ctx    context (already initialised).
 * \param iv     IV (constant, 12 bytes).
 * \param cc     initial block counter value.
 * \param data   data to decrypt (updated).
 * \param len    data length (in bytes).
 * \return  new block counter value.
 */
uint32_t br_aes_small_ctr_run(const br_aes_small_ctr_keys *ctx,
	const void *iv, uint32_t cc, void *data, size_t len);

/*
 * Constant-time AES implementation. Its size is similar to that of
 * 'aes_big', and its performance is similar to that of 'aes_small' (faster
 * decryption, slower encryption). However, it is constant-time, i.e.
 * immune to cache-timing and similar attacks.
 */

/** \brief AES block size (16 bytes). */
#define br_aes_ct_BLOCK_SIZE   16

/**
 * \brief Context for AES subkeys (`aes_ct` implementation, CBC encryption).
 *
 * First field is a pointer to the vtable; it is set by the initialisation
 * function. Other fields are not supposed to be accessed by user code.
 */
typedef struct {
	/** \brief Pointer to vtable for this context. */
	const br_block_cbcenc_class *vtable;
#ifndef BR_DOXYGEN_IGNORE
	uint32_t skey[60];
	unsigned num_rounds;
#endif
} br_aes_ct_cbcenc_keys;

/**
 * \brief Context for AES subkeys (`aes_ct` implementation, CBC decryption).
 *
 * First field is a pointer to the vtable; it is set by the initialisation
 * function. Other fields are not supposed to be accessed by user code.
 */
typedef struct {
	/** \brief Pointer to vtable for this context. */
	const br_block_cbcdec_class *vtable;
#ifndef BR_DOXYGEN_IGNORE
	uint32_t skey[60];
	unsigned num_rounds;
#endif
} br_aes_ct_cbcdec_keys;

/**
 * \brief Context for AES subkeys (`aes_ct` implementation, CTR encryption
 * and decryption).
 *
 * First field is a pointer to the vtable; it is set by the initialisation
 * function. Other fields are not supposed to be accessed by user code.
 */
typedef struct {
	/** \brief Pointer to vtable for this context. */
	const br_block_ctr_class *vtable;
#ifndef BR_DOXYGEN_IGNORE
	uint32_t skey[60];
	unsigned num_rounds;
#endif
} br_aes_ct_ctr_keys;

/**
 * \brief Class instance for AES CBC encryption (`aes_ct` implementation).
 */
extern const br_block_cbcenc_class br_aes_ct_cbcenc_vtable;

/**
 * \brief Class instance for AES CBC decryption (`aes_ct` implementation).
 */
extern const br_block_cbcdec_class br_aes_ct_cbcdec_vtable;

/**
 * \brief Class instance for AES CTR encryption and decryption
 * (`aes_ct` implementation).
 */
extern const br_block_ctr_class br_aes_ct_ctr_vtable;

/**
 * \brief Context initialisation (key schedule) for AES CBC encryption
 * (`aes_ct` implementation).
 *
 * \param ctx   context to initialise.
 * \param key   secret key.
 * \param len   secret key length (in bytes).
 */
void br_aes_ct_cbcenc_init(br_aes_ct_cbcenc_keys *ctx,
	const void *key, size_t len);

/**
 * \brief Context initialisation (key schedule) for AES CBC decryption
 * (`aes_ct` implementation).
 *
 * \param ctx   context to initialise.
 * \param key   secret key.
 * \param len   secret key length (in bytes).
 */
void br_aes_ct_cbcdec_init(br_aes_ct_cbcdec_keys *ctx,
	const void *key, size_t len);

/**
 * \brief Context initialisation (key schedule) for AES CTR encryption
 * and decryption (`aes_ct` implementation).
 *
 * \param ctx   context to initialise.
 * \param key   secret key.
 * \param len   secret key length (in bytes).
 */
void br_aes_ct_ctr_init(br_aes_ct_ctr_keys *ctx,
	const void *key, size_t len);

/**
 * \brief CBC encryption with AES (`aes_ct` implementation).
 *
 * \param ctx    context (already initialised).
 * \param iv     IV (updated).
 * \param data   data to encrypt (updated).
 * \param len    data length (in bytes, MUST be multiple of 16).
 */
void br_aes_ct_cbcenc_run(const br_aes_ct_cbcenc_keys *ctx, void *iv,
	void *data, size_t len);

/**
 * \brief CBC decryption with AES (`aes_ct` implementation).
 *
 * \param ctx    context (already initialised).
 * \param iv     IV (updated).
 * \param data   data to decrypt (updated).
 * \param len    data length (in bytes, MUST be multiple of 16).
 */
void br_aes_ct_cbcdec_run(const br_aes_ct_cbcdec_keys *ctx, void *iv,
	void *data, size_t len);

/**
 * \brief CTR encryption and decryption with AES (`aes_ct` implementation).
 *
 * \param ctx    context (already initialised).
 * \param iv     IV (constant, 12 bytes).
 * \param cc     initial block counter value.
 * \param data   data to decrypt (updated).
 * \param len    data length (in bytes).
 * \return  new block counter value.
 */
uint32_t br_aes_ct_ctr_run(const br_aes_ct_ctr_keys *ctx,
	const void *iv, uint32_t cc, void *data, size_t len);

/*
 * 64-bit constant-time AES implementation. It is similar to 'aes_ct'
 * but uses 64-bit registers, making it about twice faster than 'aes_ct'
 * on 64-bit platforms, while remaining constant-time and with a similar
 * code size. (The doubling in performance is only for CBC decryption
 * and CTR mode; CBC encryption is non-parallel and cannot benefit from
 * the larger registers.)
 */

/** \brief AES block size (16 bytes). */
#define br_aes_ct64_BLOCK_SIZE   16

/**
 * \brief Context for AES subkeys (`aes_ct64` implementation, CBC encryption).
 *
 * First field is a pointer to the vtable; it is set by the initialisation
 * function. Other fields are not supposed to be accessed by user code.
 */
typedef struct {
	/** \brief Pointer to vtable for this context. */
	const br_block_cbcenc_class *vtable;
#ifndef BR_DOXYGEN_IGNORE
	uint64_t skey[30];
	unsigned num_rounds;
#endif
} br_aes_ct64_cbcenc_keys;

/**
 * \brief Context for AES subkeys (`aes_ct64` implementation, CBC decryption).
 *
 * First field is a pointer to the vtable; it is set by the initialisation
 * function. Other fields are not supposed to be accessed by user code.
 */
typedef struct {
	/** \brief Pointer to vtable for this context. */
	const br_block_cbcdec_class *vtable;
#ifndef BR_DOXYGEN_IGNORE
	uint64_t skey[30];
	unsigned num_rounds;
#endif
} br_aes_ct64_cbcdec_keys;

/**
 * \brief Context for AES subkeys (`aes_ct64` implementation, CTR encryption
 * and decryption).
 *
 * First field is a pointer to the vtable; it is set by the initialisation
 * function. Other fields are not supposed to be accessed by user code.
 */
typedef struct {
	/** \brief Pointer to vtable for this context. */
	const br_block_ctr_class *vtable;
#ifndef BR_DOXYGEN_IGNORE
	uint64_t skey[30];
	unsigned num_rounds;
#endif
} br_aes_ct64_ctr_keys;

/**
 * \brief Class instance for AES CBC encryption (`aes_ct64` implementation).
 */
extern const br_block_cbcenc_class br_aes_ct64_cbcenc_vtable;

/**
 * \brief Class instance for AES CBC decryption (`aes_ct64` implementation).
 */
extern const br_block_cbcdec_class br_aes_ct64_cbcdec_vtable;

/**
 * \brief Class instance for AES CTR encryption and decryption
 * (`aes_ct64` implementation).
 */
extern const br_block_ctr_class br_aes_ct64_ctr_vtable;

/**
 * \brief Context initialisation (key schedule) for AES CBC encryption
 * (`aes_ct64` implementation).
 *
 * \param ctx   context to initialise.
 * \param key   secret key.
 * \param len   secret key length (in bytes).
 */
void br_aes_ct64_cbcenc_init(br_aes_ct64_cbcenc_keys *ctx,
	const void *key, size_t len);

/**
 * \brief Context initialisation (key schedule) for AES CBC decryption
 * (`aes_ct64` implementation).
 *
 * \param ctx   context to initialise.
 * \param key   secret key.
 * \param len   secret key length (in bytes).
 */
void br_aes_ct64_cbcdec_init(br_aes_ct64_cbcdec_keys *ctx,
	const void *key, size_t len);

/**
 * \brief Context initialisation (key schedule) for AES CTR encryption
 * and decryption (`aes_ct64` implementation).
 *
 * \param ctx   context to initialise.
 * \param key   secret key.
 * \param len   secret key length (in bytes).
 */
void br_aes_ct64_ctr_init(br_aes_ct64_ctr_keys *ctx,
	const void *key, size_t len);

/**
 * \brief CBC encryption with AES (`aes_ct64` implementation).
 *
 * \param ctx    context (already initialised).
 * \param iv     IV (updated).
 * \param data   data to encrypt (updated).
 * \param len    data length (in bytes, MUST be multiple of 16).
 */
void br_aes_ct64_cbcenc_run(const br_aes_ct64_cbcenc_keys *ctx, void *iv,
	void *data, size_t len);

/**
 * \brief CBC decryption with AES (`aes_ct64` implementation).
 *
 * \param ctx    context (already initialised).
 * \param iv     IV (updated).
 * \param data   data to decrypt (updated).
 * \param len    data length (in bytes, MUST be multiple of 16).
 */
void br_aes_ct64_cbcdec_run(const br_aes_ct64_cbcdec_keys *ctx, void *iv,
	void *data, size_t len);

/**
 * \brief CTR encryption and decryption with AES (`aes_ct64` implementation).
 *
 * \param ctx    context (already initialised).
 * \param iv     IV (constant, 12 bytes).
 * \param cc     initial block counter value.
 * \param data   data to decrypt (updated).
 * \param len    data length (in bytes).
 * \return  new block counter value.
 */
uint32_t br_aes_ct64_ctr_run(const br_aes_ct64_ctr_keys *ctx,
	const void *iv, uint32_t cc, void *data, size_t len);

/**
 * \brief Aggregate structure large enough to be used as context for
 * subkeys (CBC encryption) for all AES implementations.
 */
typedef union {
	const br_block_cbcenc_class *vtable;
	br_aes_big_cbcenc_keys big;
	br_aes_small_cbcenc_keys small;
	br_aes_ct_cbcenc_keys ct;
	br_aes_ct64_cbcenc_keys ct64;
} br_aes_gen_cbcenc_keys;

/**
 * \brief Aggregate structure large enough to be used as context for
 * subkeys (CBC decryption) for all AES implementations.
 */
typedef union {
	const br_block_cbcdec_class *vtable;
	br_aes_big_cbcdec_keys big;
	br_aes_small_cbcdec_keys small;
	br_aes_ct_cbcdec_keys ct;
	br_aes_ct64_cbcdec_keys ct64;
} br_aes_gen_cbcdec_keys;

/**
 * \brief Aggregate structure large enough to be used as context for
 * subkeys (CTR encryption and decryption) for all AES implementations.
 */
typedef union {
	const br_block_ctr_class *vtable;
	br_aes_big_ctr_keys big;
	br_aes_small_ctr_keys small;
	br_aes_ct_ctr_keys ct;
	br_aes_ct64_ctr_keys ct64;
} br_aes_gen_ctr_keys;

/*
 * Traditional, table-based implementation for DES/3DES. Since tables are
 * used, cache-timing attacks are conceptually possible.
 */

/** \brief DES/3DES block size (8 bytes). */
#define br_des_tab_BLOCK_SIZE   8

/**
 * \brief Context for DES subkeys (`des_tab` implementation, CBC encryption).
 *
 * First field is a pointer to the vtable; it is set by the initialisation
 * function. Other fields are not supposed to be accessed by user code.
 */
typedef struct {
	/** \brief Pointer to vtable for this context. */
	const br_block_cbcenc_class *vtable;
#ifndef BR_DOXYGEN_IGNORE
	uint32_t skey[96];
	unsigned num_rounds;
#endif
} br_des_tab_cbcenc_keys;

/**
 * \brief Context for DES subkeys (`des_tab` implementation, CBC decryption).
 *
 * First field is a pointer to the vtable; it is set by the initialisation
 * function. Other fields are not supposed to be accessed by user code.
 */
typedef struct {
	/** \brief Pointer to vtable for this context. */
	const br_block_cbcdec_class *vtable;
#ifndef BR_DOXYGEN_IGNORE
	uint32_t skey[96];
	unsigned num_rounds;
#endif
} br_des_tab_cbcdec_keys;

/**
 * \brief Class instance for DES CBC encryption (`des_tab` implementation).
 */
extern const br_block_cbcenc_class br_des_tab_cbcenc_vtable;

/**
 * \brief Class instance for DES CBC decryption (`des_tab` implementation).
 */
extern const br_block_cbcdec_class br_des_tab_cbcdec_vtable;

/**
 * \brief Context initialisation (key schedule) for DES CBC encryption
 * (`des_tab` implementation).
 *
 * \param ctx   context to initialise.
 * \param key   secret key.
 * \param len   secret key length (in bytes).
 */
void br_des_tab_cbcenc_init(br_des_tab_cbcenc_keys *ctx,
	const void *key, size_t len);

/**
 * \brief Context initialisation (key schedule) for DES CBC decryption
 * (`des_tab` implementation).
 *
 * \param ctx   context to initialise.
 * \param key   secret key.
 * \param len   secret key length (in bytes).
 */
void br_des_tab_cbcdec_init(br_des_tab_cbcdec_keys *ctx,
	const void *key, size_t len);

/**
 * \brief CBC encryption with DES (`des_tab` implementation).
 *
 * \param ctx    context (already initialised).
 * \param iv     IV (updated).
 * \param data   data to encrypt (updated).
 * \param len    data length (in bytes, MUST be multiple of 8).
 */
void br_des_tab_cbcenc_run(const br_des_tab_cbcenc_keys *ctx, void *iv,
	void *data, size_t len);

/**
 * \brief CBC decryption with DES (`des_tab` implementation).
 *
 * \param ctx    context (already initialised).
 * \param iv     IV (updated).
 * \param data   data to decrypt (updated).
 * \param len    data length (in bytes, MUST be multiple of 8).
 */
void br_des_tab_cbcdec_run(const br_des_tab_cbcdec_keys *ctx, void *iv,
	void *data, size_t len);

/*
 * Constant-time implementation for DES/3DES. It is substantially slower
 * (by a factor of about 4x), but also immune to cache-timing attacks.
 */

/** \brief DES/3DES block size (8 bytes). */
#define br_des_ct_BLOCK_SIZE   8

/**
 * \brief Context for DES subkeys (`des_ct` implementation, CBC encryption).
 *
 * First field is a pointer to the vtable; it is set by the initialisation
 * function. Other fields are not supposed to be accessed by user code.
 */
typedef struct {
	/** \brief Pointer to vtable for this context. */
	const br_block_cbcenc_class *vtable;
#ifndef BR_DOXYGEN_IGNORE
	uint32_t skey[96];
	unsigned num_rounds;
#endif
} br_des_ct_cbcenc_keys;

/**
 * \brief Context for DES subkeys (`des_ct` implementation, CBC decryption).
 *
 * First field is a pointer to the vtable; it is set by the initialisation
 * function. Other fields are not supposed to be accessed by user code.
 */
typedef struct {
	/** \brief Pointer to vtable for this context. */
	const br_block_cbcdec_class *vtable;
#ifndef BR_DOXYGEN_IGNORE
	uint32_t skey[96];
	unsigned num_rounds;
#endif
} br_des_ct_cbcdec_keys;

/**
 * \brief Class instance for DES CBC encryption (`des_ct` implementation).
 */
extern const br_block_cbcenc_class br_des_ct_cbcenc_vtable;

/**
 * \brief Class instance for DES CBC decryption (`des_ct` implementation).
 */
extern const br_block_cbcdec_class br_des_ct_cbcdec_vtable;

/**
 * \brief Context initialisation (key schedule) for DES CBC encryption
 * (`des_ct` implementation).
 *
 * \param ctx   context to initialise.
 * \param key   secret key.
 * \param len   secret key length (in bytes).
 */
void br_des_ct_cbcenc_init(br_des_ct_cbcenc_keys *ctx,
	const void *key, size_t len);

/**
 * \brief Context initialisation (key schedule) for DES CBC decryption
 * (`des_ct` implementation).
 *
 * \param ctx   context to initialise.
 * \param key   secret key.
 * \param len   secret key length (in bytes).
 */
void br_des_ct_cbcdec_init(br_des_ct_cbcdec_keys *ctx,
	const void *key, size_t len);

/**
 * \brief CBC encryption with DES (`des_ct` implementation).
 *
 * \param ctx    context (already initialised).
 * \param iv     IV (updated).
 * \param data   data to encrypt (updated).
 * \param len    data length (in bytes, MUST be multiple of 8).
 */
void br_des_ct_cbcenc_run(const br_des_ct_cbcenc_keys *ctx, void *iv,
	void *data, size_t len);

/**
 * \brief CBC decryption with DES (`des_ct` implementation).
 *
 * \param ctx    context (already initialised).
 * \param iv     IV (updated).
 * \param data   data to decrypt (updated).
 * \param len    data length (in bytes, MUST be multiple of 8).
 */
void br_des_ct_cbcdec_run(const br_des_ct_cbcdec_keys *ctx, void *iv,
	void *data, size_t len);

/*
 * These structures are large enough to accommodate subkeys for all
 * DES/3DES implementations.
 */

/**
 * \brief Aggregate structure large enough to be used as context for
 * subkeys (CBC encryption) for all DES implementations.
 */
typedef union {
	const br_block_cbcenc_class *vtable;
	br_des_tab_cbcenc_keys tab;
	br_des_ct_cbcenc_keys ct;
} br_des_gen_cbcenc_keys;

/**
 * \brief Aggregate structure large enough to be used as context for
 * subkeys (CBC decryption) for all DES implementations.
 */
typedef union {
	const br_block_cbcdec_class *vtable;
	br_des_tab_cbcdec_keys tab;
	br_des_ct_cbcdec_keys ct;
} br_des_gen_cbcdec_keys;

/**
 * \brief Type for a ChaCha20 implementation.
 *
 * An implementation follows the description in RFC 7539:
 *
 *   - Key is 256 bits (`key` points to exactly 32 bytes).
 *
 *   - IV is 96 bits (`iv` points to exactly 12 bytes).
 *
 *   - Block counter is over 32 bits and starts at value `cc`; the
 *     resulting value is returned.
 *
 * Data (pointed to by `data`, of length `len`) is encrypted/decrypted
 * in place. If `len` is not a multiple of 64, then the excess bytes from
 * the last block processing are dropped (therefore, "chunked" processing
 * works only as long as each non-final chunk has a length multiple of 64).
 *
 * \param key    secret key (32 bytes).
 * \param iv     IV (12 bytes).
 * \param cc     initial counter value.
 * \param data   data to encrypt or decrypt.
 * \param len    data length (in bytes).
 */
typedef uint32_t (*br_chacha20_run)(const void *key,
	const void *iv, uint32_t cc, void *data, size_t len);

/**
 * \brief ChaCha20 implementation (straightforward C code, constant-time).
 *
 * \see br_chacha20_run
 *
 * \param key    secret key (32 bytes).
 * \param iv     IV (12 bytes).
 * \param cc     initial counter value.
 * \param data   data to encrypt or decrypt.
 * \param len    data length (in bytes).
 */
uint32_t br_chacha20_ct_run(const void *key,
	const void *iv, uint32_t cc, void *data, size_t len);

/**
 * \brief Type for a ChaCha20+Poly1305 AEAD implementation.
 *
 * The provided data is encrypted or decrypted with ChaCha20. The
 * authentication tag is computed on the concatenation of the
 * additional data and the ciphertext, with the padding and lengths
 * as described in RFC 7539 (section 2.8).
 *
 * After decryption, the caller is responsible for checking that the
 * computed tag matches the expected value.
 *
 * \param key       secret key (32 bytes).
 * \param iv        nonce (12 bytes).
 * \param data      data to encrypt or decrypt.
 * \param len       data length (in bytes).
 * \param aad       additional authenticated data.
 * \param aad_len   length of additional authenticated data (in bytes).
 * \param tag       output buffer for the authentication tag.
 * \param ichacha   implementation of ChaCha20.
 * \param encrypt   non-zero for encryption, zero for decryption.
 */
typedef void (*br_poly1305_run)(const void *key, const void *iv,
	void *data, size_t len, const void *aad, size_t aad_len,
	void *tag, br_chacha20_run ichacha, int encrypt);

/**
 * \brief ChaCha20+Poly1305 AEAD implementation (mixed 32-bit multiplications).
 *
 * \see br_poly1305_run
 *
 * \param key       secret key (32 bytes).
 * \param iv        nonce (12 bytes).
 * \param data      data to encrypt or decrypt.
 * \param len       data length (in bytes).
 * \param aad       additional authenticated data.
 * \param aad_len   length of additional authenticated data (in bytes).
 * \param tag       output buffer for the authentication tag.
 * \param ichacha   implementation of ChaCha20.
 * \param encrypt   non-zero for encryption, zero for decryption.
 */
void br_poly1305_ctmul_run(const void *key, const void *iv,
	void *data, size_t len, const void *aad, size_t aad_len,
	void *tag, br_chacha20_run ichacha, int encrypt);

/**
 * \brief ChaCha20+Poly1305 AEAD implementation (pure 32-bit multiplications).
 *
 * \see br_poly1305_run
 *
 * \param key       secret key (32 bytes).
 * \param iv        nonce (12 bytes).
 * \param data      data to encrypt or decrypt.
 * \param len       data length (in bytes).
 * \param aad       additional authenticated data.
 * \param aad_len   length of additional authenticated data (in bytes).
 * \param tag       output buffer for the authentication tag.
 * \param ichacha   implementation of ChaCha20.
 * \param encrypt   non-zero for encryption, zero for decryption.
 */
void br_poly1305_ctmul32_run(const void *key, const void *iv,
	void *data, size_t len, const void *aad, size_t aad_len,
	void *tag, br_chacha20_run ichacha, int encrypt);

/**
 * \brief ChaCha20+Poly1305 AEAD implementation (i15).
 *
 * This implementation relies on the generic big integer code "i15"
 * (which uses pure 32-bit multiplications). As such, it may save a
 * little code footprint in a context where "i15" is already included
 * (e.g. for elliptic curves or for RSA); however, it is also
 * substantially slower than the ctmul and ctmul32 implementations.
 *
 * \see br_poly1305_run
 *
 * \param key       secret key (32 bytes).
 * \param iv        nonce (12 bytes).
 * \param data      data to encrypt or decrypt.
 * \param len       data length (in bytes).
 * \param aad       additional authenticated data.
 * \param aad_len   length of additional authenticated data (in bytes).
 * \param tag       output buffer for the authentication tag.
 * \param ichacha   implementation of ChaCha20.
 * \param encrypt   non-zero for encryption, zero for decryption.
 */
void br_poly1305_i15_run(const void *key, const void *iv,
	void *data, size_t len, const void *aad, size_t aad_len,
	void *tag, br_chacha20_run ichacha, int encrypt);

#endif


/* ===== inc/bearssl_hmac.h ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef BR_BEARSSL_HMAC_H__
#define BR_BEARSSL_HMAC_H__

#include <stddef.h>
#include <stdint.h>

/* (already included) */

/** \file bearssl_hmac.h
 *
 * # HMAC
 *
 * HMAC is initialized with a key and an underlying hash function; it
 * then fills a "key context". That context contains the processed
 * key.
 *
 * With the key context, a HMAC context can be initialized to process
 * the input bytes and obtain the MAC output. The key context is not
 * modified during that process, and can be reused.
 *
 * IMPORTANT: HMAC shall be used only with functions that have the
 * following properties:
 *
 *   - hash output size does not exceed 64 bytes;
 *   - hash internal state size does not exceed 64 bytes;
 *   - internal block length is a power of 2 between 16 and 256 bytes.
 */

/**
 * \brief HMAC key context.
 *
 * The HMAC key context is initialised with a hash function implementation
 * and a secret key. Contents are opaque (callers should not access them
 * directly). The caller is responsible for allocating the context where
 * appropriate. Context initialisation and usage incurs no dynamic
 * allocation, so there is no release function.
 */
typedef struct {
#ifndef BR_DOXYGEN_IGNORE
	const br_hash_class *dig_vtable;
	unsigned char ksi[64], kso[64];
#endif
} br_hmac_key_context;

/**
 * \brief HMAC key context initialisation.
 *
 * Initialise the key context with the provided key, using the hash function
 * identified by `digest_vtable`. This supports arbitrary key lengths.
 *
 * \param kc              HMAC key context to initialise.
 * \param digest_vtable   pointer to the hash function implementation vtable.
 * \param key             pointer to the HMAC secret key.
 * \param key_len         HMAC secret key length (in bytes).
 */
void br_hmac_key_init(br_hmac_key_context *kc,
	const br_hash_class *digest_vtable, const void *key, size_t key_len);

/**
 * \brief HMAC computation context.
 *
 * The HMAC computation context maintains the state for a single HMAC
 * computation. It is modified as input bytes are injected. The context
 * is caller-allocated and has no release function since it does not
 * dynamically allocate external resources. Its contents are opaque.
 */
typedef struct {
#ifndef BR_DOXYGEN_IGNORE
	br_hash_compat_context dig;
	unsigned char kso[64];
	size_t out_len;
#endif
} br_hmac_context;

/**
 * \brief HMAC computation initialisation.
 *
 * Initialise a HMAC context with a key context. The key context is
 * unmodified. Relevant data from the key context is immediately copied;
 * the key context can thus be independently reused, modified or released
 * without impacting this HMAC computation.
 *
 * An explicit output length can be specified; the actual output length
 * will be the minimum of that value and the natural HMAC output length.
 * If `out_len` is 0, then the natural HMAC output length is selected. The
 * "natural output length" is the output length of the underlying hash
 * function.
 *
 * \param ctx       HMAC context to initialise.
 * \param kc        HMAC key context (already initialised with the key).
 * \param out_len   HMAC output length (0 to select "natural length").
 */
void br_hmac_init(br_hmac_context *ctx,
	const br_hmac_key_context *kc, size_t out_len);

/**
 * \brief Get the HMAC output size.
 *
 * The HMAC output size is the number of bytes that will actually be
 * produced with `br_hmac_out()` with the provided context. This function
 * MUST NOT be called on a non-initialised HMAC computation context.
 * The returned value is the minimum of the HMAC natural length (output
 * size of the underlying hash function) and the `out_len` parameter which
 * was used with the last `br_hmac_init()` call on that context (if the
 * initialisation `out_len` parameter was 0, then this function will
 * return the HMAC natural length).
 *
 * \param ctx   the (already initialised) HMAC computation context.
 * \return  the HMAC actual output size.
 */
static inline size_t
br_hmac_size(br_hmac_context *ctx)
{
	return ctx->out_len;
}

/**
 * \brief Inject some bytes in HMAC.
 *
 * The provided `len` bytes are injected as extra input in the HMAC
 * computation incarnated by the `ctx` HMAC context. It is acceptable
 * that `len` is zero, in which case `data` is ignored (and may be
 * `NULL`) and this function does nothing.
 */
void br_hmac_update(br_hmac_context *ctx, const void *data, size_t len);

/**
 * \brief Compute the HMAC output.
 *
 * The destination buffer MUST be large enough to accomodate the result;
 * its length is at most the "natural length" of HMAC (i.e. the output
 * length of the underlying hash function). The context is NOT modified;
 * further bytes may be processed. Thus, "partial HMAC" values can be
 * efficiently obtained.
 *
 * Returned value is the output length (in bytes).
 *
 * \param ctx   HMAC computation context.
 * \param out   destination buffer for the HMAC output.
 * \return  the produced value length (in bytes).
 */
size_t br_hmac_out(const br_hmac_context *ctx, void *out);

/**
 * \brief Constant-time HMAC computation.
 *
 * This function compute the HMAC output in constant time. Some extra
 * input bytes are processed, then the output is computed. The extra
 * input consists in the `len` bytes pointed to by `data`. The `len`
 * parameter must lie between `min_len` and `max_len` (inclusive);
 * `max_len` bytes are actually read from `data`. Computing time (and
 * memory access pattern) will not depend upon the data byte contents or
 * the value of `len`.
 *
 * The output is written in the `out` buffer, that MUST be large enough
 * to receive it.
 *
 * The difference `max_len - min_len` MUST be less than 2<sup>30</sup>
 * (i.e. about one gigabyte).
 *
 * This function computes the output properly only if the underlying
 * hash function uses MD padding (i.e. MD5, SHA-1, SHA-224, SHA-256,
 * SHA-384 or SHA-512).
 *
 * The provided context is NOT modified.
 *
 * \param ctx       the (already initialised) HMAC computation context.
 * \param data      the extra input bytes.
 * \param len       the extra input length (in bytes).
 * \param min_len   minimum extra input length (in bytes).
 * \param max_len   maximum extra input length (in bytes).
 * \param out       destination buffer for the HMAC output.
 * \return  the produced value length (in bytes).
 */
size_t br_hmac_outCT(const br_hmac_context *ctx,
	const void *data, size_t len, size_t min_len, size_t max_len,
	void *out);

#endif


/* ===== inc/bearssl_prf.h ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef BR_BEARSSL_PRF_H__
#define BR_BEARSSL_PRF_H__

#include <stddef.h>
#include <stdint.h>

/** \file bearssl_prf.h
 *
 * # The TLS PRF
 *
 * The "PRF" is the pseudorandom function used internally during the
 * SSL/TLS handshake, notably to expand negociated shared secrets into
 * the symmetric encryption keys that will be used to process the
 * application data.
 *
 * TLS 1.0 and 1.1 define a PRF that is based on both MD5 and SHA-1. This
 * is implemented by the `br_tls10_prf()` function.
 *
 * TLS 1.2 redefines the PRF, using an explicit hash function. The
 * `br_tls12_sha256_prf()` and `br_tls12_sha384_prf()` functions apply that
 * PRF with, respectively, SHA-256 and SHA-384. Most standard cipher suites
 * rely on the SHA-256 based PRF, but some use SHA-384.
 *
 * The PRF always uses as input three parameters: a "secret" (some
 * bytes), a "label" (ASCII string), and a "seed" (again some bytes).
 * An arbitrary output length can be produced.
 */

/** \brief PRF implementation for TLS 1.0 and 1.1.
 *
 * This PRF is the one specified by TLS 1.0 and 1.1. It internally uses
 * MD5 and SHA-1.
 *
 * \param dst          destination buffer.
 * \param len          output length (in bytes).
 * \param secret       secret value (key) for this computation.
 * \param secret_len   length of "secret" (in bytes).
 * \param label        PRF label (zero-terminated ASCII string).
 * \param seed         seed for this computation (usually non-secret).
 * \param seed_len     length of "seed" (in bytes).
 */
void br_tls10_prf(void *dst, size_t len,
	const void *secret, size_t secret_len,
	const char *label, const void *seed, size_t seed_len);

/** \brief PRF implementation for TLS 1.2, with SHA-256.
 *
 * This PRF is the one specified by TLS 1.2, when the underlying hash
 * function is SHA-256.
 *
 * \param dst          destination buffer.
 * \param len          output length (in bytes).
 * \param secret       secret value (key) for this computation.
 * \param secret_len   length of "secret" (in bytes).
 * \param label        PRF label (zero-terminated ASCII string).
 * \param seed         seed for this computation (usually non-secret).
 * \param seed_len     length of "seed" (in bytes).
 */
void br_tls12_sha256_prf(void *dst, size_t len,
	const void *secret, size_t secret_len,
	const char *label, const void *seed, size_t seed_len);

/** \brief PRF implementation for TLS 1.2, with SHA-384.
 *
 * This PRF is the one specified by TLS 1.2, when the underlying hash
 * function is SHA-384.
 *
 * \param dst          destination buffer.
 * \param len          output length (in bytes).
 * \param secret       secret value (key) for this computation.
 * \param secret_len   length of "secret" (in bytes).
 * \param label        PRF label (zero-terminated ASCII string).
 * \param seed         seed for this computation (usually non-secret).
 * \param seed_len     length of "seed" (in bytes).
 */
void br_tls12_sha384_prf(void *dst, size_t len,
	const void *secret, size_t secret_len,
	const char *label, const void *seed, size_t seed_len);

/** \brief A convenient type name for a PRF implementation.
 *
 * \param dst          destination buffer.
 * \param len          output length (in bytes).
 * \param secret       secret value (key) for this computation.
 * \param secret_len   length of "secret" (in bytes).
 * \param label        PRF label (zero-terminated ASCII string).
 * \param seed         seed for this computation (usually non-secret).
 * \param seed_len     length of "seed" (in bytes).
 */
typedef void (*br_tls_prf_impl)(void *dst, size_t len,
	const void *secret, size_t secret_len,
	const char *label, const void *seed, size_t seed_len);

#endif


/* ===== inc/bearssl_rand.h ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef BR_BEARSSL_RAND_H__
#define BR_BEARSSL_RAND_H__

#include <stddef.h>
#include <stdint.h>

/** \file bearssl_rand.h
 *
 * # Pseudo-Random Generators
 *
 * A PRNG is a state-based engine that outputs pseudo-random bytes on
 * demand. It is initialized with an initial seed, and additional seed
 * bytes can be added afterwards. Bytes produced depend on the seeds and
 * also on the exact sequence of calls (including sizes requested for
 * each call).
 *
 *
 * ## Procedural and OOP API
 *
 * For the PRNG of name "`xxx`", two API are provided. The _procedural_
 * API defined a context structure `br_xxx_context` and three functions:
 *
 *   - `br_xxx_init()`
 *
 *     Initialise the context with an initial seed.
 *
 *   - `br_xxx_generate()`
 *
 *     Produce some pseudo-random bytes.
 *
 *   - `br_xxx_update()`
 *
 *     Inject some additional seed.
 *
 * The initialisation function sets the first context field (`vtable`)
 * to a pointer to the vtable that supports the OOP API. The OOP API
 * provides access to the same functions through function pointers,
 * named `init()`, `generate()` and `update()`.
 *
 * Note that the context initialisation method may accept additional
 * parameters, provided as a 'const void *' pointer at API level. These
 * additional parameters depend on the implemented PRNG.
 *
 *
 * ## HMAC_DRBG
 *
 * HMAC_DRBG is defined in [NIST SP 800-90A Revision
 * 1](http://nvlpubs.nist.gov/nistpubs/SpecialPublications/NIST.SP.800-90Ar1.pdf).
 * It uses HMAC repeatedly, over some configurable underlying hash
 * function. In BearSSL, it is implemented under the "`hmac_drbg`" name.
 * The "extra parameters" pointer for context initialisation should be
 * set to a pointer to the vtable for the underlying hash function (e.g.
 * pointer to `br_sha256_vtable` to use HMAC_DRBG with SHA-256).
 *
 * According to the NIST standard, each request shall produce up to
 * 2<sup>19</sup> bits (i.e. 64 kB of data); moreover, the context shall
 * be reseeded at least once every 2<sup>48</sup> requests. This
 * implementation does not maintain the reseed counter (the threshold is
 * too high to be reached in practice) and does not object to producing
 * more than 64 kB in a single request; thus, the code cannot fail,
 * which corresponds to the fact that the API has no room for error
 * codes. However, this implies that requesting more than 64 kB in one
 * `generate()` request, or making more than 2<sup>48</sup> requests
 * without reseeding, is formally out of NIST specification. There is
 * no currently known security penalty for exceeding the NIST limits,
 * and, in any case, HMAC_DRBG usage in implementing SSL/TLS always
 * stays much below these thresholds.
 */

/**
 * \brief Class type for PRNG implementations.
 *
 * A `br_prng_class` instance references the methods implementing a PRNG.
 * Constant instances of this structure are defined for each implemented
 * PRNG. Such instances are also called "vtables".
 */
typedef struct br_prng_class_ br_prng_class;
struct br_prng_class_ {
	/**
	 * \brief Size (in bytes) of the context structure appropriate for
	 * running this PRNG.
	 */
	size_t context_size;

	/**
	 * \brief Initialisation method.
	 *
	 * The context to initialise is provided as a pointer to its
	 * first field (the vtable pointer); this function sets that
	 * first field to a pointer to the vtable.
	 *
	 * The extra parameters depend on the implementation; each
	 * implementation defines what kind of extra parameters it
	 * expects (if any).
	 *
	 * Requirements on the initial seed depend on the implemented
	 * PRNG.
	 *
	 * \param ctx        PRNG context to initialise.
	 * \param params     extra parameters for the PRNG.
	 * \param seed       initial seed.
	 * \param seed_len   initial seed length (in bytes).
	 */
	void (*init)(const br_prng_class **ctx, const void *params,
		const void *seed, size_t seed_len);

	/**
	 * \brief Random bytes generation.
	 *
	 * This method produces `len` pseudorandom bytes, in the `out`
	 * buffer. The context is updated accordingly.
	 *
	 * \param ctx   PRNG context.
	 * \param out   output buffer.
	 * \param len   number of pseudorandom bytes to produce.
	 */
	void (*generate)(const br_prng_class **ctx, void *out, size_t len);

	/**
	 * \brief Inject additional seed bytes.
	 *
	 * The provided seed bytes are added into the PRNG internal
	 * entropy pool.
	 *
	 * \param ctx        PRNG context.
	 * \param seed       additional seed.
	 * \param seed_len   additional seed length (in bytes).
	 */
	void (*update)(const br_prng_class **ctx,
		const void *seed, size_t seed_len);
};

/**
 * \brief Context for HMAC_DRBG.
 *
 * The context contents are opaque, except the first field, which
 * supports OOP.
 */
typedef struct {
	/**
	 * \brief Pointer to the vtable.
	 *
	 * This field is set with the initialisation method/function.
	 */
	const br_prng_class *vtable;
#ifndef BR_DOXYGEN_IGNORE
	unsigned char K[64];
	unsigned char V[64];
	const br_hash_class *digest_class;
#endif
} br_hmac_drbg_context;

/**
 * \brief Statically allocated, constant vtable for HMAC_DRBG.
 */
extern const br_prng_class br_hmac_drbg_vtable;

/**
 * \brief HMAC_DRBG initialisation.
 *
 * The context to initialise is provided as a pointer to its first field
 * (the vtable pointer); this function sets that first field to a
 * pointer to the vtable.
 *
 * The `seed` value is what is called, in NIST terminology, the
 * concatenation of the "seed", "nonce" and "personalization string", in
 * that order.
 *
 * The `digest_class` parameter defines the underlying hash function.
 * Formally, the NIST standard specifies that the hash function shall
 * be only SHA-1 or one of the SHA-2 functions. This implementation also
 * works with any other implemented hash function (such as MD5), but
 * this is non-standard and therefore not recommended.
 *
 * \param ctx            HMAC_DRBG context to initialise.
 * \param digest_class   vtable for the underlying hash function.
 * \param seed           initial seed.
 * \param seed_len       initial seed length (in bytes).
 */
void br_hmac_drbg_init(br_hmac_drbg_context *ctx,
	const br_hash_class *digest_class, const void *seed, size_t seed_len);

/**
 * \brief Random bytes generation with HMAC_DRBG.
 *
 * This method produces `len` pseudorandom bytes, in the `out`
 * buffer. The context is updated accordingly. Formally, requesting
 * more than 65536 bytes in one request falls out of specification
 * limits (but it won't fail).
 *
 * \param ctx   HMAC_DRBG context.
 * \param out   output buffer.
 * \param len   number of pseudorandom bytes to produce.
 */
void br_hmac_drbg_generate(br_hmac_drbg_context *ctx, void *out, size_t len);

/**
 * \brief Inject additional seed bytes in HMAC_DRBG.
 *
 * The provided seed bytes are added into the HMAC_DRBG internal
 * entropy pool. The process does not _replace_ existing entropy,
 * thus pushing non-random bytes (i.e. bytes which are known to the
 * attackers) does not degrade the overall quality of generated bytes.
 *
 * \param ctx        HMAC_DRBG context.
 * \param seed       additional seed.
 * \param seed_len   additional seed length (in bytes).
 */
void br_hmac_drbg_update(br_hmac_drbg_context *ctx,
	const void *seed, size_t seed_len);

/**
 * \brief Get the hash function implementation used by a given instance of
 * HMAC_DRBG.
 *
 * This calls MUST NOT be performed on a context which was not
 * previously initialised.
 *
 * \param ctx   HMAC_DRBG context.
 * \return  the hash function vtable.
 */
static inline const br_hash_class *
br_hmac_drbg_get_hash(const br_hmac_drbg_context *ctx)
{
	return ctx->digest_class;
}

#endif


/* ===== inc/bearssl_ec.h ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef BR_BEARSSL_EC_H__
#define BR_BEARSSL_EC_H__

#include <stddef.h>
#include <stdint.h>

/** \file bearssl_ec.h
 *
 * # Elliptic Curves
 *
 * This file documents the EC implementations provided with BearSSL, and
 * ECDSA.
 *
 * ## Elliptic Curve API
 *
 * Only "named curves" are supported. Each EC implementation supports
 * one or several named curves, identified by symbolic identifiers.
 * These identifiers are small integers, that correspond to the values
 * registered by the
 * [IANA](http://www.iana.org/assignments/tls-parameters/tls-parameters.xhtml#tls-parameters-8).
 *
 * Since all currently defined elliptic curve identifiers are in the 0..31
 * range, it is convenient to encode support of some curves in a 32-bit
 * word, such that bit x corresponds to curve of identifier x.
 *
 * An EC implementation is incarnated by a `br_ec_impl` instance, that
 * offers the following fields:
 *
 *   - `supported_curves`
 *
 *      A 32-bit word that documents the identifiers of the curves supported
 *      by this implementation.
 *
 *   - `generator()`
 *
 *      Callback method that returns a pointer to the conventional generator
 *      point for that curve.
 *
 *   - `order()`
 *
 *      Callback method that returns a pointer to the subgroup order for
 *      that curve. That value uses unsigned big-endian encoding.
 *
 *   - `mul()`
 *
 *      Multiply a curve point with an integer.
 *
 *   - `mulgen()`
 *
 *      Multiply the curve generator with an integer. This may be faster
 *      than the generic `mul()`.
 *
 *   - `muladd()`
 *
 *      Multiply two curve points by two integers, and return the sum of
 *      the two products.
 *
 * All curve points are represented in uncompressed format. The `mul()`
 * and `muladd()` methods take care to validate that the provided points
 * are really part of the relevant curve subgroup.
 *
 * For all point multiplication functions, the following holds:
 *
 *   - Functions validate that the provided points are valid members
 *     of the relevant curve subgroup. An error is reported if that is
 *     not the case.
 *
 *   - Processing is constant-time, even if the point operands are not
 *     valid. This holds for both the source and resulting points, and
 *     the multipliers (integers). Only the byte length of the provided
 *     multiplier arrays (not their actual value length in bits) may
 *     leak through timing-based side channels.
 *
 *   - The multipliers (integers) MUST be lower than the subgroup order.
 *     If this property is not met, then the result is indeterminate,
 *     but an error value is not ncessearily returned.
 * 
 *
 * ## ECDSA
 *
 * ECDSA signatures have two standard formats, called "raw" and "asn1".
 * Internally, such a signature is a pair of modular integers `(r,s)`.
 * The "raw" format is the concatenation of the unsigned big-endian
 * encodings of these two integers, possibly left-padded with zeros so
 * that they have the same encoded length. The "asn1" format is the
 * DER encoding of an ASN.1 structure that contains the two integer
 * values:
 *
 *     ECDSASignature ::= SEQUENCE {
 *         r   INTEGER,
 *         s   INTEGER
 *     }
 *
 * In general, in all of X.509 and SSL/TLS, the "asn1" format is used.
 * BearSSL offers ECDSA implementations for both formats; conversion
 * functions between the two formats are also provided. Conversion of a
 * "raw" format signature into "asn1" may enlarge a signature by no more
 * than 9 bytes for all supported curves; conversely, conversion of an
 * "asn1" signature to "raw" may expand the signature but the "raw"
 * length will never be more than twice the length of the "asn1" length
 * (and usually it will be shorter).
 *
 * Note that for a given signature, the "raw" format is not fully
 * deterministic, in that it does not enforce a minimal common length.
 */

/*
 * Standard curve ID. These ID are equal to the assigned numerical
 * identifiers assigned to these curves for TLS:
 *    http://www.iana.org/assignments/tls-parameters/tls-parameters.xhtml#tls-parameters-8
 */

/** \brief Identifier for named curve sect163k1. */
#define BR_EC_sect163k1           1

/** \brief Identifier for named curve sect163r1. */
#define BR_EC_sect163r1           2

/** \brief Identifier for named curve sect163r2. */
#define BR_EC_sect163r2           3

/** \brief Identifier for named curve sect193r1. */
#define BR_EC_sect193r1           4

/** \brief Identifier for named curve sect193r2. */
#define BR_EC_sect193r2           5

/** \brief Identifier for named curve sect233k1. */
#define BR_EC_sect233k1           6

/** \brief Identifier for named curve sect233r1. */
#define BR_EC_sect233r1           7

/** \brief Identifier for named curve sect239k1. */
#define BR_EC_sect239k1           8

/** \brief Identifier for named curve sect283k1. */
#define BR_EC_sect283k1           9

/** \brief Identifier for named curve sect283r1. */
#define BR_EC_sect283r1          10

/** \brief Identifier for named curve sect409k1. */
#define BR_EC_sect409k1          11

/** \brief Identifier for named curve sect409r1. */
#define BR_EC_sect409r1          12

/** \brief Identifier for named curve sect571k1. */
#define BR_EC_sect571k1          13

/** \brief Identifier for named curve sect571r1. */
#define BR_EC_sect571r1          14

/** \brief Identifier for named curve secp160k1. */
#define BR_EC_secp160k1          15

/** \brief Identifier for named curve secp160r1. */
#define BR_EC_secp160r1          16

/** \brief Identifier for named curve secp160r2. */
#define BR_EC_secp160r2          17

/** \brief Identifier for named curve secp192k1. */
#define BR_EC_secp192k1          18

/** \brief Identifier for named curve secp192r1. */
#define BR_EC_secp192r1          19

/** \brief Identifier for named curve secp224k1. */
#define BR_EC_secp224k1          20

/** \brief Identifier for named curve secp224r1. */
#define BR_EC_secp224r1          21

/** \brief Identifier for named curve secp256k1. */
#define BR_EC_secp256k1          22

/** \brief Identifier for named curve secp256r1. */
#define BR_EC_secp256r1          23

/** \brief Identifier for named curve secp384r1. */
#define BR_EC_secp384r1          24

/** \brief Identifier for named curve secp521r1. */
#define BR_EC_secp521r1          25

/** \brief Identifier for named curve brainpoolP256r1. */
#define BR_EC_brainpoolP256r1    26

/** \brief Identifier for named curve brainpoolP384r1. */
#define BR_EC_brainpoolP384r1    27

/** \brief Identifier for named curve brainpoolP512r1. */
#define BR_EC_brainpoolP512r1    28

/**
 * \brief Structure for an EC public key.
 */
typedef struct {
	/** \brief Identifier for the curve used by this key. */
	int curve;
	/** \brief Public curve point (uncompressed format). */
	unsigned char *q;
	/** \brief Length of public curve point (in bytes). */
	size_t qlen;
} br_ec_public_key;

/**
 * \brief Structure for an EC private key.
 *
 * The private key is an integer modulo the curve subgroup order. The
 * encoding below tolerates extra leading zeros. In general, it is
 * recommended that the private key has the same length as the curve
 * subgroup order.
 */
typedef struct {
	/** \brief Identifier for the curve used by this key. */
	int curve;
	/** \brief Private key (integer, unsigned big-endian encoding). */
	unsigned char *x;
	/** \brief Private key length (in bytes). */
	size_t xlen;
} br_ec_private_key;

/**
 * \brief Type for an EC implementation.
 */
typedef struct {
	/**
	 * \brief Supported curves.
	 *
	 * This word is a bitfield: bit `x` is set if the curve of ID `x`
	 * is supported. E.g. an implementation supporting both NIST P-256
	 * (secp256r1, ID 23) and NIST P-384 (secp384r1, ID 24) will have
	 * value `0x01800000` in this field.
	 */
	uint32_t supported_curves;

	/**
	 * \brief Get the conventional generator.
	 *
	 * This function returns the conventional generator (encoded
	 * curve point) for the specified curve. This function MUST NOT
	 * be called if the curve is not supported.
	 *
	 * \param curve   curve identifier.
	 * \param len     receiver for the encoded generator length (in bytes).
	 * \return  the encoded generator.
	 */
	const unsigned char *(*generator)(int curve, size_t *len);

	/**
	 * \brief Get the subgroup order.
	 *
	 * This function returns the order of the subgroup generated by
	 * the conventional generator, for the specified curve. Unsigned
	 * big-endian encoding is used. This function MUST NOT be called
	 * if the curve is not supported.
	 *
	 * \param curve   curve identifier.
	 * \param len     receiver for the encoded order length (in bytes).
	 * \return  the encoded order.
	 */
	const unsigned char *(*order)(int curve, size_t *len);

	/**
	 * \brief Multiply a curve point by an integer.
	 *
	 * The source point is provided in array `G` (of size `Glen` bytes);
	 * the multiplication result is written over it. The multiplier
	 * `x` (of size `xlen` bytes) uses unsigned big-endian encoding.
	 *
	 * Rules:
	 *
	 *   - The specified curve MUST be supported.
	 *
	 *   - The source point must be a valid point on the relevant curve
	 *     subgroup (and not the "point at infinity" either). If this is
	 *     not the case, then this function returns an error (0).
	 *
	 *   - The multiplier integer MUST be non-zero and less than the
	 *     curve subgroup order. If this property does not hold, then
	 *     the result is indeterminate and an error code is not
	 *     guaranteed.
	 *
	 * Returned value is 1 on success, 0 on error. On error, the
	 * contents of `G` are indeterminate.
	 *
	 * \param G       point to multiply.
	 * \param Glen    length of the encoded point (in bytes).
	 * \param x       multiplier (unsigned big-endian).
	 * \param xlen    multiplier length (in bytes).
	 * \param curve   curve identifier.
	 * \return  1 on success, 0 on error.
	 */
	uint32_t (*mul)(unsigned char *G, size_t Glen,
		const unsigned char *x, size_t xlen, int curve);

	/**
	 * \brief Multiply the generator by an integer.
	 *
	 * The multiplier MUST be non-zero and less than the curve
	 * subgroup order. Results are indeterminate if this property
	 * does not hold.
	 *
	 * \param R       output buffer for the point.
	 * \param x       multiplier (unsigned big-endian).
	 * \param xlen    multiplier length (in bytes).
	 * \param curve   curve identifier.
	 * \return  encoded result point length (in bytes).
	 */
	size_t (*mulgen)(unsigned char *R,
		const unsigned char *x, size_t xlen, int curve);

	/**
	 * \brief Multiply two points by two integers and add the
	 * results.
	 *
	 * The point `x*A + y*B` is computed and written back in the `A`
	 * array.
	 *
	 * Rules:
	 *
	 *   - The specified curve MUST be supported.
	 *
	 *   - The source points (`A` and `B`)  must be valid points on
	 *     the relevant curve subgroup (and not the "point at
	 *     infinity" either). If this is not the case, then this
	 *     function returns an error (0).
	 *
	 *   - If the `B` pointer is `NULL`, then the conventional
	 *     subgroup generator is used. With some implementations,
	 *     this may be faster than providing a pointer to the
	 *     generator.
	 *
	 *   - The multiplier integers (`x` and `y`) MUST be non-zero
	 *     and less than the curve subgroup order. If either integer
	 *     is zero, then an error is reported, but if one of them is
	 *     not lower than the subgroup order, then the result is
	 *     indeterminate and an error code is not guaranteed.
	 *
	 *   - If the final result is the point at infinity, then an
	 *     error is returned.
	 *
	 * Returned value is 1 on success, 0 on error. On error, the
	 * contents of `A` are indeterminate.
	 *
	 * \param A       first point to multiply.
	 * \param B       second point to multiply (`NULL` for the generator).
	 * \param len     common length of the encoded points (in bytes).
	 * \param x       multiplier for `A` (unsigned big-endian).
	 * \param xlen    length of multiplier for `A` (in bytes).
	 * \param y       multiplier for `A` (unsigned big-endian).
	 * \param ylen    length of multiplier for `A` (in bytes).
	 * \param curve   curve identifier.
	 * \return  1 on success, 0 on error.
	 */
	uint32_t (*muladd)(unsigned char *A, const unsigned char *B, size_t len,
		const unsigned char *x, size_t xlen,
		const unsigned char *y, size_t ylen, int curve);
} br_ec_impl;

/**
 * \brief EC implementation "i31".
 *
 * This implementation internally uses generic code for modular integers,
 * with a representation as sequences of 31-bit words. It supports secp256r1,
 * secp384r1 and secp521r1 (aka NIST curves P-256, P-384 and P-521).
 */
extern const br_ec_impl br_ec_prime_i31;

/**
 * \brief EC implementation "i15".
 *
 * This implementation internally uses generic code for modular integers,
 * with a representation as sequences of 15-bit words. It supports secp256r1,
 * secp384r1 and secp521r1 (aka NIST curves P-256, P-384 and P-521).
 */
extern const br_ec_impl br_ec_prime_i15;

/**
 * \brief EC implementation "i15" for P-256.
 *
 * This implementation uses specialised code for curve secp256r1 (also
 * known as NIST P-256), with Karatsuba decomposition, and fast modular
 * reduction thanks to the field modulus special format. Only 32-bit
 * multiplications are used (with 32-bit results, not 64-bit).
 */
extern const br_ec_impl br_ec_p256_i15;

/**
 * \brief Convert a signature from "raw" to "asn1".
 *
 * Conversion is done "in place" and the new length is returned.
 * Conversion may enlarge the signature, but by no more than 9 bytes at
 * most. On error, 0 is returned (error conditions include an odd raw
 * signature length, or an oversized integer).
 *
 * \param sig       signature to convert.
 * \param sig_len   signature length (in bytes).
 * \return  the new signature length, or 0 on error.
 */
size_t br_ecdsa_raw_to_asn1(void *sig, size_t sig_len);

/**
 * \brief Convert a signature from "asn1" to "raw".
 *
 * Conversion is done "in place" and the new length is returned.
 * Conversion may enlarge the signature, but the new signature length
 * will be less than twice the source length at most. On error, 0 is
 * returned (error conditions include an invalid ASN.1 structure or an
 * oversized integer).
 *
 * \param sig       signature to convert.
 * \param sig_len   signature length (in bytes).
 * \return  the new signature length, or 0 on error.
 */
size_t br_ecdsa_asn1_to_raw(void *sig, size_t sig_len);

/**
 * \brief Type for an ECDSA signer function.
 *
 * A pointer to the EC implementation is provided. The hash value is
 * assumed to have the length inferred from the designated hash function
 * class.
 *
 * Signature is written in the buffer pointed to by `sig`, and the length
 * (in bytes) is returned. On error, nothing is written in the buffer,
 * and 0 is returned. This function returns 0 if the specified curve is
 * not supported by the provided EC implementation.
 *
 * The signature format is either "raw" or "asn1", depending on the
 * implementation; maximum length is predictable from the implemented
 * curve:
 *
 * | curve      | raw | asn1 |
 * | :--------- | --: | ---: |
 * | NIST P-256 |  64 |   72 |
 * | NIST P-384 |  96 |  104 |
 * | NIST P-521 | 132 |  139 |
 *
 * \param impl         EC implementation to use.
 * \param hf           hash function used to process the data.
 * \param hash_value   signed data (hashed).
 * \param sk           EC private key.
 * \param sig          destination buffer.
 * \return  the signature length (in bytes), or 0 on error.
 */
typedef size_t (*br_ecdsa_sign)(const br_ec_impl *impl,
	const br_hash_class *hf, const void *hash_value,
	const br_ec_private_key *sk, void *sig);

/**
 * \brief Type for an ECDSA signature verification function.
 *
 * A pointer to the EC implementation is provided. The hashed value,
 * computed over the purportedly signed data, is also provided with
 * its length.
 *
 * The signature format is either "raw" or "asn1", depending on the
 * implementation.
 *
 * Returned value is 1 on success (valid signature), 0 on error. This
 * function returns 0 if the specified curve is not supported by the
 * provided EC implementation.
 *
 * \param impl       EC implementation to use.
 * \param hash       signed data (hashed).
 * \param hash_len   hash value length (in bytes).
 * \param pk         EC public key.
 * \param sig        signature.
 * \param sig_len    signature length (in bytes).
 * \return  1 on success, 0 on error.
 */
typedef uint32_t (*br_ecdsa_vrfy)(const br_ec_impl *impl,
	const void *hash, size_t hash_len,
	const br_ec_public_key *pk, const void *sig, size_t sig_len);

/**
 * \brief ECDSA signature generator, "i31" implementation, "asn1" format.
 *
 * \see br_ecdsa_sign()
 *
 * \param impl         EC implementation to use.
 * \param hf           hash function used to process the data.
 * \param hash_value   signed data (hashed).
 * \param sk           EC private key.
 * \param sig          destination buffer.
 * \return  the signature length (in bytes), or 0 on error.
 */
size_t br_ecdsa_i31_sign_asn1(const br_ec_impl *impl,
	const br_hash_class *hf, const void *hash_value,
	const br_ec_private_key *sk, void *sig);

/**
 * \brief ECDSA signature generator, "i31" implementation, "raw" format.
 *
 * \see br_ecdsa_sign()
 *
 * \param impl         EC implementation to use.
 * \param hf           hash function used to process the data.
 * \param hash_value   signed data (hashed).
 * \param sk           EC private key.
 * \param sig          destination buffer.
 * \return  the signature length (in bytes), or 0 on error.
 */
size_t br_ecdsa_i31_sign_raw(const br_ec_impl *impl,
	const br_hash_class *hf, const void *hash_value,
	const br_ec_private_key *sk, void *sig);

/**
 * \brief ECDSA signature verifier, "i31" implementation, "asn1" format.
 *
 * \see br_ecdsa_vrfy()
 *
 * \param impl       EC implementation to use.
 * \param hash       signed data (hashed).
 * \param hash_len   hash value length (in bytes).
 * \param pk         EC public key.
 * \param sig        signature.
 * \param sig_len    signature length (in bytes).
 * \return  1 on success, 0 on error.
 */
uint32_t br_ecdsa_i31_vrfy_asn1(const br_ec_impl *impl,
	const void *hash, size_t hash_len,
	const br_ec_public_key *pk, const void *sig, size_t sig_len);

/**
 * \brief ECDSA signature verifier, "i31" implementation, "raw" format.
 *
 * \see br_ecdsa_vrfy()
 *
 * \param impl       EC implementation to use.
 * \param hash       signed data (hashed).
 * \param hash_len   hash value length (in bytes).
 * \param pk         EC public key.
 * \param sig        signature.
 * \param sig_len    signature length (in bytes).
 * \return  1 on success, 0 on error.
 */
uint32_t br_ecdsa_i31_vrfy_raw(const br_ec_impl *impl,
	const void *hash, size_t hash_len,
	const br_ec_public_key *pk, const void *sig, size_t sig_len);

/**
 * \brief ECDSA signature generator, "i15" implementation, "asn1" format.
 *
 * \see br_ecdsa_sign()
 *
 * \param impl         EC implementation to use.
 * \param hf           hash function used to process the data.
 * \param hash_value   signed data (hashed).
 * \param sk           EC private key.
 * \param sig          destination buffer.
 * \return  the signature length (in bytes), or 0 on error.
 */
size_t br_ecdsa_i15_sign_asn1(const br_ec_impl *impl,
	const br_hash_class *hf, const void *hash_value,
	const br_ec_private_key *sk, void *sig);

/**
 * \brief ECDSA signature generator, "i15" implementation, "raw" format.
 *
 * \see br_ecdsa_sign()
 *
 * \param impl         EC implementation to use.
 * \param hf           hash function used to process the data.
 * \param hash_value   signed data (hashed).
 * \param sk           EC private key.
 * \param sig          destination buffer.
 * \return  the signature length (in bytes), or 0 on error.
 */
size_t br_ecdsa_i15_sign_raw(const br_ec_impl *impl,
	const br_hash_class *hf, const void *hash_value,
	const br_ec_private_key *sk, void *sig);

/**
 * \brief ECDSA signature verifier, "i15" implementation, "asn1" format.
 *
 * \see br_ecdsa_vrfy()
 *
 * \param impl       EC implementation to use.
 * \param hash       signed data (hashed).
 * \param hash_len   hash value length (in bytes).
 * \param pk         EC public key.
 * \param sig        signature.
 * \param sig_len    signature length (in bytes).
 * \return  1 on success, 0 on error.
 */
uint32_t br_ecdsa_i15_vrfy_asn1(const br_ec_impl *impl,
	const void *hash, size_t hash_len,
	const br_ec_public_key *pk, const void *sig, size_t sig_len);

/**
 * \brief ECDSA signature verifier, "i15" implementation, "raw" format.
 *
 * \see br_ecdsa_vrfy()
 *
 * \param impl       EC implementation to use.
 * \param hash       signed data (hashed).
 * \param hash_len   hash value length (in bytes).
 * \param pk         EC public key.
 * \param sig        signature.
 * \param sig_len    signature length (in bytes).
 * \return  1 on success, 0 on error.
 */
uint32_t br_ecdsa_i15_vrfy_raw(const br_ec_impl *impl,
	const void *hash, size_t hash_len,
	const br_ec_public_key *pk, const void *sig, size_t sig_len);

#endif


/* ===== inc/bearssl_rsa.h ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef BR_BEARSSL_RSA_H__
#define BR_BEARSSL_RSA_H__

#include <stddef.h>
#include <stdint.h>

/** \file bearssl_rsa.h
 *
 * # RSA
 *
 * This file documents the RSA implementations provided with BearSSL.
 * Note that the SSL engine accesses these implementations through a
 * configurable API, so it is possible to, for instance, run a SSL
 * server which uses a RSA engine which is not based on this code.
 *
 * ## Key Elements
 *
 * RSA public and private keys consist in lists of big integers. All
 * such integers are represented with big-endian unsigned notation:
 * first byte is the most significant, and the value is positive (so
 * there is no dedicated "sign bit"). Public and private key structures
 * thus contain, for each such integer, a pointer to the first value byte
 * (`unsigned char *`), and a length (`size_t`) which is the number of
 * relevant bytes. As a general rule, minimal-length encoding is not
 * enforced: values may have extra leading bytes of value 0.
 *
 * RSA public keys consist in two integers:
 *
 *   - the modulus (`n`);
 *   - the public exponent (`e`).
 *
 * RSA private keys, as defined in
 * [PKCS#1](https://tools.ietf.org/html/rfc3447), contain eight integers:
 *
 *   - the modulus (`n`);
 *   - the public exponent (`e`);
 *   - the private exponent (`d`);
 *   - the first prime factor (`p`);
 *   - the second prime factor (`q`);
 *   - the first reduced exponent (`dp`, which is `d` modulo `p-1`);
 *   - the second reduced exponent (`dq`, which is `d` modulo `q-1`);
 *   - the CRT coefficient (`iq`, the inverse of `q` modulo `p`).
 *
 * However, the implementations defined in BearSSL use only five of
 * these integers: `p`, `q`, `dp`, `dq` and `iq`.
 *
 * ## Security Features and Limitations
 *
 * The implementations contained in BearSSL have the following limitations
 * and features:
 *
 *   - They are constant-time. This means that the execution time and
 *     memory access pattern may depend on the _lengths_ of the private
 *     key components, but not on their value, nor on the value of
 *     the operand. Note that this property is not achieved through
 *     random masking, but "true" constant-time code.
 *
 *   - They support only private keys with two prime factors. RSA private
 *     key with three or more prime factors are nominally supported, but
 *     rarely used; they may offer faster operations, at the expense of
 *     more code and potentially a reduction in security if there are
 *     "too many" prime factors.
 *
 *   - The public exponent may have arbitrary length. Of course, it is
 *     a good idea to keep public exponents small, so that public key
 *     operations are fast; but, contrary to some widely deployed
 *     implementations, BearSSL has no problem with public exponent
 *     longer than 32 bits.
 *
 *   - The two prime factors of the modulus need not have the same length
 *     (but severely imbalanced factor lengths might reduce security).
 *     Similarly, there is no requirement that the first factor (`p`)
 *     be greater than the second factor (`q`).
 *
 *   - Prime factors and modulus must be smaller than a compile-time limit.
 *     This is made necessary by the use of fixed-size stack buffers, and
 *     the limit has been adjusted to keep stack usage under 2 kB for the
 *     RSA operations. Currently, the maximum modulus size is 4096 bits,
 *     and the maximum prime factor size is 2080 bits.
 *
 *   - The RSA functions themselves do not enforce lower size limits,
 *     except that which is absolutely necessary for the operation to
 *     mathematically make sense (e.g. a PKCS#1 v1.5 signature with
 *     SHA-1 requires a modulus of at least 361 bits). It is up to users
 *     of this code to enforce size limitations when appropriate (e.g.
 *     the X.509 validation engine, by default, rejects RSA keys of
 *     less than 1017 bits).
 *
 *   - Within the size constraints expressed above, arbitrary bit lengths
 *     are supported. There is no requirement that prime factors or
 *     modulus have a size multiple of 8 or 16.
 *
 *   - When verifying PKCS#1 v1.5 signatures, both variants of the hash
 *     function identifying header (with and without the ASN.1 NULL) are
 *     supported. When producing such signatures, the variant with the
 *     ASN.1 NULL is used.
 *
 * ## Implementations
 *
 * Three RSA implementations are included:
 *
 *   - The **i32** implementation internally represents big integers
 *     as arrays of 32-bit integers. It is perfunctory and portable,
 *     but not very efficient.
 *
 *   - The **i31** implementation uses 32-bit integers, each containing
 *     31 bits worth of integer data. The i31 implementation is somewhat
 *     faster than the i32 implementation (the reduced integer size makes
 *     carry propagation easier) for a similar code footprint, but uses
 *     very slightly larger stack buffers (about 4% bigger).
 *
 *   - The **i15** implementation uses 16-bit integers, each containing
 *     15 bits worth of integer data. Multiplication results fit on
 *     32 bits, so this won't use the "widening" multiplication routine
 *     on ARM Cortex M0/M0+, for much better performance and constant-time
 *     execution.
 */

/**
 * \brief RSA public key.
 *
 * The structure references the modulus and the public exponent. Both
 * integers use unsigned big-endian representation; extra leading bytes
 * of value 0 are allowed.
 */
typedef struct {
	/** \brief Modulus. */
	unsigned char *n;
	/** \brief Modulus length (in bytes). */
	size_t nlen;
	/** \brief Public exponent. */
	unsigned char *e;
	/** \brief Public exponent length (in bytes). */
	size_t elen;
} br_rsa_public_key;

/**
 * \brief RSA private key.
 *
 * The structure references the primvate factors, reduced private
 * exponents, and CRT coefficient. It also contains the bit length of
 * the modulus. The big integers use unsigned big-endian representation;
 * extra leading bytes of value 0 are allowed. However, the modulus bit
 * length (`n_bitlen`) MUST be exact.
 */
typedef struct {
	/** \brief Modulus bit length (in bits, exact value). */
	uint32_t n_bitlen;
	/** \brief First prime factor. */
	unsigned char *p;
	/** \brief First prime factor length (in bytes). */
	size_t plen;
	/** \brief Second prime factor. */
	unsigned char *q;
	/** \brief Second prime factor length (in bytes). */
	size_t qlen;
	/** \brief First reduced private exponent. */
	unsigned char *dp;
	/** \brief First reduced private exponent length (in bytes). */
	size_t dplen;
	/** \brief Second reduced private exponent. */
	unsigned char *dq;
	/** \brief Second reduced private exponent length (in bytes). */
	size_t dqlen;
	/** \brief CRT coefficient. */
	unsigned char *iq;
	/** \brief CRT coefficient length (in bytes). */
	size_t iqlen;
} br_rsa_private_key;

/**
 * \brief Type for a RSA public key engine.
 *
 * The public key engine performs the modular exponentiation of the
 * provided value with the public exponent. The value is modified in
 * place.
 *
 * The value length (`xlen`) is verified to have _exactly_ the same
 * length as the modulus (actual modulus length, without extra leading
 * zeros in the modulus representation in memory). If the length does
 * not match, then this function returns 0 and `x[]` is unmodified.
 * 
 * It `xlen` is correct, then `x[]` is modified. Returned value is 1
 * on success, 0 on error. Error conditions include an oversized `x[]`
 * (the array has the same length as the modulus, but the numerical value
 * is not lower than the modulus) and an invalid modulus (e.g. an even
 * integer). If an error is reported, then the new contents of `x[]` are
 * unspecified.
 *
 * \param x      operand to exponentiate.
 * \param xlen   length of the operand (in bytes).
 * \param pk     RSA public key.
 * \return  1 on success, 0 on error.
 */
typedef uint32_t (*br_rsa_public)(unsigned char *x, size_t xlen,
	const br_rsa_public_key *pk);

/**
 * \brief Type for a RSA signature verification engine (PKCS#1 v1.5).
 *
 * Parameters are:
 *
 *   - The signature itself. The provided array is NOT modified.
 *
 *   - The encoded OID for the hash function. The provided array must begin
 *     with a single byte that contains the length of the OID value (in
 *     bytes), followed by exactly that many bytes. This parameter may
 *     also be `NULL`, in which case the raw hash value should be used
 *     with the PKCS#1 v1.5 "type 1" padding (as used in SSL/TLS up
 *     to TLS-1.1, with a 36-byte hash value).
 *
 *   - The hash output length, in bytes.
 *
 *   - The public key.
 *
 *   - An output buffer for the hash value. The caller must still compare
 *     it with the hash of the data over which the signature is computed.
 *
 * **Constraints:**
 *
 *   - Hash length MUST be no more than 64 bytes.
 *
 *   - OID value length MUST be no more than 32 bytes (i.e. `hash_oid[0]`
 *     must have a value in the 0..32 range, inclusive).
 *
 * This function verifies that the signature length (`xlen`) matches the
 * modulus length (this function returns 0 on mismatch). If the modulus
 * size exceeds the maximum supported RSA size, then the function also
 * returns 0.
 *
 * Returned value is 1 on success, 0 on error.
 *
 * Implementations of this type need not be constant-time.
 *
 * \param x          signature buffer.
 * \param xlen       signature length (in bytes).
 * \param hash_oid   encoded hash algorithm OID (or `NULL`).
 * \param hash_len   expected hash value length (in bytes).
 * \param pk         RSA public key.
 * \param hash_out   output buffer for the hash value.
 * \return  1 on success, 0 on error.
 */
typedef uint32_t (*br_rsa_pkcs1_vrfy)(const unsigned char *x, size_t xlen,
	const unsigned char *hash_oid, size_t hash_len,
	const br_rsa_public_key *pk, unsigned char *hash_out);

/**
 * \brief Type for a RSA private key engine.
 *
 * The `x[]` buffer is modified in place, and its length is inferred from
 * the modulus length (`x[]` is assumed to have a length of
 * `(sk->n_bitlen+7)/8` bytes).
 *
 * Returned value is 1 on success, 0 on error.
 *
 * \param x    operand to exponentiate.
 * \param sk   RSA private key.
 * \return  1 on success, 0 on error.
 */
typedef uint32_t (*br_rsa_private)(unsigned char *x,
	const br_rsa_private_key *sk);

/**
 * \brief Type for a RSA signature generation engine (PKCS#1 v1.5).
 *
 * Parameters are:
 *
 *   - The encoded OID for the hash function. The provided array must begin
 *     with a single byte that contains the length of the OID value (in
 *     bytes), followed by exactly that many bytes. This parameter may
 *     also be `NULL`, in which case the raw hash value should be used
 *     with the PKCS#1 v1.5 "type 1" padding (as used in SSL/TLS up
 *     to TLS-1.1, with a 36-byte hash value).
 *
 *   - The hash value computes over the data to sign (its length is
 *     expressed in bytes).
 *
 *   - The RSA private key.
 *
 *   - The output buffer, that receives the signature.
 *
 * Returned value is 1 on success, 0 on error. Error conditions include
 * a too small modulus for the provided hash OID and value, or some
 * invalid key parameters. The signature length is exactly
 * `(sk->n_bitlen+7)/8` bytes.
 *
 * This function is expected to be constant-time with regards to the
 * private key bytes (lengths of the modulus and the individual factors
 * may leak, though) and to the hashed data.
 *
 * \param hash_oid   encoded hash algorithm OID (or `NULL`).
 * \param hash       hash value.
 * \param hash_len   hash value length (in bytes).
 * \param sk         RSA private key.
 * \param x          output buffer for the signature value.
 * \return  1 on success, 0 on error.
 */
typedef uint32_t (*br_rsa_pkcs1_sign)(const unsigned char *hash_oid,
	const unsigned char *hash, size_t hash_len,
	const br_rsa_private_key *sk, unsigned char *x);

/*
 * RSA "i32" engine. Integers are internally represented as arrays of
 * 32-bit integers, and the core multiplication primitive is the
 * 32x32->64 multiplication.
 */

/**
 * \brief RSA public key engine "i32".
 *
 * \see br_rsa_public
 *
 * \param x      operand to exponentiate.
 * \param xlen   length of the operand (in bytes).
 * \param pk     RSA public key.
 * \return  1 on success, 0 on error.
 */
uint32_t br_rsa_i32_public(unsigned char *x, size_t xlen,
	const br_rsa_public_key *pk);

/**
 * \brief RSA signature verification engine "i32".
 *
 * \see br_rsa_pkcs1_vrfy
 *
 * \param x          signature buffer.
 * \param xlen       signature length (in bytes).
 * \param hash_oid   encoded hash algorithm OID (or `NULL`).
 * \param hash_len   expected hash value length (in bytes).
 * \param pk         RSA public key.
 * \param hash_out   output buffer for the hash value.
 * \return  1 on success, 0 on error.
 */
uint32_t br_rsa_i32_pkcs1_vrfy(const unsigned char *x, size_t xlen,
	const unsigned char *hash_oid, size_t hash_len,
	const br_rsa_public_key *pk, unsigned char *hash_out);

/**
 * \brief RSA private key engine "i32".
 *
 * \see br_rsa_private
 *
 * \param x    operand to exponentiate.
 * \param sk   RSA private key.
 * \return  1 on success, 0 on error.
 */
uint32_t br_rsa_i32_private(unsigned char *x,
	const br_rsa_private_key *sk);

/**
 * \brief RSA signature generation engine "i32".
 *
 * \see br_rsa_pkcs1_sign
 *
 * \param hash_oid   encoded hash algorithm OID (or `NULL`).
 * \param hash       hash value.
 * \param hash_len   hash value length (in bytes).
 * \param sk         RSA private key.
 * \param x          output buffer for the hash value.
 * \return  1 on success, 0 on error.
 */
uint32_t br_rsa_i32_pkcs1_sign(const unsigned char *hash_oid,
	const unsigned char *hash, size_t hash_len,
	const br_rsa_private_key *sk, unsigned char *x);

/*
 * RSA "i31" engine. Similar to i32, but only 31 bits are used per 32-bit
 * word. This uses slightly more stack space (about 4% more) and code
 * space, but it quite faster.
 */

/**
 * \brief RSA public key engine "i31".
 *
 * \see br_rsa_public
 *
 * \param x      operand to exponentiate.
 * \param xlen   length of the operand (in bytes).
 * \param pk     RSA public key.
 * \return  1 on success, 0 on error.
 */
uint32_t br_rsa_i31_public(unsigned char *x, size_t xlen,
	const br_rsa_public_key *pk);

/**
 * \brief RSA signature verification engine "i31".
 *
 * \see br_rsa_pkcs1_vrfy
 *
 * \param x          signature buffer.
 * \param xlen       signature length (in bytes).
 * \param hash_oid   encoded hash algorithm OID (or `NULL`).
 * \param hash_len   expected hash value length (in bytes).
 * \param pk         RSA public key.
 * \param hash_out   output buffer for the hash value.
 * \return  1 on success, 0 on error.
 */
uint32_t br_rsa_i31_pkcs1_vrfy(const unsigned char *x, size_t xlen,
	const unsigned char *hash_oid, size_t hash_len,
	const br_rsa_public_key *pk, unsigned char *hash_out);

/**
 * \brief RSA private key engine "i31".
 *
 * \see br_rsa_private
 *
 * \param x    operand to exponentiate.
 * \param sk   RSA private key.
 * \return  1 on success, 0 on error.
 */
uint32_t br_rsa_i31_private(unsigned char *x,
	const br_rsa_private_key *sk);

/**
 * \brief RSA signature generation engine "i31".
 *
 * \see br_rsa_pkcs1_sign
 *
 * \param hash_oid   encoded hash algorithm OID (or `NULL`).
 * \param hash       hash value.
 * \param hash_len   hash value length (in bytes).
 * \param sk         RSA private key.
 * \param x          output buffer for the hash value.
 * \return  1 on success, 0 on error.
 */
uint32_t br_rsa_i31_pkcs1_sign(const unsigned char *hash_oid,
	const unsigned char *hash, size_t hash_len,
	const br_rsa_private_key *sk, unsigned char *x);

/*
 * RSA "i15" engine. Integers are represented as 15-bit integers, so
 * the code uses only 32-bit multiplication (no 64-bit result), which
 * is vastly faster (and constant-time) on the ARM Cortex M0/M0+.
 */

/**
 * \brief RSA public key engine "i15".
 *
 * \see br_rsa_public
 *
 * \param x      operand to exponentiate.
 * \param xlen   length of the operand (in bytes).
 * \param pk     RSA public key.
 * \return  1 on success, 0 on error.
 */
uint32_t br_rsa_i15_public(unsigned char *x, size_t xlen,
	const br_rsa_public_key *pk);

/**
 * \brief RSA signature verification engine "i15".
 *
 * \see br_rsa_pkcs1_vrfy
 *
 * \param x          signature buffer.
 * \param xlen       signature length (in bytes).
 * \param hash_oid   encoded hash algorithm OID (or `NULL`).
 * \param hash_len   expected hash value length (in bytes).
 * \param pk         RSA public key.
 * \param hash_out   output buffer for the hash value.
 * \return  1 on success, 0 on error.
 */
uint32_t br_rsa_i15_pkcs1_vrfy(const unsigned char *x, size_t xlen,
	const unsigned char *hash_oid, size_t hash_len,
	const br_rsa_public_key *pk, unsigned char *hash_out);

/**
 * \brief RSA private key engine "i15".
 *
 * \see br_rsa_private
 *
 * \param x    operand to exponentiate.
 * \param sk   RSA private key.
 * \return  1 on success, 0 on error.
 */
uint32_t br_rsa_i15_private(unsigned char *x,
	const br_rsa_private_key *sk);

/**
 * \brief RSA signature generation engine "i15".
 *
 * \see br_rsa_pkcs1_sign
 *
 * \param hash_oid   encoded hash algorithm OID (or `NULL`).
 * \param hash       hash value.
 * \param hash_len   hash value length (in bytes).
 * \param sk         RSA private key.
 * \param x          output buffer for the hash value.
 * \return  1 on success, 0 on error.
 */
uint32_t br_rsa_i15_pkcs1_sign(const unsigned char *hash_oid,
	const unsigned char *hash, size_t hash_len,
	const br_rsa_private_key *sk, unsigned char *x);

/**
 * \brief RSA decryption helper, for SSL/TLS.
 *
 * This function performs the RSA decryption for a RSA-based key exchange
 * in a SSL/TLS server. The provided RSA engine is used. The `data`
 * parameter points to the value to decrypt, of length `len` bytes. On
 * success, the 48-byte pre-master secret is copied into `data`, starting
 * at the first byte of that buffer; on error, the contents of `data`
 * become indeterminate.
 *
 * This function first checks that the provided value length (`len`) is
 * not lower than 59 bytes, and matches the RSA modulus length; if neither
 * of this property is met, then this function returns 0 and the buffer
 * is unmodified.
 *
 * Otherwise, decryption and then padding verification are performed, both
 * in constant-time. A decryption error, or a bad padding, or an
 * incorrect decrypted value length are reported with a returned value of
 * 0; on success, 1 is returned. The caller (SSL server engine) is supposed
 * to proceed with a random pre-master secret in case of error.
 *
 * \param core   RSA private key engine.
 * \param sk     RSA private key.
 * \param data   input/output buffer.
 * \param len    length (in bytes) of the data to decrypt.
 * \return  1 on success, 0 on error.
 */
uint32_t br_rsa_ssl_decrypt(br_rsa_private core, const br_rsa_private_key *sk,
	unsigned char *data, size_t len);

#endif


/* ===== inc/bearssl_x509.h ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef BR_BEARSSL_X509_H__
#define BR_BEARSSL_X509_H__

#include <stddef.h>
#include <stdint.h>

/* (already included) */
/* (already included) */
/* (already included) */

/** \file bearssl_x509.h
 *
 * # X.509 Certificate Chain Processing
 *
 * An X.509 processing engine receives an X.509 chain, chunk by chunk,
 * as received from a SSL/TLS client or server (the client receives the
 * server's certificate chain, and the server receives the client's
 * certificate chain if it requested a client certificate). The chain
 * is thus injected in the engine in SSL order (end-entity first).
 *
 * The engine's job is to return the public key to use for SSL/TLS.
 * How exactly that key is obtained and verified is entirely up to the
 * engine.
 *
 * **The "known key" engine** returns a public key which is already known
 * from out-of-band information (e.g. the client _remembers_ the key from
 * a previous connection, as in the usual SSH model). This is the simplest
 * engine since it simply ignores the chain, thereby avoiding the need
 * for any decoding logic.
 *
 * **The "minimal" engine** implements minimal X.509 decoding and chain
 * validation:
 *
 *   - The provided chain should validate "as is". There is no attempt
 *     at reordering, skipping or downloading extra certificates.
 *
 *   - X.509 v1, v2 and v3 certificates are supported.
 *
 *   - Trust anchors are a DN and a public key. Each anchor is either a
 *     "CA" anchor, or a non-CA.
 *
 *   - If the end-entity certificate matches a non-CA anchor (subject DN
 *     is equal to the non-CA name, and public key is also identical to
 *     the anchor key), then this is a _direct trust_ case and the
 *     remaining certificates are ignored.
 *
 *   - Unless direct trust is applied, the chain must be verifiable up to
 *     a certificate whose issuer DN matches the DN from a "CA" trust anchor,
 *     and whose signature is verifiable against that anchor's public key.
 *     Subsequent certificates in the chain are ignored.
 *
 *   - The engine verifies subject/issuer DN matching, and enforces
 *     processing of Basic Constraints and Key Usage extensions. The
 *     Authority Key Identifier, Subject Key Identifier, Issuer Alt Name,
 *     Subject Directory Attribute, CRL Distribution Points, Freshest CRL,
 *     Authority Info Access and Subject Info Access extensions are
 *     ignored. The Subject Alt Name is decoded for the end-entity
 *     certificate under some conditions (see below). Other extensions
 *     are ignored if non-critical, or imply chain rejection if critical.
 *
 *   - The Subject Alt Name extension is parsed for names of type `dNSName`
 *     when decoding the end-entity certificate, and only if there is a
 *     server name to match. If there is no SAN extension, then the
 *     Common Name from the subjectDN is used. That name matching is
 *     case-insensitive and honours a single starting wildcard (i.e. if
 *     the name in the certificate starts with "`*.`" then this matches
 *     any word as first element). Note: this name matching is performed
 *     also in the "direct trust" model.
 *
 *   - DN matching is byte-to-byte equality (a future version might
 *     include some limited processing for case-insensitive matching and
 *     whitespace normalisation).
 *
 *   - Successful validation produces a public key type but also a set
 *     of allowed usages (`BR_KEYTYPE_KEYX` and/or `BR_KEYTYPE_SIGN`).
 *     The caller is responsible for checking that the key type and
 *     usages are compatible with the expected values (e.g. with the
 *     selected cipher suite, when the client validates the server's
 *     certificate).
 *
 * **Important caveats:**
 *
 *   - The "minimal" engine does not check revocation status. The relevant
 *     extensions are ignored, and CRL or OCSP responses are not gathered
 *     or checked.
 *
 *   - The "minimal" engine does not currently support Name Constraints
 *     (some basic functionality to handle sub-domains may be added in a
 *     later version).
 *
 *   - The decoder is not "validating" in the sense that it won't reject
 *     some certificates with invalid field values when these fields are
 *     not actually processed.
 */

/*
 * X.509 error codes are in the 32..63 range.
 */

/** \brief X.509 status: validation was successful; this is not actually
    an error. */
#define BR_ERR_X509_OK                    32

/** \brief X.509 status: invalid value in an ASN.1 structure. */
#define BR_ERR_X509_INVALID_VALUE         33

/** \brief X.509 status: truncated certificate. */
#define BR_ERR_X509_TRUNCATED             34

/** \brief X.509 status: empty certificate chain (no certificate at all). */
#define BR_ERR_X509_EMPTY_CHAIN           35

/** \brief X.509 status: decoding error: inner element extends beyond
    outer element size. */
#define BR_ERR_X509_INNER_TRUNC           36

/** \brief X.509 status: decoding error: unsupported tag class (application
    or private). */
#define BR_ERR_X509_BAD_TAG_CLASS         37

/** \brief X.509 status: decoding error: unsupported tag value. */
#define BR_ERR_X509_BAD_TAG_VALUE         38

/** \brief X.509 status: decoding error: indefinite length. */
#define BR_ERR_X509_INDEFINITE_LENGTH     39

/** \brief X.509 status: decoding error: extraneous element. */
#define BR_ERR_X509_EXTRA_ELEMENT         40

/** \brief X.509 status: decoding error: unexpected element. */
#define BR_ERR_X509_UNEXPECTED            41

/** \brief X.509 status: decoding error: expected constructed element, but
    is primitive. */
#define BR_ERR_X509_NOT_CONSTRUCTED       42

/** \brief X.509 status: decoding error: expected primitive element, but
    is constructed. */
#define BR_ERR_X509_NOT_PRIMITIVE         43

/** \brief X.509 status: decoding error: BIT STRING length is not multiple
    of 8. */
#define BR_ERR_X509_PARTIAL_BYTE          44

/** \brief X.509 status: decoding error: BOOLEAN value has invalid length. */
#define BR_ERR_X509_BAD_BOOLEAN           45

/** \brief X.509 status: decoding error: value is off-limits. */
#define BR_ERR_X509_OVERFLOW              46

/** \brief X.509 status: invalid distinguished name. */
#define BR_ERR_X509_BAD_DN                47

/** \brief X.509 status: invalid date/time representation. */
#define BR_ERR_X509_BAD_TIME              48

/** \brief X.509 status: certificate contains unsupported features that
    cannot be ignored. */
#define BR_ERR_X509_UNSUPPORTED           49

/** \brief X.509 status: key or signature size exceeds internal limits. */
#define BR_ERR_X509_LIMIT_EXCEEDED        50

/** \brief X.509 status: key type does not match that which was expected. */
#define BR_ERR_X509_WRONG_KEY_TYPE        51

/** \brief X.509 status: signature is invalid. */
#define BR_ERR_X509_BAD_SIGNATURE         52

/** \brief X.509 status: validation time is unknown. */
#define BR_ERR_X509_TIME_UNKNOWN          53

/** \brief X.509 status: certificate is expired or not yet valid. */
#define BR_ERR_X509_EXPIRED               54

/** \brief X.509 status: issuer/subject DN mismatch in the chain. */
#define BR_ERR_X509_DN_MISMATCH           55

/** \brief X.509 status: expected server name was not found in the chain. */
#define BR_ERR_X509_BAD_SERVER_NAME       56

/** \brief X.509 status: unknown critical extension in certificate. */
#define BR_ERR_X509_CRITICAL_EXTENSION    57

/** \brief X.509 status: not a CA, or path length constraint violation */
#define BR_ERR_X509_NOT_CA                58

/** \brief X.509 status: Key Usage extension prohibits intended usage. */
#define BR_ERR_X509_FORBIDDEN_KEY_USAGE   59

/** \brief X.509 status: public key found in certificate is too small. */
#define BR_ERR_X509_WEAK_PUBLIC_KEY       60

/** \brief X.509 status: chain could not be linked to a trust anchor. */
#define BR_ERR_X509_NOT_TRUSTED           62

/**
 * \brief Aggregate structure for public keys.
 */
typedef struct {
	/** \brief Key type: `BR_KEYTYPE_RSA` or `BR_KEYTYPE_EC` */
	unsigned char key_type;
	/** \brief Actual public key. */
	union {
		/** \brief RSA public key. */
		br_rsa_public_key rsa;
		/** \brief EC public key. */
		br_ec_public_key ec;
	} key;
} br_x509_pkey;

/**
 * \brief Distinguished Name (X.500) structure.
 *
 * The DN is DER-encoded.
 */
typedef struct {
	/** \brief Encoded DN data. */
	unsigned char *data;
	/** \brief Encoded DN length (in bytes). */
	size_t len;
} br_x500_name;

/**
 * \brief Trust anchor structure.
 */
typedef struct {
	/** \brief Encoded DN (X.500 name). */
	br_x500_name dn;
	/** \brief Anchor flags (e.g. `BR_X509_TA_CA`). */
	unsigned flags;
	/** \brief Anchor public key. */
	br_x509_pkey pkey;
} br_x509_trust_anchor;

/**
 * \brief Trust anchor flag: CA.
 *
 * A "CA" anchor is deemed fit to verify signatures on certificates.
 * A "non-CA" anchor is accepted only for direct trust (server's
 * certificate name and key match the anchor).
 */
#define BR_X509_TA_CA        0x0001

/*
 * Key type: combination of a basic key type (low 4 bits) and some
 * optional flags.
 *
 * For a public key, the basic key type only is set.
 *
 * For an expected key type, the flags indicate the intended purpose(s)
 * for the key; the basic key type may be set to 0 to indicate that any
 * key type compatible with the indicated purpose is acceptable.
 */
/** \brief Key type: algorithm is RSA. */
#define BR_KEYTYPE_RSA    1
/** \brief Key type: algorithm is EC. */
#define BR_KEYTYPE_EC     2

/**
 * \brief Key type: usage is "key exchange".
 *
 * This value is combined (with bitwise OR) with the algorithm
 * (`BR_KEYTYPE_RSA` or `BR_KEYTYPE_EC`) when informing the X.509
 * validation engine that it should find a public key of that type,
 * fit for key exchanges (e.g. `TLS_RSA_*` and `TLS_ECDH_*` cipher
 * suites).
 */
#define BR_KEYTYPE_KEYX   0x10

/**
 * \brief Key type: usage is "signature".
 *
 * This value is combined (with bitwise OR) with the algorithm
 * (`BR_KEYTYPE_RSA` or `BR_KEYTYPE_EC`) when informing the X.509
 * validation engine that it should find a public key of that type,
 * fit for signatures (e.g. `TLS_ECDHE_*` cipher suites).
 */
#define BR_KEYTYPE_SIGN   0x20

/*
 * start_chain   Called when a new chain is started. If 'server_name'
 *               is not NULL and non-empty, then it is a name that
 *               should be looked for in the EE certificate (in the
 *               SAN extension as dNSName, or in the subjectDN's CN
 *               if there is no SAN extension).
 *               The caller ensures that the provided 'server_name'
 *               pointer remains valid throughout validation.
 *
 * start_cert    Begins a new certificate in the chain. The provided
 *               length is in bytes; this is the total certificate length.
 *
 * append        Get some additional bytes for the current certificate.
 *
 * end_cert      Ends the current certificate.
 *
 * end_chain     Called at the end of the chain. Returned value is
 *               0 on success, or a non-zero error code.
 *
 * get_pkey      Returns the EE certificate public key.
 *
 * For a complete chain, start_chain() and end_chain() are always
 * called. For each certificate, start_cert(), some append() calls, then
 * end_cert() are called, in that order. There may be no append() call
 * at all if the certificate is empty (which is not valid but may happen
 * if the peer sends exactly that).
 *
 * get_pkey() shall return a pointer to a structure that is valid as
 * long as a new chain is not started. This may be a sub-structure
 * within the context for the engine. This function MAY return a valid
 * pointer to a public key even in some cases of validation failure,
 * depending on the validation engine.
 */

/**
 * \brief Class type for an X.509 engine.
 *
 * A certificate chain validation uses a caller-allocated context, which
 * contains the running state for that validation. Methods are called
 * in due order:
 *
 *   - `start_chain()` is called at the start of the validation.
 *   - Certificates are processed one by one, in SSL order (end-entity
 *     comes first). For each certificate, the following methods are
 *     called:
 *
 *       - `start_cert()` at the beginning of the certificate.
 *       - `append()` is called zero, one or more times, to provide
 *         the certificate (possibly in chunks).
 *       - `end_cert()` at the end of the certificate.
 *
 *   - `end_chain()` is called when the last certificate in the chain
 *     was processed.
 *   - `get_pkey()` is called after chain processing, if the chain
 *     validation was succesfull.
 *
 * A context structure may be reused; the `start_chain()` method shall
 * ensure (re)initialisation.
 */
typedef struct br_x509_class_ br_x509_class;
struct br_x509_class_ {
	/**
	 * \brief X.509 context size, in bytes.
	 */
	size_t context_size;

	/**
	 * \brief Start a new chain.
	 *
	 * This method shall set the vtable (first field) of the context
	 * structure.
	 *
	 * The `server_name`, if not `NULL`, will be considered as a
	 * fully qualified domain name, to be matched against the `dNSName`
	 * elements of the end-entity certificate's SAN extension (if there
	 * is no SAN, then the Common Name from the subjectDN will be used).
	 * If `server_name` is `NULL` then no such matching is performed.
	 *
	 * \param ctx           validation context.
	 * \param server_name   server name to match (or `NULL`).
	 */
	void (*start_chain)(const br_x509_class **ctx,
		const char *server_name);

	/**
	 * \brief Start a new certificate.
	 *
	 * \param ctx      validation context.
	 * \param length   new certificate length (in bytes).
	 */
	void (*start_cert)(const br_x509_class **ctx, uint32_t length);

	/**
	 * \brief Receive some bytes for the current certificate.
	 *
	 * This function may be called several times in succession for
	 * a given certificate. The caller guarantees that for each
	 * call, `len` is not zero, and the sum of all chunk lengths
	 * for a certificate matches the total certificate length which
	 * was provided in the previous `start_cert()` call.
	 *
	 * If the new certificate is empty (no byte at all) then this
	 * function won't be called at all.
	 *
	 * \param ctx   validation context.
	 * \param buf   certificate data chunk.
	 * \param len   certificate data chunk length (in bytes).
	 */
	void (*append)(const br_x509_class **ctx,
		const unsigned char *buf, size_t len);

	/**
	 * \brief Finish the current certificate.
	 *
	 * This function is called when the end of the current certificate
	 * is reached.
	 *
	 * \param ctx   validation context.
	 */
	void (*end_cert)(const br_x509_class **ctx);

	/**
	 * \brief Finish the chain.
	 *
	 * This function is called at the end of the chain. It shall
	 * return either 0 if the validation was successful, or a
	 * non-zero error code. The `BR_ERR_X509_*` constants are
	 * error codes, though other values may be possible.
	 *
	 * \param ctx   validation context.
	 * \return  0 on success, or a non-zero error code.
	 */
	unsigned (*end_chain)(const br_x509_class **ctx);

	/**
	 * \brief Get the resulting end-entity public key.
	 *
	 * The decoded public key is returned. The returned pointer
	 * may be valid only as long as the context structure is
	 * unmodified, i.e. it may cease to be valid if the context
	 * is released or reused.
	 *
	 * This function _may_ return `NULL` if the validation failed.
	 * However, returning a public key does not mean that the
	 * validation was wholly successful; some engines may return
	 * a decoded public key even if the chain did not end on a
	 * trusted anchor.
	 *
	 * If validation succeeded and `usage` is not `NULL`, then
	 * `*usage` is filled with a combination of `BR_KEYTYPE_SIGN`
	 * and/or `BR_KEYTYPE_KEYX` that specifies the validated key
	 * usage types. It is the caller's responsibility to check
	 * that value against the intended use of the public key.
	 *
	 * \param ctx   validation context.
	 * \return  the end-entity public key, or `NULL`.
	 */
	const br_x509_pkey *(*get_pkey)(
		const br_x509_class *const *ctx, unsigned *usages);
};

/**
 * \brief The "known key" X.509 engine structure.
 *
 * The structure contents are opaque (they shall not be accessed directly),
 * except for the first field (the vtable).
 *
 * The "known key" engine returns an externally configured public key,
 * and totally ignores the certificate contents.
 */
typedef struct {
	/** \brief Reference to the context vtable. */
	const br_x509_class *vtable;
#ifndef BR_DOXYGEN_IGNORE
	br_x509_pkey pkey;
	unsigned usages;
#endif
} br_x509_knownkey_context;

/**
 * \brief Class instance for the "known key" X.509 engine.
 */
extern const br_x509_class br_x509_knownkey_vtable;

/**
 * \brief Initialize a "known key" X.509 engine with a known RSA public key.
 *
 * The `usages` parameter indicates the allowed key usages for that key
 * (`BR_KEYTYPE_KEYX` and/or `BR_KEYTYPE_SIGN`).
 *
 * The provided pointers are linked in, not copied, so they must remain
 * valid while the public key may be in usage.
 *
 * \param ctx      context to initialise.
 * \param pk       known public key.
 * \param usages   allowed key usages.
 */
void br_x509_knownkey_init_rsa(br_x509_knownkey_context *ctx,
	const br_rsa_public_key *pk, unsigned usages);

/**
 * \brief Initialize a "known key" X.509 engine with a known EC public key.
 *
 * The `usages` parameter indicates the allowed key usages for that key
 * (`BR_KEYTYPE_KEYX` and/or `BR_KEYTYPE_SIGN`).
 *
 * The provided pointers are linked in, not copied, so they must remain
 * valid while the public key may be in usage.
 *
 * \param ctx      context to initialise.
 * \param pk       known public key.
 * \param usages   allowed key usages.
 */
void br_x509_knownkey_init_ec(br_x509_knownkey_context *ctx,
	const br_ec_public_key *pk, unsigned usages);

#ifndef BR_DOXYGEN_IGNORE
/*
 * The minimal X.509 engine has some state buffers which must be large
 * enough to simultaneously accommodate:
 * -- the public key extracted from the current certificate;
 * -- the signature on the current certificate or on the previous
 *    certificate;
 * -- the public key extracted from the EE certificate.
 *
 * We store public key elements in their raw unsigned big-endian
 * encoding. We want to support up to RSA-4096 with a short (up to 64
 * bits) public exponent, thus a buffer for a public key must have
 * length at least 520 bytes. Similarly, a RSA-4096 signature has length
 * 512 bytes.
 *
 * Though RSA public exponents can formally be as large as the modulus
 * (mathematically, even larger exponents would work, but PKCS#1 forbids
 * them), exponents that do not fit on 32 bits are extremely rare,
 * notably because some widespread implementations (e.g. Microsoft's
 * CryptoAPI) don't support them. Moreover, large public exponent do not
 * seem to imply any tangible security benefit, and they increase the
 * cost of public key operations. The X.509 "minimal" engine will tolerate
 * public exponents of arbitrary size as long as the modulus and the
 * exponent can fit together in the dedicated buffer.
 *
 * EC public keys are shorter than RSA public keys; even with curve
 * NIST P-521 (the largest curve we care to support), a public key is
 * encoded over 133 bytes only.
 */
#define BR_X509_BUFSIZE_KEY   520
#define BR_X509_BUFSIZE_SIG   512
#endif

/**
 * \brief Type for receiving a name element.
 *
 * An array of such structures can be provided to the X.509 decoding
 * engines. If the specified elements are found in the certificate
 * subject DN or the SAN extension, then the name contents are copied
 * as zero-terminated strings into the buffer.
 *
 * The decoder converts TeletexString and BMPString to UTF8String, and
 * ensures that the resulting string is zero-terminated. If the string
 * does not fit in the provided buffer, then the copy is aborted and an
 * error is reported.
 */
typedef struct {
	/**
	 * \brief Element OID.
	 *
	 * For X.500 name elements (to be extracted from the subject DN),
	 * this is the encoded OID for the requested name element; the
	 * first byte shall contain the length of the DER-encoded OID
	 * value, followed by the OID value (for instance, OID 2.5.4.3,
	 * for id-at-commonName, will be `03 55 04 03`). This is
	 * equivalent to full DER encoding with the length but without
	 * the tag.
	 *
	 * For SAN name elements, the first byte (`oid[0]`) has value 0,
	 * followed by another byte that matches the expected GeneralName
	 * tag. Allowed second byte values are then:
	 *
	 *   - 1: `rfc822Name`
	 *
	 *   - 2: `dNSName`
	 *
	 *   - 6: `uniformResourceIdentifier`
	 *
	 *   - 0: `otherName`
	 *
	 * If first and second byte are 0, then this is a SAN element of
	 * type `otherName`; the `oid[]` array should then contain, right
	 * after the two bytes of value 0, an encoded OID (with the same
	 * conventions as for X.500 name elements). If a match is found
	 * for that OID, then the corresponding name element will be
	 * extracted, as long as it is a supported string type.
	 */
	const unsigned char *oid;

	/**
	 * \brief Destination buffer.
	 */
	char *buf;

	/**
	 * \brief Length (in bytes) of the destination buffer.
	 *
	 * The buffer MUST NOT be smaller than 1 byte.
	 */
	size_t len;

	/**
	 * \brief Decoding status.
	 *
	 * Status is 0 if the name element was not found, 1 if it was
	 * found and decoded, or -1 on error. Error conditions include
	 * an unrecognised encoding, an invalid encoding, or a string
	 * too large for the destination buffer.
	 */
	int status;

} br_name_element;

/**
 * \brief The "minimal" X.509 engine structure.
 *
 * The structure contents are opaque (they shall not be accessed directly),
 * except for the first field (the vtable).
 *
 * The "minimal" engine performs a rudimentary but serviceable X.509 path
 * validation.
 */
typedef struct {
	const br_x509_class *vtable;

#ifndef BR_DOXYGEN_IGNORE
	/* Structure for returning the EE public key. */
	br_x509_pkey pkey;

	/* CPU for the T0 virtual machine. */
	struct {
		uint32_t *dp;
		uint32_t *rp;
		const unsigned char *ip;
	} cpu;
	uint32_t dp_stack[32];
	uint32_t rp_stack[32];
	int err;

	/* Server name to match with the SAN / CN of the EE certificate. */
	const char *server_name;

	/* Validated key usages. */
	unsigned char key_usages;

	/* Explicitly set date and time. */
	uint32_t days, seconds;

	/* Current certificate length (in bytes). Set to 0 when the
	   certificate has been fully processed. */
	uint32_t cert_length;

	/* Number of certificates processed so far in the current chain.
	   It is incremented at the end of the processing of a certificate,
	   so it is 0 for the EE. */
	uint32_t num_certs;

	/* Certificate data chunk. */
	const unsigned char *hbuf;
	size_t hlen;

	/* The pad serves as destination for various operations. */
	unsigned char pad[256];

	/* Buffer for EE public key data. */
	unsigned char ee_pkey_data[BR_X509_BUFSIZE_KEY];

	/* Buffer for currently decoded public key. */
	unsigned char pkey_data[BR_X509_BUFSIZE_KEY];

	/* Signature type: signer key type, offset to the hash
	   function OID (in the T0 data block) and hash function
	   output length (TBS hash length). */
	unsigned char cert_signer_key_type;
	uint16_t cert_sig_hash_oid;
	unsigned char cert_sig_hash_len;

	/* Current/last certificate signature. */
	unsigned char cert_sig[BR_X509_BUFSIZE_SIG];
	uint16_t cert_sig_len;

	/* Minimum RSA key length (difference in bytes from 128). */
	int16_t min_rsa_size;

	/* Configured trust anchors. */
	const br_x509_trust_anchor *trust_anchors;
	size_t trust_anchors_num;

	/*
	 * Multi-hasher for the TBS.
	 */
	unsigned char do_mhash;
	br_multihash_context mhash;
	unsigned char tbs_hash[64];

	/*
	 * Simple hasher for the subject/issuer DN.
	 */
	unsigned char do_dn_hash;
	const br_hash_class *dn_hash_impl;
	br_hash_compat_context dn_hash;
	unsigned char current_dn_hash[64];
	unsigned char next_dn_hash[64];
	unsigned char saved_dn_hash[64];

	/*
	 * Name elements to gather.
	 */
	br_name_element *name_elts;
	size_t num_name_elts;

	/*
	 * Public key cryptography implementations (signature verification).
	 */
	br_rsa_pkcs1_vrfy irsa;
	br_ecdsa_vrfy iecdsa;
	const br_ec_impl *iec;
#endif

} br_x509_minimal_context;

/**
 * \brief Class instance for the "minimal" X.509 engine.
 */
extern const br_x509_class br_x509_minimal_vtable;

/**
 * \brief Initialise a "minimal" X.509 engine.
 *
 * The `dn_hash_impl` parameter shall be a hash function internally used
 * to match X.500 names (subject/issuer DN, and anchor names). Any standard
 * hash function may be used, but a collision-resistant hash function is
 * advised.
 *
 * After initialization, some implementations for signature verification
 * (hash functions and signature algorithms) MUST be added.
 *
 * \param ctx                 context to initialise.
 * \param dn_hash_impl        hash function for DN comparisons.
 * \param trust_anchors       trust anchors.
 * \param trust_anchors_num   number of trust anchors.
 */
void br_x509_minimal_init(br_x509_minimal_context *ctx,
	const br_hash_class *dn_hash_impl,
	const br_x509_trust_anchor *trust_anchors, size_t trust_anchors_num);

/**
 * \brief Set a supported hash function in an X.509 "minimal" engine.
 *
 * Hash functions are used with signature verification algorithms.
 * Once initialised (with `br_x509_minimal_init()`), the context must
 * be configured with the hash functions it shall support for that
 * purpose. The hash function identifier MUST be one of the standard
 * hash function identifiers (1 to 6, for MD5, SHA-1, SHA-224, SHA-256,
 * SHA-384 and SHA-512).
 *
 * If `impl` is `NULL`, this _removes_ support for the designated
 * hash function.
 *
 * \param ctx    validation context.
 * \param id     hash function identifier (from 1 to 6).
 * \param impl   hash function implementation (or `NULL`).
 */
static inline void
br_x509_minimal_set_hash(br_x509_minimal_context *ctx,
	int id, const br_hash_class *impl)
{
	br_multihash_setimpl(&ctx->mhash, id, impl);
}

/**
 * \brief Set a RSA signature verification implementation in the X.509
 * "minimal" engine.
 *
 * Once initialised (with `br_x509_minimal_init()`), the context must
 * be configured with the signature verification implementations that
 * it is supposed to support. If `irsa` is `0`, then the RSA support
 * is disabled.
 *
 * \param ctx    validation context.
 * \param irsa   RSA signature verification implementation (or `0`).
 */
static inline void
br_x509_minimal_set_rsa(br_x509_minimal_context *ctx,
	br_rsa_pkcs1_vrfy irsa)
{
	ctx->irsa = irsa;
}

/**
 * \brief Set a ECDSA signature verification implementation in the X.509
 * "minimal" engine.
 *
 * Once initialised (with `br_x509_minimal_init()`), the context must
 * be configured with the signature verification implementations that
 * it is supposed to support.
 *
 * If `iecdsa` is `0`, then this call disables ECDSA support; in that
 * case, `iec` may be `NULL`. Otherwise, `iecdsa` MUST point to a function
 * that verifies ECDSA signatures with format "asn1", and it will use
 * `iec` as underlying elliptic curve support.
 *
 * \param ctx      validation context.
 * \param iec      elliptic curve implementation (or `NULL`).
 * \param iecdsa   ECDSA implementation (or `0`).
 */
static inline void
br_x509_minimal_set_ecdsa(br_x509_minimal_context *ctx,
	const br_ec_impl *iec, br_ecdsa_vrfy iecdsa)
{
	ctx->iecdsa = iecdsa;
	ctx->iec = iec;
}

/**
 * \brief Initialise a "minimal" X.509 engine with default algorithms.
 *
 * This function performs the same job as `br_x509_minimal_init()`, but
 * also sets implementations for RSA, ECDSA, and the standard hash
 * functions.
 *
 * \param ctx                 context to initialise.
 * \param trust_anchors       trust anchors.
 * \param trust_anchors_num   number of trust anchors.
 */
void br_x509_minimal_init_full(br_x509_minimal_context *ctx,
	const br_x509_trust_anchor *trust_anchors, size_t trust_anchors_num);

/**
 * \brief Set the validation time for the X.509 "minimal" engine.
 *
 * The validation time is set as two 32-bit integers, for days and
 * seconds since a fixed epoch:
 *
 *   - Days are counted in a proleptic Gregorian calendar since
 *     January 1st, 0 AD. Year "0 AD" is the one that preceded "1 AD";
 *     it is also traditionally known as "1 BC".
 *
 *   - Seconds are counted since midnight, from 0 to 86400 (a count of
 *     86400 is possible only if a leap second happened).
 *
 * The validation date and time is understood in the UTC time zone.
 *
 * If the validation date and time are not explicitly set, but BearSSL
 * was compiled with support for the system clock on the underlying
 * platform, then the current time will automatically be used. Otherwise,
 * not setting the validation date and time implies a validation
 * failure (except in case of direct trust of the EE key).
 *
 * \param ctx       validation context.
 * \param days      days since January 1st, 0 AD (Gregorian calendar).
 * \param seconds   seconds since midnight (0 to 86400).
 */
static inline void
br_x509_minimal_set_time(br_x509_minimal_context *ctx,
	uint32_t days, uint32_t seconds)
{
	ctx->days = days;
	ctx->seconds = seconds;
}

/**
 * \brief Set the minimal acceptable length for RSA keys (X.509 "minimal"
 * engine).
 *
 * The RSA key length is expressed in bytes. The default minimum key
 * length is 128 bytes, corresponding to 1017 bits. RSA keys shorter
 * than the configured length will be rejected, implying validation
 * failure. This setting applies to keys extracted from certificates
 * (both end-entity, and intermediate CA) but not to "CA" trust anchors.
 *
 * \param ctx           validation context.
 * \param byte_length   minimum RSA key length, **in bytes** (not bits).
 */
static inline void
br_x509_minimal_set_minrsa(br_x509_minimal_context *ctx, int byte_length)
{
	ctx->min_rsa_size = (int16_t)(byte_length - 128);
}

/**
 * \brief Set the name elements to gather.
 *
 * The provided array is linked in the context. The elements are
 * gathered from the EE certificate. If the same element type is
 * requested several times, then the relevant structures will be filled
 * in the order the matching values are encountered in the certificate.
 *
 * \param ctx        validation context.
 * \param elts       array of name element structures to fill.
 * \param num_elts   number of name element structures to fill.
 */
static inline void
br_x509_minimal_set_name_elements(br_x509_minimal_context *ctx,
	br_name_element *elts, size_t num_elts)
{
	ctx->name_elts = elts;
	ctx->num_name_elts = num_elts;
}

/**
 * \brief X.509 decoder context.
 *
 * This structure is _not_ for X.509 validation, but for extracting
 * names and public keys from encoded certificates. Intended usage is
 * to use (self-signed) certificates as trust anchors.
 *
 * Contents are opaque and shall not be accessed directly.
 */
typedef struct {

#ifndef BR_DOXYGEN_IGNORE
	/* Structure for returning the public key. */
	br_x509_pkey pkey;

	/* CPU for the T0 virtual machine. */
	struct {
		uint32_t *dp;
		uint32_t *rp;
		const unsigned char *ip;
	} cpu;
	uint32_t dp_stack[32];
	uint32_t rp_stack[32];
	int err;

	/* The pad serves as destination for various operations. */
	unsigned char pad[256];

	/* Flag set when decoding succeeds. */
	unsigned char decoded;

	/* Validity dates. */
	uint32_t notbefore_days, notbefore_seconds;
	uint32_t notafter_days, notafter_seconds;

	/* The "CA" flag. This is set to true if the certificate contains
	   a Basic Constraints extension that asserts CA status. */
	unsigned char isCA;

	/* DN processing: the subject DN is extracted and pushed to the
	   provided callback. */
	unsigned char copy_dn;
	void *append_dn_ctx;
	void (*append_dn)(void *ctx, const void *buf, size_t len);

	/* Certificate data chunk. */
	const unsigned char *hbuf;
	size_t hlen;

	/* Buffer for decoded public key. */
	unsigned char pkey_data[BR_X509_BUFSIZE_KEY];

	/* Type of key and hash function used in the certificate signature. */
	unsigned char signer_key_type;
	unsigned char signer_hash_id;
#endif

} br_x509_decoder_context;

/**
 * \brief Initialise an X.509 decoder context for processing a new
 * certificate.
 *
 * The `append_dn()` callback (with opaque context `append_dn_ctx`)
 * will be invoked to receive, chunk by chunk, the certificate's
 * subject DN. If `append_dn` is `0` then the subject DN will be
 * ignored.
 *
 * \param ctx             X.509 decoder context to initialise.
 * \param append_dn       DN receiver callback (or `0`).
 * \param append_dn_ctx   context for the DN receiver callback.
 */
void br_x509_decoder_init(br_x509_decoder_context *ctx,
	void (*append_dn)(void *ctx, const void *buf, size_t len),
	void *append_dn_ctx);

/**
 * \brief Push some certificate bytes into a decoder context.
 *
 * If `len` is non-zero, then that many bytes are pushed, from address
 * `data`, into the provided decoder context.
 *
 * \param ctx    X.509 decoder context.
 * \param data   certificate data chunk.
 * \param len    certificate data chunk length (in bytes).
 */
void br_x509_decoder_push(br_x509_decoder_context *ctx,
	const void *data, size_t len);

/**
 * \brief Obtain the decoded public key.
 *
 * Returned value is a pointer to a structure internal to the decoder
 * context; releasing or reusing the decoder context invalidates that
 * structure.
 *
 * If decoding was not finished, or failed, then `NULL` is returned.
 *
 * \param ctx   X.509 decoder context.
 * \return  the public key, or `NULL` on unfinished/error.
 */
static inline br_x509_pkey *
br_x509_decoder_get_pkey(br_x509_decoder_context *ctx)
{
	if (ctx->decoded && ctx->err == 0) {
		return &ctx->pkey;
	} else {
		return NULL;
	}
}

/**
 * \brief Get decoder error status.
 *
 * If no error was reported yet but the certificate decoding is not
 * finished, then the error code is `BR_ERR_X509_TRUNCATED`. If decoding
 * was successful, then 0 is returned.
 *
 * \param ctx   X.509 decoder context.
 * \return  0 on successful decoding, or a non-zero error code.
 */
static inline int
br_x509_decoder_last_error(br_x509_decoder_context *ctx)
{
	if (ctx->err != 0) {
		return ctx->err;
	}
	if (!ctx->decoded) {
		return BR_ERR_X509_TRUNCATED;
	}
	return 0;
}

/**
 * \brief Get the "isCA" flag from an X.509 decoder context.
 *
 * This flag is set if the decoded certificate claims to be a CA through
 * a Basic Constraints extension. This flag should not be read before
 * decoding completed successfully.
 *
 * \param ctx   X.509 decoder context.
 * \return  the "isCA" flag.
 */
static inline int
br_x509_decoder_isCA(br_x509_decoder_context *ctx)
{
	return ctx->isCA;
}

/**
 * \brief Get the issuing CA key type (type of algorithm used to sign the
 * decoded certificate).
 *
 * This is `BR_KEYTYPE_RSA` or `BR_KEYTYPE_EC`. The value 0 is returned
 * if the signature type was not recognised.
 *
 * \param ctx   X.509 decoder context.
 * \return  the issuing CA key type.
 */
static inline int
br_x509_decoder_get_signer_key_type(br_x509_decoder_context *ctx)
{
	return ctx->signer_key_type;
}

/**
 * \brief Get the identifier for the hash function used to sign the decoded
 * certificate.
 *
 * This is 0 if the hash function was not recognised.
 *
 * \param ctx   X.509 decoder context.
 * \return  the signature hash function identifier.
 */
static inline int
br_x509_decoder_get_signer_hash_id(br_x509_decoder_context *ctx)
{
	return ctx->signer_hash_id;
}

/**
 * \brief Type for an X.509 certificate (DER-encoded).
 */
typedef struct {
	/** \brief The DER-encoded certificate data. */
	unsigned char *data;
	/** \brief The DER-encoded certificate length (in bytes). */
	size_t data_len;
} br_x509_certificate;

/**
 * \brief Private key decoder context.
 *
 * The private key decoder recognises RSA and EC private keys, either in
 * their raw, DER-encoded format, or wrapped in an unencrypted PKCS#8
 * archive (again DER-encoded).
 *
 * Structure contents are opaque and shall not be accessed directly.
 */
typedef struct {
#ifndef BR_DOXYGEN_IGNORE
	/* Structure for returning the private key. */
	union {
		br_rsa_private_key rsa;
		br_ec_private_key ec;
	} key;

	/* CPU for the T0 virtual machine. */
	struct {
		uint32_t *dp;
		uint32_t *rp;
		const unsigned char *ip;
	} cpu;
	uint32_t dp_stack[32];
	uint32_t rp_stack[32];
	int err;

	/* Private key data chunk. */
	const unsigned char *hbuf;
	size_t hlen;

	/* The pad serves as destination for various operations. */
	unsigned char pad[256];

	/* Decoded key type; 0 until decoding is complete. */
	unsigned char key_type;

	/* Buffer for the private key elements. It shall be large enough
	   to accommodate all elements for a RSA-4096 private key (roughly
	   five 2048-bit integers, possibly a bit more). */
	unsigned char key_data[3 * BR_X509_BUFSIZE_SIG];
#endif
} br_skey_decoder_context;

/**
 * \brief Initialise a private key decoder context.
 *
 * \param ctx   key decoder context to initialise.
 */
void br_skey_decoder_init(br_skey_decoder_context *ctx);

/**
 * \brief Push some data bytes into a private key decoder context.
 *
 * If `len` is non-zero, then that many data bytes, starting at address
 * `data`, are pushed into the decoder.
 *
 * \param ctx    key decoder context.
 * \param data   private key data chunk.
 * \param len    private key data chunk length (in bytes).
 */
void br_skey_decoder_push(br_skey_decoder_context *ctx,
	const void *data, size_t len);

/**
 * \brief Get the decoding status for a private key.
 *
 * Decoding status is 0 on success, or a non-zero error code. If the
 * decoding is unfinished when this function is called, then the
 * status code `BR_ERR_X509_TRUNCATED` is returned.
 *
 * \param ctx   key decoder context.
 * \return  0 on successful decoding, or a non-zero error code.
 */
static inline int
br_skey_decoder_last_error(const br_skey_decoder_context *ctx)
{
	if (ctx->err != 0) {
		return ctx->err;
	}
	if (ctx->key_type == 0) {
		return BR_ERR_X509_TRUNCATED;
	}
	return 0;
}

/**
 * \brief Get the decoded private key type.
 *
 * Private key type is `BR_KEYTYPE_RSA` or `BR_KEYTYPE_EC`. If decoding is
 * not finished or failed, then 0 is returned.
 *
 * \param ctx   key decoder context.
 * \return  decoded private key type, or 0.
 */
static inline int
br_skey_decoder_key_type(const br_skey_decoder_context *ctx)
{
	if (ctx->err == 0) {
		return ctx->key_type;
	} else {
		return 0;
	}
}

/**
 * \brief Get the decoded RSA private key.
 *
 * This function returns `NULL` if the decoding failed, or is not
 * finished, or the key is not RSA. The returned pointer references
 * structures within the context that can become invalid if the context
 * is reused or released.
 *
 * \param ctx   key decoder context.
 * \return  decoded RSA private key, or `NULL`.
 */
static inline const br_rsa_private_key *
br_skey_decoder_get_rsa(const br_skey_decoder_context *ctx)
{
	if (ctx->err == 0 && ctx->key_type == BR_KEYTYPE_RSA) {
		return &ctx->key.rsa;
	} else {
		return NULL;
	}
}

/**
 * \brief Get the decoded EC private key.
 *
 * This function returns `NULL` if the decoding failed, or is not
 * finished, or the key is not EC. The returned pointer references
 * structures within the context that can become invalid if the context
 * is reused or released.
 *
 * \param ctx   key decoder context.
 * \return  decoded EC private key, or `NULL`.
 */
static inline const br_ec_private_key *
br_skey_decoder_get_ec(const br_skey_decoder_context *ctx)
{
	if (ctx->err == 0 && ctx->key_type == BR_KEYTYPE_EC) {
		return &ctx->key.ec;
	} else {
		return NULL;
	}
}

#endif


/* ===== inc/bearssl_ssl.h ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef BR_BEARSSL_SSL_H__
#define BR_BEARSSL_SSL_H__

#include <stddef.h>
#include <stdint.h>

/* (already included) */
/* (already included) */
/* (already included) */
/* (already included) */
/* (already included) */
/* (already included) */

/** \file bearssl_ssl.h
 *
 * # SSL
 *
 * For an overview of the SSL/TLS API, see [the BearSSL Web
 * site](https://www.bearssl.org/api1.html).
 *
 * The `BR_TLS_*` constants correspond to the standard cipher suites and
 * their values in the [IANA
 * registry](http://www.iana.org/assignments/tls-parameters/tls-parameters.xhtml#tls-parameters-4).
 *
 * The `BR_ALERT_*` constants are for standard TLS alert messages. When
 * a fatal alert message is sent of received, then the SSL engine context
 * status is set to the sum of that alert value (an integer in the 0..255
 * range) and a fixed offset (`BR_ERR_SEND_FATAL_ALERT` for a sent alert,
 * `BR_ERR_RECV_FATAL_ALERT` for a received alert).
 */

/** \brief Optimal input buffer size. */
#define BR_SSL_BUFSIZE_INPUT    (16384 + 325)

/** \brief Optimal output buffer size. */
#define BR_SSL_BUFSIZE_OUTPUT   (16384 + 85)

/** \brief Optimal buffer size for monodirectional engine
    (shared input/output buffer). */
#define BR_SSL_BUFSIZE_MONO     BR_SSL_BUFSIZE_INPUT

/** \brief Optimal buffer size for bidirectional engine
    (single buffer split into two separate input/output buffers). */
#define BR_SSL_BUFSIZE_BIDI     (BR_SSL_BUFSIZE_INPUT + BR_SSL_BUFSIZE_OUTPUT)

/*
 * Constants for known SSL/TLS protocol versions (SSL 3.0, TLS 1.0, TLS 1.1
 * and TLS 1.2). Note that though there is a constant for SSL 3.0, that
 * protocol version is not actually supported.
 */

/** \brief Protocol version: SSL 3.0 (unsupported). */
#define BR_SSL30   0x0300
/** \brief Protocol version: TLS 1.0. */
#define BR_TLS10   0x0301
/** \brief Protocol version: TLS 1.1. */
#define BR_TLS11   0x0302
/** \brief Protocol version: TLS 1.2. */
#define BR_TLS12   0x0303

/*
 * Error constants. They are used to report the reason why a context has
 * been marked as failed.
 *
 * Implementation note: SSL-level error codes should be in the 1..31
 * range. The 32..63 range is for certificate decoding and validation
 * errors. Received fatal alerts imply an error code in the 256..511 range.
 */

/** \brief SSL status: no error so far (0). */
#define BR_ERR_OK                      0

/** \brief SSL status: caller-provided parameter is incorrect. */
#define BR_ERR_BAD_PARAM               1

/** \brief SSL status: operation requested by the caller cannot be applied
    with the current context state (e.g. reading data while outgoing data
    is waiting to be sent). */
#define BR_ERR_BAD_STATE               2

/** \brief SSL status: incoming protocol or record version is unsupported. */
#define BR_ERR_UNSUPPORTED_VERSION     3

/** \brief SSL status: incoming record version does not match the expected
    version. */
#define BR_ERR_BAD_VERSION             4

/** \brief SSL status: incoming record length is invalid. */
#define BR_ERR_BAD_LENGTH              5

/** \brief SSL status: incoming record is too large to be processed, or
    buffer is too small for the handshake message to send. */
#define BR_ERR_TOO_LARGE               6

/** \brief SSL status: decryption found an invalid padding, or the record
    MAC is not correct. */
#define BR_ERR_BAD_MAC                 7

/** \brief SSL status: no initial entropy was provided, and none can be
    obtained from the OS. */
#define BR_ERR_NO_RANDOM               8

/** \brief SSL status: incoming record type is unknown. */
#define BR_ERR_UNKNOWN_TYPE            9

/** \brief SSL status: incoming record or message has wrong type with
    regards to the current engine state. */
#define BR_ERR_UNEXPECTED             10

/** \brief SSL status: ChangeCipherSpec message from the peer has invalid
    contents. */
#define BR_ERR_BAD_CCS                12

/** \brief SSL status: alert message from the peer has invalid contents
    (odd length). */
#define BR_ERR_BAD_ALERT              13

/** \brief SSL status: incoming handshake message decoding failed. */
#define BR_ERR_BAD_HANDSHAKE          14

/** \brief SSL status: ServerHello contains a session ID which is larger
    than 32 bytes. */
#define BR_ERR_OVERSIZED_ID           15

/** \brief SSL status: server wants to use a cipher suite that we did
    not claim to support. This is also reported if we tried to advertise
    a cipher suite that we do not support. */
#define BR_ERR_BAD_CIPHER_SUITE       16

/** \brief SSL status: server wants to use a compression that we did not
    claim to support. */
#define BR_ERR_BAD_COMPRESSION        17

/** \brief SSL status: server's max fragment length does not match
    client's. */
#define BR_ERR_BAD_FRAGLEN            18

/** \brief SSL status: secure renegotiation failed. */
#define BR_ERR_BAD_SECRENEG           19

/** \brief SSL status: server sent an extension type that we did not
    announce, or used the same extension type several times in a single
    ServerHello. */
#define BR_ERR_EXTRA_EXTENSION        20

/** \brief SSL status: invalid Server Name Indication contents (when
    used by the server, this extension shall be empty). */
#define BR_ERR_BAD_SNI                21

/** \brief SSL status: invalid ServerHelloDone from the server (length
    is not 0). */
#define BR_ERR_BAD_HELLO_DONE         22

/** \brief SSL status: internal limit exceeded (e.g. server's public key
    is too large). */
#define BR_ERR_LIMIT_EXCEEDED         23

/** \brief SSL status: Finished message from peer does not match the
    expected value. */
#define BR_ERR_BAD_FINISHED           24

/** \brief SSL status: session resumption attempt with distinct version
    or cipher suite. */
#define BR_ERR_RESUME_MISMATCH        25

/** \brief SSL status: unsupported or invalid algorithm (ECDHE curve,
    signature algorithm, hash function). */
#define BR_ERR_INVALID_ALGORITHM      26

/** \brief SSL status: invalid signature (on ServerKeyExchange from
    server, or in CertificateVerify from client). */
#define BR_ERR_BAD_SIGNATURE          27

/** \brief SSL status: peer's public key does not have the proper type
    or is not allowed for requested operation. */
#define BR_ERR_WRONG_KEY_USAGE        28

/** \brief SSL status: client did not send a certificate upon request,
    or the client certificate could not be validated. */
#define BR_ERR_NO_CLIENT_AUTH         29

/** \brief SSL status: I/O error or premature close on underlying
    transport stream. This error code is set only by the simplified
    I/O API ("br_sslio_*"). */
#define BR_ERR_IO                     31

/** \brief SSL status: base value for a received fatal alert.

    When a fatal alert is received from the peer, the alert value
    is added to this constant. */
#define BR_ERR_RECV_FATAL_ALERT      256

/** \brief SSL status: base value for a sent fatal alert.

    When a fatal alert is sent to the peer, the alert value is added
    to this constant. */
#define BR_ERR_SEND_FATAL_ALERT      512

/* ===================================================================== */

/**
 * \brief Decryption engine for SSL.
 *
 * When processing incoming records, the SSL engine will use a decryption
 * engine that uses a specific context structure, and has a set of
 * methods (a vtable) that follows this template.
 *
 * The decryption engine is responsible for applying decryption, verifying
 * MAC, and keeping track of the record sequence number.
 */
typedef struct br_sslrec_in_class_ br_sslrec_in_class;
struct br_sslrec_in_class_ {
	/**
	 * \brief Context size (in bytes).
	 */
	size_t context_size;

	/**
	 * \brief Test validity of the incoming record length.
	 *
	 * This function returns 1 if the announced length for an
	 * incoming record is valid, 0 otherwise,
	 *
	 * \param ctx          decryption engine context.
	 * \param record_len   incoming record length.
	 * \return  1 of a valid length, 0 otherwise.
	 */
	int (*check_length)(const br_sslrec_in_class *const *ctx,
		size_t record_len);

	/**
	 * \brief Decrypt the incoming record.
	 *
	 * This function may assume that the record length is valid
	 * (it has been previously tested with `check_length()`).
	 * Decryption is done in place; `*len` is updated with the
	 * cleartext length, and the address of the first plaintext
	 * byte is returned. If the record is correct but empty, then
	 * `*len` is set to 0 and a non-`NULL` pointer is returned.
	 *
	 * On decryption/MAC error, `NULL` is returned.
	 *
	 * \param ctx           decryption engine context.
	 * \param record_type   record type (23 for application data, etc).
	 * \param version       record version.
	 * \param payload       address of encrypted payload.
	 * \param len           pointer to payload length (updated).
	 * \return  pointer to plaintext, or `NULL` on error.
	 */
	unsigned char *(*decrypt)(const br_sslrec_in_class **ctx,
		int record_type, unsigned version,
		void *payload, size_t *len);
};

/**
 * \brief Encryption engine for SSL.
 *
 * When building outgoing records, the SSL engine will use an encryption
 * engine that uses a specific context structure, and has a set of
 * methods (a vtable) that follows this template.
 *
 * The encryption engine is responsible for applying encryption and MAC,
 * and keeping track of the record sequence number.
 */
typedef struct br_sslrec_out_class_ br_sslrec_out_class;
struct br_sslrec_out_class_ {
	/**
	 * \brief Context size (in bytes).
	 */
	size_t context_size;

	/**
	 * \brief Compute maximum plaintext sizes and offsets.
	 *
	 * When this function is called, the `*start` and `*end`
	 * values contain offsets designating the free area in the
	 * outgoing buffer for plaintext data; that free area is
	 * preceded by a 5-byte space which will receive the record
	 * header.
	 *
	 * The `max_plaintext()` function is responsible for adjusting
	 * both `*start` and `*end` to make room for any record-specific
	 * header, MAC, padding, and possible split.
	 *
	 * \param ctx     encryption engine context.
	 * \param start   pointer to start of plaintext offset (updated).
	 * \param end     pointer to start of plaintext offset (updated).
	 */
	void (*max_plaintext)(const br_sslrec_out_class *const *ctx,
		size_t *start, size_t *end);

	/**
	 * \brief Perform record encryption.
	 *
	 * This function encrypts the record. The plaintext address and
	 * length are provided. Returned value is the start of the
	 * encrypted record (or sequence of records, if a split was
	 * performed), _including_ the 5-byte header, and `*len` is
	 * adjusted to the total size of the record(s), there again
	 * including the header(s).
	 *
	 * \param ctx           decryption engine context.
	 * \param record_type   record type (23 for application data, etc).
	 * \param version       record version.
	 * \param plaintext     address of plaintext.
	 * \param len           pointer to plaintext length (updated).
	 * \return  pointer to start of built record.
	 */
	unsigned char *(*encrypt)(const br_sslrec_out_class **ctx,
		int record_type, unsigned version,
		void *plaintext, size_t *len);
};

/**
 * \brief Context for a no-encryption engine.
 *
 * The no-encryption engine processes outgoing records during the initial
 * handshake, before encryption is applied.
 */
typedef struct {
	/** \brief No-encryption engine vtable. */
	const br_sslrec_out_class *vtable;
} br_sslrec_out_clear_context;

/** \brief Static, constant vtable for the no-encryption engine. */
extern const br_sslrec_out_class br_sslrec_out_clear_vtable;

/* ===================================================================== */

/**
 * \brief Record decryption engine class, for CBC mode.
 *
 * This class type extends the decryption engine class with an
 * initialisation method that receives the parameters needed
 * for CBC processing: block cipher implementation, block cipher key,
 * HMAC parameters (hash function, key, MAC length), and IV. If the
 * IV is `NULL`, then a per-record IV will be used (TLS 1.1+).
 */
typedef struct br_sslrec_in_cbc_class_ br_sslrec_in_cbc_class;
struct br_sslrec_in_cbc_class_ {
	/**
	 * \brief Superclass, as first vtable field.
	 */
	br_sslrec_in_class inner;

	/**
	 * \brief Engine initialisation method.
	 *
	 * This method sets the vtable field in the context.
	 *
	 * \param ctx           context to initialise.
	 * \param bc_impl       block cipher implementation (CBC decryption).
	 * \param bc_key        block cipher key.
	 * \param bc_key_len    block cipher key length (in bytes).
	 * \param dig_impl      hash function for HMAC.
	 * \param mac_key       HMAC key.
	 * \param mac_key_len   HMAC key length (in bytes).
	 * \param mac_out_len   HMAC output length (in bytes).
	 * \param iv            initial IV (or `NULL`).
	 */
	void (*init)(const br_sslrec_in_cbc_class **ctx,
		const br_block_cbcdec_class *bc_impl,
		const void *bc_key, size_t bc_key_len,
		const br_hash_class *dig_impl,
		const void *mac_key, size_t mac_key_len, size_t mac_out_len,
		const void *iv);
};

/**
 * \brief Record encryption engine class, for CBC mode.
 *
 * This class type extends the encryption engine class with an
 * initialisation method that receives the parameters needed
 * for CBC processing: block cipher implementation, block cipher key,
 * HMAC parameters (hash function, key, MAC length), and IV. If the
 * IV is `NULL`, then a per-record IV will be used (TLS 1.1+).
 */
typedef struct br_sslrec_out_cbc_class_ br_sslrec_out_cbc_class;
struct br_sslrec_out_cbc_class_ {
	/**
	 * \brief Superclass, as first vtable field.
	 */
	br_sslrec_out_class inner;

	/**
	 * \brief Engine initialisation method.
	 *
	 * This method sets the vtable field in the context.
	 *
	 * \param ctx           context to initialise.
	 * \param bc_impl       block cipher implementation (CBC encryption).
	 * \param bc_key        block cipher key.
	 * \param bc_key_len    block cipher key length (in bytes).
	 * \param dig_impl      hash function for HMAC.
	 * \param mac_key       HMAC key.
	 * \param mac_key_len   HMAC key length (in bytes).
	 * \param mac_out_len   HMAC output length (in bytes).
	 * \param iv            initial IV (or `NULL`).
	 */
	void (*init)(const br_sslrec_out_cbc_class **ctx,
		const br_block_cbcenc_class *bc_impl,
		const void *bc_key, size_t bc_key_len,
		const br_hash_class *dig_impl,
		const void *mac_key, size_t mac_key_len, size_t mac_out_len,
		const void *iv);
};

/**
 * \brief Context structure for decrypting incoming records with
 * CBC + HMAC.
 *
 * The first field points to the vtable. The other fields are opaque
 * and shall not be accessed directly.
 */
typedef struct {
	/** \brief Pointer to vtable. */
	const br_sslrec_in_cbc_class *vtable;
#ifndef BR_DOXYGEN_IGNORE
	uint64_t seq;
	union {
		const br_block_cbcdec_class *vtable;
		br_aes_gen_cbcdec_keys aes;
		br_des_gen_cbcdec_keys des;
	} bc;
	br_hmac_key_context mac;
	size_t mac_len;
	unsigned char iv[16];
	int explicit_IV;
#endif
} br_sslrec_in_cbc_context;

/**
 * \brief Static, constant vtable for record decryption with CBC.
 */
extern const br_sslrec_in_cbc_class br_sslrec_in_cbc_vtable;

/**
 * \brief Context structure for encrypting outgoing records with
 * CBC + HMAC.
 *
 * The first field points to the vtable. The other fields are opaque
 * and shall not be accessed directly.
 */
typedef struct {
	/** \brief Pointer to vtable. */
	const br_sslrec_out_cbc_class *vtable;
#ifndef BR_DOXYGEN_IGNORE
	uint64_t seq;
	union {
		const br_block_cbcenc_class *vtable;
		br_aes_gen_cbcenc_keys aes;
		br_des_gen_cbcenc_keys des;
	} bc;
	br_hmac_key_context mac;
	size_t mac_len;
	unsigned char iv[16];
	int explicit_IV;
#endif
} br_sslrec_out_cbc_context;

/**
 * \brief Static, constant vtable for record encryption with CBC.
 */
extern const br_sslrec_out_cbc_class br_sslrec_out_cbc_vtable;

/* ===================================================================== */

/**
 * \brief Record decryption engine class, for GCM mode.
 *
 * This class type extends the decryption engine class with an
 * initialisation method that receives the parameters needed
 * for GCM processing: block cipher implementation, block cipher key,
 * GHASH implementation, and 4-byte IV.
 */
typedef struct br_sslrec_in_gcm_class_ br_sslrec_in_gcm_class;
struct br_sslrec_in_gcm_class_ {
	/**
	 * \brief Superclass, as first vtable field.
	 */
	br_sslrec_in_class inner;

	/**
	 * \brief Engine initialisation method.
	 *
	 * This method sets the vtable field in the context.
	 *
	 * \param ctx           context to initialise.
	 * \param bc_impl       block cipher implementation (CTR).
	 * \param key           block cipher key.
	 * \param key_len       block cipher key length (in bytes).
	 * \param gh_impl       GHASH implementation.
	 * \param iv            static IV (4 bytes).
	 */
	void (*init)(const br_sslrec_in_gcm_class **ctx,
		const br_block_ctr_class *bc_impl,
		const void *key, size_t key_len,
		br_ghash gh_impl,
		const void *iv);
};

/**
 * \brief Record encryption engine class, for GCM mode.
 *
 * This class type extends the encryption engine class with an
 * initialisation method that receives the parameters needed
 * for GCM processing: block cipher implementation, block cipher key,
 * GHASH implementation, and 4-byte IV.
 */
typedef struct br_sslrec_out_gcm_class_ br_sslrec_out_gcm_class;
struct br_sslrec_out_gcm_class_ {
	/**
	 * \brief Superclass, as first vtable field.
	 */
	br_sslrec_out_class inner;

	/**
	 * \brief Engine initialisation method.
	 *
	 * This method sets the vtable field in the context.
	 *
	 * \param ctx           context to initialise.
	 * \param bc_impl       block cipher implementation (CTR).
	 * \param key           block cipher key.
	 * \param key_len       block cipher key length (in bytes).
	 * \param gh_impl       GHASH implementation.
	 * \param iv            static IV (4 bytes).
	 */
	void (*init)(const br_sslrec_out_gcm_class **ctx,
		const br_block_ctr_class *bc_impl,
		const void *key, size_t key_len,
		br_ghash gh_impl,
		const void *iv);
};

/**
 * \brief Context structure for processing records with GCM.
 *
 * The same context structure is used for encrypting and decrypting.
 *
 * The first field points to the vtable. The other fields are opaque
 * and shall not be accessed directly.
 */
typedef struct {
	/** \brief Pointer to vtable. */
	union {
		const void *gen;
		const br_sslrec_in_gcm_class *in;
		const br_sslrec_out_gcm_class *out;
	} vtable;
#ifndef BR_DOXYGEN_IGNORE
	uint64_t seq;
	union {
		const br_block_ctr_class *vtable;
		br_aes_gen_ctr_keys aes;
	} bc;
	br_ghash gh;
	unsigned char iv[4];
	unsigned char h[16];
#endif
} br_sslrec_gcm_context;

/**
 * \brief Static, constant vtable for record decryption with GCM.
 */
extern const br_sslrec_in_gcm_class br_sslrec_in_gcm_vtable;

/**
 * \brief Static, constant vtable for record encryption with GCM.
 */
extern const br_sslrec_out_gcm_class br_sslrec_out_gcm_vtable;

/* ===================================================================== */

/**
 * \brief Record decryption engine class, for ChaCha20+Poly1305.
 *
 * This class type extends the decryption engine class with an
 * initialisation method that receives the parameters needed
 * for ChaCha20+Poly1305 processing: ChaCha20 implementation,
 * Poly1305 implementation, key, and 12-byte IV.
 */
typedef struct br_sslrec_in_chapol_class_ br_sslrec_in_chapol_class;
struct br_sslrec_in_chapol_class_ {
	/**
	 * \brief Superclass, as first vtable field.
	 */
	br_sslrec_in_class inner;

	/**
	 * \brief Engine initialisation method.
	 *
	 * This method sets the vtable field in the context.
	 *
	 * \param ctx           context to initialise.
	 * \param ichacha       ChaCha20 implementation.
	 * \param ipoly         Poly1305 implementation.
	 * \param key           secret key (32 bytes).
	 * \param iv            static IV (12 bytes).
	 */
	void (*init)(const br_sslrec_in_chapol_class **ctx,
		br_chacha20_run ichacha,
		br_poly1305_run ipoly,
		const void *key, const void *iv);
};

/**
 * \brief Record encryption engine class, for ChaCha20+Poly1305.
 *
 * This class type extends the encryption engine class with an
 * initialisation method that receives the parameters needed
 * for ChaCha20+Poly1305 processing: ChaCha20 implementation,
 * Poly1305 implementation, key, and 12-byte IV.
 */
typedef struct br_sslrec_out_chapol_class_ br_sslrec_out_chapol_class;
struct br_sslrec_out_chapol_class_ {
	/**
	 * \brief Superclass, as first vtable field.
	 */
	br_sslrec_out_class inner;

	/**
	 * \brief Engine initialisation method.
	 *
	 * This method sets the vtable field in the context.
	 *
	 * \param ctx           context to initialise.
	 * \param ichacha       ChaCha20 implementation.
	 * \param ipoly         Poly1305 implementation.
	 * \param key           secret key (32 bytes).
	 * \param iv            static IV (12 bytes).
	 */
	void (*init)(const br_sslrec_out_chapol_class **ctx,
		br_chacha20_run ichacha,
		br_poly1305_run ipoly,
		const void *key, const void *iv);
};

/**
 * \brief Context structure for processing records with ChaCha20+Poly1305.
 *
 * The same context structure is used for encrypting and decrypting.
 *
 * The first field points to the vtable. The other fields are opaque
 * and shall not be accessed directly.
 */
typedef struct {
	/** \brief Pointer to vtable. */
	union {
		const void *gen;
		const br_sslrec_in_chapol_class *in;
		const br_sslrec_out_chapol_class *out;
	} vtable;
#ifndef BR_DOXYGEN_IGNORE
	uint64_t seq;
	unsigned char key[32];
	unsigned char iv[12];
	br_chacha20_run ichacha;
	br_poly1305_run ipoly;
#endif
} br_sslrec_chapol_context;

/**
 * \brief Static, constant vtable for record decryption with ChaCha20+Poly1305.
 */
extern const br_sslrec_in_chapol_class br_sslrec_in_chapol_vtable;

/**
 * \brief Static, constant vtable for record encryption with ChaCha20+Poly1305.
 */
extern const br_sslrec_out_chapol_class br_sslrec_out_chapol_vtable;

/* ===================================================================== */

/**
 * \brief Type for session parameters, to be saved for session resumption.
 */
typedef struct {
	/** \brief Session ID buffer. */
	unsigned char session_id[32];
	/** \brief Session ID length (in bytes, at most 32). */
	unsigned char session_id_len;
	/** \brief Protocol version. */
	uint16_t version;
	/** \brief Cipher suite. */
	uint16_t cipher_suite;
	/** \brief Master secret. */
	unsigned char master_secret[48];
} br_ssl_session_parameters;

#ifndef BR_DOXYGEN_IGNORE
/*
 * Maximum numnber of cipher suites supported by a client or server.
 */
#define BR_MAX_CIPHER_SUITES   40
#endif

/**
 * \brief Context structure for SSL engine.
 *
 * This strucuture is common to the client and server; both the client
 * context (`br_ssl_client_context`) and the server context
 * (`br_ssl_server_context`) include a `br_ssl_engine_context` as their
 * first field.
 *
 * The engine context manages records, including alerts, closures, and
 * transitions to new encryption/MAC algorithms. Processing of handshake
 * records is delegated to externally provided code. This structure
 * should not be used directly.
 *
 * Structure contents are opaque and shall not be accessed directly.
 */
typedef struct {
#ifndef BR_DOXYGEN_IGNORE
	/*
	 * The error code. When non-zero, then the state is "failed" and
	 * no I/O may occur until reset.
	 */
	int err;

	/*
	 * Configured I/O buffers. They are either disjoint, or identical.
	 */
	unsigned char *ibuf, *obuf;
	size_t ibuf_len, obuf_len;

	/*
	 * Maximum fragment length applies to outgoing records; incoming
	 * records can be processed as long as they fit in the input
	 * buffer. It is guaranteed that incoming records at least as big
	 * as max_frag_len can be processed.
	 */
	uint16_t max_frag_len;
	unsigned char log_max_frag_len;
	unsigned char peer_log_max_frag_len;

	/*
	 * Buffering management registers.
	 */
	size_t ixa, ixb, ixc;
	size_t oxa, oxb, oxc;
	unsigned char iomode;
	unsigned char incrypt;

	/*
	 * Shutdown flag: when set to non-zero, incoming record bytes
	 * will not be accepted anymore. This is used after a close_notify
	 * has been received: afterwards, the engine no longer claims that
	 * it could receive bytes from the transport medium.
	 */
	unsigned char shutdown_recv;

	/*
	 * 'record_type_in' is set to the incoming record type when the
	 * record header has been received.
	 * 'record_type_out' is used to make the next outgoing record
	 * header when it is ready to go.
	 */
	unsigned char record_type_in, record_type_out;

	/*
	 * When a record is received, its version is extracted:
	 * -- if 'version_in' is 0, then it is set to the received version;
	 * -- otherwise, if the received version is not identical to
	 *    the 'version_in' contents, then a failure is reported.
	 *
	 * This implements the SSL requirement that all records shall
	 * use the negotiated protocol version, once decided (in the
	 * ServerHello). It is up to the handshake handler to adjust this
	 * field when necessary.
	 */
	uint16_t version_in;

	/*
	 * 'version_out' is used when the next outgoing record is ready
	 * to go.
	 */
	uint16_t version_out;

	/*
	 * Record handler contexts.
	 */
	union {
		const br_sslrec_in_class *vtable;
		br_sslrec_in_cbc_context cbc;
		br_sslrec_gcm_context gcm;
		br_sslrec_chapol_context chapol;
	} in;
	union {
		const br_sslrec_out_class *vtable;
		br_sslrec_out_clear_context clear;
		br_sslrec_out_cbc_context cbc;
		br_sslrec_gcm_context gcm;
		br_sslrec_chapol_context chapol;
	} out;

	/*
	 * The "application data" flag. It is set when application data
	 * can be exchanged, cleared otherwise.
	 */
	unsigned char application_data;

	/*
	 * Context RNG.
	 */
	br_hmac_drbg_context rng;
	int rng_init_done;
	int rng_os_rand_done;

	/*
	 * Supported minimum and maximum versions, and cipher suites.
	 */
	uint16_t version_min;
	uint16_t version_max;
	uint16_t suites_buf[BR_MAX_CIPHER_SUITES];
	unsigned char suites_num;

	/*
	 * For clients, the server name to send as a SNI extension. For
	 * servers, the name received in the SNI extension (if any).
	 */
	char server_name[256];

	/*
	 * "Security parameters". These are filled by the handshake
	 * handler, and used when switching encryption state.
	 */
	unsigned char client_random[32];
	unsigned char server_random[32];
	br_ssl_session_parameters session;

	/*
	 * ECDHE elements: curve and point from the peer. The server also
	 * uses that buffer for the point to send to the client.
	 */
	unsigned char ecdhe_curve;
	unsigned char ecdhe_point[133];
	unsigned char ecdhe_point_len;

	/*
	 * Secure renegotiation (RFC 5746): 'reneg' can be:
	 *   0   first handshake (server support is not known)
	 *   1   server does not support secure renegotiation
	 *   2   server supports secure renegotiation
	 *
	 * The saved_finished buffer contains the client and the
	 * server "Finished" values from the last handshake, in
	 * that order (12 bytes each).
	 */
	unsigned char reneg;
	unsigned char saved_finished[24];

	/*
	 * Behavioural flags.
	 */
	uint32_t flags;

	/*
	 * Context variables for the handshake processor. The 'pad' must
	 * be large enough to accommodate an RSA-encrypted pre-master
	 * secret, or an RSA signature; since we want to support up to
	 * RSA-4096, this means at least 512 bytes. (Other pad usages
	 * require its length to be at least 256.)
	 */
	struct {
		uint32_t *dp;
		uint32_t *rp;
		const unsigned char *ip;
	} cpu;
	uint32_t dp_stack[32];
	uint32_t rp_stack[32];
	unsigned char pad[512];
	unsigned char *hbuf_in, *hbuf_out, *saved_hbuf_out;
	size_t hlen_in, hlen_out;
	void (*hsrun)(void *ctx);

	/*
	 * The 'action' value communicates OOB information between the
	 * engine and the handshake processor.
	 *
	 * From the engine:
	 *   0  invocation triggered by I/O
	 *   1  invocation triggered by explicit close
	 *   2  invocation triggered by explicit renegotiation
	 */
	unsigned char action;

	/*
	 * State for alert messages. Value is either 0, or the value of
	 * the alert level byte (level is either 1 for warning, or 2 for
	 * fatal; we convert all other values to 'fatal').
	 */
	unsigned char alert;

	/*
	 * Closure flags. This flag is set when a close_notify has been
	 * received from the peer.
	 */
	unsigned char close_received;

	/*
	 * Multi-hasher for the handshake messages. The handshake handler
	 * is responsible for resetting it when appropriate.
	 */
	br_multihash_context mhash;

	/*
	 * Pointer to the X.509 engine. The engine is supposed to be
	 * already initialized. It is used to validate the peer's
	 * certificate.
	 */
	const br_x509_class **x509ctx;

	/*
	 * Certificate chain to send. This is used by both client and
	 * server, when they send their respective Certificate messages.
	 * If chain_len is 0, then chain may be NULL.
	 */
	const br_x509_certificate *chain;
	size_t chain_len;
	const unsigned char *cert_cur;
	size_t cert_len;

	/*
	 * List of supported protocol names (ALPN extension). If unset,
	 * (number of names is 0), then:
	 *  - the client sends no ALPN extension;
	 *  - the server ignores any incoming ALPN extension.
	 *
	 * Otherwise:
	 *  - the client sends an ALPN extension with all the names;
	 *  - the server selects the first protocol in its list that
	 *    the client also supports, or fails (fatal alert 120)
	 *    if the client sends an ALPN extension and there is no
	 *    match.
	 *
	 * The 'selected_protocol' field contains 1+n if the matching
	 * name has index n in the list (the value is 0 if no match was
	 * performed, e.g. the peer did not send an ALPN extension).
	 */
	const char **protocol_names;
	uint16_t protocol_names_num;
	uint16_t selected_protocol;

	/*
	 * Pointers to implementations; left to NULL for unsupported
	 * functions. For the raw hash functions, implementations are
	 * referenced from the multihasher (mhash field).
	 */
	br_tls_prf_impl prf10;
	br_tls_prf_impl prf_sha256;
	br_tls_prf_impl prf_sha384;
	const br_block_cbcenc_class *iaes_cbcenc;
	const br_block_cbcdec_class *iaes_cbcdec;
	const br_block_ctr_class *iaes_ctr;
	const br_block_cbcenc_class *ides_cbcenc;
	const br_block_cbcdec_class *ides_cbcdec;
	br_ghash ighash;
	br_chacha20_run ichacha;
	br_poly1305_run ipoly;
	const br_sslrec_in_cbc_class *icbc_in;
	const br_sslrec_out_cbc_class *icbc_out;
	const br_sslrec_in_gcm_class *igcm_in;
	const br_sslrec_out_gcm_class *igcm_out;
	const br_sslrec_in_chapol_class *ichapol_in;
	const br_sslrec_out_chapol_class *ichapol_out;
	const br_ec_impl *iec;
	br_rsa_pkcs1_vrfy irsavrfy;
	br_ecdsa_vrfy iecdsa;
#endif
} br_ssl_engine_context;

/**
 * \brief Get currently defined engine behavioural flags.
 *
 * \param cc   SSL engine context.
 * \return  the flags.
 */
static inline uint32_t
br_ssl_engine_get_flags(br_ssl_engine_context *cc)
{
	return cc->flags;
}

/**
 * \brief Set all engine behavioural flags.
 *
 * \param cc      SSL engine context.
 * \param flags   new value for all flags.
 */
static inline void
br_ssl_engine_set_all_flags(br_ssl_engine_context *cc, uint32_t flags)
{
	cc->flags = flags;
}

/**
 * \brief Set some engine behavioural flags.
 *
 * The flags set in the `flags` parameter are set in the context; other
 * flags are untouched.
 *
 * \param cc      SSL engine context.
 * \param flags   additional set flags.
 */
static inline void
br_ssl_engine_add_flags(br_ssl_engine_context *cc, uint32_t flags)
{
	cc->flags |= flags;
}

/**
 * \brief Clear some engine behavioural flags.
 *
 * The flags set in the `flags` parameter are cleared from the context; other
 * flags are untouched.
 *
 * \param cc      SSL engine context.
 * \param flags   flags to remove.
 */
static inline void
br_ssl_engine_remove_flags(br_ssl_engine_context *cc, uint32_t flags)
{
	cc->flags &= ~flags;
}

/**
 * \brief Behavioural flag: enforce server preferences.
 *
 * If this flag is set, then the server will enforce its own cipher suite
 * preference order; otherwise, it follows the client preferences.
 */
#define BR_OPT_ENFORCE_SERVER_PREFERENCES      ((uint32_t)1 << 0)

/**
 * \brief Behavioural flag: disable renegotiation.
 *
 * If this flag is set, then renegotiations are rejected unconditionally:
 * they won't be honoured if asked for programmatically, and requests from
 * the peer are rejected.
 */
#define BR_OPT_NO_RENEGOTIATION                ((uint32_t)1 << 1)

/**
 * \brief Behavioural flag: tolerate lack of client authentication.
 *
 * If this flag is set in a server and the server requests a client
 * certificate, but the authentication fails (the client does not send
 * a certificate, or the client's certificate chain cannot be validated),
 * then the connection keeps on. Without this flag, a failed client
 * authentication terminates the connection.
 *
 * Notes:
 *
 *   - If the client's certificate can be validated and its public key is
 *     supported, then a wrong signature value terminates the connection
 *     regardless of that flag.
 *
 *   - If using full-static ECDH, then a failure to validate the client's
 *     certificate prevents the handshake from succeeding.
 */
#define BR_OPT_TOLERATE_NO_CLIENT_AUTH         ((uint32_t)1 << 2)

/**
 * \brief Behavioural flag: fail on application protocol mismatch.
 *
 * The ALPN extension ([RFC 7301](https://tools.ietf.org/html/rfc7301))
 * allows the client to send a list of application protocol names, and
 * the server to select one. A mismatch is one of the following occurrences:
 *
 *   - On the client: the client sends a list of names, the server
 *     responds with a protocol name which is _not_ part of the list of
 *     names sent by the client.
 *
 *   - On the server: the client sends a list of names, and the server
 *     is also configured with a list of names, but there is no common
 *     protocol name between the two lists.
 *
 * Normal behaviour in case of mismatch is to report no matching name
 * (`br_ssl_engine_get_selected_protocol()` returns `NULL`) and carry on.
 * If the flag is set, then a mismatch implies a protocol failure (if
 * the mismatch is detected by the server, it will send a fatal alert).
 *
 * Note: even with this flag, `br_ssl_engine_get_selected_protocol()`
 * may still return `NULL` if the client or the server does not send an
 * ALPN extension at all.
 */
#define BR_OPT_FAIL_ON_ALPN_MISMATCH           ((uint32_t)1 << 3)

/**
 * \brief Set the minimum and maximum supported protocol versions.
 *
 * The two provided versions MUST be supported by the implementation
 * (i.e. TLS 1.0, 1.1 and 1.2), and `version_max` MUST NOT be lower
 * than `version_min`.
 *
 * \param cc            SSL engine context.
 * \param version_min   minimum supported TLS version.
 * \param version_max   maximum supported TLS version.
 */
static inline void
br_ssl_engine_set_versions(br_ssl_engine_context *cc,
	unsigned version_min, unsigned version_max)
{
	cc->version_min = version_min;
	cc->version_max = version_max;
}

/**
 * \brief Set the list of cipher suites advertised by this context.
 *
 * The provided array is copied into the context. It is the caller
 * responsibility to ensure that all provided suites will be supported
 * by the context. The engine context has enough room to receive _all_
 * suites supported by the implementation. The provided array MUST NOT
 * contain duplicates.
 *
 * If the engine is for a client, the "signaling" pseudo-cipher suite
 * `TLS_FALLBACK_SCSV` can be added at the end of the list, if the
 * calling application is performing a voluntary downgrade (voluntary
 * downgrades are not recommended, but if such a downgrade is done, then
 * adding the fallback pseudo-suite is a good idea).
 *
 * \param cc           SSL engine context.
 * \param suites       cipher suites.
 * \param suites_num   number of cipher suites.
 */
void br_ssl_engine_set_suites(br_ssl_engine_context *cc,
	const uint16_t *suites, size_t suites_num);

/**
 * \brief Set the X.509 engine.
 *
 * The caller shall ensure that the X.509 engine is properly initialised.
 *
 * \param cc        SSL engine context.
 * \param x509ctx   X.509 certificate validation context.
 */
static inline void
br_ssl_engine_set_x509(br_ssl_engine_context *cc, const br_x509_class **x509ctx)
{
	cc->x509ctx = x509ctx;
}

/**
 * \brief Set the supported protocol names.
 *
 * Protocol names are part of the ALPN extension ([RFC
 * 7301](https://tools.ietf.org/html/rfc7301)). Each protocol name is a
 * character string, containing no more than 255 characters (256 with the
 * terminating zero). When names are set, then:
 *
 *   - The client will send an ALPN extension, containing the names. If
 *     the server responds with an ALPN extension, the client will verify
 *     that the response contains one of its name, and report that name
 *     through `br_ssl_engine_get_selected_protocol()`.
 *
 *   - The server will parse incoming ALPN extension (from clients), and
 *     try to find a common protocol; if none is found, the connection
 *     is aborted with a fatal alert. On match, a response ALPN extension
 *     is sent, and name is reported through
 *     `br_ssl_engine_get_selected_protocol()`.
 *
 * The provided array is linked in, and must remain valid while the
 * connection is live.
 *
 * Names MUST NOT be empty. Names MUST NOT be longer than 255 characters
 * (excluding the terminating 0).
 *
 * \param ctx     SSL engine context.
 * \param names   list of protocol names (zero-terminated).
 * \param num     number of protocol names (MUST be 1 or more).
 */
static inline void
br_ssl_engine_set_protocol_names(br_ssl_engine_context *ctx,
	const char **names, size_t num)
{
	ctx->protocol_names = names;
	ctx->protocol_names_num = num;
}

/**
 * \brief Get the selected protocol.
 *
 * If this context was initialised with a non-empty list of protocol
 * names, and both client and server sent ALPN extensions during the
 * handshake, and a common name was found, then that name is returned.
 * Otherwise, `NULL` is returned.
 *
 * The returned pointer is one of the pointers provided to the context
 * with `br_ssl_engine_set_protocol_names()`.
 *
 * \return  the selected protocol, or `NULL`.
 */
static inline const char *
br_ssl_engine_get_selected_protocol(br_ssl_engine_context *ctx)
{
	unsigned k;

	k = ctx->selected_protocol;
	return (k == 0 || k == 0xFFFF) ? NULL : ctx->protocol_names[k - 1];
}

/**
 * \brief Set a hash function implementation (by ID).
 *
 * Hash functions set with this call will be used for SSL/TLS specific
 * usages, not X.509 certificate validation. Only "standard" hash functions
 * may be set (MD5, SHA-1, SHA-224, SHA-256, SHA-384, SHA-512). If `impl`
 * is `NULL`, then the hash function support is removed, not added.
 *
 * \param ctx    SSL engine context.
 * \param id     hash function identifier.
 * \param impl   hash function implementation (or `NULL`).
 */
static inline void
br_ssl_engine_set_hash(br_ssl_engine_context *ctx,
	int id, const br_hash_class *impl)
{
	br_multihash_setimpl(&ctx->mhash, id, impl);
}

/**
 * \brief Get a hash function implementation (by ID).
 *
 * This function retrieves a hash function implementation which was
 * set with `br_ssl_engine_set_hash()`.
 *
 * \param ctx   SSL engine context.
 * \param id    hash function identifier.
 * \return  the hash function implementation (or `NULL`).
 */
static inline const br_hash_class *
br_ssl_engine_get_hash(br_ssl_engine_context *ctx, int id)
{
	return br_multihash_getimpl(&ctx->mhash, id);
}

/**
 * \brief Set the PRF implementation (for TLS 1.0 and 1.1).
 *
 * This function sets (or removes, if `impl` is `NULL`) the implemenation
 * for the PRF used in TLS 1.0 and 1.1.
 *
 * \param cc     SSL engine context.
 * \param impl   PRF implementation (or `NULL`).
 */
static inline void
br_ssl_engine_set_prf10(br_ssl_engine_context *cc, br_tls_prf_impl impl)
{
	cc->prf10 = impl;
}

/**
 * \brief Set the PRF implementation with SHA-256 (for TLS 1.2).
 *
 * This function sets (or removes, if `impl` is `NULL`) the implemenation
 * for the SHA-256 variant of the PRF used in TLS 1.2.
 *
 * \param cc     SSL engine context.
 * \param impl   PRF implementation (or `NULL`).
 */
static inline void
br_ssl_engine_set_prf_sha256(br_ssl_engine_context *cc, br_tls_prf_impl impl)
{
	cc->prf_sha256 = impl;
}

/**
 * \brief Set the PRF implementation with SHA-384 (for TLS 1.2).
 *
 * This function sets (or removes, if `impl` is `NULL`) the implemenation
 * for the SHA-384 variant of the PRF used in TLS 1.2.
 *
 * \param cc     SSL engine context.
 * \param impl   PRF implementation (or `NULL`).
 */
static inline void
br_ssl_engine_set_prf_sha384(br_ssl_engine_context *cc, br_tls_prf_impl impl)
{
	cc->prf_sha384 = impl;
}

/**
 * \brief Set the AES/CBC implementations.
 *
 * \param cc         SSL engine context.
 * \param impl_enc   AES/CBC encryption implementation (or `NULL`).
 * \param impl_dec   AES/CBC decryption implementation (or `NULL`).
 */
static inline void
br_ssl_engine_set_aes_cbc(br_ssl_engine_context *cc,
	const br_block_cbcenc_class *impl_enc,
	const br_block_cbcdec_class *impl_dec)
{
	cc->iaes_cbcenc = impl_enc;
	cc->iaes_cbcdec = impl_dec;
}

/**
 * \brief Set the AES/CTR implementation.
 *
 * \param cc     SSL engine context.
 * \param impl   AES/CTR encryption/decryption implementation (or `NULL`).
 */
static inline void
br_ssl_engine_set_aes_ctr(br_ssl_engine_context *cc,
	const br_block_ctr_class *impl)
{
	cc->iaes_ctr = impl;
}

/**
 * \brief Set the DES/CBC implementations.
 *
 * \param cc         SSL engine context.
 * \param impl_enc   DES/CBC encryption implementation (or `NULL`).
 * \param impl_dec   DES/CBC decryption implementation (or `NULL`).
 */
static inline void
br_ssl_engine_set_des_cbc(br_ssl_engine_context *cc,
	const br_block_cbcenc_class *impl_enc,
	const br_block_cbcdec_class *impl_dec)
{
	cc->ides_cbcenc = impl_enc;
	cc->ides_cbcdec = impl_dec;
}

/**
 * \brief Set the GHASH implementation (used in GCM mode).
 *
 * \param cc     SSL engine context.
 * \param impl   GHASH implementation (or `NULL`).
 */
static inline void
br_ssl_engine_set_ghash(br_ssl_engine_context *cc, br_ghash impl)
{
	cc->ighash = impl;
}

/**
 * \brief Set the ChaCha20 implementation.
 *
 * \param cc        SSL engine context.
 * \param ichacha   ChaCha20 implementation (or `NULL`).
 */
static inline void
br_ssl_engine_set_chacha20(br_ssl_engine_context *cc,
	br_chacha20_run ichacha)
{
	cc->ichacha = ichacha;
}

/**
 * \brief Set the Poly1305 implementation.
 *
 * \param cc      SSL engine context.
 * \param ipoly   Poly1305 implementation (or `NULL`).
 */
static inline void
br_ssl_engine_set_poly1305(br_ssl_engine_context *cc,
	br_poly1305_run ipoly)
{
	cc->ipoly = ipoly;
}

/**
 * \brief Set the record encryption and decryption engines for CBC + HMAC.
 *
 * \param cc         SSL engine context.
 * \param impl_in    record CBC decryption implementation (or `NULL`).
 * \param impl_out   record CBC encryption implementation (or `NULL`).
 */
static inline void
br_ssl_engine_set_cbc(br_ssl_engine_context *cc,
	const br_sslrec_in_cbc_class *impl_in,
	const br_sslrec_out_cbc_class *impl_out)
{
	cc->icbc_in = impl_in;
	cc->icbc_out = impl_out;
}

/**
 * \brief Set the record encryption and decryption engines for GCM.
 *
 * \param cc         SSL engine context.
 * \param impl_in    record GCM decryption implementation (or `NULL`).
 * \param impl_out   record GCM encryption implementation (or `NULL`).
 */
static inline void
br_ssl_engine_set_gcm(br_ssl_engine_context *cc,
	const br_sslrec_in_gcm_class *impl_in,
	const br_sslrec_out_gcm_class *impl_out)
{
	cc->igcm_in = impl_in;
	cc->igcm_out = impl_out;
}

/**
 * \brief Set the record encryption and decryption engines for
 * ChaCha20+Poly1305.
 *
 * \param cc         SSL engine context.
 * \param impl_in    record ChaCha20 decryption implementation (or `NULL`).
 * \param impl_out   record ChaCha20 encryption implementation (or `NULL`).
 */
static inline void
br_ssl_engine_set_chapol(br_ssl_engine_context *cc,
	const br_sslrec_in_chapol_class *impl_in,
	const br_sslrec_out_chapol_class *impl_out)
{
	cc->ichapol_in = impl_in;
	cc->ichapol_out = impl_out;
}

/**
 * \brief Set the EC implementation.
 *
 * The elliptic curve implementation will be used for ECDH and ECDHE
 * cipher suites, and for ECDSA support.
 *
 * \param cc    SSL engine context.
 * \param iec   EC implementation (or `NULL`).
 */
static inline void
br_ssl_engine_set_ec(br_ssl_engine_context *cc, const br_ec_impl *iec)
{
	cc->iec = iec;
}

/**
 * \brief Set the RSA signature verification implementation.
 *
 * On the client, this is used to verify the server's signature on its
 * ServerKeyExchange message (for ECDHE_RSA cipher suites). On the server,
 * this is used to verify the client's CertificateVerify message (if a
 * client certificate is requested, and that certificate contains a RSA key).
 *
 * \param cc         SSL engine context.
 * \param irsavrfy   RSA signature verification implementation.
 */
static inline void
br_ssl_engine_set_rsavrfy(br_ssl_engine_context *cc, br_rsa_pkcs1_vrfy irsavrfy)
{
	cc->irsavrfy = irsavrfy;
}

/*
 * \brief Set the ECDSA implementation (signature verification).
 *
 * On the client, this is used to verify the server's signature on its
 * ServerKeyExchange message (for ECDHE_ECDSA cipher suites). On the server,
 * this is used to verify the client's CertificateVerify message (if a
 * client certificate is requested, that certificate contains an EC key,
 * and full-static ECDH is not used).
 *
 * The ECDSA implementation will use the EC core implementation configured
 * in the engine context.
 *
 * \param cc       client context.
 * \param iecdsa   ECDSA verification implementation.
 */
static inline void
br_ssl_engine_set_ecdsa(br_ssl_engine_context *cc, br_ecdsa_vrfy iecdsa)
{
	cc->iecdsa = iecdsa;
}

/**
 * \brief Set the I/O buffer for the SSL engine.
 *
 * Once this call has been made, `br_ssl_client_reset()` or
 * `br_ssl_server_reset()` MUST be called before using the context.
 *
 * The provided buffer will be used as long as the engine context is
 * used. The caller is responsible for keeping it available.
 *
 * If `bidi` is 0, then the engine will operate in half-duplex mode
 * (it won't be able to send data while there is unprocessed incoming
 * data in the buffer, and it won't be able to receive data while there
 * is unsent data in the buffer). The optimal buffer size in half-duplex
 * mode is `BR_SSL_BUFSIZE_MONO`; if the buffer is larger, then extra
 * bytes are ignored. If the buffer is smaller, then this limits the
 * capacity of the engine to support all allowed record sizes.
 *
 * If `bidi` is 1, then the engine will split the buffer into two
 * parts, for separate handling of outgoing and incoming data. This
 * enables full-duplex processing, but requires more RAM. The optimal
 * buffer size in full-duplex mode is `BR_SSL_BUFSIZE_BIDI`; if the
 * buffer is larger, then extra bytes are ignored. If the buffer is
 * smaller, then the split will favour the incoming part, so that
 * interoperability is maximised.
 *
 * \param cc          SSL engine context
 * \param iobuf       I/O buffer.
 * \param iobuf_len   I/O buffer length (in bytes).
 * \param bidi        non-zero for full-duplex mode.
 */
void br_ssl_engine_set_buffer(br_ssl_engine_context *cc,
	void *iobuf, size_t iobuf_len, int bidi);

/**
 * \brief Set the I/O buffers for the SSL engine.
 *
 * Once this call has been made, `br_ssl_client_reset()` or
 * `br_ssl_server_reset()` MUST be called before using the context.
 *
 * This function is similar to `br_ssl_engine_set_buffer()`, except
 * that it enforces full-duplex mode, and the two I/O buffers are
 * provided as separate chunks.
 *
 * The macros `BR_SSL_BUFSIZE_INPUT` and `BR_SSL_BUFSIZE_OUTPUT`
 * evaluate to the optimal (maximum) sizes for the input and output
 * buffer, respectively.
 *
 * \param cc         SSL engine context
 * \param ibuf       input buffer.
 * \param ibuf_len   input buffer length (in bytes).
 * \param obuf       output buffer.
 * \param obuf_len   output buffer length (in bytes).
 */
void br_ssl_engine_set_buffers_bidi(br_ssl_engine_context *cc,
	void *ibuf, size_t ibuf_len, void *obuf, size_t obuf_len);

/**
 * \brief Inject some "initial entropy" in the context.
 *
 * This entropy will be added to what can be obtained from the
 * underlying operating system, if that OS is supported.
 *
 * This function may be called several times; all injected entropy chunks
 * are cumulatively mixed.
 *
 * If entropy gathering from the OS is supported and compiled in, then this
 * step is optional. Otherwise, it is mandatory to inject randomness, and
 * the caller MUST take care to push (as one or several successive calls)
 * enough entropy to achieve cryptographic resistance (at least 80 bits,
 * preferably 128 or more). The engine will report an error if no entropy
 * was provided and none can be obtained from the OS.
 *
 * Take care that this function cannot assess the cryptographic quality of
 * the provided bytes.
 *
 * In all generality, "entropy" must here be considered to mean "that
 * which the attacker cannot predict". If your OS/architecture does not
 * have a suitable source of randomness, then you can make do with the
 * combination of a large enough secret value (possibly a copy of an
 * asymmetric private key that you also store on the system) AND a
 * non-repeating value (e.g. current time, provided that the local clock
 * cannot be reset or altered by the attacker).
 *
 * \param cc     SSL engine context.
 * \param data   extra entropy to inject.
 * \param len    length of the extra data (in bytes).
 */
void br_ssl_engine_inject_entropy(br_ssl_engine_context *cc,
	const void *data, size_t len);

/**
 * \brief Get the "server name" in this engine.
 *
 * For clients, this is the name provided with `br_ssl_client_reset()`;
 * for servers, this is the name received from the client as part of the
 * ClientHello message. If there is no such name (e.g. the client did
 * not send an SNI extension) then the returned string is empty
 * (returned pointer points to a byte of value 0).
 *
 * The returned pointer refers to a buffer inside the context, which may
 * be overwritten as part of normal SSL activity (even within the same
 * connection, if a renegotiation occurs).
 *
 * \param cc   SSL engine context.
 * \return  the server name (possibly empty).
 */
static inline const char *
br_ssl_engine_get_server_name(const br_ssl_engine_context *cc)
{
	return cc->server_name;
}

/**
 * \brief Get the protocol version.
 *
 * This function returns the protocol version that is used by the
 * engine. That value is set after sending (for a server) or receiving
 * (for a client) the ServerHello message.
 *
 * \param cc   SSL engine context.
 * \return  the protocol version.
 */
static inline unsigned
br_ssl_engine_get_version(const br_ssl_engine_context *cc)
{
	return cc->session.version;
}

/**
 * \brief Get a copy of the session parameters.
 *
 * The session parameters are filled during the handshake, so this
 * function shall not be called before completion of the handshake.
 * The initial handshake is completed when the context first allows
 * application data to be injected.
 *
 * This function copies the current session parameters into the provided
 * structure. Beware that the session parameters include the master
 * secret, which is sensitive data, to handle with great care.
 *
 * \param cc   SSL engine context.
 * \param pp   destination structure for the session parameters.
 */
static inline void
br_ssl_engine_get_session_parameters(const br_ssl_engine_context *cc,
	br_ssl_session_parameters *pp)
{
	memcpy(pp, &cc->session, sizeof *pp);
}

/**
 * \brief Set the session parameters to the provided values.
 *
 * This function is meant to be used in the client, before doing a new
 * handshake; a session resumption will be attempted with these
 * parameters. In the server, this function has no effect.
 *
 * \param cc   SSL engine context.
 * \param pp   source structure for the session parameters.
 */
static inline void
br_ssl_engine_set_session_parameters(br_ssl_engine_context *cc,
	const br_ssl_session_parameters *pp)
{
	memcpy(&cc->session, pp, sizeof *pp);
}

/**
 * \brief Get the current engine state.
 *
 * An SSL engine (client or server) has, at any time, a state which is
 * the combination of zero, one or more of these flags:
 *
 *   - `BR_SSL_CLOSED`
 *
 *     Engine is finished, no more I/O (until next reset).
 *
 *   - `BR_SSL_SENDREC`
 *
 *     Engine has some bytes to send to the peer.
 *
 *   - `BR_SSL_RECVREC`
 *
 *     Engine expects some bytes from the peer.
 *
 *   - `BR_SSL_SENDAPP`
 *
 *     Engine may receive application data to send (or flush).
 *
 *   - `BR_SSL_RECVAPP`
 *
 *     Engine has obtained some application data from the peer,
 *     that should be read by the caller.
 *
 * If no flag at all is set (state value is 0), then the engine is not
 * fully initialised yet.
 *
 * The `BR_SSL_CLOSED` flag is exclusive; when it is set, no other flag
 * is set. To distinguish between a normal closure and an error, use
 * `br_ssl_engine_last_error()`.
 *
 * Generally speaking, `BR_SSL_SENDREC` and `BR_SSL_SENDAPP` are mutually
 * exclusive: the input buffer, at any point, either accumulates
 * plaintext data, or contains an assembled record that is being sent.
 * Similarly, `BR_SSL_RECVREC` and `BR_SSL_RECVAPP` are mutually exclusive.
 * This may change in a future library version.
 *
 * \param cc   SSL engine context.
 * \return  the current engine state.
 */
unsigned br_ssl_engine_current_state(const br_ssl_engine_context *cc);

/** \brief SSL engine state: closed or failed. */
#define BR_SSL_CLOSED    0x0001
/** \brief SSL engine state: record data is ready to be sent to the peer. */
#define BR_SSL_SENDREC   0x0002
/** \brief SSL engine state: engine may receive records from the peer. */
#define BR_SSL_RECVREC   0x0004
/** \brief SSL engine state: engine may accept application data to send. */
#define BR_SSL_SENDAPP   0x0008
/** \brief SSL engine state: engine has received application data. */
#define BR_SSL_RECVAPP   0x0010

/**
 * \brief Get the engine error indicator.
 *
 * The error indicator is `BR_ERR_OK` (0) if no error was encountered
 * since the last call to `br_ssl_client_reset()` or
 * `br_ssl_server_reset()`. Other status values are "sticky": they
 * remain set, and prevent all I/O activity, until cleared. Only the
 * reset calls clear the error indicator.
 *
 * \param cc   SSL engine context.
 * \return  0, or a non-zero error code.
 */
static inline int
br_ssl_engine_last_error(const br_ssl_engine_context *cc)
{
	return cc->err;
}

/*
 * There are four I/O operations, each identified by a symbolic name:
 *
 *   sendapp   inject application data in the engine
 *   recvapp   retrieving application data from the engine
 *   sendrec   sending records on the transport medium
 *   recvrec   receiving records from the transport medium
 *
 * Terminology works thus: in a layered model where the SSL engine sits
 * between the application and the network, "send" designates operations
 * where bytes flow from application to network, and "recv" for the
 * reverse operation. Application data (the plaintext that is to be
 * conveyed through SSL) is "app", while encrypted records are "rec".
 * Note that from the SSL engine point of view, "sendapp" and "recvrec"
 * designate bytes that enter the engine ("inject" operation), while
 * "recvapp" and "sendrec" designate bytes that exit the engine
 * ("extract" operation).
 *
 * For the operation 'xxx', two functions are defined:
 *
 *   br_ssl_engine_xxx_buf
 *      Returns a pointer and length to the buffer to use for that
 *      operation. '*len' is set to the number of bytes that may be read
 *      from the buffer (extract operation) or written to the buffer
 *      (inject operation). If no byte may be exchanged for that operation
 *      at that point, then '*len' is set to zero, and NULL is returned.
 *      The engine state is unmodified by this call.
 *
 *   br_ssl_engine_xxx_ack
 *      Informs the engine that 'len' bytes have been read from the buffer
 *      (extract operation) or written to the buffer (inject operation).
 *      The 'len' value MUST NOT be zero. The 'len' value MUST NOT exceed
 *      that which was obtained from a preceeding br_ssl_engine_xxx_buf()
 *      call.
 */

/**
 * \brief Get buffer for application data to send.
 *
 * If the engine is ready to accept application data to send to the
 * peer, then this call returns a pointer to the buffer where such
 * data shall be written, and its length is written in `*len`.
 * Otherwise, `*len` is set to 0 and `NULL` is returned.
 *
 * \param cc    SSL engine context.
 * \param len   receives the application data output buffer length, or 0.
 * \return  the application data output buffer, or `NULL`.
 */
unsigned char *br_ssl_engine_sendapp_buf(
	const br_ssl_engine_context *cc, size_t *len);

/**
 * \brief Inform the engine of some new application data.
 *
 * After writing `len` bytes in the buffer returned by
 * `br_ssl_engine_sendapp_buf()`, the application shall call this
 * function to trigger any relevant processing. The `len` parameter
 * MUST NOT be 0, and MUST NOT exceed the value obtained in the
 * `br_ssl_engine_sendapp_buf()` call.
 *
 * \param cc    SSL engine context.
 * \param len   number of bytes pushed (not zero).
 */
void br_ssl_engine_sendapp_ack(br_ssl_engine_context *cc, size_t len);

/**
 * \brief Get buffer for received application data.
 *
 * If the engine has received application data from the peer, hen this
 * call returns a pointer to the buffer from where such data shall be
 * read, and its length is written in `*len`. Otherwise, `*len` is set
 * to 0 and `NULL` is returned.
 *
 * \param cc    SSL engine context.
 * \param len   receives the application data input buffer length, or 0.
 * \return  the application data input buffer, or `NULL`.
 */
unsigned char *br_ssl_engine_recvapp_buf(
	const br_ssl_engine_context *cc, size_t *len);

/**
 * \brief Acknowledge some received application data.
 *
 * After reading `len` bytes from the buffer returned by
 * `br_ssl_engine_recvapp_buf()`, the application shall call this
 * function to trigger any relevant processing. The `len` parameter
 * MUST NOT be 0, and MUST NOT exceed the value obtained in the
 * `br_ssl_engine_recvapp_buf()` call.
 *
 * \param cc    SSL engine context.
 * \param len   number of bytes read (not zero).
 */
void br_ssl_engine_recvapp_ack(br_ssl_engine_context *cc, size_t len);

/**
 * \brief Get buffer for record data to send.
 *
 * If the engine has prepared some records to send to the peer, then this
 * call returns a pointer to the buffer from where such data shall be
 * read, and its length is written in `*len`. Otherwise, `*len` is set
 * to 0 and `NULL` is returned.
 *
 * \param cc    SSL engine context.
 * \param len   receives the record data output buffer length, or 0.
 * \return  the record data output buffer, or `NULL`.
 */
unsigned char *br_ssl_engine_sendrec_buf(
	const br_ssl_engine_context *cc, size_t *len);

/**
 * \brief Acknowledge some sent record data.
 *
 * After reading `len` bytes from the buffer returned by
 * `br_ssl_engine_sendrec_buf()`, the application shall call this
 * function to trigger any relevant processing. The `len` parameter
 * MUST NOT be 0, and MUST NOT exceed the value obtained in the
 * `br_ssl_engine_sendrec_buf()` call.
 *
 * \param cc    SSL engine context.
 * \param len   number of bytes read (not zero).
 */
void br_ssl_engine_sendrec_ack(br_ssl_engine_context *cc, size_t len);

/**
 * \brief Get buffer for incoming records.
 *
 * If the engine is ready to accept records from the peer, then this
 * call returns a pointer to the buffer where such data shall be
 * written, and its length is written in `*len`. Otherwise, `*len` is
 * set to 0 and `NULL` is returned.
 *
 * \param cc    SSL engine context.
 * \param len   receives the record data input buffer length, or 0.
 * \return  the record data input buffer, or `NULL`.
 */
unsigned char *br_ssl_engine_recvrec_buf(
	const br_ssl_engine_context *cc, size_t *len);

/**
 * \brief Inform the engine of some new record data.
 *
 * After writing `len` bytes in the buffer returned by
 * `br_ssl_engine_recvrec_buf()`, the application shall call this
 * function to trigger any relevant processing. The `len` parameter
 * MUST NOT be 0, and MUST NOT exceed the value obtained in the
 * `br_ssl_engine_recvrec_buf()` call.
 *
 * \param cc    SSL engine context.
 * \param len   number of bytes pushed (not zero).
 */
void br_ssl_engine_recvrec_ack(br_ssl_engine_context *cc, size_t len);

/**
 * \brief Flush buffered application data.
 *
 * If some application data has been buffered in the engine, then wrap
 * it into a record and mark it for sending. If no application data has
 * been buffered but the engine would be ready to accept some, AND the
 * `force` parameter is non-zero, then an empty record is assembled and
 * marked for sending. In all other cases, this function does nothing.
 *
 * Empty records are technically legal, but not all existing SSL/TLS
 * implementations support them. Empty records can be useful as a
 * transparent "keep-alive" mechanism to maintain some low-level
 * network activity.
 *
 * \param cc      SSL engine context.
 * \param force   non-zero to force sending an empty record.
 */
void br_ssl_engine_flush(br_ssl_engine_context *cc, int force);

/**
 * \brief Initiate a closure.
 *
 * If, at that point, the context is open and in ready state, then a
 * `close_notify` alert is assembled and marked for sending; this
 * triggers the closure protocol. Otherwise, no such alert is assembled.
 *
 * \param cc   SSL engine context.
 */
void br_ssl_engine_close(br_ssl_engine_context *cc);

/**
 * \brief Initiate a renegotiation.
 *
 * If the engine is failed or closed, or if the peer is known not to
 * support secure renegotiation (RFC 5746), or if renegotiations have
 * been disabled with the `BR_OPT_NO_RENEGOTIATION` flag, then this
 * function returns 0 and nothing else happens.
 *
 * Otherwise, this function returns 1, and a renegotiation attempt is
 * triggered (if a handshake is already ongoing at that point, then
 * no new handshake is triggered).
 *
 * \param cc   SSL engine context.
 * \return  1 on success, 0 on error.
 */
int br_ssl_engine_renegotiate(br_ssl_engine_context *cc);

/*
 * Pre-declaration for the SSL client context.
 */
typedef struct br_ssl_client_context_ br_ssl_client_context;

/**
 * \brief Type for the client certificate, if requested by the server.
 */
typedef struct {
	/**
	 * \brief Authentication type.
	 *
	 * This is either `BR_AUTH_RSA` (RSA signature), `BR_AUTH_ECDSA`
	 * (ECDSA signature), or `BR_AUTH_ECDH` (static ECDH key exchange).
	 */
	int auth_type;

	/**
	 * \brief Hash function for computing the CertificateVerify.
	 *
	 * This is the symbolic identifier for the hash function that
	 * will be used to produce the hash of handshake messages, to
	 * be signed into the CertificateVerify. For full static ECDH
	 * (client and server certificates are both EC in the same
	 * curve, and static ECDH is used), this value is set to -1.
	 *
	 * Take care that with TLS 1.0 and 1.1, that value MUST match
	 * the protocol requirements: value must be 0 (MD5+SHA-1) for
	 * a RSA signature, or 2 (SHA-1) for an ECDSA signature. Only
	 * TLS 1.2 allows for other hash functions.
	 */
	int hash_id;

	/**
	 * \brief Certificate chain to send to the server.
	 *
	 * This is an array of `br_x509_certificate` objects, each
	 * normally containing a DER-encoded certificate. The client
	 * code does not try to decode these elements. If there is no
	 * chain to send to the server, then this pointer shall be
	 * set to `NULL`.
	 */
	const br_x509_certificate *chain;

	/**
	 * \brief Certificate chain length (number of certificates).
	 *
	 * If there is no chain to send to the server, then this value
	 * shall be set to 0.
	 */
	size_t chain_len;

} br_ssl_client_certificate;

/*
 * Note: the constants below for signatures match the TLS constants.
 */

/** \brief Client authentication type: static ECDH. */
#define BR_AUTH_ECDH    0
/** \brief Client authentication type: RSA signature. */
#define BR_AUTH_RSA     1
/** \brief Client authentication type: ECDSA signature. */
#define BR_AUTH_ECDSA   3

/**
 * \brief Class type for a certificate handler (client side).
 *
 * A certificate handler selects a client certificate chain to send to
 * the server, upon explicit request from that server. It receives
 * the list of trust anchor DN from the server, and supported types
 * of certificates and signatures, and returns the chain to use. It
 * is also invoked to perform the corresponding private key operation
 * (a signature, or an ECDH computation).
 *
 * The SSL client engine will first push the trust anchor DN with
 * `start_name_list()`, `start_name()`, `append_name()`, `end_name()`
 * and `end_name_list()`. Then it will call `choose()`, to select the
 * actual chain (and signature/hash algorithms). Finally, it will call
 * either `do_sign()` or `do_keyx()`, depending on the algorithm choices.
 */
typedef struct br_ssl_client_certificate_class_ br_ssl_client_certificate_class;
struct br_ssl_client_certificate_class_ {
	/**
	 * \brief Context size (in bytes).
	 */
	size_t context_size;

	/**
	 * \brief Begin reception of a list of trust anchor names. This
	 * is called while parsing the incoming CertificateRequest.
	 *
	 * \param pctx   certificate handler context.
	 */
	void (*start_name_list)(const br_ssl_client_certificate_class **pctx);

	/**
	 * \brief Begin reception of a new trust anchor name.
	 *
	 * The total encoded name length is provided; it is less than
	 * 65535 bytes.
	 *
	 * \param pctx   certificate handler context.
	 * \param len    encoded name length (in bytes).
	 */
	void (*start_name)(const br_ssl_client_certificate_class **pctx,
		size_t len);

	/**
	 * \brief Receive some more bytes for the current trust anchor name.
	 *
	 * The provided reference (`data`) points to a transient buffer
	 * they may be reused as soon as this function returns. The chunk
	 * length (`len`) is never zero.
	 *
	 * \param pctx   certificate handler context.
	 * \param data   anchor name chunk.
	 * \param len    anchor name chunk length (in bytes).
	 */
	void (*append_name)(const br_ssl_client_certificate_class **pctx,
		const unsigned char *data, size_t len);

	/**
	 * \brief End current trust anchor name.
	 *
	 * This function is called when all the encoded anchor name data
	 * has been provided.
	 *
	 * \param pctx   certificate handler context.
	 */
	void (*end_name)(const br_ssl_client_certificate_class **pctx);

	/**
	 * \brief End list of trust anchor names.
	 *
	 * This function is called when all the anchor names in the
	 * CertificateRequest message have been obtained.
	 *
	 * \param pctx   certificate handler context.
	 */
	void (*end_name_list)(const br_ssl_client_certificate_class **pctx);

	/**
	 * \brief Select client certificate and algorithms.
	 *
	 * This callback function shall fill the provided `choices`
	 * structure with the selected algorithms and certificate chain.
	 * The `hash_id`, `chain` and `chain_len` fields must be set. If
	 * the client cannot or does not wish to send a certificate,
	 * then it shall set `chain` to `NULL` and `chain_len` to 0.
	 *
	 * The `auth_types` parameter describes the authentication types,
	 * signature algorithms and hash functions that are supported by
	 * both the client context and the server, and compatible with
	 * the current protocol version. This is a bit field with the
	 * following contents:
	 *
	 *   - If RSA signatures with hash function x are supported, then
	 *     bit x is set.
	 *
	 *   - If ECDSA signatures with hash function x are supported,
	 *     then bit 8+x is set.
	 *
	 *   - If static ECDH is supported, with a RSA-signed certificate,
	 *     then bit 16 is set.
	 *
	 *   - If static ECDH is supported, with an ECDSA-signed certificate,
	 *     then bit 17 is set.
	 *
	 * Notes:
	 *
	 *   - When using TLS 1.0 or 1.1, the hash function for RSA
	 *     signatures is always the special MD5+SHA-1 (id 0), and the
	 *     hash function for ECDSA signatures is always SHA-1 (id 2).
	 *
	 *   - When using TLS 1.2, the list of hash functions is trimmed
	 *     down to include only hash functions that the client context
	 *     can support. The actual server list can be obtained with
	 *     `br_ssl_client_get_server_hashes()`; that list may be used
	 *     to select the certificate chain to send to the server.
	 *
	 * \param pctx         certificate handler context.
	 * \param cc           SSL client context.
	 * \param auth_types   supported authentication types and algorithms.
	 * \param choices      destination structure for the policy choices.
	 */
	void (*choose)(const br_ssl_client_certificate_class **pctx,
		const br_ssl_client_context *cc, uint32_t auth_types,
		br_ssl_client_certificate *choices);

	/**
	 * \brief Perform key exchange (client part).
	 *
	 * This callback is invoked in case of a full static ECDH key
	 * exchange:
	 *
	 *   - the cipher suite uses `ECDH_RSA` or `ECDH_ECDSA`;
	 *
	 *   - the server requests a client certificate;
	 *
	 *   - the client has, and sends, a client certificate that
	 *     uses an EC key in the same curve as the server's key,
	 *     and chooses static ECDH (the `hash_id` field in the choice
	 *     structure was set to -1).
	 *
	 * In that situation, this callback is invoked to compute the
	 * client-side ECDH: the provided `data` (of length `len` bytes)
	 * is the server's public key point (as decoded from its
	 * certificate), and the client shall multiply that point with
	 * its own private key, and write back the X coordinate of the
	 * resulting point in the same buffer, starting at offset 1
	 * (therefore, writing back the complete encoded point works).
	 *
	 * The callback must uphold the following:
	 *
	 *   - If the input array does not have the proper length for
	 *     an encoded curve point, then an error (0) shall be reported.
	 *
	 *   - If the input array has the proper length, then processing
	 *     MUST be constant-time, even if the data is not a valid
	 *     encoded point.
	 *
	 *   - This callback MUST check that the input point is valid.
	 *
	 * Returned value is 1 on success, 0 on error.
	 *
	 * \param pctx   certificate handler context.
	 * \param data   server public key point.
	 * \param len    server public key point length (in bytes).
	 * \return  1 on success, 0 on error.
	 */
	uint32_t (*do_keyx)(const br_ssl_client_certificate_class **pctx,
		unsigned char *data, size_t len);

	/**
	 * \brief Perform a signature (client authentication).
	 *
	 * This callback is invoked when a client certificate was sent,
	 * and static ECDH is not used. It shall compute a signature,
	 * using the client's private key, over the provided hash value
	 * (which is the hash of all previous handshake messages).
	 *
	 * On input, the hash value to sign is in `data`, of size
	 * `hv_len`; the involved hash function is identified by
	 * `hash_id`. The signature shall be computed and written
	 * back into `data`; the total size of that buffer is `len`
	 * bytes.
	 *
	 * This callback shall verify that the signature length does not
	 * exceed `len` bytes, and abstain from writing the signature if
	 * it does not fit.
	 *
	 * For RSA signatures, the `hash_id` may be 0, in which case
	 * this is the special header-less signature specified in TLS 1.0
	 * and 1.1, with a 36-byte hash value. Otherwise, normal PKCS#1
	 * v1.5 signatures shall be computed.
	 *
	 * For ECDSA signatures, the signature value shall use the ASN.1
	 * based encoding.
	 *
	 * Returned value is the signature length (in bytes), or 0 on error.
	 *
	 * \param pctx      certificate handler context.
	 * \param hash_id   hash function identifier.
	 * \param hv_len    hash value length (in bytes).
	 * \param data      input/output buffer (hash value, then signature).
	 * \param len       total buffer length (in bytes).
	 * \return  signature length (in bytes) on success, or 0 on error.
	 */
	size_t (*do_sign)(const br_ssl_client_certificate_class **pctx,
		int hash_id, size_t hv_len, unsigned char *data, size_t len);
};

/**
 * \brief A single-chain RSA client certificate handler.
 *
 * This handler uses a single certificate chain, with a RSA
 * signature. The list of trust anchor DN is ignored.
 *
 * Apart from the first field (vtable pointer), its contents are
 * opaque and shall not be accessed directly.
 */
typedef struct {
	/** \brief Pointer to vtable. */
	const br_ssl_client_certificate_class *vtable;
#ifndef BR_DOXYGEN_IGNORE
	const br_x509_certificate *chain;
	size_t chain_len;
	const br_rsa_private_key *sk;
	br_rsa_pkcs1_sign irsasign;
#endif
} br_ssl_client_certificate_rsa_context;

/**
 * \brief A single-chain EC client certificate handler.
 *
 * This handler uses a single certificate chain, with a RSA
 * signature. The list of trust anchor DN is ignored.
 *
 * This handler may support both static ECDH, and ECDSA signatures
 * (either usage may be selectively disabled).
 *
 * Apart from the first field (vtable pointer), its contents are
 * opaque and shall not be accessed directly.
 */
typedef struct {
	/** \brief Pointer to vtable. */
	const br_ssl_client_certificate_class *vtable;
#ifndef BR_DOXYGEN_IGNORE
	const br_x509_certificate *chain;
	size_t chain_len;
	const br_ec_private_key *sk;
	unsigned allowed_usages;
	unsigned issuer_key_type;
	const br_multihash_context *mhash;
	const br_ec_impl *iec;
	br_ecdsa_sign iecdsa;
#endif
} br_ssl_client_certificate_ec_context;

/**
 * \brief Context structure for a SSL client.
 *
 * The first field (called `eng`) is the SSL engine; all functions that
 * work on a `br_ssl_engine_context` structure shall take as parameter
 * a pointer to that field. The other structure fields are opaque and
 * must not be accessed directly.
 */
struct br_ssl_client_context_ {
	/**
	 * \brief The encapsulated engine context.
	 */
	br_ssl_engine_context eng;

#ifndef BR_DOXYGEN_IGNORE
	/*
	 * Minimum ClientHello length; padding with an extension (RFC
	 * 7685) is added if necessary to match at least that length.
	 * Such padding is nominally unnecessary, but it has been used
	 * to work around some server implementation bugs.
	 */
	uint16_t min_clienthello_len;

	/*
	 * Bit field for algoithms (hash + signature) supported by the
	 * server when requesting a client certificate.
	 */
	uint16_t hashes;

	/*
	 * Server's public key curve.
	 */
	int server_curve;

	/*
	 * Context for certificate handler.
	 */
	const br_ssl_client_certificate_class **client_auth_vtable;

	/*
	 * Client authentication type.
	 */
	unsigned char auth_type;

	/*
	 * Hash function to use for the client signature. This is 0xFF
	 * if static ECDH is used.
	 */
	unsigned char hash_id;

	/*
	 * For the core certificate handlers, thus avoiding (in most
	 * cases) the need for an externally provided policy context.
	 */
	union {
		const br_ssl_client_certificate_class *vtable;
		br_ssl_client_certificate_rsa_context single_rsa;
		br_ssl_client_certificate_ec_context single_ec;
	} client_auth;

	/*
	 * Implementations.
	 */
	br_rsa_public irsapub;
#endif
};

/**
 * \brief Get the hash functions and signature algorithms supported by
 * the server.
 *
 * This is a field of bits: for hash function of ID x, bit x is set if
 * the hash function is supported in RSA signatures, 8+x if it is supported
 * with ECDSA. This information is conveyed by the server when requesting
 * a client certificate.
 *
 * \param cc   client context.
 * \return  the server-supported hash functions (for signatures).
 */
static inline uint16_t
br_ssl_client_get_server_hashes(const br_ssl_client_context *cc)
{
	return cc->hashes;
}

/**
 * \brief Get the server key curve.
 *
 * This function returns the ID for the curve used by the server's public
 * key. This is set when the server's certificate chain is processed;
 * this value is 0 if the server's key is not an EC key.
 *
 * \return  the server's public key curve ID, or 0.
 */
static inline int
br_ssl_client_get_server_curve(const br_ssl_client_context *cc)
{
	return cc->server_curve;
}

/*
 * Each br_ssl_client_init_xxx() function sets the list of supported
 * cipher suites and used implementations, as specified by the profile
 * name 'xxx'. Defined profile names are:
 *
 *    full    all supported versions and suites; constant-time implementations
 *    TODO: add other profiles
 */

/**
 * \brief SSL client profile: full.
 *
 * This function initialises the provided SSL client context with
 * all supported algorithms and cipher suites. It also initialises
 * a companion X.509 validation engine with all supported algorithms,
 * and the provided trust anchors; the X.509 engine will be used by
 * the client context to validate the server's certificate.
 *
 * \param cc                  client context to initialise.
 * \param xc                  X.509 validation context to initialise.
 * \param trust_anchors       trust anchors to use.
 * \param trust_anchors_num   number of trust anchors.
 */
void br_ssl_client_init_full(br_ssl_client_context *cc,
	br_x509_minimal_context *xc,
	const br_x509_trust_anchor *trust_anchors, size_t trust_anchors_num);

/**
 * \brief Clear the complete contents of a SSL client context.
 *
 * Everything is cleared, including the reference to the configured buffer,
 * implementations, cipher suites and state. This is a preparatory step
 * to assembling a custom profile.
 *
 * \param cc   client context to clear.
 */
void br_ssl_client_zero(br_ssl_client_context *cc);

/**
 * \brief Set an externally provided client certificate handler context.
 *
 * The handler's methods are invoked when the server requests a client
 * certificate.
 *
 * \param cc     client context.
 * \param pctx   certificate handler context (pointer to its vtable field).
 */
static inline void
br_ssl_client_set_client_certificate(br_ssl_client_context *cc,
	const br_ssl_client_certificate_class **pctx)
{
	cc->client_auth_vtable = pctx;
}

/**
 * \brief Set the RSA public-key operations implementation.
 *
 * This will be used to encrypt the pre-master secret with the server's
 * RSA public key (RSA-encryption cipher suites only).
 *
 * \param cc        client context.
 * \param irsapub   RSA public-key encryption implementation.
 */
static inline void
br_ssl_client_set_rsapub(br_ssl_client_context *cc, br_rsa_public irsapub)
{
	cc->irsapub = irsapub;
}

/**
 * \brief Set the minimum ClientHello length (RFC 7685 padding).
 *
 * If this value is set and the ClientHello would be shorter, then
 * the Pad ClientHello extension will be added with enough padding bytes
 * to reach the target size. Because of the extension header, the resulting
 * size will sometimes be slightly more than `len` bytes if the target
 * size cannot be exactly met.
 *
 * The target length relates to the _contents_ of the ClientHello, not
 * counting its 4-byte header. For instance, if `len` is set to 512,
 * then the padding will bring the ClientHello size to 516 bytes with its
 * header, and 521 bytes when counting the 5-byte record header.
 *
 * \param cc    client context.
 * \param len   minimum ClientHello length (in bytes).
 */
static inline void
br_ssl_client_set_min_clienthello_len(br_ssl_client_context *cc, uint16_t len)
{
	cc->min_clienthello_len = len;
}

/**
 * \brief Prepare or reset a client context for a new connection.
 *
 * The `server_name` parameter is used to fill the SNI extension; the
 * X.509 "minimal" engine will also match that name against the server
 * names included in the server's certificate. If the parameter is
 * `NULL` then no SNI extension will be sent, and the X.509 "minimal"
 * engine (if used for server certificate validation) will not check
 * presence of any specific name in the received certificate.
 *
 * Therefore, setting the `server_name` to `NULL` shall be reserved
 * to cases where alternate or additional methods are used to ascertain
 * that the right server public key is used (e.g. a "known key" model).
 *
 * If `resume_session` is non-zero and the context was previously used
 * then the session parameters may be reused (depending on whether the
 * server previously sent a non-empty session ID, and accepts the session
 * resumption). The session parameters for session resumption can also
 * be set explicitly with `br_ssl_engine_set_session_parameters()`.
 *
 * On failure, the context is marked as failed, and this function
 * returns 0. A possible failure condition is when no initial entropy
 * was injected, and none could be obtained from the OS (either OS
 * randomness gathering is not supported, or it failed).
 *
 * \param cc               client context.
 * \param server_name      target server name, or `NULL`.
 * \param resume_session   non-zero to try session resumption.
 * \return  0 on failure, 1 on success.
 */
int br_ssl_client_reset(br_ssl_client_context *cc,
	const char *server_name, int resume_session);

/**
 * \brief Forget any session in the context.
 *
 * This means that the next handshake that uses this context will
 * necessarily be a full handshake (this applies both to new connections
 * and to renegotiations).
 *
 * \param cc   client context.
 */
static inline void
br_ssl_client_forget_session(br_ssl_client_context *cc)
{
	cc->eng.session.session_id_len = 0;
}

/**
 * \brief Set client certificate chain and key (single RSA case).
 *
 * This function sets a client certificate chain, that the client will
 * send to the server whenever a client certificate is requested. This
 * certificate uses an RSA public key; the corresponding private key is
 * invoked for authentication. Trust anchor names sent by the server are
 * ignored.
 *
 * The provided chain and private key are linked in the client context;
 * they must remain valid as long as they may be used, i.e. normally
 * for the duration of the connection, since they might be invoked
 * again upon renegotiations.
 *
 * \param cc          SSL client context.
 * \param chain       client certificate chain (SSL order: EE comes first).
 * \param chain_len   client chain length (number of certificates).
 * \param sk          client private key.
 * \param irsasign    RSA signature implementation (PKCS#1 v1.5).
 */
void br_ssl_client_set_single_rsa(br_ssl_client_context *cc,
	const br_x509_certificate *chain, size_t chain_len,
	const br_rsa_private_key *sk, br_rsa_pkcs1_sign irsasign);

/*
 * \brief Set the client certificate chain and key (single EC case).
 *
 * This function sets a client certificate chain, that the client will
 * send to the server whenever a client certificate is requested. This
 * certificate uses an EC public key; the corresponding private key is
 * invoked for authentication. Trust anchor names sent by the server are
 * ignored.
 *
 * The provided chain and private key are linked in the client context;
 * they must remain valid as long as they may be used, i.e. normally
 * for the duration of the connection, since they might be invoked
 * again upon renegotiations.
 *
 * The `allowed_usages` is a combination of usages, namely
 * `BR_KEYTYPE_KEYX` and/or `BR_KEYTYPE_SIGN`. The `BR_KEYTYPE_KEYX`
 * value allows full static ECDH, while the `BR_KEYTYPE_SIGN` value
 * allows ECDSA signatures. If ECDSA signatures are used, then an ECDSA
 * signature implementation must be provided; otherwise, the `iecdsa`
 * parameter may be 0.
 *
 * The `cert_issuer_key_type` value is either `BR_KEYTYPE_RSA` or
 * `BR_KEYTYPE_EC`; it is the type of the public key used the the CA
 * that issued (signed) the client certificate. That value is used with
 * full static ECDH: support of the certificate by the server depends
 * on how the certificate was signed. (Note: when using TLS 1.2, this
 * parameter is ignored; but its value matters for TLS 1.0 and 1.1.)
 *
 * \param cc                     server context.
 * \param chain                  server certificate chain to send.
 * \param chain_len              chain length (number of certificates).
 * \param sk                     server private key (EC).
 * \param allowed_usages         allowed private key usages.
 * \param cert_issuer_key_type   issuing CA's key type.
 * \param iec                    EC core implementation.
 * \param iecdsa                 ECDSA signature implementation ("asn1" format).
 */
void br_ssl_client_set_single_ec(br_ssl_client_context *cc,
	const br_x509_certificate *chain, size_t chain_len,
	const br_ec_private_key *sk, unsigned allowed_usages,
	unsigned cert_issuer_key_type,
	const br_ec_impl *iec, br_ecdsa_sign iecdsa);

/**
 * \brief Type for a "translated cipher suite", as an array of two
 * 16-bit integers.
 *
 * The first element is the cipher suite identifier (as used on the wire).
 * The second element is the concatenation of four 4-bit elements which
 * characterise the cipher suite contents. In most to least significant
 * order, these 4-bit elements are:
 *
 *   - Bits 12 to 15: key exchange + server key type
 *
 *     | val | symbolic constant        | suite type  | details                                          |
 *     | :-- | :----------------------- | :---------- | :----------------------------------------------- |
 *     |  0  | `BR_SSLKEYX_RSA`         | RSA         | RSA key exchange, key is RSA (encryption)        |
 *     |  1  | `BR_SSLKEYX_ECDHE_RSA`   | ECDHE_RSA   | ECDHE key exchange, key is RSA (signature)       |
 *     |  2  | `BR_SSLKEYX_ECDHE_ECDSA` | ECDHE_ECDSA | ECDHE key exchange, key is EC (signature)        |
 *     |  3  | `BR_SSLKEYX_ECDH_RSA`    | ECDH_RSA    | Key is EC (key exchange), cert signed with RSA   |
 *     |  4  | `BR_SSLKEYX_ECDH_ECDSA`  | ECDH_ECDSA  | Key is EC (key exchange), cert signed with ECDSA |
 *
 *   - Bits 8 to 11: symmetric encryption algorithm
 *
 *     | val | symbolic constant      | symmetric encryption | key strength (bits) |
 *     | :-- | :--------------------- | :------------------- | :------------------ |
 *     |  0  | `BR_SSLENC_3DES_CBC`   | 3DES/CBC             | 168                 |
 *     |  1  | `BR_SSLENC_AES128_CBC` | AES-128/CBC          | 128                 |
 *     |  2  | `BR_SSLENC_AES256_CBC` | AES-256/CBC          | 256                 |
 *     |  3  | `BR_SSLENC_AES128_GCM` | AES-128/GCM          | 128                 |
 *     |  4  | `BR_SSLENC_AES256_GCM` | AES-256/GCM          | 256                 |
 *     |  5  | `BR_SSLENC_CHACHA20`   | ChaCha20/Poly1305    | 256                 |
 *
 *   - Bits 4 to 7: MAC algorithm
 *
 *     | val | symbolic constant  | MAC type     | details                               |
 *     | :-- | :----------------- | :----------- | :------------------------------------ |
 *     |  0  | `BR_SSLMAC_AEAD`   | AEAD         | No dedicated MAC (encryption is AEAD) |
 *     |  2  | `BR_SSLMAC_SHA1`   | HMAC/SHA-1   | Value matches `br_sha1_ID`            |
 *     |  4  | `BR_SSLMAC_SHA256` | HMAC/SHA-256 | Value matches `br_sha256_ID`          |
 *     |  5  | `BR_SSLMAC_SHA384` | HMAC/SHA-384 | Value matches `br_sha384_ID`          |
 *
 *   - Bits 0 to 3: hash function for PRF when used with TLS-1.2
 *
 *     | val | symbolic constant  | hash function | details                              |
 *     | :-- | :----------------- | :------------ | :----------------------------------- |
 *     |  4  | `BR_SSLPRF_SHA256` | SHA-256       | Value matches `br_sha256_ID`         |
 *     |  5  | `BR_SSLPRF_SHA384` | SHA-384       | Value matches `br_sha384_ID`         |
 *
 * For instance, cipher suite `TLS_RSA_WITH_AES_128_GCM_SHA256` has
 * standard identifier 0x009C, and is translated to 0x0304, for, in
 * that order: RSA key exchange (0), AES-128/GCM (3), AEAD integrity (0),
 * SHA-256 in the TLS PRF (4).
 */
typedef uint16_t br_suite_translated[2];

#ifndef BR_DOXYGEN_IGNORE
/*
 * Constants are already documented in the br_suite_translated type.
 */

#define BR_SSLKEYX_RSA           0
#define BR_SSLKEYX_ECDHE_RSA     1
#define BR_SSLKEYX_ECDHE_ECDSA   2
#define BR_SSLKEYX_ECDH_RSA      3
#define BR_SSLKEYX_ECDH_ECDSA    4

#define BR_SSLENC_3DES_CBC       0
#define BR_SSLENC_AES128_CBC     1
#define BR_SSLENC_AES256_CBC     2
#define BR_SSLENC_AES128_GCM     3
#define BR_SSLENC_AES256_GCM     4
#define BR_SSLENC_CHACHA20       5

#define BR_SSLMAC_AEAD           0
#define BR_SSLMAC_SHA1           br_sha1_ID
#define BR_SSLMAC_SHA256         br_sha256_ID
#define BR_SSLMAC_SHA384         br_sha384_ID

#define BR_SSLPRF_SHA256         br_sha256_ID
#define BR_SSLPRF_SHA384         br_sha384_ID

#endif

/*
 * Pre-declaration for the SSL server context.
 */
typedef struct br_ssl_server_context_ br_ssl_server_context;

/**
 * \brief Type for the server policy choices, taken after analysis of
 * the client message (ClientHello).
 */
typedef struct {
	/**
	 * \brief Cipher suite to use with that client.
	 */
	uint16_t cipher_suite;

	/**
	 * \brief Hash function for signing the ServerKeyExchange.
	 *
	 * This is the symbolic identifier for the hash function that
	 * will be used to sign the ServerKeyExchange message, for ECDHE
	 * cipher suites. This is ignored for RSA and ECDH cipher suites.
	 *
	 * Take care that with TLS 1.0 and 1.1, that value MUST match
	 * the protocol requirements: value must be 0 (MD5+SHA-1) for
	 * a RSA signature, or 2 (SHA-1) for an ECDSA signature. Only
	 * TLS 1.2 allows for other hash functions.
	 */
	int hash_id;

	/**
	 * \brief Certificate chain to send to the client.
	 *
	 * This is an array of `br_x509_certificate` objects, each
	 * normally containing a DER-encoded certificate. The server
	 * code does not try to decode these elements.
	 */
	const br_x509_certificate *chain;

	/**
	 * \brief Certificate chain length (number of certificates).
	 */
	size_t chain_len;

} br_ssl_server_choices;

/**
 * \brief Class type for a policy handler (server side).
 *
 * A policy handler selects the policy parameters for a connection
 * (cipher suite and other algorithms, and certificate chain to send to
 * the client); it also performs the server-side computations involving
 * its permanent private key.
 *
 * The SSL server engine will invoke first `choose()`, once the
 * ClientHello message has been received, then either `do_keyx()`
 * `do_sign()`, depending on the cipher suite.
 */
typedef struct br_ssl_server_policy_class_ br_ssl_server_policy_class;
struct br_ssl_server_policy_class_ {
	/**
	 * \brief Context size (in bytes).
	 */
	size_t context_size;

	/**
	 * \brief Select algorithms and certificates for this connection.
	 *
	 * This callback function shall fill the provided `choices`
	 * structure with the policy choices for this connection. This
	 * entails selecting the cipher suite, hash function for signing
	 * the ServerKeyExchange (applicable only to ECDHE cipher suites),
	 * and certificate chain to send.
	 *
	 * The callback receives a pointer to the server context that
	 * contains the relevant data. In particular, the functions
	 * `br_ssl_server_get_client_suites()`,
	 * `br_ssl_server_get_client_hashes()` and
	 * `br_ssl_server_get_client_curves()` can be used to obtain
	 * the cipher suites, hash functions and elliptic curves
	 * supported by both the client and server, respectively. The
	 * `br_ssl_engine_get_version()` and `br_ssl_engine_get_server_name()`
	 * functions yield the protocol version and requested server name
	 * (SNI), respectively.
	 *
	 * This function may modify its context structure (`pctx`) in
	 * arbitrary ways to keep track of its own choices.
	 *
	 * This function shall return 1 if appropriate policy choices
	 * could be made, or 0 if this connection cannot be pursued.
	 *
	 * \param pctx      policy context.
	 * \param cc        SSL server context.
	 * \param choices   destination structure for the policy choices.
	 * \return  1 on success, 0 on error.
	 */
	int (*choose)(const br_ssl_server_policy_class **pctx,
		const br_ssl_server_context *cc,
		br_ssl_server_choices *choices);

	/**
	 * \brief Perform key exchange (server part).
	 *
	 * This callback is invoked to perform the server-side cryptographic
	 * operation for a key exchange that is not ECDHE. This callback
	 * uses the private key.
	 *
	 * **For RSA key exchange**, the provided `data` (of length `len`
	 * bytes) shall be decrypted with the server's private key, and
	 * the 48-byte premaster secret copied back to the first 48 bytes
	 * of `data`.
	 *
	 *   - The caller makes sure that `len` is at least 59 bytes.
	 *
	 *   - This callback MUST check that the provided length matches
	 *     that of the key modulus; it shall report an error otherwise.
	 *
	 *   - If the length matches that of the RSA key modulus, then
	 *     processing MUST be constant-time, even if decryption fails,
	 *     or the padding is incorrect, or the plaintext message length
	 *     is not exactly 48 bytes.
	 *
	 *   - This callback needs not check the two first bytes of the
	 *     obtained pre-master secret (the caller will do that).
	 *
	 *   - If an error is reported (0), then what the callback put
	 *     in the first 48 bytes of `data` is unimportant (the caller
	 *     will use random bytes instead).
	 *
	 * **For ECDH key exchange**, the provided `data` (of length `len`
	 * bytes) is the elliptic curve point from the client. The
	 * callback shall multiply it with its private key, and store
	 * the resulting X coordinate in `data`, starting at offset 1
	 * (thus, simply encoding the point in compressed or uncompressed
	 * format in `data` is fine).
	 *
	 *   - If the input array does not have the proper length for
	 *     an encoded curve point, then an error (0) shall be reported.
	 *
	 *   - If the input array has the proper length, then processing
	 *     MUST be constant-time, even if the data is not a valid
	 *     encoded point.
	 *
	 *   - This callback MUST check that the input point is valid.
	 *
	 * Returned value is 1 on success, 0 on error.
	 *
	 * \param pctx   policy context.
	 * \param data   key exchange data from the client.
	 * \param len    key exchange data length (in bytes).
	 * \return  1 on success, 0 on error.
	 */
	uint32_t (*do_keyx)(const br_ssl_server_policy_class **pctx,
		unsigned char *data, size_t len);

	/**
	 * \brief Perform a signature (for a ServerKeyExchange message).
	 *
	 * This callback function is invoked for ECDHE cipher suites.
	 * On input, the hash value to sign is in `data`, of size
	 * `hv_len`; the involved hash function is identified by
	 * `hash_id`. The signature shall be computed and written
	 * back into `data`; the total size of that buffer is `len`
	 * bytes.
	 *
	 * This callback shall verify that the signature length does not
	 * exceed `len` bytes, and abstain from writing the signature if
	 * it does not fit.
	 *
	 * For RSA signatures, the `hash_id` may be 0, in which case
	 * this is the special header-less signature specified in TLS 1.0
	 * and 1.1, with a 36-byte hash value. Otherwise, normal PKCS#1
	 * v1.5 signatures shall be computed.
	 *
	 * Returned value is the signature length (in bytes), or 0 on error.
	 *
	 * \param pctx      policy context.
	 * \param hash_id   hash function identifier.
	 * \param hv_len    hash value length (in bytes).
	 * \param data      input/output buffer (hash value, then signature).
	 * \param len       total buffer length (in bytes).
	 * \return  signature length (in bytes) on success, or 0 on error.
	 */
	size_t (*do_sign)(const br_ssl_server_policy_class **pctx,
		int hash_id, size_t hv_len, unsigned char *data, size_t len);
};

/**
 * \brief A single-chain RSA policy handler.
 *
 * This policy context uses a single certificate chain, and a RSA
 * private key. The context can be restricted to only signatures or
 * only key exchange.
 *
 * Apart from the first field (vtable pointer), its contents are
 * opaque and shall not be accessed directly.
 */
typedef struct {
	/** \brief Pointer to vtable. */
	const br_ssl_server_policy_class *vtable;
#ifndef BR_DOXYGEN_IGNORE
	const br_x509_certificate *chain;
	size_t chain_len;
	const br_rsa_private_key *sk;
	unsigned allowed_usages;
	br_rsa_private irsacore;
	br_rsa_pkcs1_sign irsasign;
#endif
} br_ssl_server_policy_rsa_context;

/**
 * \brief A single-chain EC policy handler.
 *
 * This policy context uses a single certificate chain, and an EC
 * private key. The context can be restricted to only signatures or
 * only key exchange.
 *
 * Due to how TLS is defined, this context must be made aware whether
 * the server certificate was itself signed with RSA or ECDSA. The code
 * does not try to decode the certificate to obtain that information.
 *
 * Apart from the first field (vtable pointer), its contents are
 * opaque and shall not be accessed directly.
 */
typedef struct {
	/** \brief Pointer to vtable. */
	const br_ssl_server_policy_class *vtable;
#ifndef BR_DOXYGEN_IGNORE
	const br_x509_certificate *chain;
	size_t chain_len;
	const br_ec_private_key *sk;
	unsigned allowed_usages;
	unsigned cert_issuer_key_type;
	const br_multihash_context *mhash;
	const br_ec_impl *iec;
	br_ecdsa_sign iecdsa;
#endif
} br_ssl_server_policy_ec_context;

/**
 * \brief Class type for a session parameter cache.
 *
 * Session parameters are saved in the cache with `save()`, and
 * retrieved with `load()`. The cache implementation can apply any
 * storage and eviction strategy that it sees fit. The SSL server
 * context that performs the request is provided, so that its
 * functionalities may be used by the implementation (e.g. hash
 * functions or random number generation).
 */
typedef struct br_ssl_session_cache_class_ br_ssl_session_cache_class;
struct br_ssl_session_cache_class_ {
	/**
	 * \brief Context size (in bytes).
	 */
	size_t context_size;

	/**
	 * \brief Record a session.
	 *
	 * This callback should record the provided session parameters.
	 * The `params` structure is transient, so its contents shall
	 * be copied into the cache. The session ID has been randomly
	 * generated and always has length exactly 32 bytes.
	 *
	 * \param ctx          session cache context.
	 * \param server_ctx   SSL server context.
	 * \param params       session parameters to save.
	 */
	void (*save)(const br_ssl_session_cache_class **ctx,
		br_ssl_server_context *server_ctx,
		const br_ssl_session_parameters *params);

	/**
	 * \brief Lookup a session in the cache.
	 *
	 * The session ID to lookup is in `params` and always has length
	 * exactly 32 bytes. If the session parameters are found in the
	 * cache, then the parameters shall be copied into the `params`
	 * structure. Returned value is 1 on successful lookup, 0
	 * otherwise.
	 *
	 * \param ctx          session cache context.
	 * \param server_ctx   SSL server context.
	 * \param params       destination for session parameters.
	 * \return  1 if found, 0 otherwise.
	 */
	int (*load)(const br_ssl_session_cache_class **ctx,
		br_ssl_server_context *server_ctx,
		br_ssl_session_parameters *params);
};

/**
 * \brief Context for a basic cache system.
 *
 * The system stores session parameters in a buffer provided at
 * initialisation time. Each entry uses exactly 100 bytes, and
 * buffer sizes up to 4294967295 bytes are supported.
 *
 * Entries are evicted with a LRU (Least Recently Used) policy. A
 * search tree is maintained to keep lookups fast even with large
 * caches.
 *
 * Apart from the first field (vtable pointer), the structure
 * contents are opaque and shall not be accessed directly.
 */
typedef struct {
	/** \brief Pointer to vtable. */
	const br_ssl_session_cache_class *vtable;
#ifndef BR_DOXYGEN_IGNORE
	unsigned char *store;
	size_t store_len, store_ptr;
	unsigned char index_key[32];
	const br_hash_class *hash;
	int init_done;
	uint32_t head, tail, root;
#endif
} br_ssl_session_cache_lru;

/**
 * \brief Initialise a LRU session cache with the provided storage space.
 *
 * The provided storage space must remain valid as long as the cache
 * is used. Arbitrary lengths are supported, up to 4294967295 bytes;
 * each entry uses up exactly 100 bytes.
 *
 * \param cc          session cache context.
 * \param store       storage space for cached entries.
 * \param store_len   storage space length (in bytes).
 */
void br_ssl_session_cache_lru_init(br_ssl_session_cache_lru *cc,
	unsigned char *store, size_t store_len);

/**
 * \brief Context structure for a SSL server.
 *
 * The first field (called `eng`) is the SSL engine; all functions that
 * work on a `br_ssl_engine_context` structure shall take as parameter
 * a pointer to that field. The other structure fields are opaque and
 * must not be accessed directly.
 */
struct br_ssl_server_context_ {
	/**
	 * \brief The encapsulated engine context.
	 */
	br_ssl_engine_context eng;

#ifndef BR_DOXYGEN_IGNORE
	/*
	 * Maximum version from the client.
	 */
	uint16_t client_max_version;

	/*
	 * Session cache.
	 */
	const br_ssl_session_cache_class **cache_vtable;

	/*
	 * Translated cipher suites supported by the client. The list
	 * is trimmed to include only the cipher suites that the
	 * server also supports; they are in the same order as in the
	 * client message.
	 */
	br_suite_translated client_suites[BR_MAX_CIPHER_SUITES];
	unsigned char client_suites_num;

	/*
	 * Hash functions supported by the client, with ECDSA and RSA
	 * (bit mask). For hash function with id 'x', set bit index is
	 * x for RSA, x+8 for ECDSA.
	 */
	uint16_t hashes;

	/*
	 * Curves supported by the client (bit mask, for named curves).
	 */
	uint32_t curves;

	/*
	 * Context for chain handler.
	 */
	const br_ssl_server_policy_class **policy_vtable;
	unsigned char sign_hash_id;

	/*
	 * For the core handlers, thus avoiding (in most cases) the
	 * need for an externally provided policy context.
	 */
	union {
		const br_ssl_server_policy_class *vtable;
		br_ssl_server_policy_rsa_context single_rsa;
		br_ssl_server_policy_ec_context single_ec;
	} chain_handler;

	/*
	 * Buffer for the ECDHE private key.
	 */
	unsigned char ecdhe_key[70];
	size_t ecdhe_key_len;

	/*
	 * Trust anchor names for client authentication. "ta_names" and
	 * "tas" cannot be both non-NULL.
	 */
	const br_x500_name *ta_names;
	const br_x509_trust_anchor *tas;
	size_t num_tas;
	size_t cur_dn_index;
	const unsigned char *cur_dn;
	size_t cur_dn_len;

	/*
	 * Buffer for the hash value computed over all handshake messages
	 * prior to CertificateVerify, and identifier for the hash function.
	 */
	unsigned char hash_CV[64];
	size_t hash_CV_len;
	int hash_CV_id;

	/*
	 * Server-specific implementations.
	 * (none for now)
	 */
#endif
};

/*
 * Each br_ssl_server_init_xxx() function sets the list of supported
 * cipher suites and used implementations, as specified by the profile
 * name 'xxx'. Defined profile names are:
 *
 *    full_rsa    all supported algorithm, server key type is RSA
 *    full_ec     all supported algorithm, server key type is EC
 *    TODO: add other profiles
 *
 * Naming scheme for "minimal" profiles: min123
 *
 * -- character 1: key exchange
 *      r = RSA
 *      e = ECDHE_RSA
 *      f = ECDHE_ECDSA
 *      u = ECDH_RSA
 *      v = ECDH_ECDSA
 * -- character 2: version / PRF
 *      0 = TLS 1.0 / 1.1 with MD5+SHA-1
 *      2 = TLS 1.2 with SHA-256
 *      3 = TLS 1.2 with SHA-384
 * -- character 3: encryption
 *      a = AES/CBC
 *      g = AES/GCM
 *      d = 3DES/CBC
 */

/**
 * \brief SSL server profile: full_rsa.
 *
 * This function initialises the provided SSL server context with
 * all supported algorithms and cipher suites that rely on a RSA
 * key pair.
 *
 * \param cc          server context to initialise.
 * \param chain       server certificate chain.
 * \param chain_len   certificate chain length (number of certificate).
 * \param sk          RSA private key.
 */
void br_ssl_server_init_full_rsa(br_ssl_server_context *cc,
	const br_x509_certificate *chain, size_t chain_len,
	const br_rsa_private_key *sk);

/**
 * \brief SSL server profile: full_ec.
 *
 * This function initialises the provided SSL server context with
 * all supported algorithms and cipher suites that rely on an EC
 * key pair.
 *
 * The key type of the CA that issued the server's certificate must
 * be provided, since it matters for ECDH cipher suites (ECDH_RSA
 * suites require a RSA-powered CA). The key type is either
 * `BR_KEYTYPE_RSA` or `BR_KEYTYPE_EC`.
 *
 * \param cc                     server context to initialise.
 * \param chain                  server certificate chain.
 * \param chain_len              chain length (number of certificates).
 * \param cert_issuer_key_type   certificate issuer's key type.
 * \param sk                     EC private key.
 */
void br_ssl_server_init_full_ec(br_ssl_server_context *cc,
	const br_x509_certificate *chain, size_t chain_len,
	unsigned cert_issuer_key_type, const br_ec_private_key *sk);

/**
 * \brief SSL server profile: minr2g.
 *
 * This profile uses only TLS_RSA_WITH_AES_128_GCM_SHA256. Server key is
 * RSA, and RSA key exchange is used (not forward secure, but uses little
 * CPU in the client).
 *
 * \param cc          server context to initialise.
 * \param chain       server certificate chain.
 * \param chain_len   certificate chain length (number of certificate).
 * \param sk          RSA private key.
 */
void br_ssl_server_init_minr2g(br_ssl_server_context *cc,
	const br_x509_certificate *chain, size_t chain_len,
	const br_rsa_private_key *sk);

/**
 * \brief SSL server profile: mine2g.
 *
 * This profile uses only TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256. Server key
 * is RSA, and ECDHE key exchange is used. This suite provides forward
 * security, with a higher CPU expense on the client, and a somewhat
 * larger code footprint (compared to "minr2g").
 *
 * \param cc          server context to initialise.
 * \param chain       server certificate chain.
 * \param chain_len   certificate chain length (number of certificate).
 * \param sk          RSA private key.
 */
void br_ssl_server_init_mine2g(br_ssl_server_context *cc,
	const br_x509_certificate *chain, size_t chain_len,
	const br_rsa_private_key *sk);

/**
 * \brief SSL server profile: minf2g.
 *
 * This profile uses only TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256.
 * Server key is EC, and ECDHE key exchange is used. This suite provides
 * forward security, with a higher CPU expense on the client and server
 * (by a factor of about 3 to 4), and a somewhat larger code footprint
 * (compared to "minu2g" and "minv2g").
 *
 * \param cc          server context to initialise.
 * \param chain       server certificate chain.
 * \param chain_len   certificate chain length (number of certificate).
 * \param sk          EC private key.
 */
void br_ssl_server_init_minf2g(br_ssl_server_context *cc,
	const br_x509_certificate *chain, size_t chain_len,
	const br_ec_private_key *sk);

/**
 * \brief SSL server profile: minu2g.
 *
 * This profile uses only TLS_ECDH_RSA_WITH_AES_128_GCM_SHA256.
 * Server key is EC, and ECDH key exchange is used; the issuing CA used
 * a RSA key.
 *
 * The "minu2g" and "minv2g" profiles do not provide forward secrecy,
 * but are the lightest on the server (for CPU usage), and are rather
 * inexpensive on the client as well.
 *
 * \param cc          server context to initialise.
 * \param chain       server certificate chain.
 * \param chain_len   certificate chain length (number of certificate).
 * \param sk          EC private key.
 */
void br_ssl_server_init_minu2g(br_ssl_server_context *cc,
	const br_x509_certificate *chain, size_t chain_len,
	const br_ec_private_key *sk);

/**
 * \brief SSL server profile: minv2g.
 *
 * This profile uses only TLS_ECDH_ECDSA_WITH_AES_128_GCM_SHA256.
 * Server key is EC, and ECDH key exchange is used; the issuing CA used
 * an EC key.
 *
 * The "minu2g" and "minv2g" profiles do not provide forward secrecy,
 * but are the lightest on the server (for CPU usage), and are rather
 * inexpensive on the client as well.
 *
 * \param cc          server context to initialise.
 * \param chain       server certificate chain.
 * \param chain_len   certificate chain length (number of certificate).
 * \param sk          EC private key.
 */
void br_ssl_server_init_minv2g(br_ssl_server_context *cc,
	const br_x509_certificate *chain, size_t chain_len,
	const br_ec_private_key *sk);

/**
 * \brief SSL server profile: mine2c.
 *
 * This profile uses only TLS_ECDHE_RSA_WITH_CHACHA20_POLY1305_SHA256.
 * Server key is RSA, and ECDHE key exchange is used. This suite
 * provides forward security.
 *
 * \param cc          server context to initialise.
 * \param chain       server certificate chain.
 * \param chain_len   certificate chain length (number of certificate).
 * \param sk          RSA private key.
 */
void br_ssl_server_init_mine2c(br_ssl_server_context *cc,
	const br_x509_certificate *chain, size_t chain_len,
	const br_rsa_private_key *sk);

/**
 * \brief SSL server profile: minf2c.
 *
 * This profile uses only TLS_ECDHE_ECDSA_WITH_CHACHA20_POLY1305_SHA256.
 * Server key is EC, and ECDHE key exchange is used. This suite provides
 * forward security.
 *
 * \param cc          server context to initialise.
 * \param chain       server certificate chain.
 * \param chain_len   certificate chain length (number of certificate).
 * \param sk          EC private key.
 */
void br_ssl_server_init_minf2c(br_ssl_server_context *cc,
	const br_x509_certificate *chain, size_t chain_len,
	const br_ec_private_key *sk);

/**
 * \brief Get the supported client suites.
 *
 * This function shall be called only after the ClientHello has been
 * processed, typically from the policy engine. The returned array
 * contains the cipher suites that are supported by both the client
 * and the server; these suites are in client preference order, unless
 * the `BR_OPT_ENFORCE_SERVER_PREFERENCES` flag was set, in which case
 * they are in server preference order.
 *
 * The suites are _translated_, which means that each suite is given
 * as two 16-bit integers: the standard suite identifier, and its
 * translated version, broken down into its individual components,
 * as explained with the `br_suite_translated` type.
 *
 * The returned array is allocated in the context and will be rewritten
 * by each handshake.
 *
 * \param cc    server context.
 * \param num   receives the array size (number of suites).
 * \return  the translated common cipher suites, in preference order.
 */
static inline const br_suite_translated *
br_ssl_server_get_client_suites(const br_ssl_server_context *cc, size_t *num)
{
	*num = cc->client_suites_num;
	return cc->client_suites;
}

/**
 * \brief Get the hash functions supported by the client.
 *
 * This is a field of bits: for hash function of ID x, bit x is set if
 * the hash function is supported in RSA signatures, 8+x if it is supported
 * with ECDSA.
 *
 * \param cc   server context.
 * \return  the client-supported hash functions (for signatures).
 */
static inline uint16_t
br_ssl_server_get_client_hashes(const br_ssl_server_context *cc)
{
	return cc->hashes;
}

/**
 * \brief Get the elliptic curves supported by the client.
 *
 * This is a bit field (bit x is set if curve of ID x is supported).
 *
 * \param cc   server context.
 * \return  the client-supported elliptic curves.
 */
static inline uint32_t
br_ssl_server_get_client_curves(const br_ssl_server_context *cc)
{
	return cc->curves;
}

/**
 * \brief Clear the complete contents of a SSL server context.
 *
 * Everything is cleared, including the reference to the configured buffer,
 * implementations, cipher suites and state. This is a preparatory step
 * to assembling a custom profile.
 *
 * \param cc   server context to clear.
 */
void br_ssl_server_zero(br_ssl_server_context *cc);

/**
 * \brief Set an externally provided policy context.
 *
 * The policy context's methods are invoked to decide the cipher suite
 * and certificate chain, and to perform operations involving the server's
 * private key.
 *
 * \param cc     server context.
 * \param pctx   policy context (pointer to its vtable field).
 */
static inline void
br_ssl_server_set_policy(br_ssl_server_context *cc,
	const br_ssl_server_policy_class **pctx)
{
	cc->policy_vtable = pctx;
}

/**
 * \brief Set the server certificate chain and key (single RSA case).
 *
 * This function uses a policy context included in the server context.
 * It configures use of a single server certificate chain with a RSA
 * private key. The `allowed_usages` is a combination of usages, namely
 * `BR_KEYTYPE_KEYX` and/or `BR_KEYTYPE_SIGN`; this enables or disables
 * the corresponding cipher suites (i.e. `TLS_RSA_*` use the RSA key for
 * key exchange, while `TLS_ECDHE_RSA_*` use the RSA key for signatures).
 *
 * \param cc               server context.
 * \param chain            server certificate chain to send to the client.
 * \param chain_len        chain length (number of certificates).
 * \param sk               server private key (RSA).
 * \param allowed_usages   allowed private key usages.
 * \param irsacore         RSA core implementation.
 * \param irsasign         RSA signature implementation (PKCS#1 v1.5).
 */
void br_ssl_server_set_single_rsa(br_ssl_server_context *cc,
	const br_x509_certificate *chain, size_t chain_len,
	const br_rsa_private_key *sk, unsigned allowed_usages,
	br_rsa_private irsacore, br_rsa_pkcs1_sign irsasign);

/**
 * \brief Set the server certificate chain and key (single EC case).
 *
 * This function uses a policy context included in the server context.
 * It configures use of a single server certificate chain with an EC
 * private key. The `allowed_usages` is a combination of usages, namely
 * `BR_KEYTYPE_KEYX` and/or `BR_KEYTYPE_SIGN`; this enables or disables
 * the corresponding cipher suites (i.e. `TLS_ECDH_*` use the EC key for
 * key exchange, while `TLS_ECDHE_ECDSA_*` use the EC key for signatures).
 *
 * In order to support `TLS_ECDH_*` cipher suites (non-ephemeral ECDH),
 * the algorithm type of the key used by the issuing CA to sign the
 * server's certificate must be provided, as `cert_issuer_key_type`
 * parameter (this value is either `BR_KEYTYPE_RSA` or `BR_KEYTYPE_EC`).
 *
 * \param cc                     server context.
 * \param chain                  server certificate chain to send.
 * \param chain_len              chain length (number of certificates).
 * \param sk                     server private key (EC).
 * \param allowed_usages         allowed private key usages.
 * \param cert_issuer_key_type   issuing CA's key type.
 * \param iec                    EC core implementation.
 * \param iecdsa                 ECDSA signature implementation ("asn1" format).
 */
void br_ssl_server_set_single_ec(br_ssl_server_context *cc,
	const br_x509_certificate *chain, size_t chain_len,
	const br_ec_private_key *sk, unsigned allowed_usages,
	unsigned cert_issuer_key_type,
	const br_ec_impl *iec, br_ecdsa_sign iecdsa);

/**
 * \brief Activate client certificate authentication.
 *
 * The trust anchor encoded X.500 names (DN) to send to the client are
 * provided. A client certificate will be requested and validated through
 * the X.509 validator configured in the SSL engine. If `num` is 0, then
 * client certificate authentication is disabled.
 *
 * If the client does not send a certificate, or on validation failure,
 * the handshake aborts. Unauthenticated clients can be tolerated by
 * setting the `BR_OPT_TOLERATE_NO_CLIENT_AUTH` flag.
 *
 * The provided array is linked in, not copied, so that pointer must
 * remain valid as long as anchor names may be used.
 *
 * \param cc         server context.
 * \param ta_names   encoded trust anchor names.
 * \param num        number of encoded trust anchor names.
 */
static inline void
br_ssl_server_set_trust_anchor_names(br_ssl_server_context *cc,
	const br_x500_name *ta_names, size_t num)
{
	cc->ta_names = ta_names;
	cc->tas = NULL;
	cc->num_tas = num;
}

/**
 * \brief Activate client certificate authentication.
 *
 * This is a variant for `br_ssl_server_set_trust_anchor_names()`: the
 * trust anchor names are provided not as an array of stand-alone names
 * (`br_x500_name` structures), but as an array of trust anchors
 * (`br_x509_trust_anchor` structures). The server engine itself will
 * only use the `dn` field of each trust anchor. This is meant to allow
 * defining a single array of trust anchors, to be used here and in the
 * X.509 validation engine itself.
 *
 * The provided array is linked in, not copied, so that pointer must
 * remain valid as long as anchor names may be used.
 *
 * \param cc    server context.
 * \param tas   trust anchors (only names are used).
 * \param num   number of trust anchors.
 */
static inline void
br_ssl_server_set_trust_anchor_names_alt(br_ssl_server_context *cc,
	const br_x509_trust_anchor *tas, size_t num)
{
	cc->ta_names = NULL;
	cc->tas = tas;
	cc->num_tas = num;
}

/**
 * \brief Configure the cache for session parameters.
 *
 * The cache context is provided as a pointer to its first field (vtable
 * pointer).
 *
 * \param cc       server context.
 * \param vtable   session cache context.
 */
static inline void
br_ssl_server_set_cache(br_ssl_server_context *cc,
	const br_ssl_session_cache_class **vtable)
{
	cc->cache_vtable = vtable;
}

/**
 * \brief Prepare or reset a server context for handling an incoming client.
 *
 * \param cc   server context.
 * \return  1 on success, 0 on error.
 */
int br_ssl_server_reset(br_ssl_server_context *cc);

/* ===================================================================== */

/*
 * Context for the simplified I/O context. The transport medium is accessed
 * through the low_read() and low_write() callback functions, each with
 * its own opaque context pointer.
 *
 *  low_read()    read some bytes, at most 'len' bytes, into data[]. The
 *                returned value is the number of read bytes, or -1 on error.
 *                The 'len' parameter is guaranteed never to exceed 20000,
 *                so the length always fits in an 'int' on all platforms.
 *
 *  low_write()   write up to 'len' bytes, to be read from data[]. The
 *                returned value is the number of written bytes, or -1 on
 *                error. The 'len' parameter is guaranteed never to exceed
 *                20000, so the length always fits in an 'int' on all
 *                parameters.
 *
 * A socket closure (if the transport medium is a socket) should be reported
 * as an error (-1). The callbacks shall endeavour to block until at least
 * one byte can be read or written; a callback returning 0 at times is
 * acceptable, but this normally leads to the callback being immediately
 * called again, so the callback should at least always try to block for
 * some time if no I/O can take place.
 *
 * The SSL engine naturally applies some buffering, so the callbacks need
 * not apply buffers of their own.
 */
/**
 * \brief Context structure for the simplified SSL I/O wrapper.
 *
 * This structure is initialised with `br_sslio_init()`. Its contents
 * are opaque and shall not be accessed directly.
 */
typedef struct {
#ifndef BR_DOXYGEN_IGNORE
	br_ssl_engine_context *engine;
	int (*low_read)(void *read_context,
		unsigned char *data, size_t len);
	void *read_context;
	int (*low_write)(void *write_context,
		const unsigned char *data, size_t len);
	void *write_context;
#endif
} br_sslio_context;

/**
 * \brief Initialise a simplified I/O wrapper context.
 *
 * The simplified I/O wrapper offers a simpler read/write API for a SSL
 * engine (client or server), using the provided callback functions for
 * reading data from, or writing data to, the transport medium.
 *
 * The callback functions have the following semantics:
 *
 *   - Each callback receives an opaque context value (of type `void *`)
 *     that the callback may use arbitrarily (or possibly ignore).
 *
 *   - `low_read()` reads at least one byte, at most `len` bytes, from
 *     the transport medium. Read bytes shall be written in `data`.
 *
 *   - `low_write()` writes at least one byte, at most `len` bytes, unto
 *     the transport medium. The bytes to write are read from `data`.
 *
 *   - The `len` parameter is never zero, and is always lower than 20000.
 *
 *   - The number of processed bytes (read or written) is returned. Since
 *     that number is less than 20000, it always fits on an `int`.
 *
 *   - On error, the callbacks return -1. Reaching end-of-stream is an
 *     error. Errors are permanent: the SSL connection is terminated.
 *
 *   - Callbacks SHOULD NOT return 0. This is tolerated, as long as
 *     callbacks endeavour to block for some non-negligible amount of
 *     time until at least one byte can be sent or received (if a
 *     callback returns 0, then the wrapper invokes it again
 *     immediately).
 *
 *   - Callbacks MAY return as soon as at least one byte is processed;
 *     they MAY also insist on reading or writing _all_ requested bytes.
 *     Since SSL is a self-terminated protocol (each record has a length
 *     header), this does not change semantics.
 *
 *   - Callbacks need not apply any buffering (for performance) since SSL
 *     itself uses buffers.
 *
 * \param ctx             wrapper context to initialise.
 * \param engine          SSL engine to wrap.
 * \param low_read        callback for reading data from the transport.
 * \param read_context    context pointer for `low_read()`.
 * \param low_write       callback for writing data on the transport.
 * \param write_context   context pointer for `low_write()`.
 */
void br_sslio_init(br_sslio_context *ctx,
	br_ssl_engine_context *engine,
	int (*low_read)(void *read_context,
		unsigned char *data, size_t len),
	void *read_context,
	int (*low_write)(void *write_context,
		const unsigned char *data, size_t len),
	void *write_context);

/**
 * \brief Read some application data from a SSL connection.
 *
 * If `len` is zero, then this function returns 0 immediately. In
 * all other cases, it never returns 0.
 *
 * This call returns only when at least one byte has been obtained.
 * Returned value is the number of bytes read, or -1 on error. The
 * number of bytes always fits on an 'int' (data from a single SSL/TLS
 * record is returned).
 *
 * On error or SSL closure, this function returns -1. The caller should
 * inspect the error status on the SSL engine to distinguish between
 * normal closure and error.
 *
 * \param cc    SSL wrapper context.
 * \param dst   destination buffer for application data.
 * \param len   maximum number of bytes to obtain.
 * \return  number of bytes obtained, or -1 on error.
 */
int br_sslio_read(br_sslio_context *cc, void *dst, size_t len);

/**
 * \brief Read application data from a SSL connection.
 *
 * This calls returns only when _all_ requested `len` bytes are read,
 * or an error is reached. Returned value is 0 on success, -1 on error.
 * A normal (verified) SSL closure before that many bytes are obtained
 * is reported as an error by this function.
 *
 * \param cc    SSL wrapper context.
 * \param dst   destination buffer for application data.
 * \param len   number of bytes to obtain.
 * \return  0 on success, or -1 on error.
 */
int br_sslio_read_all(br_sslio_context *cc, void *dst, size_t len);

/**
 * \brief Write some application data unto a SSL connection.
 *
 * If `len` is zero, then this function returns 0 immediately. In
 * all other cases, it never returns 0.
 *
 * This call returns only when at least one byte has been written.
 * Returned value is the number of bytes written, or -1 on error. The
 * number of bytes always fits on an 'int' (less than 20000).
 *
 * On error or SSL closure, this function returns -1. The caller should
 * inspect the error status on the SSL engine to distinguish between
 * normal closure and error.
 *
 * **Important:** SSL is buffered; a "written" byte is a byte that was
 * injected into the wrapped SSL engine, but this does not necessarily mean
 * that it has been scheduled for sending. Use `br_sslio_flush()` to
 * ensure that all pending data has been sent to the transport medium.
 *
 * \param cc    SSL wrapper context.
 * \param src   source buffer for application data.
 * \param len   maximum number of bytes to write.
 * \return  number of bytes written, or -1 on error.
 */
int br_sslio_write(br_sslio_context *cc, const void *src, size_t len);

/**
 * \brief Write application data unto a SSL connection.
 *
 * This calls returns only when _all_ requested `len` bytes have been
 * written, or an error is reached. Returned value is 0 on success, -1
 * on error. A normal (verified) SSL closure before that many bytes are
 * written is reported as an error by this function.
 *
 * **Important:** SSL is buffered; a "written" byte is a byte that was
 * injected into the wrapped SSL engine, but this does not necessarily mean
 * that it has been scheduled for sending. Use `br_sslio_flush()` to
 * ensure that all pending data has been sent to the transport medium.
 *
 * \param cc    SSL wrapper context.
 * \param src   source buffer for application data.
 * \param len   number of bytes to write.
 * \return  0 on success, or -1 on error.
 */
int br_sslio_write_all(br_sslio_context *cc, const void *src, size_t len);

/**
 * \brief Flush pending data.
 *
 * This call makes sure that any buffered application data in the
 * provided context (including the wrapped SSL engine) has been sent
 * to the transport medium (i.e. accepted by the `low_write()` callback
 * method). If there is no such pending data, then this function does
 * nothing (and returns a success, i.e. 0).
 *
 * If the underlying transport medium has its own buffers, then it is
 * up to the caller to ensure the corresponding flushing.
 *
 * Returned value is 0 on success, -1 on error.
 *
 * \param cc    SSL wrapper context.
 * \return  0 on success, or -1 on error.
 */
int br_sslio_flush(br_sslio_context *cc);

/**
 * \brief Close the SSL connection.
 *
 * This call runs the SSL closure protocol (sending a `close_notify`,
 * receiving the response `close_notify`). When it returns, the SSL
 * connection is finished. It is still up to the caller to manage the
 * possible transport-level termination, if applicable (alternatively,
 * the underlying transport stream may be reused for non-SSL messages).
 *
 * Returned value is 0 on success, -1 on error. A failure by the peer
 * to process the complete closure protocol (i.e. sending back the
 * `close_notify`) is an error.
 *
 * \param cc    SSL wrapper context.
 * \return  0 on success, or -1 on error.
 */
int br_sslio_close(br_sslio_context *cc);

/* ===================================================================== */

/*
 * Symbolic constants for cipher suites.
 */

/* From RFC 5246 */
#define BR_TLS_NULL_WITH_NULL_NULL                   0x0000
#define BR_TLS_RSA_WITH_NULL_MD5                     0x0001
#define BR_TLS_RSA_WITH_NULL_SHA                     0x0002
#define BR_TLS_RSA_WITH_NULL_SHA256                  0x003B
#define BR_TLS_RSA_WITH_RC4_128_MD5                  0x0004
#define BR_TLS_RSA_WITH_RC4_128_SHA                  0x0005
#define BR_TLS_RSA_WITH_3DES_EDE_CBC_SHA             0x000A
#define BR_TLS_RSA_WITH_AES_128_CBC_SHA              0x002F
#define BR_TLS_RSA_WITH_AES_256_CBC_SHA              0x0035
#define BR_TLS_RSA_WITH_AES_128_CBC_SHA256           0x003C
#define BR_TLS_RSA_WITH_AES_256_CBC_SHA256           0x003D
#define BR_TLS_DH_DSS_WITH_3DES_EDE_CBC_SHA          0x000D
#define BR_TLS_DH_RSA_WITH_3DES_EDE_CBC_SHA          0x0010
#define BR_TLS_DHE_DSS_WITH_3DES_EDE_CBC_SHA         0x0013
#define BR_TLS_DHE_RSA_WITH_3DES_EDE_CBC_SHA         0x0016
#define BR_TLS_DH_DSS_WITH_AES_128_CBC_SHA           0x0030
#define BR_TLS_DH_RSA_WITH_AES_128_CBC_SHA           0x0031
#define BR_TLS_DHE_DSS_WITH_AES_128_CBC_SHA          0x0032
#define BR_TLS_DHE_RSA_WITH_AES_128_CBC_SHA          0x0033
#define BR_TLS_DH_DSS_WITH_AES_256_CBC_SHA           0x0036
#define BR_TLS_DH_RSA_WITH_AES_256_CBC_SHA           0x0037
#define BR_TLS_DHE_DSS_WITH_AES_256_CBC_SHA          0x0038
#define BR_TLS_DHE_RSA_WITH_AES_256_CBC_SHA          0x0039
#define BR_TLS_DH_DSS_WITH_AES_128_CBC_SHA256        0x003E
#define BR_TLS_DH_RSA_WITH_AES_128_CBC_SHA256        0x003F
#define BR_TLS_DHE_DSS_WITH_AES_128_CBC_SHA256       0x0040
#define BR_TLS_DHE_RSA_WITH_AES_128_CBC_SHA256       0x0067
#define BR_TLS_DH_DSS_WITH_AES_256_CBC_SHA256        0x0068
#define BR_TLS_DH_RSA_WITH_AES_256_CBC_SHA256        0x0069
#define BR_TLS_DHE_DSS_WITH_AES_256_CBC_SHA256       0x006A
#define BR_TLS_DHE_RSA_WITH_AES_256_CBC_SHA256       0x006B
#define BR_TLS_DH_anon_WITH_RC4_128_MD5              0x0018
#define BR_TLS_DH_anon_WITH_3DES_EDE_CBC_SHA         0x001B
#define BR_TLS_DH_anon_WITH_AES_128_CBC_SHA          0x0034
#define BR_TLS_DH_anon_WITH_AES_256_CBC_SHA          0x003A
#define BR_TLS_DH_anon_WITH_AES_128_CBC_SHA256       0x006C
#define BR_TLS_DH_anon_WITH_AES_256_CBC_SHA256       0x006D

/* From RFC 4492 */
#define BR_TLS_ECDH_ECDSA_WITH_NULL_SHA              0xC001
#define BR_TLS_ECDH_ECDSA_WITH_RC4_128_SHA           0xC002
#define BR_TLS_ECDH_ECDSA_WITH_3DES_EDE_CBC_SHA      0xC003
#define BR_TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA       0xC004
#define BR_TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA       0xC005
#define BR_TLS_ECDHE_ECDSA_WITH_NULL_SHA             0xC006
#define BR_TLS_ECDHE_ECDSA_WITH_RC4_128_SHA          0xC007
#define BR_TLS_ECDHE_ECDSA_WITH_3DES_EDE_CBC_SHA     0xC008
#define BR_TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA      0xC009
#define BR_TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA      0xC00A
#define BR_TLS_ECDH_RSA_WITH_NULL_SHA                0xC00B
#define BR_TLS_ECDH_RSA_WITH_RC4_128_SHA             0xC00C
#define BR_TLS_ECDH_RSA_WITH_3DES_EDE_CBC_SHA        0xC00D
#define BR_TLS_ECDH_RSA_WITH_AES_128_CBC_SHA         0xC00E
#define BR_TLS_ECDH_RSA_WITH_AES_256_CBC_SHA         0xC00F
#define BR_TLS_ECDHE_RSA_WITH_NULL_SHA               0xC010
#define BR_TLS_ECDHE_RSA_WITH_RC4_128_SHA            0xC011
#define BR_TLS_ECDHE_RSA_WITH_3DES_EDE_CBC_SHA       0xC012
#define BR_TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA        0xC013
#define BR_TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA        0xC014
#define BR_TLS_ECDH_anon_WITH_NULL_SHA               0xC015
#define BR_TLS_ECDH_anon_WITH_RC4_128_SHA            0xC016
#define BR_TLS_ECDH_anon_WITH_3DES_EDE_CBC_SHA       0xC017
#define BR_TLS_ECDH_anon_WITH_AES_128_CBC_SHA        0xC018
#define BR_TLS_ECDH_anon_WITH_AES_256_CBC_SHA        0xC019

/* From RFC 5288 */
#define BR_TLS_RSA_WITH_AES_128_GCM_SHA256           0x009C
#define BR_TLS_RSA_WITH_AES_256_GCM_SHA384           0x009D
#define BR_TLS_DHE_RSA_WITH_AES_128_GCM_SHA256       0x009E
#define BR_TLS_DHE_RSA_WITH_AES_256_GCM_SHA384       0x009F
#define BR_TLS_DH_RSA_WITH_AES_128_GCM_SHA256        0x00A0
#define BR_TLS_DH_RSA_WITH_AES_256_GCM_SHA384        0x00A1
#define BR_TLS_DHE_DSS_WITH_AES_128_GCM_SHA256       0x00A2
#define BR_TLS_DHE_DSS_WITH_AES_256_GCM_SHA384       0x00A3
#define BR_TLS_DH_DSS_WITH_AES_128_GCM_SHA256        0x00A4
#define BR_TLS_DH_DSS_WITH_AES_256_GCM_SHA384        0x00A5
#define BR_TLS_DH_anon_WITH_AES_128_GCM_SHA256       0x00A6
#define BR_TLS_DH_anon_WITH_AES_256_GCM_SHA384       0x00A7

/* From RFC 5289 */
#define BR_TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256   0xC023
#define BR_TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA384   0xC024
#define BR_TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA256    0xC025
#define BR_TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA384    0xC026
#define BR_TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256     0xC027
#define BR_TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA384     0xC028
#define BR_TLS_ECDH_RSA_WITH_AES_128_CBC_SHA256      0xC029
#define BR_TLS_ECDH_RSA_WITH_AES_256_CBC_SHA384      0xC02A
#define BR_TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256   0xC02B
#define BR_TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384   0xC02C
#define BR_TLS_ECDH_ECDSA_WITH_AES_128_GCM_SHA256    0xC02D
#define BR_TLS_ECDH_ECDSA_WITH_AES_256_GCM_SHA384    0xC02E
#define BR_TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256     0xC02F
#define BR_TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384     0xC030
#define BR_TLS_ECDH_RSA_WITH_AES_128_GCM_SHA256      0xC031
#define BR_TLS_ECDH_RSA_WITH_AES_256_GCM_SHA384      0xC032

/* From RFC 7905 */
#define BR_TLS_ECDHE_RSA_WITH_CHACHA20_POLY1305_SHA256     0xCCA8
#define BR_TLS_ECDHE_ECDSA_WITH_CHACHA20_POLY1305_SHA256   0xCCA9
#define BR_TLS_DHE_RSA_WITH_CHACHA20_POLY1305_SHA256       0xCCAA
#define BR_TLS_PSK_WITH_CHACHA20_POLY1305_SHA256           0xCCAB
#define BR_TLS_ECDHE_PSK_WITH_CHACHA20_POLY1305_SHA256     0xCCAC
#define BR_TLS_DHE_PSK_WITH_CHACHA20_POLY1305_SHA256       0xCCAD
#define BR_TLS_RSA_PSK_WITH_CHACHA20_POLY1305_SHA256       0xCCAE

/* From RFC 7507 */
#define BR_TLS_FALLBACK_SCSV                         0x5600

/*
 * Symbolic constants for alerts.
 */
#define BR_ALERT_CLOSE_NOTIFY                0
#define BR_ALERT_UNEXPECTED_MESSAGE         10
#define BR_ALERT_BAD_RECORD_MAC             20
#define BR_ALERT_RECORD_OVERFLOW            22
#define BR_ALERT_DECOMPRESSION_FAILURE      30
#define BR_ALERT_HANDSHAKE_FAILURE          40
#define BR_ALERT_BAD_CERTIFICATE            42
#define BR_ALERT_UNSUPPORTED_CERTIFICATE    43
#define BR_ALERT_CERTIFICATE_REVOKED        44
#define BR_ALERT_CERTIFICATE_EXPIRED        45
#define BR_ALERT_CERTIFICATE_UNKNOWN        46
#define BR_ALERT_ILLEGAL_PARAMETER          47
#define BR_ALERT_UNKNOWN_CA                 48
#define BR_ALERT_ACCESS_DENIED              49
#define BR_ALERT_DECODE_ERROR               50
#define BR_ALERT_DECRYPT_ERROR              51
#define BR_ALERT_PROTOCOL_VERSION           70
#define BR_ALERT_INSUFFICIENT_SECURITY      71
#define BR_ALERT_INTERNAL_ERROR             80
#define BR_ALERT_USER_CANCELED              90
#define BR_ALERT_NO_RENEGOTIATION          100
#define BR_ALERT_UNSUPPORTED_EXTENSION     110
#define BR_ALERT_NO_APPLICATION_PROTOCOL   120

#endif


/* ===== inc/bearssl_pem.h ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef BR_BEARSSL_PEM_H__
#define BR_BEARSSL_PEM_H__

#include <stddef.h>
#include <stdint.h>

/** \file bearssl_pem.h
 *
 * # PEM Support
 *
 * PEM is a traditional encoding layer use to store binary objects (in
 * particular X.509 certificates, and private keys) in text files. While
 * the acronym comes from an old, defunct standard ("Privacy Enhanced
 * Mail"), the format has been reused, with some variations, by many
 * systems, and is a _de facto_ standard, even though it is not, actually,
 * specified in all clarity anywhere.
 *
 * ## Format Details
 *
 * BearSSL contains a generic, streamed PEM decoder, which handles the
 * following format:
 *
 *   - The input source (a sequence of bytes) is assumed to be the
 *     encoding of a text file in an ASCII-compatible charset. This
 *     includes ISO-8859-1, Windows-1252, and UTF-8 encodings. Each
 *     line ends on a newline character (U+000A LINE FEED). The
 *     U+000D CARRIAGE RETURN characters are ignored, so the code
 *     accepts both Windows-style and Unix-style line endings.
 *
 *   - Each object begins with a banner that occurs at the start of
 *     a line; the first banner characters are "`-----BEGIN `" (five
 *     dashes, the word "BEGIN", and a space). The banner matching is
 *     not case-sensitive.
 *
 *   - The _object name_ consists in the characters that follow the
 *     banner start sequence, up to the end of the line, but without
 *     trailing dashes (in "normal" PEM, there are five trailing
 *     dashes, but this implementation is not picky about these dashes).
 *     The BearSSL decoder normalises the name characters to uppercase
 *     (for ASCII letters only) and accepts names up to 127 characters.
 *
 *   - The object ends with a banner that again occurs at the start of
 *     a line, and starts with "`-----END `" (again case-insensitive).
 *
 *   - Between that start and end banner, only Base64 data shall occur.
 *     Base64 converts each sequence of three bytes into four
 *     characters; the four characters are ASCII letters, digits, "`+`"
 *     or "`-`" signs, and one or two "`=`" signs may occur in the last
 *     quartet. Whitespace is ignored (whitespace is any ASCII character
 *     of code 32 or less, so control characters are whitespace) and
 *     lines may have arbitrary length; the only restriction is that the
 *     four characters of a quartet must appear on the same line (no
 *     line break inside a quartet).
 *
 *   - A single file may contain more than one PEM object. Bytes that
 *     occur between objects are ignored.
 *
 *
 * ## PEM Decoder API
 *
 * The PEM decoder offers a state-machine API. The caller allocates a
 * decoder context, then injects source bytes. Source bytes are pushed
 * with `br_pem_decoder_push()`. The decoder stops accepting bytes when
 * it reaches an "event", which is either the start of an object, the
 * end of an object, or a decoding error within an object.
 *
 * The `br_pem_decoder_event()` function is used to obtain the current
 * event; it also clears it, thus allowing the decoder to accept more
 * bytes. When a object start event is raised, the decoder context
 * offers the found object name (normalised to ASCII uppercase).
 *
 * When an object is reached, the caller must set an appropriate callback
 * function, which will receive (by chunks) the decoded object data.
 *
 * Since the decoder context makes no dynamic allocation, it requires
 * no explicit deallocation.
 */

/**
 * \brief PEM decoder context.
 *
 * Contents are opaque (they should not be accessed directly).
 */
typedef struct {
#ifndef BR_DOXYGEN_IGNORE
	/* CPU for the T0 virtual machine. */
	struct {
		uint32_t *dp;
		uint32_t *rp;
		const unsigned char *ip;
	} cpu;
	uint32_t dp_stack[32];
	uint32_t rp_stack[32];
	int err;

	const unsigned char *hbuf;
	size_t hlen;

	void (*dest)(void *dest_ctx, const void *src, size_t len);
	void *dest_ctx;

	unsigned char event;
	char name[128];
	unsigned char buf[255];
	size_t ptr;
#endif
} br_pem_decoder_context;

/**
 * \brief Initialise a PEM decoder structure.
 *
 * \param ctx   decoder context to initialise.
 */
void br_pem_decoder_init(br_pem_decoder_context *ctx);

/**
 * \brief Push some bytes into the decoder.
 *
 * Returned value is the number of bytes actually consumed; this may be
 * less than the number of provided bytes if an event is raised. When an
 * event is raised, it must be read (with `br_pem_decoder_event()`);
 * until the event is read, this function will return 0.
 *
 * \param ctx    decoder context.
 * \param data   new data bytes.
 * \param len    number of new data bytes.
 * \return  the number of bytes actually received (may be less than `len`).
 */
size_t br_pem_decoder_push(br_pem_decoder_context *ctx,
	const void *data, size_t len);

/**
 * \brief Set the receiver for decoded data.
 *
 * When an object is entered, the provided function (with opaque context
 * pointer) will be called repeatedly with successive chunks of decoded
 * data for that object. If `dest` is set to 0, then decoded data is
 * simply ignored. The receiver can be set at any time, but, in practice,
 * it should be called immediately after receiving a "start of object"
 * event.
 *
 * \param ctx        decoder context.
 * \param dest       callback for receiving decoded data.
 * \param dest_ctx   opaque context pointer for the `dest` callback.
 */
static inline void
br_pem_decoder_setdest(br_pem_decoder_context *ctx,
	void (*dest)(void *dest_ctx, const void *src, size_t len),
	void *dest_ctx)
{
	ctx->dest = dest;
	ctx->dest_ctx = dest_ctx;
}

/**
 * \brief Get the last event.
 *
 * If an event was raised, then this function returns the event value, and
 * also clears it, thereby allowing the decoder to proceed. If no event
 * was raised since the last call to `br_pem_decoder_event()`, then this
 * function returns 0.
 *
 * \param ctx   decoder context.
 * \return  the raised event, or 0.
 */
int br_pem_decoder_event(br_pem_decoder_context *ctx);

/**
 * \brief Event: start of object.
 *
 * This event is raised when the start of a new object has been detected.
 * The object name (normalised to uppercase) can be accessed with
 * `br_pem_decoder_name()`.
 */
#define BR_PEM_BEGIN_OBJ   1

/**
 * \brief Event: end of object.
 *
 * This event is raised when the end of the current object is reached
 * (normally, i.e. with no decoding error).
 */
#define BR_PEM_END_OBJ     2

/**
 * \brief Event: decoding error.
 *
 * This event is raised when decoding fails within an object.
 * This formally closes the current object and brings the decoder back
 * to the "out of any object" state. The offending line in the source
 * is consumed.
 */
#define BR_PEM_ERROR       3

/**
 * \brief Get the name of the encountered object.
 *
 * The encountered object name is defined only when the "start of object"
 * event is raised. That name is normalised to uppercase (for ASCII letters
 * only) and does not include trailing dashes.
 *
 * \param ctx   decoder context.
 * \return  the current object name.
 */
static inline const char *
br_pem_decoder_name(br_pem_decoder_context *ctx)
{
	return ctx->name;
}

#endif


/* ===== inc/bearssl.h ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef BR_BEARSSL_H__
#define BR_BEARSSL_H__

#include <stddef.h>
#include <stdint.h>

/** \mainpage BearSSL API
 *
 * # API Layout
 *
 * The functions and structures defined by the BearSSL API are located
 * in various header files:
 *
 * | Header file     | Elements                                          |
 * | :-------------- | :------------------------------------------------ |
 * | bearssl_hash.h  | Hash functions                                    |
 * | bearssl_hmac.h  | HMAC                                              |
 * | bearssl_rand.h  | Pseudorandom byte generators                      |
 * | bearssl_prf.h   | PRF implementations (for SSL/TLS)                 |
 * | bearssl_block.h | Symmetric encryption                              |
 * | bearssl_rsa.h   | RSA encryption and signatures                     |
 * | bearssl_ec.h    | Elliptic curves support (including ECDSA)         |
 * | bearssl_ssl.h   | SSL/TLS engine interface                          |
 * | bearssl_x509.h  | X.509 certificate decoding and validation         |
 * | bearssl_pem.h   | Base64/PEM decoding support functions             |
 *
 * Applications using BearSSL are supposed to simply include `bearssl.h`
 * as follows:
 *
 *     #include <bearssl.h>
 *
 * The `bearssl.h` file itself includes all the other header files. It is
 * possible to include specific header files, but it has no practical
 * advantage for the application. The API is separated into separate
 * header files only for documentation convenience.
 *
 *
 * # Conventions
 *
 * ## MUST and SHALL
 *
 * In all descriptions, the usual "MUST", "SHALL", "MAY",... terminology
 * is used. Failure to meet requirements expressed with a "MUST" or
 * "SHALL" implies undefined behaviour, which means that segmentation
 * faults, buffer overflows, and other similar adverse events, may occur.
 *
 * In general, BearSSL is not very forgiving of programming errors, and
 * does not include much failsafes or error reporting when the problem
 * does not arise from external transient conditions, and can be fixed
 * only in the application code. This is done so in order to make the
 * total code footprint lighter.
 *
 *
 * ## `NULL` values
 *
 * Function parameters with a pointer type shall not be `NULL` unless
 * explicitly authorised by the documentation. As an exception, when
 * the pointer aims at a sequence of bytes and is accompanied with
 * a length parameter, and the length is zero (meaning that there is
 * no byte at all to retrieve), then the pointer may be `NULL` even if
 * not explicitly allowed.
 *
 *
 * ## Memory Allocation
 *
 * BearSSL does not perform dynamic memory allocation. This implies that
 * for any functionality that requires a non-transient state, the caller
 * is responsible for allocating the relevant context structure. Such
 * allocation can be done in any appropriate area, including static data
 * segments, the heap, and the stack, provided that proper alignment is
 * respected. The header files define these context structures
 * (including size and contents), so the C compiler should handle
 * alignment automatically.
 *
 * Since there is no dynamic resource allocation, there is also nothing to
 * release. When the calling code is done with a BearSSL feature, it
 * may simple release the context structures it allocated itself, with
 * no "close function" to call. If the context structures were allocated
 * on the stack (as local variables), then even that release operation is
 * implicit.
 *
 *
 * ## Structure Contents
 *
 * Except when explicitly indicated, structure contents are opaque: they
 * are included in the header files so that calling code may know the
 * structure sizes and alignment requirements, but callers SHALL NOT
 * access individual fields directly. For fields that are supposed to
 * be read from or written to, the API defines accessor functions (the
 * simplest of these accessor functions are defined as `static inline`
 * functions, and the C compiler will optimise them away).
 *
 *
 * # API Usage
 *
 * BearSSL usage for running a SSL/TLS client or server is described
 * on the [BearSSL Web site](https://www.bearssl.org/api1.html). The
 * BearSSL source archive also comes with sample code.
 */

/* (already included) */
/* (already included) */
/* (already included) */
/* (already included) */
/* (already included) */
/* (already included) */
/* (already included) */
/* (already included) */
/* (already included) */
/* (already included) */

#endif


/* ===== src/config.h ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef CONFIG_H__
#define CONFIG_H__

/*
 * This file contains compile-time flags that can override the
 * autodetection performed in relevant files. Each flag is a macro; it
 * deactivates the feature if defined to 0, activates it if defined to a
 * non-zero integer (normally 1). If the macro is not defined, then
 * autodetection applies.
 */

/*
 * When BR_64 is enabled, 64-bit integer types are assumed to be
 * efficient (i.e. the architecture has 64-bit registers and can
 * do 64-bit operations as fast as 32-bit operations).
 *
#define BR_64   1
 */

/*
 * When BR_SLOW_MUL is enabled, multiplications are assumed to be
 * substantially slow with regards to other integer operations, thus
 * making it worth to make more operations for a given task if it allows
 * using less multiplications.
 *
#define BR_SLOW_MUL   1
 */

/*
 * When BR_SLOW_MUL15 is enabled, short multplications (on 15-bit words)
 * are assumed to be substantially slow with regards to other integer
 * operations, thus making it worth to make more integer operations if
 * it allows using less multiplications.
 *
#define BR_SLOW_MUL15   1
 */

/*
 * When BR_CT_MUL31 is enabled, multiplications of 31-bit values (used
 * in the "i31" big integer implementation) use an alternate implementation
 * which is slower and larger than the normal multiplication, but should
 * ensure constant-time multiplications even on architectures where the
 * multiplication opcode takes a variable number of cycles to complete.
 *
#define BR_CT_MUL31   1
 */

/*
 * When BR_CT_MUL15 is enabled, multiplications of 15-bit values (held
 * in 32-bit words) use an alternate implementation which is slower and
 * larger than the normal multiplication, but should ensure
 * constant-time multiplications on most/all architectures where the
 * basic multiplication is not constant-time.
#define BR_CT_MUL15   1
 */

/*
 * When BR_NO_ARITH_SHIFT is enabled, arithmetic right shifts (with sign
 * extension) are performed with a sequence of operations which is bigger
 * and slower than a simple right shift on a signed value. This avoids
 * relying on an implementation-defined behaviour. However, most if not
 * all C compilers use sign extension for right shifts on signed values,
 * so this alternate macro is disabled by default.
#define BR_NO_ARITH_SHIFT   1
 */

/*
 * When BR_USE_URANDOM is enabled, the SSL engine will use /dev/urandom
 * to automatically obtain quality randomness for seedings its internal
 * PRNG.
 *
#define BR_USE_URANDOM   1
 */

/*
 * When BR_USE_WIN32_RAND is enabled, the SSL engine will use the Win32
 * (CryptoAPI) functions (CryptAcquireContext(), CryptGenRandom()...) to
 * automatically obtain quality randomness for seedings its internal PRNG.
 *
 * Note: if both BR_USE_URANDOM and BR_USE_WIN32_RAND are defined, the
 * former takes precedence.
 *
#define BR_USE_WIN32_RAND   1
 */

/*
 * When BR_USE_UNIX_TIME is enabled, the X.509 validation engine obtains
 * the current time from the OS by calling time(), and assuming that the
 * returned value (a 'time_t') is an integer that counts time in seconds
 * since the Unix Epoch (Jan 1st, 1970, 00:00 UTC).
 *
#define BR_USE_UNIX_TIME   1
 */

/*
 * When BR_USE_WIN32_TIME is enabled, the X.509 validation engine obtains
 * the current time from the OS by calling the Win32 function
 * GetSystemTimeAsFileTime().
 *
 * Note: if both BR_USE_UNIX_TIME and BR_USE_WIN32_TIME are defined, the
 * former takes precedence.
 *
#define BR_USE_WIN32_TIME   1
 */

#endif


/* ===== src/inner.h ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef INNER_H__
#define INNER_H__

#include <string.h>
#include <limits.h>

/* (already included) */
/* (already included) */

/*
 * Maximum size for a RSA modulus (in bits). Allocated stack buffers
 * depend on that size, so this value should be kept small. Currently,
 * 2048-bit RSA keys offer adequate security, and should still do so for
 * the next few decades; however, a number of widespread PKI have
 * already set their root keys to RSA-4096, so we should be able to
 * process such keys.
 *
 * This value MUST be a multiple of 64.
 */
#define BR_MAX_RSA_SIZE   4096

/*
 * Maximum size for a RSA factor (in bits). This is for RSA private-key
 * operations. Default is to support factors up to a bit more than half
 * the maximum modulus size.
 *
 * This value MUST be a multiple of 32.
 */
#define BR_MAX_RSA_FACTOR   ((BR_MAX_RSA_SIZE + 64) >> 1)

/*
 * Maximum size for an EC curve (modulus or order), in bits. Size of
 * stack buffers depends on that parameter. This size MUST be a multiple
 * of 8 (so that decoding an integer with that many bytes does not
 * overflow).
 */
#define BR_MAX_EC_SIZE   528

/*
 * Some macros to recognize the current architecture. Right now, we are
 * interested into automatically recognizing architecture with efficient
 * 64-bit types so that we may automatically use implementations that
 * use 64-bit registers in that case. Future versions may detect, e.g.,
 * availability of SSE2 intrinsics.
 *
 * If 'unsigned long' is a 64-bit type, then we assume that 64-bit types
 * are efficient. Otherwise, we rely on macros that depend on compiler,
 * OS and architecture. In any case, failure to detect the architecture
 * as 64-bit means that the 32-bit code will be used, and that code
 * works also on 64-bit architectures (the 64-bit code may simply be
 * more efficient).
 *
 * The test on 'unsigned long' should already catch most cases, the one
 * notable exception being Windows code where 'unsigned long' is kept to
 * 32-bit for compatbility with all the legacy code that liberally uses
 * the 'DWORD' type for 32-bit values.
 *
 * Macro names are taken from: http://nadeausoftware.com/articles/2012/02/c_c_tip_how_detect_processor_type_using_compiler_predefined_macros
 */
#ifndef BR_64
#if ((ULONG_MAX >> 31) >> 31) == 3
#define BR_64   1
#elif defined(__ia64) || defined(__itanium__) || defined(_M_IA64)
#define BR_64   1
#elif defined(__powerpc64__) || defined(__ppc64__) || defined(__PPC64__) \
	|| defined(__64BIT__) || defined(_LP64) || defined(__LP64__)
#define BR_64   1
#elif defined(__sparc64__)
#define BR_64   1
#elif defined(__x86_64__) || defined(_M_X64)
#define BR_64   1
#endif
#endif

/* ==================================================================== */
/*
 * Encoding/decoding functions.
 *
 * 32-bit and 64-bit decoding, both little-endian and big-endian, is
 * implemented with the inline functions below. These functions are
 * generic: they don't depend on the architecture natural endianness,
 * and they can handle unaligned accesses. Optimized versions for some
 * specific architectures may be implemented at a later time.
 */

static inline void
br_enc16le(void *dst, unsigned x)
{
	unsigned char *buf;

	buf = dst;
	buf[0] = (unsigned char)x;
	buf[1] = (unsigned char)(x >> 8);
}

static inline void
br_enc16be(void *dst, unsigned x)
{
	unsigned char *buf;

	buf = dst;
	buf[0] = (unsigned char)(x >> 8);
	buf[1] = (unsigned char)x;
}

static inline unsigned
br_dec16le(const void *src)
{
	const unsigned char *buf;

	buf = src;
	return (unsigned)buf[0] | ((unsigned)buf[1] << 8);
}

static inline unsigned
br_dec16be(const void *src)
{
	const unsigned char *buf;

	buf = src;
	return ((unsigned)buf[0] << 8) | (unsigned)buf[1];
}

static inline void
br_enc32le(void *dst, uint32_t x)
{
	unsigned char *buf;

	buf = dst;
	buf[0] = (unsigned char)x;
	buf[1] = (unsigned char)(x >> 8);
	buf[2] = (unsigned char)(x >> 16);
	buf[3] = (unsigned char)(x >> 24);
}

static inline void
br_enc32be(void *dst, uint32_t x)
{
	unsigned char *buf;

	buf = dst;
	buf[0] = (unsigned char)(x >> 24);
	buf[1] = (unsigned char)(x >> 16);
	buf[2] = (unsigned char)(x >> 8);
	buf[3] = (unsigned char)x;
}

static inline uint32_t
br_dec32le(const void *src)
{
	const unsigned char *buf;

	buf = src;
	return (uint32_t)buf[0]
		| ((uint32_t)buf[1] << 8)
		| ((uint32_t)buf[2] << 16)
		| ((uint32_t)buf[3] << 24);
}

static inline uint32_t
br_dec32be(const void *src)
{
	const unsigned char *buf;

	buf = src;
	return ((uint32_t)buf[0] << 24)
		| ((uint32_t)buf[1] << 16)
		| ((uint32_t)buf[2] << 8)
		| (uint32_t)buf[3];
}

static inline void
br_enc64le(void *dst, uint64_t x)
{
	unsigned char *buf;

	buf = dst;
	br_enc32le(buf, (uint32_t)x);
	br_enc32le(buf + 4, (uint32_t)(x >> 32));
}

static inline void
br_enc64be(void *dst, uint64_t x)
{
	unsigned char *buf;

	buf = dst;
	br_enc32be(buf, (uint32_t)(x >> 32));
	br_enc32be(buf + 4, (uint32_t)x);
}

static inline uint64_t
br_dec64le(const void *src)
{
	const unsigned char *buf;

	buf = src;
	return (uint64_t)br_dec32le(buf)
		| ((uint64_t)br_dec32le(buf + 4) << 32);
}

static inline uint64_t
br_dec64be(const void *src)
{
	const unsigned char *buf;

	buf = src;
	return ((uint64_t)br_dec32be(buf) << 32)
		| (uint64_t)br_dec32be(buf + 4);
}

/*
 * Range decoding and encoding (for several successive values).
 */
void br_range_dec16le(uint16_t *v, size_t num, const void *src);
void br_range_dec16be(uint16_t *v, size_t num, const void *src);
void br_range_enc16le(void *dst, const uint16_t *v, size_t num);
void br_range_enc16be(void *dst, const uint16_t *v, size_t num);

void br_range_dec32le(uint32_t *v, size_t num, const void *src);
void br_range_dec32be(uint32_t *v, size_t num, const void *src);
void br_range_enc32le(void *dst, const uint32_t *v, size_t num);
void br_range_enc32be(void *dst, const uint32_t *v, size_t num);

void br_range_dec64le(uint64_t *v, size_t num, const void *src);
void br_range_dec64be(uint64_t *v, size_t num, const void *src);
void br_range_enc64le(void *dst, const uint64_t *v, size_t num);
void br_range_enc64be(void *dst, const uint64_t *v, size_t num);

/*
 * Byte-swap a 32-bit integer.
 */
static inline uint32_t
br_swap32(uint32_t x)
{
	x = ((x & (uint32_t)0x00FF00FF) << 8)
		| ((x >> 8) & (uint32_t)0x00FF00FF);
	return (x << 16) | (x >> 16);
}

/* ==================================================================== */
/*
 * Support code for hash functions.
 */

/*
 * IV for MD5, SHA-1, SHA-224 and SHA-256.
 */
extern const uint32_t br_md5_IV[];
extern const uint32_t br_sha1_IV[];
extern const uint32_t br_sha224_IV[];
extern const uint32_t br_sha256_IV[];

/*
 * Round functions for MD5, SHA-1, SHA-224 and SHA-256 (SHA-224 and
 * SHA-256 use the same round function).
 */
void br_md5_round(const unsigned char *buf, uint32_t *val);
void br_sha1_round(const unsigned char *buf, uint32_t *val);
void br_sha2small_round(const unsigned char *buf, uint32_t *val);

/*
 * The core function for the TLS PRF. It computes
 * P_hash(secret, label + seed), and XORs the result into the dst buffer.
 */
void br_tls_phash(void *dst, size_t len,
	const br_hash_class *dig,
	const void *secret, size_t secret_len,
	const char *label, const void *seed, size_t seed_len);

/*
 * Copy all configured hash implementations from a multihash context
 * to another.
 */
static inline void
br_multihash_copyimpl(br_multihash_context *dst,
	const br_multihash_context *src)
{
	memcpy(dst->impl, src->impl, sizeof src->impl);
}

/* ==================================================================== */
/*
 * Constant-time primitives. These functions manipulate 32-bit values in
 * order to provide constant-time comparisons and multiplexers.
 *
 * Boolean values (the "ctl" bits) MUST have value 0 or 1.
 *
 * Implementation notes:
 * =====================
 *
 * The uintN_t types are unsigned and with width exactly N bits; the C
 * standard guarantees that computations are performed modulo 2^N, and
 * there can be no overflow. Negation (unary '-') works on unsigned types
 * as well.
 *
 * The intN_t types are guaranteed to have width exactly N bits, with no
 * padding bit, and using two's complement representation. Casting
 * intN_t to uintN_t really is conversion modulo 2^N. Beware that intN_t
 * types, being signed, trigger implementation-defined behaviour on
 * overflow (including raising some signal): with GCC, while modular
 * arithmetics are usually applied, the optimizer may assume that
 * overflows don't occur (unless the -fwrapv command-line option is
 * added); Clang has the additional -ftrapv option to explicitly trap on
 * integer overflow or underflow.
 */

/*
 * Negate a boolean.
 */
static inline uint32_t
NOT(uint32_t ctl)
{
	return ctl ^ 1;
}

/*
 * Multiplexer: returns x if ctl == 1, y if ctl == 0.
 */
static inline uint32_t
MUX(uint32_t ctl, uint32_t x, uint32_t y)
{
	return y ^ (-ctl & (x ^ y));
}

/*
 * Equality check: returns 1 if x == y, 0 otherwise.
 */
static inline uint32_t
EQ(uint32_t x, uint32_t y)
{
	uint32_t q;

	q = x ^ y;
	return NOT((q | -q) >> 31);
}

/*
 * Inequality check: returns 1 if x != y, 0 otherwise.
 */
static inline uint32_t
NEQ(uint32_t x, uint32_t y)
{
	uint32_t q;

	q = x ^ y;
	return (q | -q) >> 31;
}

/*
 * Comparison: returns 1 if x > y, 0 otherwise.
 */
static inline uint32_t
GT(uint32_t x, uint32_t y)
{
	/*
	 * If both x < 2^31 and x < 2^31, then y-x will have its high
	 * bit set if x > y, cleared otherwise.
	 *
	 * If either x >= 2^31 or y >= 2^31 (but not both), then the
	 * result is the high bit of x.
	 *
	 * If both x >= 2^31 and y >= 2^31, then we can virtually
	 * subtract 2^31 from both, and we are back to the first case.
	 * Since (y-2^31)-(x-2^31) = y-x, the subtraction is already
	 * fine.
	 */
	uint32_t z;

	z = y - x;
	return (z ^ ((x ^ y) & (x ^ z))) >> 31;
}

/*
 * Other comparisons (greater-or-equal, lower-than, lower-or-equal).
 */
#define GE(x, y)   NOT(GT(y, x))
#define LT(x, y)   GT(y, x)
#define LE(x, y)   NOT(GT(x, y))

/*
 * General comparison: returned value is -1, 0 or 1, depending on
 * whether x is lower than, equal to, or greater than y.
 */
static inline int32_t
CMP(uint32_t x, uint32_t y)
{
	return (int32_t)GT(x, y) | -(int32_t)GT(y, x);
}

/*
 * Returns 1 if x == 0, 0 otherwise. Take care that the operand is signed.
 */
static inline uint32_t
EQ0(int32_t x)
{
	uint32_t q;

	q = (uint32_t)x;
	return ~(q | -q) >> 31;
}

/*
 * Returns 1 if x > 0, 0 otherwise. Take care that the operand is signed.
 */
static inline uint32_t
GT0(int32_t x)
{
	/*
	 * High bit of -x is 0 if x == 0, but 1 if x > 0.
	 */
	uint32_t q;

	q = (uint32_t)x;
	return (~q & -q) >> 31;
}

/*
 * Returns 1 if x >= 0, 0 otherwise. Take care that the operand is signed.
 */
static inline uint32_t
GE0(int32_t x)
{
	return ~(uint32_t)x >> 31;
}

/*
 * Returns 1 if x < 0, 0 otherwise. Take care that the operand is signed.
 */
static inline uint32_t
LT0(int32_t x)
{
	return (uint32_t)x >> 31;
}

/*
 * Returns 1 if x <= 0, 0 otherwise. Take care that the operand is signed.
 */
static inline uint32_t
LE0(int32_t x)
{
	uint32_t q;

	/*
	 * ~-x has its high bit set if and only if -x is nonnegative (as
	 * a signed int), i.e. x is in the -(2^31-1) to 0 range. We must
	 * do an OR with x itself to account for x = -2^31.
	 */
	q = (uint32_t)x;
	return (q | ~-q) >> 31;
}

/*
 * Conditional copy: src[] is copied into dst[] if and only if ctl is 1.
 * dst[] and src[] may overlap completely (but not partially).
 */
void br_ccopy(uint32_t ctl, void *dst, const void *src, size_t len);

#define CCOPY   br_ccopy

/*
 * Compute the bit length of a 32-bit integer. Returned value is between 0
 * and 32 (inclusive).
 */
static inline uint32_t
BIT_LENGTH(uint32_t x)
{
	uint32_t k, c;

	k = NEQ(x, 0);
	c = GT(x, 0xFFFF); x = MUX(c, x >> 16, x); k += c << 4;
	c = GT(x, 0x00FF); x = MUX(c, x >>  8, x); k += c << 3;
	c = GT(x, 0x000F); x = MUX(c, x >>  4, x); k += c << 2;
	c = GT(x, 0x0003); x = MUX(c, x >>  2, x); k += c << 1;
	k += GT(x, 0x0001);
	return k;
}

/*
 * Compute the minimum of x and y.
 */
static inline uint32_t
MIN(uint32_t x, uint32_t y)
{
	return MUX(GT(x, y), y, x);
}

/*
 * Compute the maximum of x and y.
 */
static inline uint32_t
MAX(uint32_t x, uint32_t y)
{
	return MUX(GT(x, y), x, y);
}

/*
 * Multiply two 32-bit integers, with a 64-bit result. This default
 * implementation assumes that the basic multiplication operator
 * yields constant-time code.
 */
#define MUL(x, y)   ((uint64_t)(x) * (uint64_t)(y))

#if BR_CT_MUL31

/*
 * Alternate implementation of MUL31, that will be constant-time on some
 * (old) platforms where the default MUL31 is not. Unfortunately, it is
 * also substantially slower, and yields larger code, on more modern
 * platforms, which is why it is deactivated by default.
 *
 * MUL31_lo() must do some extra work because on some platforms, the
 * _signed_ multiplication may return early if the top bits are 1.
 * Simply truncating (casting) the output of MUL31() would not be
 * sufficient, because the compiler may notice that we keep only the low
 * word, and then replace automatically the unsigned multiplication with
 * a signed multiplication opcode.
 */
#define MUL31(x, y)   ((uint64_t)((x) | (uint32_t)0x80000000) \
                       * (uint64_t)((y) | (uint32_t)0x80000000) \
                       - ((uint64_t)(x) << 31) - ((uint64_t)(y) << 31) \
                       - ((uint64_t)1 << 62))
static inline uint32_t
MUL31_lo(uint32_t x, uint32_t y)
{
	uint32_t xl, xh;
	uint32_t yl, yh;

	xl = (x & 0xFFFF) | (uint32_t)0x80000000;
	xh = (x >> 16) | (uint32_t)0x80000000;
	yl = (y & 0xFFFF) | (uint32_t)0x80000000;
	yh = (y >> 16) | (uint32_t)0x80000000;
	return (xl * yl + ((xl * yh + xh * yl) << 16)) & (uint32_t)0x7FFFFFFF;
}

#else

/*
 * Multiply two 31-bit integers, with a 62-bit result. This default
 * implementation assumes that the basic multiplication operator
 * yields constant-time code.
 * The MUL31_lo() macro returns only the low 31 bits of the product.
 */
#define MUL31(x, y)     ((uint64_t)(x) * (uint64_t)(y))
#define MUL31_lo(x, y)  (((uint32_t)(x) * (uint32_t)(y)) & (uint32_t)0x7FFFFFFF)

#endif

/*
 * Multiply two words together; the sum of the lengths of the two
 * operands must not exceed 31 (for instance, one operand may use 16
 * bits if the other fits on 15). If BR_CT_MUL15 is non-zero, then the
 * macro will contain some extra operations that help in making the
 * operation constant-time on some platforms, where the basic 32-bit
 * multiplication is not constant-time.
 */
#if BR_CT_MUL15
#define MUL15(x, y)   (((uint32_t)(x) | (uint32_t)0x80000000) \
                       * ((uint32_t)(y) | (uint32_t)0x80000000) \
		       & (uint32_t)0x7FFFFFFF)
#else
#define MUL15(x, y)   ((uint32_t)(x) * (uint32_t)(y))
#endif

/*
 * Arithmetic right shift (sign bit is copied). What happens when
 * right-shifting a negative value is _implementation-defined_, so it
 * does not trigger undefined behaviour, but it is still up to each
 * compiler to define (and document) what it does. Most/all compilers
 * will do an arithmetic shift, the sign bit being used to fill the
 * holes; this is a native operation on the underlying CPU, and it would
 * make little sense for the compiler to do otherwise. GCC explicitly
 * documents that it follows that convention.
 *
 * Still, if BR_NO_ARITH_SHIFT is defined (and non-zero), then an
 * alternate version will be used, that does not rely on such
 * implementation-defined behaviour. Unfortunately, it is also slower
 * and yields bigger code, which is why it is deactivated by default.
 */
#if BR_NO_ARITH_SHIFT
#define ARSH(x, n)   (((uint32_t)(x) >> (n)) \
                      | ((-((uint32_t)(x) >> 31)) << (32 - (n))))
#else
#define ARSH(x, n)   ((*(int32_t *)&(x)) >> (n))
#endif

/*
 * Constant-time division. The dividend hi:lo is divided by the
 * divisor d; the quotient is returned and the remainder is written
 * in *r. If hi == d, then the quotient does not fit on 32 bits;
 * returned value is thus truncated. If hi > d, returned values are
 * indeterminate.
 */
uint32_t br_divrem(uint32_t hi, uint32_t lo, uint32_t d, uint32_t *r);

/*
 * Wrapper for br_divrem(); the remainder is returned, and the quotient
 * is discarded.
 */
static inline uint32_t
br_rem(uint32_t hi, uint32_t lo, uint32_t d)
{
	uint32_t r;

	br_divrem(hi, lo, d, &r);
	return r;
}

/*
 * Wrapper for br_divrem(); the quotient is returned, and the remainder
 * is discarded.
 */
static inline uint32_t
br_div(uint32_t hi, uint32_t lo, uint32_t d)
{
	uint32_t r;

	return br_divrem(hi, lo, d, &r);
}

/* ==================================================================== */

/*
 * Integers 'i32'
 * --------------
 *
 * The 'i32' functions implement computations on big integers using
 * an internal representation as an array of 32-bit integers. For
 * an array x[]:
 *  -- x[0] contains the "announced bit length" of the integer
 *  -- x[1], x[2]... contain the value in little-endian order (x[1]
 *     contains the least significant 32 bits)
 *
 * Multiplications rely on the elementary 32x32->64 multiplication.
 *
 * The announced bit length specifies the number of bits that are
 * significant in the subsequent 32-bit words. Unused bits in the
 * last (most significant) word are set to 0; subsequent words are
 * uninitialized and need not exist at all.
 *
 * The execution time and memory access patterns of all computations
 * depend on the announced bit length, but not on the actual word
 * values. For modular integers, the announced bit length of any integer
 * modulo n is equal to the actual bit length of n; thus, computations
 * on modular integers are "constant-time" (only the modulus length may
 * leak).
 */

/*
 * Compute the actual bit length of an integer. The argument x should
 * point to the first (least significant) value word of the integer.
 * The len 'xlen' contains the number of 32-bit words to access.
 *
 * CT: value or length of x does not leak.
 */
uint32_t br_i32_bit_length(uint32_t *x, size_t xlen);

/*
 * Decode an integer from its big-endian unsigned representation. The
 * "true" bit length of the integer is computed, but all words of x[]
 * corresponding to the full 'len' bytes of the source are set.
 *
 * CT: value or length of x does not leak.
 */
void br_i32_decode(uint32_t *x, const void *src, size_t len);

/*
 * Decode an integer from its big-endian unsigned representation. The
 * integer MUST be lower than m[]; the announced bit length written in
 * x[] will be equal to that of m[]. All 'len' bytes from the source are
 * read.
 *
 * Returned value is 1 if the decode value fits within the modulus, 0
 * otherwise. In the latter case, the x[] buffer will be set to 0 (but
 * still with the announced bit length of m[]).
 *
 * CT: value or length of x does not leak. Memory access pattern depends
 * only of 'len' and the announced bit length of m. Whether x fits or
 * not does not leak either.
 */
uint32_t br_i32_decode_mod(uint32_t *x,
	const void *src, size_t len, const uint32_t *m);

/*
 * Reduce an integer (a[]) modulo another (m[]). The result is written
 * in x[] and its announced bit length is set to be equal to that of m[].
 *
 * x[] MUST be distinct from a[] and m[].
 *
 * CT: only announced bit lengths leak, not values of x, a or m.
 */
void br_i32_reduce(uint32_t *x, const uint32_t *a, const uint32_t *m);

/*
 * Decode an integer from its big-endian unsigned representation, and
 * reduce it modulo the provided modulus m[]. The announced bit length
 * of the result is set to be equal to that of the modulus.
 *
 * x[] MUST be distinct from m[].
 */
void br_i32_decode_reduce(uint32_t *x,
	const void *src, size_t len, const uint32_t *m);

/*
 * Encode an integer into its big-endian unsigned representation. The
 * output length in bytes is provided (parameter 'len'); if the length
 * is too short then the integer is appropriately truncated; if it is
 * too long then the extra bytes are set to 0.
 */
void br_i32_encode(void *dst, size_t len, const uint32_t *x);

/*
 * Multiply x[] by 2^32 and then add integer z, modulo m[]. This
 * function assumes that x[] and m[] have the same announced bit
 * length, and the announced bit length of m[] matches its true
 * bit length.
 *
 * x[] and m[] MUST be distinct arrays.
 *
 * CT: only the common announced bit length of x and m leaks, not
 * the values of x, z or m.
 */
void br_i32_muladd_small(uint32_t *x, uint32_t z, const uint32_t *m);

/*
 * Extract one word from an integer. The offset is counted in bits.
 * The word MUST entirely fit within the word elements corresponding
 * to the announced bit length of a[].
 */
static inline uint32_t
br_i32_word(const uint32_t *a, uint32_t off)
{
	size_t u;
	unsigned j;

	u = (size_t)(off >> 5) + 1;
	j = (unsigned)off & 31;
	if (j == 0) {
		return a[u];
	} else {
		return (a[u] >> j) | (a[u + 1] << (32 - j));
	}
}

/*
 * Test whether an integer is zero.
 */
uint32_t br_i32_iszero(const uint32_t *x);

/*
 * Add b[] to a[] and return the carry (0 or 1). If ctl is 0, then a[]
 * is unmodified, but the carry is still computed and returned. The
 * arrays a[] and b[] MUST have the same announced bit length.
 *
 * a[] and b[] MAY be the same array, but partial overlap is not allowed.
 */
uint32_t br_i32_add(uint32_t *a, const uint32_t *b, uint32_t ctl);

/*
 * Subtract b[] from a[] and return the carry (0 or 1). If ctl is 0,
 * then a[] is unmodified, but the carry is still computed and returned.
 * The arrays a[] and b[] MUST have the same announced bit length.
 *
 * a[] and b[] MAY be the same array, but partial overlap is not allowed.
 */
uint32_t br_i32_sub(uint32_t *a, const uint32_t *b, uint32_t ctl);

/*
 * Compute d+a*b, result in d. The initial announced bit length of d[]
 * MUST match that of a[]. The d[] array MUST be large enough to
 * accommodate the full result, plus (possibly) an extra word. The
 * resulting announced bit length of d[] will be the sum of the announced
 * bit lengths of a[] and b[] (therefore, it may be larger than the actual
 * bit length of the numerical result).
 *
 * a[] and b[] may be the same array. d[] must be disjoint from both a[]
 * and b[].
 */
void br_i32_mulacc(uint32_t *d, const uint32_t *a, const uint32_t *b);

/*
 * Zeroize an integer. The announced bit length is set to the provided
 * value, and the corresponding words are set to 0.
 */
static inline void
br_i32_zero(uint32_t *x, uint32_t bit_len)
{
	*x ++ = bit_len;
	memset(x, 0, ((bit_len + 31) >> 5) * sizeof *x);
}

/*
 * Compute -(1/x) mod 2^32. If x is even, then this function returns 0.
 */
uint32_t br_i32_ninv32(uint32_t x);

/*
 * Convert a modular integer to Montgomery representation. The integer x[]
 * MUST be lower than m[], but with the same announced bit length.
 */
void br_i32_to_monty(uint32_t *x, const uint32_t *m);

/*
 * Convert a modular integer back from Montgomery representation. The
 * integer x[] MUST be lower than m[], but with the same announced bit
 * length. The "m0i" parameter is equal to -(1/m0) mod 2^32, where m0 is
 * the least significant value word of m[] (this works only if m[] is
 * an odd integer).
 */
void br_i32_from_monty(uint32_t *x, const uint32_t *m, uint32_t m0i);

/*
 * Compute a modular Montgomery multiplication. d[] is filled with the
 * value of x*y/R modulo m[] (where R is the Montgomery factor). The
 * array d[] MUST be distinct from x[], y[] and m[]. x[] and y[] MUST be
 * numerically lower than m[]. x[] and y[] MAY be the same array. The
 * "m0i" parameter is equal to -(1/m0) mod 2^32, where m0 is the least
 * significant value word of m[] (this works only if m[] is an odd
 * integer).
 */
void br_i32_montymul(uint32_t *d, const uint32_t *x, const uint32_t *y,
	const uint32_t *m, uint32_t m0i);

/*
 * Compute a modular exponentiation. x[] MUST be an integer modulo m[]
 * (same announced bit length, lower value). m[] MUST be odd. The
 * exponent is in big-endian unsigned notation, over 'elen' bytes. The
 * "m0i" parameter is equal to -(1/m0) mod 2^32, where m0 is the least
 * significant value word of m[] (this works only if m[] is an odd
 * integer). The t1[] and t2[] parameters must be temporary arrays,
 * each large enough to accommodate an integer with the same size as m[].
 */
void br_i32_modpow(uint32_t *x, const unsigned char *e, size_t elen,
	const uint32_t *m, uint32_t m0i, uint32_t *t1, uint32_t *t2);

/* ==================================================================== */

/*
 * Integers 'i31'
 * --------------
 *
 * The 'i31' functions implement computations on big integers using
 * an internal representation as an array of 32-bit integers. For
 * an array x[]:
 *  -- x[0] encodes the array length and the "announced bit length"
 *     of the integer: namely, if the announced bit length is k,
 *     then x[0] = ((k / 31) << 5) + (k % 31).
 *  -- x[1], x[2]... contain the value in little-endian order, 31
 *     bits per word (x[1] contains the least significant 31 bits).
 *     The upper bit of each word is 0.
 *
 * Multiplications rely on the elementary 32x32->64 multiplication.
 *
 * The announced bit length specifies the number of bits that are
 * significant in the subsequent 32-bit words. Unused bits in the
 * last (most significant) word are set to 0; subsequent words are
 * uninitialized and need not exist at all.
 *
 * The execution time and memory access patterns of all computations
 * depend on the announced bit length, but not on the actual word
 * values. For modular integers, the announced bit length of any integer
 * modulo n is equal to the actual bit length of n; thus, computations
 * on modular integers are "constant-time" (only the modulus length may
 * leak).
 */

/*
 * Test whether an integer is zero.
 */
uint32_t br_i31_iszero(const uint32_t *x);

/*
 * Add b[] to a[] and return the carry (0 or 1). If ctl is 0, then a[]
 * is unmodified, but the carry is still computed and returned. The
 * arrays a[] and b[] MUST have the same announced bit length.
 *
 * a[] and b[] MAY be the same array, but partial overlap is not allowed.
 */
uint32_t br_i31_add(uint32_t *a, const uint32_t *b, uint32_t ctl);

/*
 * Subtract b[] from a[] and return the carry (0 or 1). If ctl is 0,
 * then a[] is unmodified, but the carry is still computed and returned.
 * The arrays a[] and b[] MUST have the same announced bit length.
 *
 * a[] and b[] MAY be the same array, but partial overlap is not allowed.
 */
uint32_t br_i31_sub(uint32_t *a, const uint32_t *b, uint32_t ctl);

/*
 * Compute the ENCODED actual bit length of an integer. The argument x
 * should point to the first (least significant) value word of the
 * integer. The len 'xlen' contains the number of 32-bit words to
 * access. The upper bit of each value word MUST be 0.
 * Returned value is ((k / 31) << 5) + (k % 31) if the bit length is k.
 *
 * CT: value or length of x does not leak.
 */
uint32_t br_i31_bit_length(uint32_t *x, size_t xlen);

/*
 * Decode an integer from its big-endian unsigned representation. The
 * "true" bit length of the integer is computed and set in the encoded
 * announced bit length (x[0]), but all words of x[] corresponding to
 * the full 'len' bytes of the source are set.
 *
 * CT: value or length of x does not leak.
 */
void br_i31_decode(uint32_t *x, const void *src, size_t len);

/*
 * Decode an integer from its big-endian unsigned representation. The
 * integer MUST be lower than m[]; the (encoded) announced bit length
 * written in x[] will be equal to that of m[]. All 'len' bytes from the
 * source are read.
 *
 * Returned value is 1 if the decode value fits within the modulus, 0
 * otherwise. In the latter case, the x[] buffer will be set to 0 (but
 * still with the announced bit length of m[]).
 *
 * CT: value or length of x does not leak. Memory access pattern depends
 * only of 'len' and the announced bit length of m. Whether x fits or
 * not does not leak either.
 */
uint32_t br_i31_decode_mod(uint32_t *x,
	const void *src, size_t len, const uint32_t *m);

/*
 * Zeroize an integer. The announced bit length is set to the provided
 * value, and the corresponding words are set to 0. The ENCODED bit length
 * is expected here.
 */
static inline void
br_i31_zero(uint32_t *x, uint32_t bit_len)
{
	*x ++ = bit_len;
	memset(x, 0, ((bit_len + 31) >> 5) * sizeof *x);
}

/*
 * Right-shift an integer. The shift amount must be lower than 31
 * bits.
 */
void br_i31_rshift(uint32_t *x, int count);

/*
 * Reduce an integer (a[]) modulo another (m[]). The result is written
 * in x[] and its announced bit length is set to be equal to that of m[].
 *
 * x[] MUST be distinct from a[] and m[].
 *
 * CT: only announced bit lengths leak, not values of x, a or m.
 */
void br_i31_reduce(uint32_t *x, const uint32_t *a, const uint32_t *m);

/*
 * Decode an integer from its big-endian unsigned representation, and
 * reduce it modulo the provided modulus m[]. The announced bit length
 * of the result is set to be equal to that of the modulus.
 *
 * x[] MUST be distinct from m[].
 */
void br_i31_decode_reduce(uint32_t *x,
	const void *src, size_t len, const uint32_t *m);

/*
 * Multiply x[] by 2^31 and then add integer z, modulo m[]. This
 * function assumes that x[] and m[] have the same announced bit
 * length, the announced bit length of m[] matches its true
 * bit length.
 *
 * x[] and m[] MUST be distinct arrays. z MUST fit in 31 bits (upper
 * bit set to 0).
 *
 * CT: only the common announced bit length of x and m leaks, not
 * the values of x, z or m.
 */
void br_i31_muladd_small(uint32_t *x, uint32_t z, const uint32_t *m);

/*
 * Encode an integer into its big-endian unsigned representation. The
 * output length in bytes is provided (parameter 'len'); if the length
 * is too short then the integer is appropriately truncated; if it is
 * too long then the extra bytes are set to 0.
 */
void br_i31_encode(void *dst, size_t len, const uint32_t *x);

/*
 * Compute -(1/x) mod 2^31. If x is even, then this function returns 0.
 */
uint32_t br_i31_ninv31(uint32_t x);

/*
 * Compute a modular Montgomery multiplication. d[] is filled with the
 * value of x*y/R modulo m[] (where R is the Montgomery factor). The
 * array d[] MUST be distinct from x[], y[] and m[]. x[] and y[] MUST be
 * numerically lower than m[]. x[] and y[] MAY be the same array. The
 * "m0i" parameter is equal to -(1/m0) mod 2^31, where m0 is the least
 * significant value word of m[] (this works only if m[] is an odd
 * integer).
 */
void br_i31_montymul(uint32_t *d, const uint32_t *x, const uint32_t *y,
	const uint32_t *m, uint32_t m0i);

/*
 * Convert a modular integer to Montgomery representation. The integer x[]
 * MUST be lower than m[], but with the same announced bit length.
 */
void br_i31_to_monty(uint32_t *x, const uint32_t *m);

/*
 * Convert a modular integer back from Montgomery representation. The
 * integer x[] MUST be lower than m[], but with the same announced bit
 * length. The "m0i" parameter is equal to -(1/m0) mod 2^32, where m0 is
 * the least significant value word of m[] (this works only if m[] is
 * an odd integer).
 */
void br_i31_from_monty(uint32_t *x, const uint32_t *m, uint32_t m0i);

/*
 * Compute a modular exponentiation. x[] MUST be an integer modulo m[]
 * (same announced bit length, lower value). m[] MUST be odd. The
 * exponent is in big-endian unsigned notation, over 'elen' bytes. The
 * "m0i" parameter is equal to -(1/m0) mod 2^31, where m0 is the least
 * significant value word of m[] (this works only if m[] is an odd
 * integer). The t1[] and t2[] parameters must be temporary arrays,
 * each large enough to accommodate an integer with the same size as m[].
 */
void br_i31_modpow(uint32_t *x, const unsigned char *e, size_t elen,
	const uint32_t *m, uint32_t m0i, uint32_t *t1, uint32_t *t2);

/*
 * Compute d+a*b, result in d. The initial announced bit length of d[]
 * MUST match that of a[]. The d[] array MUST be large enough to
 * accommodate the full result, plus (possibly) an extra word. The
 * resulting announced bit length of d[] will be the sum of the announced
 * bit lengths of a[] and b[] (therefore, it may be larger than the actual
 * bit length of the numerical result).
 *
 * a[] and b[] may be the same array. d[] must be disjoint from both a[]
 * and b[].
 */
void br_i31_mulacc(uint32_t *d, const uint32_t *a, const uint32_t *b);

/* ==================================================================== */

static inline void
br_i15_zero(uint16_t *x, uint16_t bit_len)
{
	*x ++ = bit_len;
	memset(x, 0, ((bit_len + 15) >> 4) * sizeof *x);
}

uint32_t br_i15_iszero(const uint16_t *x);

uint16_t br_i15_ninv15(uint16_t x);

uint32_t br_i15_add(uint16_t *a, const uint16_t *b, uint32_t ctl);

uint32_t br_i15_sub(uint16_t *a, const uint16_t *b, uint32_t ctl);

void br_i15_muladd_small(uint16_t *x, uint16_t z, const uint16_t *m);

void br_i15_montymul(uint16_t *d, const uint16_t *x, const uint16_t *y,
	const uint16_t *m, uint16_t m0i);

void br_i15_to_monty(uint16_t *x, const uint16_t *m);

void br_i15_modpow(uint16_t *x, const unsigned char *e, size_t elen,
	const uint16_t *m, uint16_t m0i, uint16_t *t1, uint16_t *t2);

void br_i15_encode(void *dst, size_t len, const uint16_t *x);

uint32_t br_i15_decode_mod(uint16_t *x,
	const void *src, size_t len, const uint16_t *m);

void br_i15_rshift(uint16_t *x, int count);

uint32_t br_i15_bit_length(uint16_t *x, size_t xlen);

void br_i15_decode(uint16_t *x, const void *src, size_t len);

void br_i15_from_monty(uint16_t *x, const uint16_t *m, uint16_t m0i);

void br_i15_decode_reduce(uint16_t *x,
	const void *src, size_t len, const uint16_t *m);

void br_i15_reduce(uint16_t *x, const uint16_t *a, const uint16_t *m);

void br_i15_mulacc(uint16_t *d, const uint16_t *a, const uint16_t *b);

/* ==================================================================== */

static inline size_t
br_digest_size(const br_hash_class *digest_class)
{
	return (size_t)(digest_class->desc >> BR_HASHDESC_OUT_OFF)
		& BR_HASHDESC_OUT_MASK;
}

/*
 * Get the output size (in bytes) of a hash function.
 */
size_t br_digest_size_by_ID(int digest_id);

/*
 * Get the OID (encoded OBJECT IDENTIFIER value, without tag and length)
 * for a hash function. If digest_id is not a supported digest identifier
 * (in particular if it is equal to 0, i.e. br_md5sha1_ID), then NULL is
 * returned and *len is set to 0.
 */
const unsigned char *br_digest_OID(int digest_id, size_t *len);

/* ==================================================================== */
/*
 * DES support functions.
 */

/*
 * Apply DES Initial Permutation.
 */
void br_des_do_IP(uint32_t *xl, uint32_t *xr);

/*
 * Apply DES Final Permutation (inverse of IP).
 */
void br_des_do_invIP(uint32_t *xl, uint32_t *xr);

/*
 * Key schedule unit: for a DES key (8 bytes), compute 16 subkeys. Each
 * subkey is two 28-bit words represented as two 32-bit words; the PC-2
 * bit extration is NOT applied.
 */
void br_des_keysched_unit(uint32_t *skey, const void *key);

/*
 * Reversal of 16 DES sub-keys (for decryption).
 */
void br_des_rev_skey(uint32_t *skey);

/*
 * DES/3DES key schedule for 'des_tab' (encryption direction). Returned
 * value is the number of rounds.
 */
unsigned br_des_tab_keysched(uint32_t *skey, const void *key, size_t key_len);

/*
 * DES/3DES key schedule for 'des_ct' (encryption direction). Returned
 * value is the number of rounds.
 */
unsigned br_des_ct_keysched(uint32_t *skey, const void *key, size_t key_len);

/*
 * DES/3DES subkey decompression (from the compressed bitsliced subkeys).
 */
void br_des_ct_skey_expand(uint32_t *sk_exp,
	unsigned num_rounds, const uint32_t *skey);

/*
 * DES/3DES block encryption/decryption ('des_tab').
 */
void br_des_tab_process_block(unsigned num_rounds,
	const uint32_t *skey, void *block);

/*
 * DES/3DES block encryption/decryption ('des_ct').
 */
void br_des_ct_process_block(unsigned num_rounds,
	const uint32_t *skey, void *block);

/* ==================================================================== */
/*
 * AES support functions.
 */

/*
 * The AES S-box (256-byte table).
 */
extern const unsigned char br_aes_S[];

/*
 * AES key schedule. skey[] is filled with n+1 128-bit subkeys, where n
 * is the number of rounds (10 to 14, depending on key size). The number
 * of rounds is returned. If the key size is invalid (not 16, 24 or 32),
 * then 0 is returned.
 *
 * This implementation uses a 256-byte table and is NOT constant-time.
 */
unsigned br_aes_keysched(uint32_t *skey, const void *key, size_t key_len);

/*
 * AES key schedule for decryption ('aes_big' implementation).
 */
unsigned br_aes_big_keysched_inv(uint32_t *skey,
	const void *key, size_t key_len);

/*
 * AES block encryption with the 'aes_big' implementation (fast, but
 * not constant-time). This function encrypts a single block "in place".
 */
void br_aes_big_encrypt(unsigned num_rounds, const uint32_t *skey, void *data);

/*
 * AES block decryption with the 'aes_big' implementation (fast, but
 * not constant-time). This function decrypts a single block "in place".
 */
void br_aes_big_decrypt(unsigned num_rounds, const uint32_t *skey, void *data);

/*
 * AES block encryption with the 'aes_small' implementation (small, but
 * slow and not constant-time). This function encrypts a single block
 * "in place".
 */
void br_aes_small_encrypt(unsigned num_rounds,
	const uint32_t *skey, void *data);

/*
 * AES block decryption with the 'aes_small' implementation (small, but
 * slow and not constant-time). This function decrypts a single block
 * "in place".
 */
void br_aes_small_decrypt(unsigned num_rounds,
	const uint32_t *skey, void *data);

/*
 * The constant-time implementation is "bitsliced": the 128-bit state is
 * split over eight 32-bit words q* in the following way:
 *
 * -- Input block consists in 16 bytes:
 *    a00 a10 a20 a30 a01 a11 a21 a31 a02 a12 a22 a32 a03 a13 a23 a33
 * In the terminology of FIPS 197, this is a 4x4 matrix which is read
 * column by column.
 *
 * -- Each byte is split into eight bits which are distributed over the
 * eight words, at the same rank. Thus, for a byte x at rank k, bit 0
 * (least significant) of x will be at rank k in q0 (if that bit is b,
 * then it contributes "b << k" to the value of q0), bit 1 of x will be
 * at rank k in q1, and so on.
 *
 * -- Ranks given to bits are in "row order" and are either all even, or
 * all odd. Two independent AES states are thus interleaved, one using
 * the even ranks, the other the odd ranks. Row order means:
 *    a00 a01 a02 a03 a10 a11 a12 a13 a20 a21 a22 a23 a30 a31 a32 a33
 *
 * Converting input bytes from two AES blocks to bitslice representation
 * is done in the following way:
 * -- Decode first block into the four words q0 q2 q4 q6, in that order,
 * using little-endian convention.
 * -- Decode second block into the four words q1 q3 q5 q7, in that order,
 * using little-endian convention.
 * -- Call br_aes_ct_ortho().
 *
 * Converting back to bytes is done by using the reverse operations. Note
 * that br_aes_ct_ortho() is its own inverse.
 */

/*
 * Perform bytewise orthogonalization of eight 32-bit words. Bytes
 * of q0..q7 are spread over all words: for a byte x that occurs
 * at rank i in q[j] (byte x uses bits 8*i to 8*i+7 in q[j]), the bit
 * of rank k in x (0 <= k <= 7) goes to q[k] at rank 8*i+j.
 *
 * This operation is an involution.
 */
void br_aes_ct_ortho(uint32_t *q);

/*
 * The AES S-box, as a bitsliced constant-time version. The input array
 * consists in eight 32-bit words; 32 S-box instances are computed in
 * parallel. Bits 0 to 7 of each S-box input (bit 0 is least significant)
 * are spread over the words 0 to 7, at the same rank.
 */
void br_aes_ct_bitslice_Sbox(uint32_t *q);

/*
 * Like br_aes_bitslice_Sbox(), but for the inverse S-box.
 */
void br_aes_ct_bitslice_invSbox(uint32_t *q);

/*
 * Compute AES encryption on bitsliced data. Since input is stored on
 * eight 32-bit words, two block encryptions are actually performed
 * in parallel.
 */
void br_aes_ct_bitslice_encrypt(unsigned num_rounds,
	const uint32_t *skey, uint32_t *q);

/*
 * Compute AES decryption on bitsliced data. Since input is stored on
 * eight 32-bit words, two block decryptions are actually performed
 * in parallel.
 */
void br_aes_ct_bitslice_decrypt(unsigned num_rounds,
	const uint32_t *skey, uint32_t *q);

/*
 * AES key schedule, constant-time version. skey[] is filled with n+1
 * 128-bit subkeys, where n is the number of rounds (10 to 14, depending
 * on key size). The number of rounds is returned. If the key size is
 * invalid (not 16, 24 or 32), then 0 is returned.
 */
unsigned br_aes_ct_keysched(uint32_t *comp_skey,
	const void *key, size_t key_len);

/*
 * Expand AES subkeys as produced by br_aes_ct_keysched(), into
 * a larger array suitable for br_aes_ct_bitslice_encrypt() and
 * br_aes_ct_bitslice_decrypt().
 */
void br_aes_ct_skey_expand(uint32_t *skey,
	unsigned num_rounds, const uint32_t *comp_skey);

/*
 * For the ct64 implementation, the same bitslicing technique is used,
 * but four instances are interleaved. First instance uses bits 0, 4,
 * 8, 12,... of each word; second instance uses bits 1, 5, 9, 13,...
 * and so on.
 */

/*
 * Perform bytewise orthogonalization of eight 64-bit words. Bytes
 * of q0..q7 are spread over all words: for a byte x that occurs
 * at rank i in q[j] (byte x uses bits 8*i to 8*i+7 in q[j]), the bit
 * of rank k in x (0 <= k <= 7) goes to q[k] at rank 8*i+j.
 *
 * This operation is an involution.
 */
void br_aes_ct64_ortho(uint64_t *q);

/*
 * Interleave bytes for an AES input block. If input bytes are
 * denoted 0123456789ABCDEF, and have been decoded with little-endian
 * convention (w[0] contains 0123, with '3' being most significant;
 * w[1] contains 4567, and so on), then output word q0 will be
 * set to 08192A3B (again little-endian convention) and q1 will
 * be set to 4C5D6E7F.
 */
void br_aes_ct64_interleave_in(uint64_t *q0, uint64_t *q1, const uint32_t *w);

/*
 * Perform the opposite of br_aes_ct64_interleave_in().
 */
void br_aes_ct64_interleave_out(uint32_t *w, uint64_t q0, uint64_t q1);

/*
 * The AES S-box, as a bitsliced constant-time version. The input array
 * consists in eight 64-bit words; 64 S-box instances are computed in
 * parallel. Bits 0 to 7 of each S-box input (bit 0 is least significant)
 * are spread over the words 0 to 7, at the same rank.
 */
void br_aes_ct64_bitslice_Sbox(uint64_t *q);

/*
 * Like br_aes_bitslice_Sbox(), but for the inverse S-box.
 */
void br_aes_ct64_bitslice_invSbox(uint64_t *q);

/*
 * Compute AES encryption on bitsliced data. Since input is stored on
 * eight 64-bit words, four block encryptions are actually performed
 * in parallel.
 */
void br_aes_ct64_bitslice_encrypt(unsigned num_rounds,
	const uint64_t *skey, uint64_t *q);

/*
 * Compute AES decryption on bitsliced data. Since input is stored on
 * eight 64-bit words, four block decryptions are actually performed
 * in parallel.
 */
void br_aes_ct64_bitslice_decrypt(unsigned num_rounds,
	const uint64_t *skey, uint64_t *q);

/*
 * AES key schedule, constant-time version. skey[] is filled with n+1
 * 128-bit subkeys, where n is the number of rounds (10 to 14, depending
 * on key size). The number of rounds is returned. If the key size is
 * invalid (not 16, 24 or 32), then 0 is returned.
 */
unsigned br_aes_ct64_keysched(uint64_t *comp_skey,
	const void *key, size_t key_len);

/*
 * Expand AES subkeys as produced by br_aes_ct64_keysched(), into
 * a larger array suitable for br_aes_ct64_bitslice_encrypt() and
 * br_aes_ct64_bitslice_decrypt().
 */
void br_aes_ct64_skey_expand(uint64_t *skey,
	unsigned num_rounds, const uint64_t *comp_skey);

/* ==================================================================== */
/*
 * RSA.
 */

/*
 * Apply proper PKCS#1 v1.5 padding (for signatures). 'hash_oid' is
 * the encoded hash function OID, or NULL.
 */
uint32_t br_rsa_pkcs1_sig_pad(const unsigned char *hash_oid,
	const unsigned char *hash, size_t hash_len,
	uint32_t n_bitlen, unsigned char *x);

/*
 * Check PKCS#1 v1.5 padding (for signatures). 'hash_oid' is the encoded
 * hash function OID, or NULL. The provided 'sig' value is _after_ the
 * modular exponentiation, i.e. it should be the padded hash. On
 * success, the hashed message is extracted.
 */
uint32_t br_rsa_pkcs1_sig_unpad(const unsigned char *sig, size_t sig_len,
	const unsigned char *hash_oid, size_t hash_len,
	unsigned char *hash_out);

/* ==================================================================== */
/*
 * Elliptic curves.
 */

/*
 * Type for generic EC parameters: curve order (unsigned big-endian
 * encoding) and encoded conventional generator.
 */
typedef struct {
	int curve;
	const unsigned char *order;
	size_t order_len;
	const unsigned char *generator;
	size_t generator_len;
} br_ec_curve_def;

extern const br_ec_curve_def br_secp256r1;
extern const br_ec_curve_def br_secp384r1;
extern const br_ec_curve_def br_secp521r1;

#if 0
/* obsolete */
/*
 * Type for the parameters for a "prime curve":
 *   coordinates are in GF(p), with p prime
 *   curve equation is Y^2 = X^3 - 3*X + b
 *   b is in Montgomery representation
 *   curve order is n and is prime
 *   base point is G (encoded) and has order n
 */
typedef struct {
	const uint32_t *p;
	const uint32_t *b;
	const uint32_t p0i;
} br_ec_prime_i31_curve;

extern const br_ec_prime_i31_curve br_ec_prime_i31_secp256r1;
extern const br_ec_prime_i31_curve br_ec_prime_i31_secp384r1;
extern const br_ec_prime_i31_curve br_ec_prime_i31_secp521r1;

#define BR_EC_I31_LEN   ((BR_MAX_EC_SIZE + 61) / 31)
#endif

/*
 * Decode some bytes as an i31 integer, with truncation (corresponding
 * to the 'bits2int' operation in RFC 6979). The target ENCODED bit
 * length is provided as last parameter. The resulting value will have
 * this declared bit length, and consists the big-endian unsigned decoding
 * of exactly that many bits in the source (capped at the source length).
 */
void br_ecdsa_i31_bits2int(uint32_t *x,
	const void *src, size_t len, uint32_t ebitlen);

/*
 * Decode some bytes as an i15 integer, with truncation (corresponding
 * to the 'bits2int' operation in RFC 6979). The target ENCODED bit
 * length is provided as last parameter. The resulting value will have
 * this declared bit length, and consists the big-endian unsigned decoding
 * of exactly that many bits in the source (capped at the source length).
 */
void br_ecdsa_i15_bits2int(uint16_t *x,
	const void *src, size_t len, uint32_t ebitlen);

/* ==================================================================== */
/*
 * SSL/TLS support functions.
 */

/*
 * Record types.
 */
#define BR_SSL_CHANGE_CIPHER_SPEC    20
#define BR_SSL_ALERT                 21
#define BR_SSL_HANDSHAKE             22
#define BR_SSL_APPLICATION_DATA      23

/*
 * Handshake message types.
 */
#define BR_SSL_HELLO_REQUEST          0
#define BR_SSL_CLIENT_HELLO           1
#define BR_SSL_SERVER_HELLO           2
#define BR_SSL_CERTIFICATE           11
#define BR_SSL_SERVER_KEY_EXCHANGE   12
#define BR_SSL_CERTIFICATE_REQUEST   13
#define BR_SSL_SERVER_HELLO_DONE     14
#define BR_SSL_CERTIFICATE_VERIFY    15
#define BR_SSL_CLIENT_KEY_EXCHANGE   16
#define BR_SSL_FINISHED              20

/*
 * Alert levels.
 */
#define BR_LEVEL_WARNING   1
#define BR_LEVEL_FATAL     2

/*
 * Low-level I/O state.
 */
#define BR_IO_FAILED   0
#define BR_IO_IN       1
#define BR_IO_OUT      2
#define BR_IO_INOUT    3

/*
 * Mark a SSL engine as failed. The provided error code is recorded if
 * the engine was not already marked as failed. If 'err' is 0, then the
 * engine is marked as closed (without error).
 */
void br_ssl_engine_fail(br_ssl_engine_context *cc, int err);

/*
 * Test whether the engine is closed (normally or as a failure).
 */
static inline int
br_ssl_engine_closed(const br_ssl_engine_context *cc)
{
	return cc->iomode == BR_IO_FAILED;
}

/*
 * Configure a new maximum fragment length. If possible, the maximum
 * length for outgoing records is immediately adjusted (if there are
 * not already too many buffered bytes for that).
 */
void br_ssl_engine_new_max_frag_len(
	br_ssl_engine_context *rc, unsigned max_frag_len);

/*
 * Test whether the current incoming record has been fully received
 * or not. This functions returns 0 only if a complete record header
 * has been received, but some of the (possibly encrypted) payload
 * has not yet been obtained.
 */
int br_ssl_engine_recvrec_finished(const br_ssl_engine_context *rc);

/*
 * Flush the current record (if not empty). This is meant to be called
 * from the handshake processor only.
 */
void br_ssl_engine_flush_record(br_ssl_engine_context *cc);

/*
 * Test whether there is some accumulated payload to send.
 */
static inline int
br_ssl_engine_has_pld_to_send(const br_ssl_engine_context *rc)
{
	return rc->oxa != rc->oxb && rc->oxa != rc->oxc;
}

/*
 * Initialize RNG in engine. Returned value is 1 on success, 0 on error.
 * This function will try to use the OS-provided RNG, if available. If
 * there is no OS-provided RNG, or if it failed, and no entropy was
 * injected by the caller, then a failure will be reported. On error,
 * the context error code is set.
 */
int br_ssl_engine_init_rand(br_ssl_engine_context *cc);

/*
 * Reset the handshake-related parts of the engine.
 */
void br_ssl_engine_hs_reset(br_ssl_engine_context *cc,
	void (*hsinit)(void *), void (*hsrun)(void *));

/*
 * Get the PRF to use for this context, for the provided PRF hash
 * function ID.
 */
br_tls_prf_impl br_ssl_engine_get_PRF(br_ssl_engine_context *cc, int prf_id);

/*
 * Consume the provided pre-master secret and compute the corresponding
 * master secret. The 'prf_id' is the ID of the hash function to use
 * with the TLS 1.2 PRF (ignored if the version is TLS 1.0 or 1.1).
 */
void br_ssl_engine_compute_master(br_ssl_engine_context *cc,
	int prf_id, const void *pms, size_t len);

/*
 * Switch to CBC decryption for incoming records.
 *    cc               the engine context
 *    is_client        non-zero for a client, zero for a server
 *    prf_id           id of hash function for PRF (ignored if not TLS 1.2+)
 *    mac_id           id of hash function for HMAC
 *    bc_impl          block cipher implementation (CBC decryption)
 *    cipher_key_len   block cipher key length (in bytes)
 */
void br_ssl_engine_switch_cbc_in(br_ssl_engine_context *cc,
	int is_client, int prf_id, int mac_id,
	const br_block_cbcdec_class *bc_impl, size_t cipher_key_len);

/*
 * Switch to CBC encryption for outgoing records.
 *    cc               the engine context
 *    is_client        non-zero for a client, zero for a server
 *    prf_id           id of hash function for PRF (ignored if not TLS 1.2+)
 *    mac_id           id of hash function for HMAC
 *    bc_impl          block cipher implementation (CBC encryption)
 *    cipher_key_len   block cipher key length (in bytes)
 */
void br_ssl_engine_switch_cbc_out(br_ssl_engine_context *cc,
	int is_client, int prf_id, int mac_id,
	const br_block_cbcenc_class *bc_impl, size_t cipher_key_len);

/*
 * Switch to GCM decryption for incoming records.
 *    cc               the engine context
 *    is_client        non-zero for a client, zero for a server
 *    prf_id           id of hash function for PRF
 *    bc_impl          block cipher implementation (CTR)
 *    cipher_key_len   block cipher key length (in bytes)
 */
void br_ssl_engine_switch_gcm_in(br_ssl_engine_context *cc,
	int is_client, int prf_id,
	const br_block_ctr_class *bc_impl, size_t cipher_key_len);

/*
 * Switch to GCM encryption for outgoing records.
 *    cc               the engine context
 *    is_client        non-zero for a client, zero for a server
 *    prf_id           id of hash function for PRF
 *    bc_impl          block cipher implementation (CTR)
 *    cipher_key_len   block cipher key length (in bytes)
 */
void br_ssl_engine_switch_gcm_out(br_ssl_engine_context *cc,
	int is_client, int prf_id,
	const br_block_ctr_class *bc_impl, size_t cipher_key_len);

/*
 * Switch to ChaCha20+Poly1305 decryption for incoming records.
 *    cc               the engine context
 *    is_client        non-zero for a client, zero for a server
 *    prf_id           id of hash function for PRF
 */
void br_ssl_engine_switch_chapol_in(br_ssl_engine_context *cc,
	int is_client, int prf_id);

/*
 * Switch to ChaCha20+Poly1305 encryption for outgoing records.
 *    cc               the engine context
 *    is_client        non-zero for a client, zero for a server
 *    prf_id           id of hash function for PRF
 */
void br_ssl_engine_switch_chapol_out(br_ssl_engine_context *cc,
	int is_client, int prf_id);

/*
 * Calls to T0-generated code.
 */
void br_ssl_hs_client_init_main(void *ctx);
void br_ssl_hs_client_run(void *ctx);
void br_ssl_hs_server_init_main(void *ctx);
void br_ssl_hs_server_run(void *ctx);

/*
 * Get the hash function to use for signatures, given a bit mask of
 * supported hash functions. This implements a strict choice order
 * (namely SHA-256, SHA-384, SHA-512, SHA-224, SHA-1). If the mask
 * does not document support of any of these hash functions, then this
 * functions returns 0.
 */
int br_ssl_choose_hash(unsigned bf);

/* ==================================================================== */

#endif


/* ===== src/codec/ccopy.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

/* see inner.h */
void
br_ccopy(uint32_t ctl, void *dst, const void *src, size_t len)
{
	unsigned char *d;
	const unsigned char *s;

	d = dst;
	s = src;
	while (len -- > 0) {
		uint32_t x, y;

		x = *s ++;
		y = *d;
		*d = MUX(ctl, x, y);
		d ++;
	}
}


/* ===== src/codec/dec16be.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

/* see inner.h */
void
br_range_dec16be(uint16_t *v, size_t num, const void *src)
{
	const unsigned char *buf;

	buf = src;
	while (num -- > 0) {
		*v ++ = br_dec16be(buf);
		buf += 2;
	}
}


/* ===== src/codec/dec32be.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

/* see inner.h */
void
br_range_dec32be(uint32_t *v, size_t num, const void *src)
{
	const unsigned char *buf;

	buf = src;
	while (num -- > 0) {
		*v ++ = br_dec32be(buf);
		buf += 4;
	}
}


/* ===== src/codec/enc16be.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

/* see inner.h */
void
br_range_enc16be(void *dst, const uint16_t *v, size_t num)
{
	unsigned char *buf;

	buf = dst;
	while (num -- > 0) {
		br_enc16be(buf, *v ++);
		buf += 2;
	}
}


/* ===== src/codec/enc16le.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

/* see inner.h */
void
br_range_enc16le(void *dst, const uint16_t *v, size_t num)
{
	unsigned char *buf;

	buf = dst;
	while (num -- > 0) {
		br_enc16le(buf, *v ++);
		buf += 2;
	}
}


/* ===== src/codec/enc32be.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

/* see inner.h */
void
br_range_enc32be(void *dst, const uint32_t *v, size_t num)
{
	unsigned char *buf;

	buf = dst;
	while (num -- > 0) {
		br_enc32be(buf, *v ++);
		buf += 4;
	}
}


/* ===== src/codec/enc64be.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

/* see inner.h */
void
br_range_enc64be(void *dst, const uint64_t *v, size_t num)
{
	unsigned char *buf;

	buf = dst;
	while (num -- > 0) {
		br_enc64be(buf, *v ++);
		buf += 8;
	}
}


/* ===== src/codec/pemdec.c ===== */
/* Automatically generated code; do not modify directly. */

#include <stddef.h>
#include <stdint.h>

typedef struct {
	uint32_t *dp;
	uint32_t *rp;
	const unsigned char *ip;
} t0_context;

static uint32_t
t0_parse7E_unsigned(const unsigned char **p)
{
	uint32_t x;

	x = 0;
	for (;;) {
		unsigned y;

		y = *(*p) ++;
		x = (x << 7) | (uint32_t)(y & 0x7F);
		if (y < 0x80) {
			return x;
		}
	}
}

static int32_t
t0_parse7E_signed(const unsigned char **p)
{
	int neg;
	uint32_t x;

	neg = ((**p) >> 6) & 1;
	x = (uint32_t)-neg;
	for (;;) {
		unsigned y;

		y = *(*p) ++;
		x = (x << 7) | (uint32_t)(y & 0x7F);
		if (y < 0x80) {
			if (neg) {
				return -(int32_t)~x - 1;
			} else {
				return (int32_t)x;
			}
		}
	}
}

#define T0_VBYTE(x, n)   (unsigned char)((((uint32_t)(x) >> (n)) & 0x7F) | 0x80)
#define T0_FBYTE(x, n)   (unsigned char)(((uint32_t)(x) >> (n)) & 0x7F)
#define T0_SBYTE(x)      (unsigned char)((((uint32_t)(x) >> 28) + 0xF8) ^ 0xF8)
#define T0_INT1(x)       T0_FBYTE(x, 0)
#define T0_INT2(x)       T0_VBYTE(x, 7), T0_FBYTE(x, 0)
#define T0_INT3(x)       T0_VBYTE(x, 14), T0_VBYTE(x, 7), T0_FBYTE(x, 0)
#define T0_INT4(x)       T0_VBYTE(x, 21), T0_VBYTE(x, 14), T0_VBYTE(x, 7), T0_FBYTE(x, 0)
#define T0_INT5(x)       T0_SBYTE(x), T0_VBYTE(x, 21), T0_VBYTE(x, 14), T0_VBYTE(x, 7), T0_FBYTE(x, 0)

static const uint8_t t0_datablock[];


void br_pem_decoder_init_main(void *t0ctx);

void br_pem_decoder_run(void *t0ctx);



/* (already included) */

#define CTX   ((br_pem_decoder_context *)((unsigned char *)t0ctx - offsetof(br_pem_decoder_context, cpu)))

/* see bearssl_pem.h */
void
br_pem_decoder_init(br_pem_decoder_context *ctx)
{
	memset(ctx, 0, sizeof *ctx);
	ctx->cpu.dp = &ctx->dp_stack[0];
	ctx->cpu.rp = &ctx->rp_stack[0];
	br_pem_decoder_init_main(&ctx->cpu);
	br_pem_decoder_run(&ctx->cpu);
}

/* see bearssl_pem.h */
size_t
br_pem_decoder_push(br_pem_decoder_context *ctx,
	const void *data, size_t len)
{
	if (ctx->event) {
		return 0;
	}
	ctx->hbuf = data;
	ctx->hlen = len;
	br_pem_decoder_run(&ctx->cpu);
	return len - ctx->hlen;
}

/* see bearssl_pem.h */
int
br_pem_decoder_event(br_pem_decoder_context *ctx)
{
	int event;

	event = ctx->event;
	ctx->event = 0;
	return event;
}



static const uint8_t t0_datablock[] = {
	0x00, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x42, 0x45, 0x47, 0x49, 0x4E, 0x20,
	0x00, 0x2D, 0x2D, 0x2D, 0x2D, 0x45, 0x4E, 0x44, 0x20, 0x00
};

static const uint8_t t0_codeblock[] = {
	0x00, 0x01, 0x00, 0x09, 0x00, 0x00, 0x01, 0x01, 0x07, 0x00, 0x00, 0x01,
	0x01, 0x08, 0x00, 0x00, 0x13, 0x13, 0x00, 0x00, 0x01,
	T0_INT2(offsetof(br_pem_decoder_context, event)), 0x00, 0x00, 0x01,
	T0_INT2(offsetof(br_pem_decoder_context, name)), 0x00, 0x00, 0x05,
	0x14, 0x2C, 0x14, 0x01, 0x0A, 0x0D, 0x06, 0x03, 0x13, 0x04, 0x76, 0x01,
	0x2D, 0x0C, 0x06, 0x05, 0x2E, 0x01, 0x03, 0x2D, 0x00, 0x01, 0x0D, 0x27,
	0x05, 0x04, 0x01, 0x03, 0x2D, 0x00, 0x15, 0x2E, 0x01, 0x02, 0x2D, 0x00,
	0x01, 0x01, 0x7F, 0x03, 0x00, 0x24, 0x01, 0x00, 0x17, 0x0D, 0x06, 0x03,
	0x13, 0x04, 0x3C, 0x01, 0x7F, 0x17, 0x0D, 0x06, 0x13, 0x13, 0x02, 0x00,
	0x05, 0x06, 0x2E, 0x01, 0x03, 0x2D, 0x04, 0x03, 0x01, 0x7F, 0x22, 0x01,
	0x00, 0x00, 0x04, 0x23, 0x01, 0x01, 0x17, 0x0D, 0x06, 0x09, 0x13, 0x01,
	0x00, 0x22, 0x01, 0x00, 0x00, 0x04, 0x14, 0x01, 0x02, 0x17, 0x0D, 0x06,
	0x06, 0x13, 0x01, 0x7F, 0x00, 0x04, 0x08, 0x13, 0x01, 0x03, 0x2D, 0x01,
	0x00, 0x00, 0x13, 0x01, 0x00, 0x03, 0x00, 0x04, 0xFF, 0x33, 0x01, 0x2C,
	0x14, 0x01, 0x2D, 0x0D, 0x06, 0x04, 0x13, 0x01, 0x7F, 0x00, 0x14, 0x31,
	0x06, 0x02, 0x13, 0x29, 0x14, 0x01, 0x0A, 0x0D, 0x06, 0x04, 0x13, 0x01,
	0x02, 0x00, 0x25, 0x14, 0x1C, 0x06, 0x05, 0x13, 0x2E, 0x01, 0x03, 0x00,
	0x03, 0x00, 0x29, 0x14, 0x01, 0x0A, 0x0D, 0x06, 0x04, 0x13, 0x01, 0x03,
	0x00, 0x25, 0x14, 0x1C, 0x06, 0x05, 0x13, 0x2E, 0x01, 0x03, 0x00, 0x02,
	0x00, 0x01, 0x06, 0x0A, 0x07, 0x03, 0x00, 0x29, 0x14, 0x01, 0x0A, 0x0D,
	0x06, 0x04, 0x13, 0x01, 0x03, 0x00, 0x14, 0x01, 0x3D, 0x0D, 0x06, 0x2E,
	0x13, 0x29, 0x14, 0x01, 0x0A, 0x0D, 0x06, 0x04, 0x13, 0x01, 0x03, 0x00,
	0x2F, 0x05, 0x04, 0x13, 0x01, 0x03, 0x00, 0x01, 0x3D, 0x0C, 0x06, 0x03,
	0x01, 0x03, 0x00, 0x02, 0x00, 0x01, 0x0F, 0x10, 0x06, 0x03, 0x01, 0x03,
	0x00, 0x02, 0x00, 0x01, 0x04, 0x0F, 0x1B, 0x01, 0x01, 0x00, 0x25, 0x14,
	0x1C, 0x06, 0x05, 0x13, 0x2E, 0x01, 0x03, 0x00, 0x02, 0x00, 0x01, 0x06,
	0x0A, 0x07, 0x03, 0x00, 0x29, 0x14, 0x01, 0x0A, 0x0D, 0x06, 0x04, 0x13,
	0x01, 0x03, 0x00, 0x14, 0x01, 0x3D, 0x0D, 0x06, 0x20, 0x13, 0x2F, 0x05,
	0x03, 0x01, 0x03, 0x00, 0x02, 0x00, 0x01, 0x03, 0x10, 0x06, 0x03, 0x01,
	0x03, 0x00, 0x02, 0x00, 0x01, 0x0A, 0x0F, 0x1B, 0x02, 0x00, 0x01, 0x02,
	0x0F, 0x1B, 0x01, 0x01, 0x00, 0x25, 0x14, 0x1C, 0x06, 0x05, 0x13, 0x2E,
	0x01, 0x03, 0x00, 0x02, 0x00, 0x01, 0x06, 0x0A, 0x07, 0x03, 0x00, 0x02,
	0x00, 0x01, 0x10, 0x0F, 0x1B, 0x02, 0x00, 0x01, 0x08, 0x0F, 0x1B, 0x02,
	0x00, 0x1B, 0x01, 0x00, 0x00, 0x00, 0x14, 0x14, 0x01, 0x80, 0x41, 0x0E,
	0x1A, 0x01, 0x80, 0x5A, 0x0B, 0x10, 0x06, 0x05, 0x01, 0x80, 0x41, 0x08,
	0x00, 0x14, 0x14, 0x01, 0x80, 0x61, 0x0E, 0x1A, 0x01, 0x80, 0x7A, 0x0B,
	0x10, 0x06, 0x05, 0x01, 0x80, 0x47, 0x08, 0x00, 0x14, 0x14, 0x01, 0x30,
	0x0E, 0x1A, 0x01, 0x39, 0x0B, 0x10, 0x06, 0x04, 0x01, 0x04, 0x07, 0x00,
	0x14, 0x01, 0x2B, 0x0D, 0x06, 0x04, 0x13, 0x01, 0x3E, 0x00, 0x14, 0x01,
	0x2F, 0x0D, 0x06, 0x04, 0x13, 0x01, 0x3F, 0x00, 0x01, 0x3D, 0x0C, 0x1E,
	0x00, 0x00, 0x28, 0x01, 0x01, 0x2D, 0x23, 0x06, 0x02, 0x04, 0x7B, 0x04,
	0x75, 0x00, 0x14, 0x12, 0x2A, 0x14, 0x05, 0x04, 0x1F, 0x01, 0x7F, 0x00,
	0x2C, 0x2A, 0x14, 0x01, 0x0A, 0x0D, 0x06, 0x05, 0x13, 0x1F, 0x01, 0x00,
	0x00, 0x0D, 0x05, 0x05, 0x13, 0x2E, 0x01, 0x00, 0x00, 0x1D, 0x04, 0x5E,
	0x00, 0x01, 0x01, 0x27, 0x06, 0x0B, 0x21, 0x01, 0x80, 0x7F, 0x2B, 0x14,
	0x06, 0x02, 0x30, 0x00, 0x13, 0x04, 0x6E, 0x00, 0x2C, 0x14, 0x31, 0x05,
	0x01, 0x00, 0x13, 0x04, 0x77, 0x00, 0x14, 0x14, 0x01, 0x80, 0x61, 0x0E,
	0x1A, 0x01, 0x80, 0x7A, 0x0B, 0x10, 0x06, 0x03, 0x01, 0x20, 0x08, 0x00,
	0x01, 0x14, 0x03, 0x00, 0x1A, 0x17, 0x05, 0x05, 0x1F, 0x2E, 0x01, 0x00,
	0x00, 0x2C, 0x14, 0x01, 0x0A, 0x0D, 0x06, 0x06, 0x1F, 0x02, 0x00, 0x1A,
	0x08, 0x00, 0x14, 0x01, 0x0D, 0x0D, 0x06, 0x03, 0x13, 0x04, 0x03, 0x2A,
	0x17, 0x19, 0x1D, 0x1A, 0x1E, 0x1A, 0x04, 0x59, 0x00, 0x18, 0x14, 0x1C,
	0x05, 0x01, 0x00, 0x13, 0x11, 0x04, 0x76, 0x00, 0x20, 0x19, 0x11, 0x00,
	0x00, 0x2C, 0x01, 0x0A, 0x0C, 0x06, 0x02, 0x04, 0x78, 0x00, 0x01, 0x01,
	0x7F, 0x03, 0x00, 0x2C, 0x14, 0x01, 0x0A, 0x0C, 0x06, 0x09, 0x31, 0x05,
	0x04, 0x01, 0x00, 0x03, 0x00, 0x04, 0x70, 0x13, 0x02, 0x00, 0x00, 0x00,
	0x14, 0x06, 0x14, 0x1E, 0x14, 0x21, 0x07, 0x16, 0x01, 0x2D, 0x0C, 0x06,
	0x08, 0x21, 0x07, 0x1D, 0x01, 0x00, 0x1A, 0x19, 0x00, 0x04, 0x69, 0x21,
	0x19, 0x00, 0x00, 0x14, 0x01, 0x0A, 0x0C, 0x1A, 0x01, 0x20, 0x0B, 0x10,
	0x00
};

static const uint16_t t0_caddr[] = {
	0,
	5,
	10,
	15,
	19,
	24,
	29,
	67,
	149,
	384,
	464,
	476,
	511,
	530,
	540,
	559,
	603,
	614,
	619,
	629,
	654,
	681
};

#define T0_INTERPRETED   28

#define T0_ENTER(ip, rp, slot)   do { \
		const unsigned char *t0_newip; \
		uint32_t t0_lnum; \
		t0_newip = &t0_codeblock[t0_caddr[(slot) - T0_INTERPRETED]]; \
		t0_lnum = t0_parse7E_unsigned(&t0_newip); \
		(rp) += t0_lnum; \
		*((rp) ++) = (uint32_t)((ip) - &t0_codeblock[0]) + (t0_lnum << 16); \
		(ip) = t0_newip; \
	} while (0)

#define T0_DEFENTRY(name, slot) \
void \
name(void *ctx) \
{ \
	t0_context *t0ctx = ctx; \
	t0ctx->ip = &t0_codeblock[0]; \
	T0_ENTER(t0ctx->ip, t0ctx->rp, slot); \
}

T0_DEFENTRY(br_pem_decoder_init_main, 38)

#define T0_NEXT(t0ipp)   (*(*(t0ipp)) ++)

void
br_pem_decoder_run(void *t0ctx)
{
	uint32_t *dp, *rp;
	const unsigned char *ip;

#define T0_LOCAL(x)    (*(rp - 2 - (x)))
#define T0_POP()       (*-- dp)
#define T0_POPi()      (*(int32_t *)(-- dp))
#define T0_PEEK(x)     (*(dp - 1 - (x)))
#define T0_PEEKi(x)    (*(int32_t *)(dp - 1 - (x)))
#define T0_PUSH(v)     do { *dp = (v); dp ++; } while (0)
#define T0_PUSHi(v)    do { *(int32_t *)dp = (v); dp ++; } while (0)
#define T0_RPOP()      (*-- rp)
#define T0_RPOPi()     (*(int32_t *)(-- rp))
#define T0_RPUSH(v)    do { *rp = (v); rp ++; } while (0)
#define T0_RPUSHi(v)   do { *(int32_t *)rp = (v); rp ++; } while (0)
#define T0_ROLL(x)     do { \
	size_t t0len = (size_t)(x); \
	uint32_t t0tmp = *(dp - 1 - t0len); \
	memmove(dp - t0len - 1, dp - t0len, t0len * sizeof *dp); \
	*(dp - 1) = t0tmp; \
} while (0)
#define T0_SWAP()      do { \
	uint32_t t0tmp = *(dp - 2); \
	*(dp - 2) = *(dp - 1); \
	*(dp - 1) = t0tmp; \
} while (0)
#define T0_ROT()       do { \
	uint32_t t0tmp = *(dp - 3); \
	*(dp - 3) = *(dp - 2); \
	*(dp - 2) = *(dp - 1); \
	*(dp - 1) = t0tmp; \
} while (0)
#define T0_NROT()       do { \
	uint32_t t0tmp = *(dp - 1); \
	*(dp - 1) = *(dp - 2); \
	*(dp - 2) = *(dp - 3); \
	*(dp - 3) = t0tmp; \
} while (0)
#define T0_PICK(x)      do { \
	uint32_t t0depth = (x); \
	T0_PUSH(T0_PEEK(t0depth)); \
} while (0)
#define T0_CO()         do { \
	goto t0_exit; \
} while (0)
#define T0_RET()        goto t0_next

	dp = ((t0_context *)t0ctx)->dp;
	rp = ((t0_context *)t0ctx)->rp;
	ip = ((t0_context *)t0ctx)->ip;
	goto t0_next;
	for (;;) {
		uint32_t t0x;

	t0_next:
		t0x = T0_NEXT(&ip);
		if (t0x < T0_INTERPRETED) {
			switch (t0x) {
				int32_t t0off;

			case 0: /* ret */
				t0x = T0_RPOP();
				rp -= (t0x >> 16);
				t0x &= 0xFFFF;
				if (t0x == 0) {
					ip = NULL;
					goto t0_exit;
				}
				ip = &t0_codeblock[t0x];
				break;
			case 1: /* literal constant */
				T0_PUSHi(t0_parse7E_signed(&ip));
				break;
			case 2: /* read local */
				T0_PUSH(T0_LOCAL(t0_parse7E_unsigned(&ip)));
				break;
			case 3: /* write local */
				T0_LOCAL(t0_parse7E_unsigned(&ip)) = T0_POP();
				break;
			case 4: /* jump */
				t0off = t0_parse7E_signed(&ip);
				ip += t0off;
				break;
			case 5: /* jump if */
				t0off = t0_parse7E_signed(&ip);
				if (T0_POP()) {
					ip += t0off;
				}
				break;
			case 6: /* jump if not */
				t0off = t0_parse7E_signed(&ip);
				if (!T0_POP()) {
					ip += t0off;
				}
				break;
			case 7: {
				/* + */

	uint32_t b = T0_POP();
	uint32_t a = T0_POP();
	T0_PUSH(a + b);

				}
				break;
			case 8: {
				/* - */

	uint32_t b = T0_POP();
	uint32_t a = T0_POP();
	T0_PUSH(a - b);

				}
				break;
			case 9: {
				/* < */

	int32_t b = T0_POPi();
	int32_t a = T0_POPi();
	T0_PUSH(-(uint32_t)(a < b));

				}
				break;
			case 10: {
				/* << */

	int c = (int)T0_POPi();
	uint32_t x = T0_POP();
	T0_PUSH(x << c);

				}
				break;
			case 11: {
				/* <= */

	int32_t b = T0_POPi();
	int32_t a = T0_POPi();
	T0_PUSH(-(uint32_t)(a <= b));

				}
				break;
			case 12: {
				/* <> */

	uint32_t b = T0_POP();
	uint32_t a = T0_POP();
	T0_PUSH(-(uint32_t)(a != b));

				}
				break;
			case 13: {
				/* = */

	uint32_t b = T0_POP();
	uint32_t a = T0_POP();
	T0_PUSH(-(uint32_t)(a == b));

				}
				break;
			case 14: {
				/* >= */

	int32_t b = T0_POPi();
	int32_t a = T0_POPi();
	T0_PUSH(-(uint32_t)(a >= b));

				}
				break;
			case 15: {
				/* >> */

	int c = (int)T0_POPi();
	int32_t x = T0_POPi();
	T0_PUSHi(x >> c);

				}
				break;
			case 16: {
				/* and */

	uint32_t b = T0_POP();
	uint32_t a = T0_POP();
	T0_PUSH(a & b);

				}
				break;
			case 17: {
				/* co */
 T0_CO(); 
				}
				break;
			case 18: {
				/* data-get8 */

	size_t addr = T0_POP();
	T0_PUSH(t0_datablock[addr]);

				}
				break;
			case 19: {
				/* drop */
 (void)T0_POP(); 
				}
				break;
			case 20: {
				/* dup */
 T0_PUSH(T0_PEEK(0)); 
				}
				break;
			case 21: {
				/* flush-buf */

	if (CTX->ptr > 0) {
		CTX->dest(CTX->dest_ctx, CTX->buf, CTX->ptr);
		CTX->ptr = 0;
	}

				}
				break;
			case 22: {
				/* get8 */

	size_t addr = T0_POP();
	T0_PUSH(*((unsigned char *)CTX + addr));

				}
				break;
			case 23: {
				/* over */
 T0_PUSH(T0_PEEK(1)); 
				}
				break;
			case 24: {
				/* read8-native */

	if (CTX->hlen > 0) {
		T0_PUSH(*CTX->hbuf ++);
		CTX->hlen --;
	} else {
		T0_PUSHi(-1);
	}

				}
				break;
			case 25: {
				/* set8 */

	size_t addr = T0_POP();
	unsigned x = T0_POP();
	*((unsigned char *)CTX + addr) = x;

				}
				break;
			case 26: {
				/* swap */
 T0_SWAP(); 
				}
				break;
			case 27: {
				/* write8 */

	unsigned char x = (unsigned char)T0_POP();
	CTX->buf[CTX->ptr ++] = x;
	if (CTX->ptr == sizeof CTX->buf) {
		if (CTX->dest) {
			CTX->dest(CTX->dest_ctx, CTX->buf, sizeof CTX->buf);
		}
		CTX->ptr = 0;
	}

				}
				break;
			}

		} else {
			T0_ENTER(ip, rp, t0x);
		}
	}
t0_exit:
	((t0_context *)t0ctx)->dp = dp;
	((t0_context *)t0ctx)->rp = rp;
	((t0_context *)t0ctx)->ip = ip;
}


/* ===== src/hash/dig_oid.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

/*
 * This file contains the encoded OID for the standard hash functions.
 * Such OID appear in, for instance, the PKCS#1 v1.5 padding for RSA
 * signatures.
 */

static const unsigned char md5_OID[] = {
	0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x02, 0x05
};

static const unsigned char sha1_OID[] = {
	0x2B, 0x0E, 0x03, 0x02, 0x1A
};

static const unsigned char sha224_OID[] = {
	0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x04
};

static const unsigned char sha256_OID[] = {
	0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01
};

static const unsigned char sha384_OID[] = {
	0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x02
};

static const unsigned char sha512_OID[] = {
	0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x03
};

/* see inner.h */
const unsigned char *
br_digest_OID(int digest_id, size_t *len)
{
	switch (digest_id) {
	case br_md5_ID:
		*len = sizeof md5_OID;
		return md5_OID;
	case br_sha1_ID:
		*len = sizeof sha1_OID;
		return sha1_OID;
	case br_sha224_ID:
		*len = sizeof sha224_OID;
		return sha224_OID;
	case br_sha256_ID:
		*len = sizeof sha256_OID;
		return sha256_OID;
	case br_sha384_ID:
		*len = sizeof sha384_OID;
		return sha384_OID;
	case br_sha512_ID:
		*len = sizeof sha512_OID;
		return sha512_OID;
	default:
		*len = 0;
		return NULL;
	}
}


/* ===== src/hash/dig_size.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

/* see inner.h */
size_t
br_digest_size_by_ID(int digest_id)
{
	switch (digest_id) {
	case br_md5sha1_ID:
		return br_md5_SIZE + br_sha1_SIZE;
	case br_md5_ID:
		return br_md5_SIZE;
	case br_sha1_ID:
		return br_sha1_SIZE;
	case br_sha224_ID:
		return br_sha224_SIZE;
	case br_sha256_ID:
		return br_sha256_SIZE;
	case br_sha384_ID:
		return br_sha384_SIZE;
	case br_sha512_ID:
		return br_sha512_SIZE;
	default:
		/* abort(); */
		return 0;
	}
}


/* ===== src/hash/ghash_ctmul.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

/*
 * We compute "carryless multiplications" through normal integer
 * multiplications, masking out enough bits to create "holes" in which
 * carries may expand without altering our bits; we really use 8 data
 * bits per 32-bit word, spaced every fourth bit. Accumulated carries
 * may not exceed 8 in total, which fits in 4 bits.
 *
 * It would be possible to use a 3-bit spacing, allowing two operands,
 * one with 7 non-zero data bits, the other one with 10 or 11 non-zero
 * data bits; this asymmetric splitting makes the overall code more
 * complex with thresholds and exceptions, and does not appear to be
 * worth the effort.
 */

/*
 * We cannot really autodetect whether multiplications are "slow" or
 * not. A typical example is the ARM Cortex M0+, which exists in two
 * versions: one with a 1-cycle multiplication opcode, the other with
 * a 32-cycle multiplication opcode. They both use exactly the same
 * architecture and ABI, and cannot be distinguished from each other
 * at compile-time.
 *
 * Since most modern CPU (even embedded CPU) still have fast
 * multiplications, we use the "fast mul" code by default.
 */

#if BR_SLOW_MUL

/*
 * This implementation uses Karatsuba-like reduction to make fewer
 * integer multiplications (9 instead of 16), at the expense of extra
 * logical operations (XOR, shifts...). On modern x86 CPU that offer
 * fast, pipelined multiplications, this code is about twice slower than
 * the simpler code with 16 multiplications. This tendency may be
 * reversed on low-end platforms with expensive multiplications.
 */

#define MUL32(h, l, x, y)   do { \
		uint64_t mul32tmp = MUL(x, y); \
		(h) = (uint32_t)(mul32tmp >> 32); \
		(l) = (uint32_t)mul32tmp; \
	} while (0)

static inline void
bmul(uint32_t *hi, uint32_t *lo, uint32_t x, uint32_t y)
{
	uint32_t x0, x1, x2, x3;
	uint32_t y0, y1, y2, y3;
	uint32_t a0, a1, a2, a3, a4, a5, a6, a7, a8;
	uint32_t b0, b1, b2, b3, b4, b5, b6, b7, b8;

	x0 = x & (uint32_t)0x11111111;
	x1 = x & (uint32_t)0x22222222;
	x2 = x & (uint32_t)0x44444444;
	x3 = x & (uint32_t)0x88888888;
	y0 = y & (uint32_t)0x11111111;
	y1 = y & (uint32_t)0x22222222;
	y2 = y & (uint32_t)0x44444444;
	y3 = y & (uint32_t)0x88888888;

	/*
	 * (x0+W*x1)*(y0+W*y1) -> a0:b0
	 * (x2+W*x3)*(y2+W*y3) -> a3:b3
	 * ((x0+x2)+W*(x1+x3))*((y0+y2)+W*(y1+y3)) -> a6:b6
	 */
	a0 = x0;
	b0 = y0;
	a1 = x1 >> 1;
	b1 = y1 >> 1;
	a2 = a0 ^ a1;
	b2 = b0 ^ b1;
	a3 = x2 >> 2;
	b3 = y2 >> 2;
	a4 = x3 >> 3;
	b4 = y3 >> 3;
	a5 = a3 ^ a4;
	b5 = b3 ^ b4;
	a6 = a0 ^ a3;
	b6 = b0 ^ b3;
	a7 = a1 ^ a4;
	b7 = b1 ^ b4;
	a8 = a6 ^ a7;
	b8 = b6 ^ b7;

	MUL32(b0, a0, b0, a0);
	MUL32(b1, a1, b1, a1);
	MUL32(b2, a2, b2, a2);
	MUL32(b3, a3, b3, a3);
	MUL32(b4, a4, b4, a4);
	MUL32(b5, a5, b5, a5);
	MUL32(b6, a6, b6, a6);
	MUL32(b7, a7, b7, a7);
	MUL32(b8, a8, b8, a8);

	a0 &= (uint32_t)0x11111111;
	a1 &= (uint32_t)0x11111111;
	a2 &= (uint32_t)0x11111111;
	a3 &= (uint32_t)0x11111111;
	a4 &= (uint32_t)0x11111111;
	a5 &= (uint32_t)0x11111111;
	a6 &= (uint32_t)0x11111111;
	a7 &= (uint32_t)0x11111111;
	a8 &= (uint32_t)0x11111111;
	b0 &= (uint32_t)0x11111111;
	b1 &= (uint32_t)0x11111111;
	b2 &= (uint32_t)0x11111111;
	b3 &= (uint32_t)0x11111111;
	b4 &= (uint32_t)0x11111111;
	b5 &= (uint32_t)0x11111111;
	b6 &= (uint32_t)0x11111111;
	b7 &= (uint32_t)0x11111111;
	b8 &= (uint32_t)0x11111111;

	a2 ^= a0 ^ a1;
	b2 ^= b0 ^ b1;
	a0 ^= (a2 << 1) ^ (a1 << 2);
	b0 ^= (b2 << 1) ^ (b1 << 2);
	a5 ^= a3 ^ a4;
	b5 ^= b3 ^ b4;
	a3 ^= (a5 << 1) ^ (a4 << 2);
	b3 ^= (b5 << 1) ^ (b4 << 2);
	a8 ^= a6 ^ a7;
	b8 ^= b6 ^ b7;
	a6 ^= (a8 << 1) ^ (a7 << 2);
	b6 ^= (b8 << 1) ^ (b7 << 2);
	a6 ^= a0 ^ a3;
	b6 ^= b0 ^ b3;
	*lo = a0 ^ (a6 << 2) ^ (a3 << 4);
	*hi = b0 ^ (b6 << 2) ^ (b3 << 4) ^ (a6 >> 30) ^ (a3 >> 28);
}

#else

/*
 * Simple multiplication in GF(2)[X], using 16 integer multiplications.
 */

static inline void
bmul(uint32_t *hi, uint32_t *lo, uint32_t x, uint32_t y)
{
	uint32_t x0, x1, x2, x3;
	uint32_t y0, y1, y2, y3;
	uint64_t z0, z1, z2, z3;
	uint64_t z;

	x0 = x & (uint32_t)0x11111111;
	x1 = x & (uint32_t)0x22222222;
	x2 = x & (uint32_t)0x44444444;
	x3 = x & (uint32_t)0x88888888;
	y0 = y & (uint32_t)0x11111111;
	y1 = y & (uint32_t)0x22222222;
	y2 = y & (uint32_t)0x44444444;
	y3 = y & (uint32_t)0x88888888;
	z0 = MUL(x0, y0) ^ MUL(x1, y3) ^ MUL(x2, y2) ^ MUL(x3, y1);
	z1 = MUL(x0, y1) ^ MUL(x1, y0) ^ MUL(x2, y3) ^ MUL(x3, y2);
	z2 = MUL(x0, y2) ^ MUL(x1, y1) ^ MUL(x2, y0) ^ MUL(x3, y3);
	z3 = MUL(x0, y3) ^ MUL(x1, y2) ^ MUL(x2, y1) ^ MUL(x3, y0);
	z0 &= (uint64_t)0x1111111111111111;
	z1 &= (uint64_t)0x2222222222222222;
	z2 &= (uint64_t)0x4444444444444444;
	z3 &= (uint64_t)0x8888888888888888;
	z = z0 | z1 | z2 | z3;
	*lo = (uint32_t)z;
	*hi = (uint32_t)(z >> 32);
}

#endif

/* see bearssl_hash.h */
void
br_ghash_ctmul(void *y, const void *h, const void *data, size_t len)
{
	const unsigned char *buf, *hb;
	unsigned char *yb;
	uint32_t yw[4];
	uint32_t hw[4];

	/*
	 * Throughout the loop we handle the y and h values as arrays
	 * of 32-bit words.
	 */
	buf = data;
	yb = y;
	hb = h;
	yw[3] = br_dec32be(yb);
	yw[2] = br_dec32be(yb + 4);
	yw[1] = br_dec32be(yb + 8);
	yw[0] = br_dec32be(yb + 12);
	hw[3] = br_dec32be(hb);
	hw[2] = br_dec32be(hb + 4);
	hw[1] = br_dec32be(hb + 8);
	hw[0] = br_dec32be(hb + 12);
	while (len > 0) {
		const unsigned char *src;
		unsigned char tmp[16];
		int i;
		uint32_t a[9], b[9], zw[8];
		uint32_t c0, c1, c2, c3, d0, d1, d2, d3, e0, e1, e2, e3;

		/*
		 * Get the next 16-byte block (using zero-padding if
		 * necessary).
		 */
		if (len >= 16) {
			src = buf;
			buf += 16;
			len -= 16;
		} else {
			memcpy(tmp, buf, len);
			memset(tmp + len, 0, (sizeof tmp) - len);
			src = tmp;
			len = 0;
		}

		/*
		 * Decode the block. The GHASH standard mandates
		 * big-endian encoding.
		 */
		yw[3] ^= br_dec32be(src);
		yw[2] ^= br_dec32be(src + 4);
		yw[1] ^= br_dec32be(src + 8);
		yw[0] ^= br_dec32be(src + 12);

		/*
		 * We multiply two 128-bit field elements. We use
		 * Karatsuba to turn that into three 64-bit
		 * multiplications, which are themselves done with a
		 * total of nine 32-bit multiplications.
		 */

		/*
		 * y[0,1]*h[0,1] -> 0..2
		 * y[2,3]*h[2,3] -> 3..5
		 * (y[0,1]+y[2,3])*(h[0,1]+h[2,3]) -> 6..8
		 */
		a[0] = yw[0];
		b[0] = hw[0];
		a[1] = yw[1];
		b[1] = hw[1];
		a[2] = a[0] ^ a[1];
		b[2] = b[0] ^ b[1];

		a[3] = yw[2];
		b[3] = hw[2];
		a[4] = yw[3];
		b[4] = hw[3];
		a[5] = a[3] ^ a[4];
		b[5] = b[3] ^ b[4];

		a[6] = a[0] ^ a[3];
		b[6] = b[0] ^ b[3];
		a[7] = a[1] ^ a[4];
		b[7] = b[1] ^ b[4];
		a[8] = a[6] ^ a[7];
		b[8] = b[6] ^ b[7];

		for (i = 0; i < 9; i ++) {
			bmul(&b[i], &a[i], b[i], a[i]);
		}

		c0 = a[0];
		c1 = b[0] ^ a[2] ^ a[0] ^ a[1];
		c2 = a[1] ^ b[2] ^ b[0] ^ b[1];
		c3 = b[1];
		d0 = a[3];
		d1 = b[3] ^ a[5] ^ a[3] ^ a[4];
		d2 = a[4] ^ b[5] ^ b[3] ^ b[4];
		d3 = b[4];
		e0 = a[6];
		e1 = b[6] ^ a[8] ^ a[6] ^ a[7];
		e2 = a[7] ^ b[8] ^ b[6] ^ b[7];
		e3 = b[7];

		e0 ^= c0 ^ d0;
		e1 ^= c1 ^ d1;
		e2 ^= c2 ^ d2;
		e3 ^= c3 ^ d3;
		c2 ^= e0;
		c3 ^= e1;
		d0 ^= e2;
		d1 ^= e3;

		/*
		 * GHASH specification has the bits "reversed" (most
		 * significant is in fact least significant), which does
		 * not matter for a carryless multiplication, except that
		 * the 255-bit result must be shifted by 1 bit.
		 */
		zw[0] = c0 << 1;
		zw[1] = (c1 << 1) | (c0 >> 31);
		zw[2] = (c2 << 1) | (c1 >> 31);
		zw[3] = (c3 << 1) | (c2 >> 31);
		zw[4] = (d0 << 1) | (c3 >> 31);
		zw[5] = (d1 << 1) | (d0 >> 31);
		zw[6] = (d2 << 1) | (d1 >> 31);
		zw[7] = (d3 << 1) | (d2 >> 31);

		/*
		 * We now do the reduction modulo the field polynomial
		 * to get back to 128 bits.
		 */
		for (i = 0; i < 4; i ++) {
			uint32_t lw;

			lw = zw[i];
			zw[i + 4] ^= lw ^ (lw >> 1) ^ (lw >> 2) ^ (lw >> 7);
			zw[i + 3] ^= (lw << 31) ^ (lw << 30) ^ (lw << 25);
		}
		memcpy(yw, zw + 4, sizeof yw);
	}

	/*
	 * Encode back the result.
	 */
	br_enc32be(yb, yw[3]);
	br_enc32be(yb + 4, yw[2]);
	br_enc32be(yb + 8, yw[1]);
	br_enc32be(yb + 12, yw[0]);
}


/* ===== src/hash/ghash_ctmul32.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

/*
 * This implementation uses 32-bit multiplications, and only the low
 * 32 bits for each multiplication result. This is meant primarily for
 * the ARM Cortex M0 and M0+, whose multiplication opcode does not yield
 * the upper 32 bits; but it might also be useful on architectures where
 * access to the upper 32 bits requires use of specific registers that
 * create contention (e.g. on i386, "mul" necessarily outputs the result
 * in edx:eax, while "imul" can use any registers but is limited to the
 * low 32 bits).
 *
 * The implementation trick that is used here is bit-reversing (bit 0
 * is swapped with bit 31, bit 1 with bit 30, and so on). In GF(2)[X],
 * for all values x and y, we have:
 *    rev32(x) * rev32(y) = rev64(x * y)
 * In other words, if we bit-reverse (over 32 bits) the operands, then we
 * bit-reverse (over 64 bits) the result.
 */

/*
 * Multiplication in GF(2)[X], truncated to its low 32 bits.
 */
static inline uint32_t
bmul32(uint32_t x, uint32_t y)
{
	uint32_t x0, x1, x2, x3;
	uint32_t y0, y1, y2, y3;
	uint32_t z0, z1, z2, z3;

	x0 = x & (uint32_t)0x11111111;
	x1 = x & (uint32_t)0x22222222;
	x2 = x & (uint32_t)0x44444444;
	x3 = x & (uint32_t)0x88888888;
	y0 = y & (uint32_t)0x11111111;
	y1 = y & (uint32_t)0x22222222;
	y2 = y & (uint32_t)0x44444444;
	y3 = y & (uint32_t)0x88888888;
	z0 = (x0 * y0) ^ (x1 * y3) ^ (x2 * y2) ^ (x3 * y1);
	z1 = (x0 * y1) ^ (x1 * y0) ^ (x2 * y3) ^ (x3 * y2);
	z2 = (x0 * y2) ^ (x1 * y1) ^ (x2 * y0) ^ (x3 * y3);
	z3 = (x0 * y3) ^ (x1 * y2) ^ (x2 * y1) ^ (x3 * y0);
	z0 &= (uint32_t)0x11111111;
	z1 &= (uint32_t)0x22222222;
	z2 &= (uint32_t)0x44444444;
	z3 &= (uint32_t)0x88888888;
	return z0 | z1 | z2 | z3;
}

/*
 * Bit-reverse a 32-bit word.
 */
static uint32_t
rev32(uint32_t x)
{
#define RMS(m, s)   do { \
		x = ((x & (uint32_t)(m)) << (s)) \
			| ((x >> (s)) & (uint32_t)(m)); \
	} while (0)

	RMS(0x55555555, 1);
	RMS(0x33333333, 2);
	RMS(0x0F0F0F0F, 4);
	RMS(0x00FF00FF, 8);
	return (x << 16) | (x >> 16);

#undef RMS
}

/* see bearssl_hash.h */
void
br_ghash_ctmul32(void *y, const void *h, const void *data, size_t len)
{
	/*
	 * This implementation is similar to br_ghash_ctmul() except
	 * that we have to do the multiplication twice, with the
	 * "normal" and "bit reversed" operands. Hence we end up with
	 * eighteen 32-bit multiplications instead of nine.
	 */

	const unsigned char *buf, *hb;
	unsigned char *yb;
	uint32_t yw[4];
	uint32_t hw[4], hwr[4];

	buf = data;
	yb = y;
	hb = h;
	yw[3] = br_dec32be(yb);
	yw[2] = br_dec32be(yb + 4);
	yw[1] = br_dec32be(yb + 8);
	yw[0] = br_dec32be(yb + 12);
	hw[3] = br_dec32be(hb);
	hw[2] = br_dec32be(hb + 4);
	hw[1] = br_dec32be(hb + 8);
	hw[0] = br_dec32be(hb + 12);
	hwr[3] = rev32(hw[3]);
	hwr[2] = rev32(hw[2]);
	hwr[1] = rev32(hw[1]);
	hwr[0] = rev32(hw[0]);
	while (len > 0) {
		const unsigned char *src;
		unsigned char tmp[16];
		int i;
		uint32_t a[18], b[18], c[18];
		uint32_t d0, d1, d2, d3, d4, d5, d6, d7;
		uint32_t zw[8];

		if (len >= 16) {
			src = buf;
			buf += 16;
			len -= 16;
		} else {
			memcpy(tmp, buf, len);
			memset(tmp + len, 0, (sizeof tmp) - len);
			src = tmp;
			len = 0;
		}
		yw[3] ^= br_dec32be(src);
		yw[2] ^= br_dec32be(src + 4);
		yw[1] ^= br_dec32be(src + 8);
		yw[0] ^= br_dec32be(src + 12);

		/*
		 * We are using Karatsuba: the 128x128 multiplication is
		 * reduced to three 64x64 multiplications, hence nine
		 * 32x32 multiplications. With the bit-reversal trick,
		 * we have to perform 18 32x32 multiplications.
		 */

		/*
		 * y[0,1]*h[0,1] -> 0,1,4
		 * y[2,3]*h[2,3] -> 2,3,5
		 * (y[0,1]+y[2,3])*(h[0,1]+h[2,3]) -> 6,7,8
		 */

		a[0] = yw[0];
		a[1] = yw[1];
		a[2] = yw[2];
		a[3] = yw[3];
		a[4] = a[0] ^ a[1];
		a[5] = a[2] ^ a[3];
		a[6] = a[0] ^ a[2];
		a[7] = a[1] ^ a[3];
		a[8] = a[6] ^ a[7];

		a[ 9] = rev32(yw[0]);
		a[10] = rev32(yw[1]);
		a[11] = rev32(yw[2]);
		a[12] = rev32(yw[3]);
		a[13] = a[ 9] ^ a[10];
		a[14] = a[11] ^ a[12];
		a[15] = a[ 9] ^ a[11];
		a[16] = a[10] ^ a[12];
		a[17] = a[15] ^ a[16];

		b[0] = hw[0];
		b[1] = hw[1];
		b[2] = hw[2];
		b[3] = hw[3];
		b[4] = b[0] ^ b[1];
		b[5] = b[2] ^ b[3];
		b[6] = b[0] ^ b[2];
		b[7] = b[1] ^ b[3];
		b[8] = b[6] ^ b[7];

		b[ 9] = hwr[0];
		b[10] = hwr[1];
		b[11] = hwr[2];
		b[12] = hwr[3];
		b[13] = b[ 9] ^ b[10];
		b[14] = b[11] ^ b[12];
		b[15] = b[ 9] ^ b[11];
		b[16] = b[10] ^ b[12];
		b[17] = b[15] ^ b[16];

		for (i = 0; i < 18; i ++) {
			c[i] = bmul32(a[i], b[i]);
		}

		c[4] ^= c[0] ^ c[1];
		c[5] ^= c[2] ^ c[3];
		c[8] ^= c[6] ^ c[7];

		c[13] ^= c[ 9] ^ c[10];
		c[14] ^= c[11] ^ c[12];
		c[17] ^= c[15] ^ c[16];

		/*
		 * y[0,1]*h[0,1] -> 0,9^4,1^13,10
		 * y[2,3]*h[2,3] -> 2,11^5,3^14,12
		 * (y[0,1]+y[2,3])*(h[0,1]+h[2,3]) -> 6,15^8,7^17,16
		 */
		d0 = c[0];
		d1 = c[4] ^ (rev32(c[9]) >> 1);
		d2 = c[1] ^ c[0] ^ c[2] ^ c[6] ^ (rev32(c[13]) >> 1);
		d3 = c[4] ^ c[5] ^ c[8]
			^ (rev32(c[10] ^ c[9] ^ c[11] ^ c[15]) >> 1);
		d4 = c[2] ^ c[1] ^ c[3] ^ c[7]
			^ (rev32(c[13] ^ c[14] ^ c[17]) >> 1);
		d5 = c[5] ^ (rev32(c[11] ^ c[10] ^ c[12] ^ c[16]) >> 1);
		d6 = c[3] ^ (rev32(c[14]) >> 1);
		d7 = rev32(c[12]) >> 1;

		zw[0] = d0 << 1;
		zw[1] = (d1 << 1) | (d0 >> 31);
		zw[2] = (d2 << 1) | (d1 >> 31);
		zw[3] = (d3 << 1) | (d2 >> 31);
		zw[4] = (d4 << 1) | (d3 >> 31);
		zw[5] = (d5 << 1) | (d4 >> 31);
		zw[6] = (d6 << 1) | (d5 >> 31);
		zw[7] = (d7 << 1) | (d6 >> 31);

		for (i = 0; i < 4; i ++) {
			uint32_t lw;

			lw = zw[i];
			zw[i + 4] ^= lw ^ (lw >> 1) ^ (lw >> 2) ^ (lw >> 7);
			zw[i + 3] ^= (lw << 31) ^ (lw << 30) ^ (lw << 25);
		}
		memcpy(yw, zw + 4, sizeof yw);
	}
	br_enc32be(yb, yw[3]);
	br_enc32be(yb + 4, yw[2]);
	br_enc32be(yb + 8, yw[1]);
	br_enc32be(yb + 12, yw[0]);
}


/* ===== src/hash/md5.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

#define F(B, C, D)     ((((C) ^ (D)) & (B)) ^ (D))
#define G(B, C, D)     ((((C) ^ (B)) & (D)) ^ (C))
#define H(B, C, D)     ((B) ^ (C) ^ (D))
#define I(B, C, D)     ((C) ^ ((B) | ~(D)))

#define ROTL(x, n)    (((x) << (n)) | ((x) >> (32 - (n))))

/* see inner.h */
const uint32_t br_md5_IV[4] = {
	0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476
};

static const uint32_t K[64] = {
	0xD76AA478, 0xE8C7B756, 0x242070DB, 0xC1BDCEEE,
	0xF57C0FAF, 0x4787C62A, 0xA8304613, 0xFD469501,
	0x698098D8, 0x8B44F7AF, 0xFFFF5BB1, 0x895CD7BE,
	0x6B901122, 0xFD987193, 0xA679438E, 0x49B40821,

	0xF61E2562, 0xC040B340, 0x265E5A51, 0xE9B6C7AA,
	0xD62F105D, 0x02441453, 0xD8A1E681, 0xE7D3FBC8,
	0x21E1CDE6, 0xC33707D6, 0xF4D50D87, 0x455A14ED,
	0xA9E3E905, 0xFCEFA3F8, 0x676F02D9, 0x8D2A4C8A,

	0xFFFA3942, 0x8771F681, 0x6D9D6122, 0xFDE5380C,
	0xA4BEEA44, 0x4BDECFA9, 0xF6BB4B60, 0xBEBFBC70,
	0x289B7EC6, 0xEAA127FA, 0xD4EF3085, 0x04881D05,
	0xD9D4D039, 0xE6DB99E5, 0x1FA27CF8, 0xC4AC5665,

	0xF4292244, 0x432AFF97, 0xAB9423A7, 0xFC93A039,
	0x655B59C3, 0x8F0CCC92, 0xFFEFF47D, 0x85845DD1,
	0x6FA87E4F, 0xFE2CE6E0, 0xA3014314, 0x4E0811A1,
	0xF7537E82, 0xBD3AF235, 0x2AD7D2BB, 0xEB86D391
};

static const unsigned char MP[48] = {
	1, 6, 11, 0, 5, 10, 15, 4, 9, 14, 3, 8, 13, 2, 7, 12,
	5, 8, 11, 14, 1, 4, 7, 10, 13, 0, 3, 6, 9, 12, 15, 2,
	0, 7, 14, 5, 12, 3, 10, 1, 8, 15, 6, 13, 4, 11, 2, 9
};

/* see inner.h */
void
br_md5_round(const unsigned char *buf, uint32_t *val)
{
	uint32_t m[16];
	uint32_t a, b, c, d;
	int i;

	a = val[0];
	b = val[1];
	c = val[2];
	d = val[3];
	/* obsolete
	for (i = 0; i < 16; i ++) {
		m[i] = br_dec32le(buf + (i << 2));
	}
	*/
	br_range_dec32le(m, 16, buf);

	for (i = 0; i < 16; i += 4) {
		a = b + ROTL(a + F(b, c, d) + m[i + 0] + K[i + 0],  7);
		d = a + ROTL(d + F(a, b, c) + m[i + 1] + K[i + 1], 12);
		c = d + ROTL(c + F(d, a, b) + m[i + 2] + K[i + 2], 17);
		b = c + ROTL(b + F(c, d, a) + m[i + 3] + K[i + 3], 22);
	}
	for (i = 16; i < 32; i += 4) {
		a = b + ROTL(a + G(b, c, d) + m[MP[i - 16]] + K[i + 0],  5);
		d = a + ROTL(d + G(a, b, c) + m[MP[i - 15]] + K[i + 1],  9);
		c = d + ROTL(c + G(d, a, b) + m[MP[i - 14]] + K[i + 2], 14);
		b = c + ROTL(b + G(c, d, a) + m[MP[i - 13]] + K[i + 3], 20);
	}
	for (i = 32; i < 48; i += 4) {
		a = b + ROTL(a + H(b, c, d) + m[MP[i - 16]] + K[i + 0],  4);
		d = a + ROTL(d + H(a, b, c) + m[MP[i - 15]] + K[i + 1], 11);
		c = d + ROTL(c + H(d, a, b) + m[MP[i - 14]] + K[i + 2], 16);
		b = c + ROTL(b + H(c, d, a) + m[MP[i - 13]] + K[i + 3], 23);
	}
	for (i = 48; i < 64; i += 4) {
		a = b + ROTL(a + I(b, c, d) + m[MP[i - 16]] + K[i + 0],  6);
		d = a + ROTL(d + I(a, b, c) + m[MP[i - 15]] + K[i + 1], 10);
		c = d + ROTL(c + I(d, a, b) + m[MP[i - 14]] + K[i + 2], 15);
		b = c + ROTL(b + I(c, d, a) + m[MP[i - 13]] + K[i + 3], 21);
	}

	val[0] += a;
	val[1] += b;
	val[2] += c;
	val[3] += d;
}

/* see bearssl.h */
void
br_md5_init(br_md5_context *cc)
{
	cc->vtable = &br_md5_vtable;
	memcpy(cc->val, br_md5_IV, sizeof cc->val);
	cc->count = 0;
}

/* see bearssl.h */
void
br_md5_update(br_md5_context *cc, const void *data, size_t len)
{
	const unsigned char *buf;
	size_t ptr;

	buf = data;
	ptr = (size_t)cc->count & 63;
	while (len > 0) {
		size_t clen;

		clen = 64 - ptr;
		if (clen > len) {
			clen = len;
		}
		memcpy(cc->buf + ptr, buf, clen);
		ptr += clen;
		buf += clen;
		len -= clen;
		cc->count += (uint64_t)clen;
		if (ptr == 64) {
			br_md5_round(cc->buf, cc->val);
			ptr = 0;
		}
	}
}

/* see bearssl.h */
void
br_md5_out(const br_md5_context *cc, void *dst)
{
	unsigned char buf[64];
	uint32_t val[4];
	size_t ptr;

	ptr = (size_t)cc->count & 63;
	memcpy(buf, cc->buf, ptr);
	memcpy(val, cc->val, sizeof val);
	buf[ptr ++] = 0x80;
	if (ptr > 56) {
		memset(buf + ptr, 0, 64 - ptr);
		br_md5_round(buf, val);
		memset(buf, 0, 56);
	} else {
		memset(buf + ptr, 0, 56 - ptr);
	}
	br_enc64le(buf + 56, cc->count << 3);
	br_md5_round(buf, val);
	br_range_enc32le(dst, val, 4);
}

/* see bearssl.h */
uint64_t
br_md5_state(const br_md5_context *cc, void *dst)
{
	br_range_enc32le(dst, cc->val, 4);
	return cc->count;
}

/* see bearssl.h */
void
br_md5_set_state(br_md5_context *cc, const void *stb, uint64_t count)
{
	br_range_dec32le(cc->val, 4, stb);
	cc->count = count;
}

/* see bearssl.h */
const br_hash_class br_md5_vtable = {
	sizeof(br_md5_context),
	BR_HASHDESC_ID(br_md5_ID)
		| BR_HASHDESC_OUT(16)
		| BR_HASHDESC_STATE(16)
		| BR_HASHDESC_LBLEN(6)
		| BR_HASHDESC_MD_PADDING,
	(void (*)(const br_hash_class **))&br_md5_init,
	(void (*)(const br_hash_class **, const void *, size_t))&br_md5_update,
	(void (*)(const br_hash_class *const *, void *))&br_md5_out,
	(uint64_t (*)(const br_hash_class *const *, void *))&br_md5_state,
	(void (*)(const br_hash_class **, const void *, uint64_t))
		&br_md5_set_state
};


/* ===== src/hash/md5sha1.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

/* see bearssl.h */
void
br_md5sha1_init(br_md5sha1_context *cc)
{
	cc->vtable = &br_md5sha1_vtable;
	memcpy(cc->val_md5, br_md5_IV, sizeof cc->val_md5);
	memcpy(cc->val_sha1, br_sha1_IV, sizeof cc->val_sha1);
	cc->count = 0;
}

/* see bearssl.h */
void
br_md5sha1_update(br_md5sha1_context *cc, const void *data, size_t len)
{
	const unsigned char *buf;
	size_t ptr;

	buf = data;
	ptr = (size_t)cc->count & 63;
	while (len > 0) {
		size_t clen;

		clen = 64 - ptr;
		if (clen > len) {
			clen = len;
		}
		memcpy(cc->buf + ptr, buf, clen);
		ptr += clen;
		buf += clen;
		len -= clen;
		cc->count += (uint64_t)clen;
		if (ptr == 64) {
			br_md5_round(cc->buf, cc->val_md5);
			br_sha1_round(cc->buf, cc->val_sha1);
			ptr = 0;
		}
	}
}

/* see bearssl.h */
void
br_md5sha1_out(const br_md5sha1_context *cc, void *dst)
{
	unsigned char buf[64];
	uint32_t val_md5[4];
	uint32_t val_sha1[5];
	size_t ptr;
	unsigned char *out;
	uint64_t count;

	count = cc->count;
	ptr = (size_t)count & 63;
	memcpy(buf, cc->buf, ptr);
	memcpy(val_md5, cc->val_md5, sizeof val_md5);
	memcpy(val_sha1, cc->val_sha1, sizeof val_sha1);
	buf[ptr ++] = 0x80;
	if (ptr > 56) {
		memset(buf + ptr, 0, 64 - ptr);
		br_md5_round(buf, val_md5);
		br_sha1_round(buf, val_sha1);
		memset(buf, 0, 56);
	} else {
		memset(buf + ptr, 0, 56 - ptr);
	}
	count <<= 3;
	br_enc64le(buf + 56, count);
	br_md5_round(buf, val_md5);
	br_enc64be(buf + 56, count);
	br_sha1_round(buf, val_sha1);
	out = dst;
	br_range_enc32le(out, val_md5, 4);
	br_range_enc32be(out + 16, val_sha1, 5);
}

/* see bearssl.h */
uint64_t
br_md5sha1_state(const br_md5sha1_context *cc, void *dst)
{
	unsigned char *out;

	out = dst;
	br_range_enc32le(out, cc->val_md5, 4);
	br_range_enc32be(out + 16, cc->val_sha1, 5);
	return cc->count;
}

/* see bearssl.h */
void
br_md5sha1_set_state(br_md5sha1_context *cc, const void *stb, uint64_t count)
{
	const unsigned char *buf;

	buf = stb;
	br_range_dec32le(cc->val_md5, 4, buf);
	br_range_dec32be(cc->val_sha1, 5, buf + 16);
	cc->count = count;
}

/* see bearssl.h */
const br_hash_class br_md5sha1_vtable = {
	sizeof(br_md5sha1_context),
	BR_HASHDESC_ID(br_md5sha1_ID)
		| BR_HASHDESC_OUT(36)
		| BR_HASHDESC_STATE(36)
		| BR_HASHDESC_LBLEN(6),
	(void (*)(const br_hash_class **))&br_md5sha1_init,
	(void (*)(const br_hash_class **, const void *, size_t))
		&br_md5sha1_update,
	(void (*)(const br_hash_class *const *, void *))
		&br_md5sha1_out,
	(uint64_t (*)(const br_hash_class *const *, void *))
		&br_md5sha1_state,
	(void (*)(const br_hash_class **, const void *, uint64_t))
		&br_md5sha1_set_state
};


/* ===== src/hash/multihash.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

/*
 * An aggregate context that is large enough for all supported hash
 * functions.
 */
typedef union {
	const br_hash_class *vtable;
	br_md5_context md5;
	br_sha1_context sha1;
	br_sha224_context sha224;
	br_sha256_context sha256;
	br_sha384_context sha384;
	br_sha512_context sha512;
} gen_hash_context;

/*
 * Get the offset to the state for a specific hash function within the
 * context structure. This shall be called only for the supported hash
 * functions,
 */
static size_t
get_state_offset(int id)
{
	if (id >= 5) {
		/*
		 * SHA-384 has id 5, and SHA-512 has id 6. Both use
		 * eight 64-bit words for their state.
		 */
		return offsetof(br_multihash_context, val_64)
			+ ((size_t)(id - 5) * (8 * sizeof(uint64_t)));
	} else {
		/*
		 * MD5 has id 1, SHA-1 has id 2, SHA-224 has id 3 and
		 * SHA-256 has id 4. They use 32-bit words for their
		 * states (4 words for MD5, 5 for SHA-1, 8 for SHA-224
		 * and 8 for SHA-256).
		 */
		unsigned x;

		x = id - 1;
		x = ((x + (x & (x >> 1))) << 2) + (x >> 1);
		return offsetof(br_multihash_context, val_32)
			+ x * sizeof(uint32_t);
	}
}

/* see bearssl_hash.h */
void
br_multihash_zero(br_multihash_context *ctx)
{
	/*
	 * This is not standard, but yields very short and efficient code,
	 * and it works "everywhere".
	 */
	memset(ctx, 0, sizeof *ctx);
}

/* see bearssl_hash.h */
void
br_multihash_init(br_multihash_context *ctx)
{
	int i;

	ctx->count = 0;
	for (i = 1; i <= 6; i ++) {
		const br_hash_class *hc;

		hc = ctx->impl[i - 1];
		if (hc != NULL) {
			gen_hash_context g;

			hc->init(&g.vtable);
			hc->state(&g.vtable,
				(unsigned char *)ctx + get_state_offset(i));
		}
	}
}

/* see bearssl_hash.h */
void
br_multihash_update(br_multihash_context *ctx, const void *data, size_t len)
{
	const unsigned char *buf;
	size_t ptr;

	buf = data;
	ptr = (size_t)ctx->count & 127;
	while (len > 0) {
		size_t clen;

		clen = 128 - ptr;
		if (clen > len) {
			clen = len;
		}
		memcpy(ctx->buf + ptr, buf, clen);
		ptr += clen;
		buf += clen;
		len -= clen;
		ctx->count += (uint64_t)clen;
		if (ptr == 128) {
			int i;

			for (i = 1; i <= 6; i ++) {
				const br_hash_class *hc;

				hc = ctx->impl[i - 1];
				if (hc != NULL) {
					gen_hash_context g;
					unsigned char *state;

					state = (unsigned char *)ctx
						+ get_state_offset(i);
					hc->set_state(&g.vtable,
						state, ctx->count - 128);
					hc->update(&g.vtable, ctx->buf, 128);
					hc->state(&g.vtable, state);
				}
			}
			ptr = 0;
		}
	}
}

/* see bearssl_hash.h */
size_t
br_multihash_out(const br_multihash_context *ctx, int id, void *dst)
{
	const br_hash_class *hc;
	gen_hash_context g;
	const unsigned char *state;

	hc = ctx->impl[id - 1];
	if (hc == NULL) {
		return 0;
	}
	state = (const unsigned char *)ctx + get_state_offset(id);
	hc->set_state(&g.vtable, state, ctx->count & ~(uint64_t)127);
	hc->update(&g.vtable, ctx->buf, ctx->count & (uint64_t)127);
	hc->out(&g.vtable, dst);
	return (hc->desc >> BR_HASHDESC_OUT_OFF) & BR_HASHDESC_OUT_MASK;
}


/* ===== src/hash/sha1.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

#define F(B, C, D)     ((((C) ^ (D)) & (B)) ^ (D))
#define G(B, C, D)     ((B) ^ (C) ^ (D))
#define H(B, C, D)     (((D) & (C)) | (((D) | (C)) & (B)))
#define I(B, C, D)     G(B, C, D)

#define ROTL(x, n)    (((x) << (n)) | ((x) >> (32 - (n))))

#define K1     ((uint32_t)0x5A827999)
#define K2     ((uint32_t)0x6ED9EBA1)
#define K3     ((uint32_t)0x8F1BBCDC)
#define K4     ((uint32_t)0xCA62C1D6)

/* see inner.h */
const uint32_t br_sha1_IV[5] = {
	0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0
};

/* see inner.h */
void
br_sha1_round(const unsigned char *buf, uint32_t *val)
{
	uint32_t m[80];
	uint32_t a, b, c, d, e;
	int i;

	a = val[0];
	b = val[1];
	c = val[2];
	d = val[3];
	e = val[4];
	br_range_dec32be(m, 16, buf);
	for (i = 16; i < 80; i ++) {
		uint32_t x = m[i - 3] ^ m[i - 8] ^ m[i - 14] ^ m[i - 16];
		m[i] = ROTL(x, 1);
	}

	for (i = 0; i < 20; i += 5) {
		e += ROTL(a, 5) + F(b, c, d) + K1 + m[i + 0]; b = ROTL(b, 30);
		d += ROTL(e, 5) + F(a, b, c) + K1 + m[i + 1]; a = ROTL(a, 30);
		c += ROTL(d, 5) + F(e, a, b) + K1 + m[i + 2]; e = ROTL(e, 30);
		b += ROTL(c, 5) + F(d, e, a) + K1 + m[i + 3]; d = ROTL(d, 30);
		a += ROTL(b, 5) + F(c, d, e) + K1 + m[i + 4]; c = ROTL(c, 30);
	}
	for (i = 20; i < 40; i += 5) {
		e += ROTL(a, 5) + G(b, c, d) + K2 + m[i + 0]; b = ROTL(b, 30);
		d += ROTL(e, 5) + G(a, b, c) + K2 + m[i + 1]; a = ROTL(a, 30);
		c += ROTL(d, 5) + G(e, a, b) + K2 + m[i + 2]; e = ROTL(e, 30);
		b += ROTL(c, 5) + G(d, e, a) + K2 + m[i + 3]; d = ROTL(d, 30);
		a += ROTL(b, 5) + G(c, d, e) + K2 + m[i + 4]; c = ROTL(c, 30);
	}
	for (i = 40; i < 60; i += 5) {
		e += ROTL(a, 5) + H(b, c, d) + K3 + m[i + 0]; b = ROTL(b, 30);
		d += ROTL(e, 5) + H(a, b, c) + K3 + m[i + 1]; a = ROTL(a, 30);
		c += ROTL(d, 5) + H(e, a, b) + K3 + m[i + 2]; e = ROTL(e, 30);
		b += ROTL(c, 5) + H(d, e, a) + K3 + m[i + 3]; d = ROTL(d, 30);
		a += ROTL(b, 5) + H(c, d, e) + K3 + m[i + 4]; c = ROTL(c, 30);
	}
	for (i = 60; i < 80; i += 5) {
		e += ROTL(a, 5) + I(b, c, d) + K4 + m[i + 0]; b = ROTL(b, 30);
		d += ROTL(e, 5) + I(a, b, c) + K4 + m[i + 1]; a = ROTL(a, 30);
		c += ROTL(d, 5) + I(e, a, b) + K4 + m[i + 2]; e = ROTL(e, 30);
		b += ROTL(c, 5) + I(d, e, a) + K4 + m[i + 3]; d = ROTL(d, 30);
		a += ROTL(b, 5) + I(c, d, e) + K4 + m[i + 4]; c = ROTL(c, 30);
	}

	val[0] += a;
	val[1] += b;
	val[2] += c;
	val[3] += d;
	val[4] += e;
}

/* see bearssl.h */
void
br_sha1_init(br_sha1_context *cc)
{
	cc->vtable = &br_sha1_vtable;
	memcpy(cc->val, br_sha1_IV, sizeof cc->val);
	cc->count = 0;
}

/* see bearssl.h */
void
br_sha1_update(br_sha1_context *cc, const void *data, size_t len)
{
	const unsigned char *buf;
	size_t ptr;

	buf = data;
	ptr = (size_t)cc->count & 63;
	while (len > 0) {
		size_t clen;

		clen = 64 - ptr;
		if (clen > len) {
			clen = len;
		}
		memcpy(cc->buf + ptr, buf, clen);
		ptr += clen;
		buf += clen;
		len -= clen;
		cc->count += (uint64_t)clen;
		if (ptr == 64) {
			br_sha1_round(cc->buf, cc->val);
			ptr = 0;
		}
	}
}

/* see bearssl.h */
void
br_sha1_out(const br_sha1_context *cc, void *dst)
{
	unsigned char buf[64];
	uint32_t val[5];
	size_t ptr;

	ptr = (size_t)cc->count & 63;
	memcpy(buf, cc->buf, ptr);
	memcpy(val, cc->val, sizeof val);
	buf[ptr ++] = 0x80;
	if (ptr > 56) {
		memset(buf + ptr, 0, 64 - ptr);
		br_sha1_round(buf, val);
		memset(buf, 0, 56);
	} else {
		memset(buf + ptr, 0, 56 - ptr);
	}
	br_enc64be(buf + 56, cc->count << 3);
	br_sha1_round(buf, val);
	br_range_enc32be(dst, val, 5);
}

/* see bearssl.h */
uint64_t
br_sha1_state(const br_sha1_context *cc, void *dst)
{
	br_range_enc32be(dst, cc->val, 5);
	return cc->count;
}

/* see bearssl.h */
void
br_sha1_set_state(br_sha1_context *cc, const void *stb, uint64_t count)
{
	br_range_dec32be(cc->val, 5, stb);
	cc->count = count;
}

/* see bearssl.h */
const br_hash_class br_sha1_vtable = {
	sizeof(br_sha1_context),
	BR_HASHDESC_ID(br_sha1_ID)
		| BR_HASHDESC_OUT(20)
		| BR_HASHDESC_STATE(20)
		| BR_HASHDESC_LBLEN(6)
		| BR_HASHDESC_MD_PADDING
		| BR_HASHDESC_MD_PADDING_BE,
	(void (*)(const br_hash_class **))&br_sha1_init,
	(void (*)(const br_hash_class **, const void *, size_t))&br_sha1_update,
	(void (*)(const br_hash_class *const *, void *))&br_sha1_out,
	(uint64_t (*)(const br_hash_class *const *, void *))&br_sha1_state,
	(void (*)(const br_hash_class **, const void *, uint64_t))
		&br_sha1_set_state
};


/* ===== src/hash/sha2big.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

#define CH(X, Y, Z)    ((((Y) ^ (Z)) & (X)) ^ (Z))
#define MAJ(X, Y, Z)   (((Y) & (Z)) | (((Y) | (Z)) & (X)))

#define ROTR(x, n)    (((uint64_t)(x) << (64 - (n))) | ((uint64_t)(x) >> (n)))

#define BSG5_0(x)      (ROTR(x, 28) ^ ROTR(x, 34) ^ ROTR(x, 39))
#define BSG5_1(x)      (ROTR(x, 14) ^ ROTR(x, 18) ^ ROTR(x, 41))
#define SSG5_0(x)      (ROTR(x, 1) ^ ROTR(x, 8) ^ (uint64_t)((x) >> 7))
#define SSG5_1(x)      (ROTR(x, 19) ^ ROTR(x, 61) ^ (uint64_t)((x) >> 6))

static const uint64_t IV384[8] = {
	0xCBBB9D5DC1059ED8, 0x629A292A367CD507,
	0x9159015A3070DD17, 0x152FECD8F70E5939,
	0x67332667FFC00B31, 0x8EB44A8768581511,
	0xDB0C2E0D64F98FA7, 0x47B5481DBEFA4FA4
};

static const uint64_t IV512[8] = {
	0x6A09E667F3BCC908, 0xBB67AE8584CAA73B,
	0x3C6EF372FE94F82B, 0xA54FF53A5F1D36F1,
	0x510E527FADE682D1, 0x9B05688C2B3E6C1F,
	0x1F83D9ABFB41BD6B, 0x5BE0CD19137E2179
};

static const uint64_t K[80] = {
	0x428A2F98D728AE22, 0x7137449123EF65CD,
	0xB5C0FBCFEC4D3B2F, 0xE9B5DBA58189DBBC,
	0x3956C25BF348B538, 0x59F111F1B605D019,
	0x923F82A4AF194F9B, 0xAB1C5ED5DA6D8118,
	0xD807AA98A3030242, 0x12835B0145706FBE,
	0x243185BE4EE4B28C, 0x550C7DC3D5FFB4E2,
	0x72BE5D74F27B896F, 0x80DEB1FE3B1696B1,
	0x9BDC06A725C71235, 0xC19BF174CF692694,
	0xE49B69C19EF14AD2, 0xEFBE4786384F25E3,
	0x0FC19DC68B8CD5B5, 0x240CA1CC77AC9C65,
	0x2DE92C6F592B0275, 0x4A7484AA6EA6E483,
	0x5CB0A9DCBD41FBD4, 0x76F988DA831153B5,
	0x983E5152EE66DFAB, 0xA831C66D2DB43210,
	0xB00327C898FB213F, 0xBF597FC7BEEF0EE4,
	0xC6E00BF33DA88FC2, 0xD5A79147930AA725,
	0x06CA6351E003826F, 0x142929670A0E6E70,
	0x27B70A8546D22FFC, 0x2E1B21385C26C926,
	0x4D2C6DFC5AC42AED, 0x53380D139D95B3DF,
	0x650A73548BAF63DE, 0x766A0ABB3C77B2A8,
	0x81C2C92E47EDAEE6, 0x92722C851482353B,
	0xA2BFE8A14CF10364, 0xA81A664BBC423001,
	0xC24B8B70D0F89791, 0xC76C51A30654BE30,
	0xD192E819D6EF5218, 0xD69906245565A910,
	0xF40E35855771202A, 0x106AA07032BBD1B8,
	0x19A4C116B8D2D0C8, 0x1E376C085141AB53,
	0x2748774CDF8EEB99, 0x34B0BCB5E19B48A8,
	0x391C0CB3C5C95A63, 0x4ED8AA4AE3418ACB,
	0x5B9CCA4F7763E373, 0x682E6FF3D6B2B8A3,
	0x748F82EE5DEFB2FC, 0x78A5636F43172F60,
	0x84C87814A1F0AB72, 0x8CC702081A6439EC,
	0x90BEFFFA23631E28, 0xA4506CEBDE82BDE9,
	0xBEF9A3F7B2C67915, 0xC67178F2E372532B,
	0xCA273ECEEA26619C, 0xD186B8C721C0C207,
	0xEADA7DD6CDE0EB1E, 0xF57D4F7FEE6ED178,
	0x06F067AA72176FBA, 0x0A637DC5A2C898A6,
	0x113F9804BEF90DAE, 0x1B710B35131C471B,
	0x28DB77F523047D84, 0x32CAAB7B40C72493,
	0x3C9EBE0A15C9BEBC, 0x431D67C49C100D4C,
	0x4CC5D4BECB3E42B6, 0x597F299CFC657E2A,
	0x5FCB6FAB3AD6FAEC, 0x6C44198C4A475817
};

static void
sha2big_round(const unsigned char *buf, uint64_t *val)
{

#define SHA2BIG_STEP(A, B, C, D, E, F, G, H, j)   do { \
		uint64_t T1, T2; \
		T1 = H + BSG5_1(E) + CH(E, F, G) + K[j] + w[j]; \
		T2 = BSG5_0(A) + MAJ(A, B, C); \
		D += T1; \
		H = T1 + T2; \
	} while (0)

	int i;
	uint64_t a, b, c, d, e, f, g, h;
	uint64_t w[80];

	br_range_dec64be(w, 16, buf);
	for (i = 16; i < 80; i ++) {
		w[i] = SSG5_1(w[i - 2]) + w[i - 7]
			+ SSG5_0(w[i - 15]) + w[i - 16];
	}
	a = val[0];
	b = val[1];
	c = val[2];
	d = val[3];
	e = val[4];
	f = val[5];
	g = val[6];
	h = val[7];
	for (i = 0; i < 80; i += 8) {
		SHA2BIG_STEP(a, b, c, d, e, f, g, h, i + 0);
		SHA2BIG_STEP(h, a, b, c, d, e, f, g, i + 1);
		SHA2BIG_STEP(g, h, a, b, c, d, e, f, i + 2);
		SHA2BIG_STEP(f, g, h, a, b, c, d, e, i + 3);
		SHA2BIG_STEP(e, f, g, h, a, b, c, d, i + 4);
		SHA2BIG_STEP(d, e, f, g, h, a, b, c, i + 5);
		SHA2BIG_STEP(c, d, e, f, g, h, a, b, i + 6);
		SHA2BIG_STEP(b, c, d, e, f, g, h, a, i + 7);
	}
	val[0] += a;
	val[1] += b;
	val[2] += c;
	val[3] += d;
	val[4] += e;
	val[5] += f;
	val[6] += g;
	val[7] += h;
}

static void
sha2big_update(br_sha384_context *cc, const void *data, size_t len)
{
	const unsigned char *buf;
	size_t ptr;

	buf = data;
	ptr = (size_t)cc->count & 127;
	cc->count += (uint64_t)len;
	while (len > 0) {
		size_t clen;

		clen = 128 - ptr;
		if (clen > len) {
			clen = len;
		}
		memcpy(cc->buf + ptr, buf, clen);
		ptr += clen;
		buf += clen;
		len -= clen;
		if (ptr == 128) {
			sha2big_round(cc->buf, cc->val);
			ptr = 0;
		}
	}
}

static void
sha2big_out(const br_sha384_context *cc, void *dst, int num)
{
	unsigned char buf[128];
	uint64_t val[8];
	size_t ptr;

	ptr = (size_t)cc->count & 127;
	memcpy(buf, cc->buf, ptr);
	memcpy(val, cc->val, sizeof val);
	buf[ptr ++] = 0x80;
	if (ptr > 112) {
		memset(buf + ptr, 0, 128 - ptr);
		sha2big_round(buf, val);
		memset(buf, 0, 112);
	} else {
		memset(buf + ptr, 0, 112 - ptr);
	}
	br_enc64be(buf + 112, cc->count >> 61);
	br_enc64be(buf + 120, cc->count << 3);
	sha2big_round(buf, val);
	br_range_enc64be(dst, val, num);
}

/* see bearssl.h */
void
br_sha384_init(br_sha384_context *cc)
{
	cc->vtable = &br_sha384_vtable;
	memcpy(cc->val, IV384, sizeof IV384);
	cc->count = 0;
}

/* see bearssl.h */
void
br_sha384_update(br_sha384_context *cc, const void *data, size_t len)
{
	sha2big_update(cc, data, len);
}

/* see bearssl.h */
void
br_sha384_out(const br_sha384_context *cc, void *dst)
{
	sha2big_out(cc, dst, 6);
}

/* see bearssl.h */
uint64_t
br_sha384_state(const br_sha384_context *cc, void *dst)
{
	br_range_enc64be(dst, cc->val, 8);
	return cc->count;
}

/* see bearssl.h */
void
br_sha384_set_state(br_sha384_context *cc, const void *stb, uint64_t count)
{
	br_range_dec64be(cc->val, 8, stb);
	cc->count = count;
}

/* see bearssl.h */
void
br_sha512_init(br_sha512_context *cc)
{
	cc->vtable = &br_sha512_vtable;
	memcpy(cc->val, IV512, sizeof IV512);
	cc->count = 0;
}

/* see bearssl.h */
void
br_sha512_out(const br_sha512_context *cc, void *dst)
{
	sha2big_out(cc, dst, 8);
}

/* see bearssl.h */
const br_hash_class br_sha384_vtable = {
	sizeof(br_sha384_context),
	BR_HASHDESC_ID(br_sha384_ID)
		| BR_HASHDESC_OUT(48)
		| BR_HASHDESC_STATE(64)
		| BR_HASHDESC_LBLEN(7)
		| BR_HASHDESC_MD_PADDING
		| BR_HASHDESC_MD_PADDING_BE
		| BR_HASHDESC_MD_PADDING_128,
	(void (*)(const br_hash_class **))&br_sha384_init,
	(void (*)(const br_hash_class **, const void *, size_t))
		&br_sha384_update,
	(void (*)(const br_hash_class *const *, void *))&br_sha384_out,
	(uint64_t (*)(const br_hash_class *const *, void *))&br_sha384_state,
	(void (*)(const br_hash_class **, const void *, uint64_t))
		&br_sha384_set_state
};

/* see bearssl.h */
const br_hash_class br_sha512_vtable = {
	sizeof(br_sha512_context),
	BR_HASHDESC_ID(br_sha512_ID)
		| BR_HASHDESC_OUT(64)
		| BR_HASHDESC_STATE(64)
		| BR_HASHDESC_LBLEN(7)
		| BR_HASHDESC_MD_PADDING
		| BR_HASHDESC_MD_PADDING_BE
		| BR_HASHDESC_MD_PADDING_128,
	(void (*)(const br_hash_class **))&br_sha512_init,
	(void (*)(const br_hash_class **, const void *, size_t))
		&br_sha512_update,
	(void (*)(const br_hash_class *const *, void *))&br_sha512_out,
	(uint64_t (*)(const br_hash_class *const *, void *))&br_sha512_state,
	(void (*)(const br_hash_class **, const void *, uint64_t))
		&br_sha512_set_state
};


/* ===== src/hash/sha2small.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

#define CH(X, Y, Z)    ((((Y) ^ (Z)) & (X)) ^ (Z))
#define MAJ(X, Y, Z)   (((Y) & (Z)) | (((Y) | (Z)) & (X)))

#define ROTR(x, n)    (((uint32_t)(x) << (32 - (n))) | ((uint32_t)(x) >> (n)))

#define BSG2_0(x)      (ROTR(x, 2) ^ ROTR(x, 13) ^ ROTR(x, 22))
#define BSG2_1(x)      (ROTR(x, 6) ^ ROTR(x, 11) ^ ROTR(x, 25))
#define SSG2_0(x)      (ROTR(x, 7) ^ ROTR(x, 18) ^ (uint32_t)((x) >> 3))
#define SSG2_1(x)      (ROTR(x, 17) ^ ROTR(x, 19) ^ (uint32_t)((x) >> 10))

/* see inner.h */
const uint32_t br_sha224_IV[8] = {
	0xC1059ED8, 0x367CD507, 0x3070DD17, 0xF70E5939,
	0xFFC00B31, 0x68581511, 0x64F98FA7, 0xBEFA4FA4
};

/* see inner.h */
const uint32_t br_sha256_IV[8] = {
	0x6A09E667, 0xBB67AE85, 0x3C6EF372, 0xA54FF53A,
	0x510E527F, 0x9B05688C, 0x1F83D9AB, 0x5BE0CD19
};

static const uint32_t K[64] = {
	0x428A2F98, 0x71374491, 0xB5C0FBCF, 0xE9B5DBA5,
	0x3956C25B, 0x59F111F1, 0x923F82A4, 0xAB1C5ED5,
	0xD807AA98, 0x12835B01, 0x243185BE, 0x550C7DC3,
	0x72BE5D74, 0x80DEB1FE, 0x9BDC06A7, 0xC19BF174,
	0xE49B69C1, 0xEFBE4786, 0x0FC19DC6, 0x240CA1CC,
	0x2DE92C6F, 0x4A7484AA, 0x5CB0A9DC, 0x76F988DA,
	0x983E5152, 0xA831C66D, 0xB00327C8, 0xBF597FC7,
	0xC6E00BF3, 0xD5A79147, 0x06CA6351, 0x14292967,
	0x27B70A85, 0x2E1B2138, 0x4D2C6DFC, 0x53380D13,
	0x650A7354, 0x766A0ABB, 0x81C2C92E, 0x92722C85,
	0xA2BFE8A1, 0xA81A664B, 0xC24B8B70, 0xC76C51A3,
	0xD192E819, 0xD6990624, 0xF40E3585, 0x106AA070,
	0x19A4C116, 0x1E376C08, 0x2748774C, 0x34B0BCB5,
	0x391C0CB3, 0x4ED8AA4A, 0x5B9CCA4F, 0x682E6FF3,
	0x748F82EE, 0x78A5636F, 0x84C87814, 0x8CC70208,
	0x90BEFFFA, 0xA4506CEB, 0xBEF9A3F7, 0xC67178F2
};

/* see inner.h */
void
br_sha2small_round(const unsigned char *buf, uint32_t *val)
{

#define SHA2_STEP(A, B, C, D, E, F, G, H, j)   do { \
		uint32_t T1, T2; \
		T1 = H + BSG2_1(E) + CH(E, F, G) + K[j] + w[j]; \
		T2 = BSG2_0(A) + MAJ(A, B, C); \
		D += T1; \
		H = T1 + T2; \
	} while (0)

	int i;
	uint32_t a, b, c, d, e, f, g, h;
	uint32_t w[64];

	br_range_dec32be(w, 16, buf);
	for (i = 16; i < 64; i ++) {
		w[i] = SSG2_1(w[i - 2]) + w[i - 7]
			+ SSG2_0(w[i - 15]) + w[i - 16];
	}
	a = val[0];
	b = val[1];
	c = val[2];
	d = val[3];
	e = val[4];
	f = val[5];
	g = val[6];
	h = val[7];
	for (i = 0; i < 64; i += 8) {
		SHA2_STEP(a, b, c, d, e, f, g, h, i + 0);
		SHA2_STEP(h, a, b, c, d, e, f, g, i + 1);
		SHA2_STEP(g, h, a, b, c, d, e, f, i + 2);
		SHA2_STEP(f, g, h, a, b, c, d, e, i + 3);
		SHA2_STEP(e, f, g, h, a, b, c, d, i + 4);
		SHA2_STEP(d, e, f, g, h, a, b, c, i + 5);
		SHA2_STEP(c, d, e, f, g, h, a, b, i + 6);
		SHA2_STEP(b, c, d, e, f, g, h, a, i + 7);
	}
	val[0] += a;
	val[1] += b;
	val[2] += c;
	val[3] += d;
	val[4] += e;
	val[5] += f;
	val[6] += g;
	val[7] += h;

#if 0
/* obsolete */
#define SHA2_MEXP1(pc)   do { \
		W[pc] = br_dec32be(buf + ((pc) << 2)); \
	} while (0)

#define SHA2_MEXP2(pc)   do { \
		W[(pc) & 0x0F] = SSG2_1(W[((pc) - 2) & 0x0F]) \
			+ W[((pc) - 7) & 0x0F] \
			+ SSG2_0(W[((pc) - 15) & 0x0F]) + W[(pc) & 0x0F]; \
	} while (0)

#define SHA2_STEPn(n, a, b, c, d, e, f, g, h, pc)   do { \
		uint32_t t1, t2; \
		SHA2_MEXP ## n(pc); \
		t1 = h + BSG2_1(e) + CH(e, f, g) \
			+ K[pcount + (pc)] + W[(pc) & 0x0F]; \
		t2 = BSG2_0(a) + MAJ(a, b, c); \
		d += t1; \
		h = t1 + t2; \
	} while (0)

#define SHA2_STEP1(a, b, c, d, e, f, g, h, pc) \
	SHA2_STEPn(1, a, b, c, d, e, f, g, h, pc)
#define SHA2_STEP2(a, b, c, d, e, f, g, h, pc) \
	SHA2_STEPn(2, a, b, c, d, e, f, g, h, pc)

	uint32_t A, B, C, D, E, F, G, H;
	uint32_t W[16];
	unsigned pcount;

	A = val[0];
	B = val[1];
	C = val[2];
	D = val[3];
	E = val[4];
	F = val[5];
	G = val[6];
	H = val[7];
	pcount = 0;
	SHA2_STEP1(A, B, C, D, E, F, G, H,  0);
	SHA2_STEP1(H, A, B, C, D, E, F, G,  1);
	SHA2_STEP1(G, H, A, B, C, D, E, F,  2);
	SHA2_STEP1(F, G, H, A, B, C, D, E,  3);
	SHA2_STEP1(E, F, G, H, A, B, C, D,  4);
	SHA2_STEP1(D, E, F, G, H, A, B, C,  5);
	SHA2_STEP1(C, D, E, F, G, H, A, B,  6);
	SHA2_STEP1(B, C, D, E, F, G, H, A,  7);
	SHA2_STEP1(A, B, C, D, E, F, G, H,  8);
	SHA2_STEP1(H, A, B, C, D, E, F, G,  9);
	SHA2_STEP1(G, H, A, B, C, D, E, F, 10);
	SHA2_STEP1(F, G, H, A, B, C, D, E, 11);
	SHA2_STEP1(E, F, G, H, A, B, C, D, 12);
	SHA2_STEP1(D, E, F, G, H, A, B, C, 13);
	SHA2_STEP1(C, D, E, F, G, H, A, B, 14);
	SHA2_STEP1(B, C, D, E, F, G, H, A, 15);
	for (pcount = 16; pcount < 64; pcount += 16) {
		SHA2_STEP2(A, B, C, D, E, F, G, H,  0);
		SHA2_STEP2(H, A, B, C, D, E, F, G,  1);
		SHA2_STEP2(G, H, A, B, C, D, E, F,  2);
		SHA2_STEP2(F, G, H, A, B, C, D, E,  3);
		SHA2_STEP2(E, F, G, H, A, B, C, D,  4);
		SHA2_STEP2(D, E, F, G, H, A, B, C,  5);
		SHA2_STEP2(C, D, E, F, G, H, A, B,  6);
		SHA2_STEP2(B, C, D, E, F, G, H, A,  7);
		SHA2_STEP2(A, B, C, D, E, F, G, H,  8);
		SHA2_STEP2(H, A, B, C, D, E, F, G,  9);
		SHA2_STEP2(G, H, A, B, C, D, E, F, 10);
		SHA2_STEP2(F, G, H, A, B, C, D, E, 11);
		SHA2_STEP2(E, F, G, H, A, B, C, D, 12);
		SHA2_STEP2(D, E, F, G, H, A, B, C, 13);
		SHA2_STEP2(C, D, E, F, G, H, A, B, 14);
		SHA2_STEP2(B, C, D, E, F, G, H, A, 15);
	}
	val[0] += A;
	val[1] += B;
	val[2] += C;
	val[3] += D;
	val[4] += E;
	val[5] += F;
	val[6] += G;
	val[7] += H;
#endif
}

static void
sha2small_update(br_sha224_context *cc, const void *data, size_t len)
{
	const unsigned char *buf;
	size_t ptr;

	buf = data;
	ptr = (size_t)cc->count & 63;
	cc->count += (uint64_t)len;
	while (len > 0) {
		size_t clen;

		clen = 64 - ptr;
		if (clen > len) {
			clen = len;
		}
		memcpy(cc->buf + ptr, buf, clen);
		ptr += clen;
		buf += clen;
		len -= clen;
		if (ptr == 64) {
			br_sha2small_round(cc->buf, cc->val);
			ptr = 0;
		}
	}
}

static void
sha2small_out(const br_sha224_context *cc, void *dst, int num)
{
	unsigned char buf[64];
	uint32_t val[8];
	size_t ptr;

	ptr = (size_t)cc->count & 63;
	memcpy(buf, cc->buf, ptr);
	memcpy(val, cc->val, sizeof val);
	buf[ptr ++] = 0x80;
	if (ptr > 56) {
		memset(buf + ptr, 0, 64 - ptr);
		br_sha2small_round(buf, val);
		memset(buf, 0, 56);
	} else {
		memset(buf + ptr, 0, 56 - ptr);
	}
	br_enc64be(buf + 56, cc->count << 3);
	br_sha2small_round(buf, val);
	br_range_enc32be(dst, val, num);
}

/* see bearssl.h */
void
br_sha224_init(br_sha224_context *cc)
{
	cc->vtable = &br_sha224_vtable;
	memcpy(cc->val, br_sha224_IV, sizeof cc->val);
	cc->count = 0;
}

/* see bearssl.h */
void
br_sha224_update(br_sha224_context *cc, const void *data, size_t len)
{
	sha2small_update(cc, data, len);
}

/* see bearssl.h */
void
br_sha224_out(const br_sha224_context *cc, void *dst)
{
	sha2small_out(cc, dst, 7);
}

/* see bearssl.h */
uint64_t
br_sha224_state(const br_sha224_context *cc, void *dst)
{
	br_range_enc32be(dst, cc->val, 8);
	return cc->count;
}

/* see bearssl.h */
void
br_sha224_set_state(br_sha224_context *cc, const void *stb, uint64_t count)
{
	br_range_dec32be(cc->val, 8, stb);
	cc->count = count;
}

/* see bearssl.h */
void
br_sha256_init(br_sha256_context *cc)
{
	cc->vtable = &br_sha256_vtable;
	memcpy(cc->val, br_sha256_IV, sizeof cc->val);
	cc->count = 0;
}

/* see bearssl.h */
void
br_sha256_out(const br_sha256_context *cc, void *dst)
{
	sha2small_out(cc, dst, 8);
}

/* see bearssl.h */
const br_hash_class br_sha224_vtable = {
	sizeof(br_sha224_context),
	BR_HASHDESC_ID(br_sha224_ID)
		| BR_HASHDESC_OUT(28)
		| BR_HASHDESC_STATE(32)
		| BR_HASHDESC_LBLEN(6)
		| BR_HASHDESC_MD_PADDING
		| BR_HASHDESC_MD_PADDING_BE,
	(void (*)(const br_hash_class **))&br_sha224_init,
	(void (*)(const br_hash_class **,
		const void *, size_t))&br_sha224_update,
	(void (*)(const br_hash_class *const *, void *))&br_sha224_out,
	(uint64_t (*)(const br_hash_class *const *, void *))&br_sha224_state,
	(void (*)(const br_hash_class **, const void *, uint64_t))
		&br_sha224_set_state
};

/* see bearssl.h */
const br_hash_class br_sha256_vtable = {
	sizeof(br_sha256_context),
	BR_HASHDESC_ID(br_sha256_ID)
		| BR_HASHDESC_OUT(32)
		| BR_HASHDESC_STATE(32)
		| BR_HASHDESC_LBLEN(6)
		| BR_HASHDESC_MD_PADDING
		| BR_HASHDESC_MD_PADDING_BE,
	(void (*)(const br_hash_class **))&br_sha256_init,
	(void (*)(const br_hash_class **,
		const void *, size_t))&br_sha256_update,
	(void (*)(const br_hash_class *const *, void *))&br_sha256_out,
	(uint64_t (*)(const br_hash_class *const *, void *))&br_sha256_state,
	(void (*)(const br_hash_class **, const void *, uint64_t))
		&br_sha256_set_state
};


/* ===== src/mac/hmac.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

static inline size_t
block_size(const br_hash_class *dig)
{
	unsigned ls;
	
	ls = (unsigned)(dig->desc >> BR_HASHDESC_LBLEN_OFF)
		& BR_HASHDESC_LBLEN_MASK;
	return (size_t)1 << ls;
}

static void
process_key(const br_hash_class **hc, void *ks,
	const void *key, size_t key_len, unsigned bb)
{
	unsigned char tmp[256];
	size_t blen, u;

	blen = block_size(*hc);
	memcpy(tmp, key, key_len);
	for (u = 0; u < key_len; u ++) {
		tmp[u] ^= (unsigned char)bb;
	}
	memset(tmp + key_len, bb, blen - key_len);
	(*hc)->init(hc);
	(*hc)->update(hc, tmp, blen);
	(*hc)->state(hc, ks);
}

/* see bearssl.h */
void
br_hmac_key_init(br_hmac_key_context *kc,
	const br_hash_class *dig, const void *key, size_t key_len)
{
	br_hash_compat_context hc;
	unsigned char kbuf[64];

	kc->dig_vtable = dig;
	hc.vtable = dig;
	if (key_len > block_size(dig)) {
		dig->init(&hc.vtable);
		dig->update(&hc.vtable, key, key_len);
		dig->out(&hc.vtable, kbuf);
		key = kbuf;
		key_len = br_digest_size(dig);
	}
	process_key(&hc.vtable, kc->ksi, key, key_len, 0x36);
	process_key(&hc.vtable, kc->kso, key, key_len, 0x5C);
}

/* see bearssl.h */
void
br_hmac_init(br_hmac_context *ctx,
	const br_hmac_key_context *kc, size_t out_len)
{
	const br_hash_class *dig;
	size_t blen, hlen;

	dig = kc->dig_vtable;
	blen = block_size(dig);
	dig->init(&ctx->dig.vtable);
	dig->set_state(&ctx->dig.vtable, kc->ksi, (uint64_t)blen);
	memcpy(ctx->kso, kc->kso, sizeof kc->kso);
	hlen = br_digest_size(dig);
	if (out_len > 0 && out_len < hlen) {
		hlen = out_len;
	}
	ctx->out_len = hlen;
}

/* see bearssl.h */
void
br_hmac_update(br_hmac_context *ctx, const void *data, size_t len)
{
	ctx->dig.vtable->update(&ctx->dig.vtable, data, len);
}

/* see bearssl.h */
size_t
br_hmac_out(const br_hmac_context *ctx, void *out)
{
	const br_hash_class *dig;
	br_hash_compat_context hc;
	unsigned char tmp[64];
	size_t blen, hlen;

	dig = ctx->dig.vtable;
	dig->out(&ctx->dig.vtable, tmp);
	blen = block_size(dig);
	dig->init(&hc.vtable);
	dig->set_state(&hc.vtable, ctx->kso, (uint64_t)blen);
	hlen = br_digest_size(dig);
	dig->update(&hc.vtable, tmp, hlen);
	dig->out(&hc.vtable, tmp);
	memcpy(out, tmp, ctx->out_len);
	return ctx->out_len;
}


/* ===== src/mac/hmac_ct.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

static inline size_t
hash_size(const br_hash_class *dig)
{
	return (unsigned)(dig->desc >> BR_HASHDESC_OUT_OFF)
		& BR_HASHDESC_OUT_MASK;
}

static inline size_t
block_size(const br_hash_class *dig)
{
	unsigned ls;
	
	ls = (unsigned)(dig->desc >> BR_HASHDESC_LBLEN_OFF)
		& BR_HASHDESC_LBLEN_MASK;
	return (size_t)1 << ls;
}

/* see bearssl.h */
size_t
br_hmac_outCT(const br_hmac_context *ctx,
	const void *data, size_t len, size_t min_len, size_t max_len,
	void *out)
{
	/*
	 * Method implemented here is inspired from the descriptions on:
	 *    https://www.imperialviolet.org/2013/02/04/luckythirteen.html
	 *
	 * Principle: we input bytes one by one. We use a MUX to push
	 * padding bytes instead of data bytes when appropriate. At each
	 * block limit, we get the current hash function state: this is
	 * a potential output, since we handle MD padding ourselves.
	 *
	 * be     1 for big-endian, 0 for little-endian
	 * po     minimal MD padding length
	 * bs     block size (always a power of 2)
	 * hlen   hash output size
	 */

	const br_hash_class *dig;
	br_hash_compat_context hc;
	int be;
	uint32_t po, bs;
	uint32_t kr, km, kl, kz, u;
	uint64_t count, ncount, bit_len;
	unsigned char tmp1[64], tmp2[64];
	size_t hlen;

	/*
	 * Copy the current hash context.
	 */
	hc = ctx->dig;

	/*
	 * Get function-specific information.
	 */
	dig = hc.vtable;
	be = (dig->desc & BR_HASHDESC_MD_PADDING_BE) != 0;
	po = 9;
	if (dig->desc & BR_HASHDESC_MD_PADDING_128) {
		po += 8;
	}
	bs = block_size(dig);
	hlen = hash_size(dig);

	/*
	 * Get current input length and compute total bit length.
	 */
	count = dig->state(&hc.vtable, tmp1);
	bit_len = (count + (uint64_t)len) << 3;

	/*
	 * We can input the blocks that we are sure we will use.
	 * This offers better performance (no MUX for these blocks)
	 * and also ensures that the remaining lengths fit on 32 bits.
	 */
	ncount = (count + (uint64_t)min_len) & ~(uint64_t)(bs - 1);
	if (ncount > count) {
		size_t zlen;

		zlen = (size_t)(ncount - count);
		dig->update(&hc.vtable, data, zlen);
		data = (const unsigned char *)data + zlen;
		len -= zlen;
		max_len -= zlen;
		count = ncount;
	}

	/*
	 * At that point:
	 * -- 'count' contains the number of bytes already processed
	 * (in total).
	 * -- We must input 'len' bytes. 'min_len' is unimportant: we
	 * used it to know how many full blocks we could process
	 * directly. Now only len and max_len matter.
	 *
	 * We compute kr, kl, kz and km.
	 *  kr   number of input bytes already in the current block
	 *  km   index of the first byte after the end of the last padding
	 *       block, if length is max_len
	 *  kz   index of the last byte of the actual last padding block
	 *  kl   index of the start of the encoded length
	 *
	 * km, kz and kl are counted from the current offset in the
	 * input data.
	 */
	kr = (uint32_t)count & (bs - 1);
	kz = ((kr + (uint32_t)len + po + bs - 1) & ~(bs - 1)) - 1 - kr;
	kl = kz - 7;
	km = ((kr + (uint32_t)max_len + po + bs - 1) & ~(bs - 1)) - kr;

	/*
	 * We must now process km bytes. For index u from 0 to km-1:
	 *   d is from data[] if u < max_len, 0x00 otherwise
	 *   e is an encoded length byte or 0x00, depending on u
	 * The tests for d and e need not be constant-time, since
	 * they relate only to u and max_len, not to the actual length.
	 *
	 * Actual input length is then:
	 *   d      if u < len
	 *   0x80   if u == len
	 *   0x00   if u > len and u < kl
	 *   e      if u >= kl
	 *
	 * Hash state is obtained whenever we reach a full block. This
	 * is the result we want if and only if u == kz.
	 */
	for (u = 0; u < km; u ++) {
		uint32_t v;
		uint32_t d, e, x0, x1;
		unsigned char x[1];

		d = (u < max_len) ? ((const unsigned char *)data)[u] : 0x00;
		v = (kr + u) & (bs - 1);
		if (v >= (bs - 8)) {
			unsigned j;

			j = (v - (bs - 8)) << 3;
			if (be) {
				e = (uint32_t)(bit_len >> (56 - j));
			} else {
				e = (uint32_t)(bit_len >> j);
			}
			e &= 0xFF;
		} else {
			e = 0x00;
		}
		x0 = MUX(EQ(u, (uint32_t)len), 0x80, d);
		x1 = MUX(LT(u, kl), 0x00, e);
		x[0] = MUX(LE(u, (uint32_t)len), x0, x1);
		dig->update(&hc.vtable, x, 1);
		if (v == (bs - 1)) {
			dig->state(&hc.vtable, tmp1);
			CCOPY(EQ(u, kz), tmp2, tmp1, hlen);
		}
	}

	/*
	 * Inner hash output is in tmp2[]; we finish processing.
	 */
	dig->init(&hc.vtable);
	dig->set_state(&hc.vtable, ctx->kso, (uint64_t)bs);
	dig->update(&hc.vtable, tmp2, hlen);
	dig->out(&hc.vtable, tmp2);
	memcpy(out, tmp2, ctx->out_len);
	return ctx->out_len;
}


/* ===== src/rand/hmac_drbg.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

/* see bearssl.h */
void
br_hmac_drbg_init(br_hmac_drbg_context *ctx,
	const br_hash_class *digest_class, const void *seed, size_t len)
{
	size_t hlen;

	ctx->vtable = &br_hmac_drbg_vtable;
	hlen = br_digest_size(digest_class);
	memset(ctx->K, 0x00, hlen);
	memset(ctx->V, 0x01, hlen);
	ctx->digest_class = digest_class;
	br_hmac_drbg_update(ctx, seed, len);
}

/* see bearssl.h */
void
br_hmac_drbg_generate(br_hmac_drbg_context *ctx, void *out, size_t len)
{
	const br_hash_class *dig;
	br_hmac_key_context kc;
	br_hmac_context hc;
	size_t hlen;
	unsigned char *buf;
	unsigned char x;

	dig = ctx->digest_class;
	hlen = br_digest_size(dig);
	br_hmac_key_init(&kc, dig, ctx->K, hlen);
	buf = out;
	while (len > 0) {
		size_t clen;

		br_hmac_init(&hc, &kc, 0);
		br_hmac_update(&hc, ctx->V, hlen);
		br_hmac_out(&hc, ctx->V);
		clen = hlen;
		if (clen > len) {
			clen = len;
		}
		memcpy(buf, ctx->V, clen);
		buf += clen;
		len -= clen;
	}

	/*
	 * To prepare the state for the next request, we should call
	 * br_hmac_drbg_update() with an empty additional seed. However,
	 * we already have an initialized HMAC context with the right
	 * initial key, and we don't want to push another one on the
	 * stack, so we inline that update() call here.
	 */
	br_hmac_init(&hc, &kc, 0);
	br_hmac_update(&hc, ctx->V, hlen);
	x = 0x00;
	br_hmac_update(&hc, &x, 1);
	br_hmac_out(&hc, ctx->K);
	br_hmac_key_init(&kc, dig, ctx->K, hlen);
	br_hmac_init(&hc, &kc, 0);
	br_hmac_update(&hc, ctx->V, hlen);
	br_hmac_out(&hc, ctx->V);
}

/* see bearssl.h */
void
br_hmac_drbg_update(br_hmac_drbg_context *ctx, const void *seed, size_t len)
{
	const br_hash_class *dig;
	br_hmac_key_context kc;
	br_hmac_context hc;
	size_t hlen;
	unsigned char x;

	dig = ctx->digest_class;
	hlen = br_digest_size(dig);

	/*
	 * 1. K = HMAC(K, V || 0x00 || seed)
	 */
	br_hmac_key_init(&kc, dig, ctx->K, hlen);
	br_hmac_init(&hc, &kc, 0);
	br_hmac_update(&hc, ctx->V, hlen);
	x = 0x00;
	br_hmac_update(&hc, &x, 1);
	br_hmac_update(&hc, seed, len);
	br_hmac_out(&hc, ctx->K);
	br_hmac_key_init(&kc, dig, ctx->K, hlen);

	/*
	 * 2. V = HMAC(K, V)
	 */
	br_hmac_init(&hc, &kc, 0);
	br_hmac_update(&hc, ctx->V, hlen);
	br_hmac_out(&hc, ctx->V);

	/*
	 * 3. If the additional seed is empty, then stop here.
	 */
	if (len == 0) {
		return;
	}

	/*
	 * 4. K = HMAC(K, V || 0x01 || seed)
	 */
	br_hmac_init(&hc, &kc, 0);
	br_hmac_update(&hc, ctx->V, hlen);
	x = 0x01;
	br_hmac_update(&hc, &x, 1);
	br_hmac_update(&hc, seed, len);
	br_hmac_out(&hc, ctx->K);
	br_hmac_key_init(&kc, dig, ctx->K, hlen);

	/*
	 * 5. V = HMAC(K, V)
	 */
	br_hmac_init(&hc, &kc, 0);
	br_hmac_update(&hc, ctx->V, hlen);
	br_hmac_out(&hc, ctx->V);
}

/* see bearssl.h */
const br_prng_class br_hmac_drbg_vtable = {
	sizeof(br_hmac_drbg_context),
	(void (*)(const br_prng_class **, const void *, const void *, size_t))
		&br_hmac_drbg_init,
	(void (*)(const br_prng_class **, void *, size_t))
		&br_hmac_drbg_generate,
	(void (*)(const br_prng_class **, const void *, size_t))
		&br_hmac_drbg_update
};


/* ===== src/int/i31_add.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

/* see inner.h */
uint32_t
br_i31_add(uint32_t *a, const uint32_t *b, uint32_t ctl)
{
	uint32_t cc;
	size_t u, m;

	cc = 0;
	m = (a[0] + 63) >> 5;
	for (u = 1; u < m; u ++) {
		uint32_t aw, bw, naw;

		aw = a[u];
		bw = b[u];
		naw = aw + bw + cc;
		cc = naw >> 31;
		a[u] = MUX(ctl, naw & (uint32_t)0x7FFFFFFF, aw);
	}
	return cc;
}


/* ===== src/int/i31_bitlen.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

/* see inner.h */
uint32_t
br_i31_bit_length(uint32_t *x, size_t xlen)
{
	uint32_t tw, twk;

	tw = 0;
	twk = 0;
	while (xlen -- > 0) {
		uint32_t w, c;

		c = EQ(tw, 0);
		w = x[xlen];
		tw = MUX(c, w, tw);
		twk = MUX(c, (uint32_t)xlen, twk);
	}
	return (twk << 5) + BIT_LENGTH(tw);
}


/* ===== src/int/i31_decmod.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

/* see inner.h */
uint32_t
br_i31_decode_mod(uint32_t *x, const void *src, size_t len, const uint32_t *m)
{
	/*
	 * Two-pass algorithm: in the first pass, we determine whether the
	 * value fits; in the second pass, we do the actual write.
	 *
	 * During the first pass, 'r' contains the comparison result so
	 * far:
	 *  0x00000000   value is equal to the modulus
	 *  0x00000001   value is greater than the modulus
	 *  0xFFFFFFFF   value is lower than the modulus
	 *
	 * Since we iterate starting with the least significant bytes (at
	 * the end of src[]), each new comparison overrides the previous
	 * except when the comparison yields 0 (equal).
	 *
	 * During the second pass, 'r' is either 0xFFFFFFFF (value fits)
	 * or 0x00000000 (value does not fit).
	 *
	 * We must iterate over all bytes of the source, _and_ possibly
	 * some extra virutal bytes (with value 0) so as to cover the
	 * complete modulus as well. We also add 4 such extra bytes beyond
	 * the modulus length because it then guarantees that no accumulated
	 * partial word remains to be processed.
	 */
	const unsigned char *buf;
	size_t mlen, tlen;
	int pass;
	uint32_t r;

	buf = src;
	mlen = (m[0] + 31) >> 5;
	tlen = (mlen << 2);
	if (tlen < len) {
		tlen = len;
	}
	tlen += 4;
	r = 0;
	for (pass = 0; pass < 2; pass ++) {
		size_t u, v;
		uint32_t acc;
		int acc_len;

		v = 1;
		acc = 0;
		acc_len = 0;
		for (u = 0; u < tlen; u ++) {
			uint32_t b;

			if (u < len) {
				b = buf[len - 1 - u];
			} else {
				b = 0;
			}
			acc |= (b << acc_len);
			acc_len += 8;
			if (acc_len >= 31) {
				uint32_t xw;

				xw = acc & (uint32_t)0x7FFFFFFF;
				acc_len -= 31;
				acc = b >> (8 - acc_len);
				if (v <= mlen) {
					if (pass) {
						x[v] = r & xw;
					} else {
						uint32_t cc;

						cc = (uint32_t)CMP(xw, m[v]);
						r = MUX(EQ(cc, 0), r, cc);
					}
				} else {
					if (!pass) {
						r = MUX(EQ(xw, 0), r, 1);
					}
				}
				v ++;
			}
		}

		/*
		 * When we reach this point at the end of the first pass:
		 * r is either 0, 1 or -1; we want to set r to 0 if it
		 * is equal to 0 or 1, and leave it to -1 otherwise.
		 *
		 * When we reach this point at the end of the second pass:
		 * r is either 0 or -1; we want to leave that value
		 * untouched. This is a subcase of the previous.
		 */
		r >>= 1;
		r |= (r << 1);
	}

	x[0] = m[0];
	return r & (uint32_t)1;
}


/* ===== src/int/i31_decode.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

/* see inner.h */
void
br_i31_decode(uint32_t *x, const void *src, size_t len)
{
	const unsigned char *buf;
	size_t u, v;
	uint32_t acc;
	int acc_len;

	buf = src;
	u = len;
	v = 1;
	acc = 0;
	acc_len = 0;
	while (u -- > 0) {
		uint32_t b;

		b = buf[u];
		acc |= (b << acc_len);
		acc_len += 8;
		if (acc_len >= 31) {
			x[v ++] = acc & (uint32_t)0x7FFFFFFF;
			acc_len -= 31;
			acc = b >> (8 - acc_len);
		}
	}
	if (acc_len != 0) {
		x[v ++] = acc;
	}
	x[0] = br_i31_bit_length(x + 1, v - 1);
}


/* ===== src/int/i31_decred.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

/* see inner.h */
void
br_i31_decode_reduce(uint32_t *x,
	const void *src, size_t len, const uint32_t *m)
{
	uint32_t m_ebitlen, m_rbitlen;
	size_t mblen, k;
	const unsigned char *buf;
	uint32_t acc;
	int acc_len;

	/*
	 * Get the encoded bit length.
	 */
	m_ebitlen = m[0];

	/*
	 * Special case for an invalid (null) modulus.
	 */
	if (m_ebitlen == 0) {
		x[0] = 0;
		return;
	}

	/*
	 * Clear the destination.
	 */
	br_i31_zero(x, m_ebitlen);

	/*
	 * First decode directly as many bytes as possible. This requires
	 * computing the actual bit length.
	 */
	m_rbitlen = m_ebitlen >> 5;
	m_rbitlen = (m_ebitlen & 31) + (m_rbitlen << 5) - m_rbitlen;
	mblen = (m_rbitlen + 7) >> 3;
	k = mblen - 1;
	if (k >= len) {
		br_i31_decode(x, src, len);
		x[0] = m_ebitlen;
		return;
	}
	buf = src;
	br_i31_decode(x, buf, k);
	x[0] = m_ebitlen;

	/*
	 * Input remaining bytes, using 31-bit words.
	 */
	acc = 0;
	acc_len = 0;
	while (k < len) {
		uint32_t v;

		v = buf[k ++];
		if (acc_len >= 23) {
			acc_len -= 23;
			acc <<= (8 - acc_len);
			acc |= v >> acc_len;
			br_i31_muladd_small(x, acc, m);
			acc = v & (0xFF >> (8 - acc_len));
		} else {
			acc = (acc << 8) | v;
			acc_len += 8;
		}
	}

	/*
	 * We may have some bits accumulated. We then perform a shift to
	 * be able to inject these bits as a full 31-bit word.
	 */
	if (acc_len != 0) {
		acc = (acc | (x[1] << acc_len)) & 0x7FFFFFFF;
		br_i31_rshift(x, 31 - acc_len);
		br_i31_muladd_small(x, acc, m);
	}
}


/* ===== src/int/i31_encode.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

/* see inner.h */
void
br_i31_encode(void *dst, size_t len, const uint32_t *x)
{
	unsigned char *buf;
	size_t k, xlen;
	uint32_t acc;
	int acc_len;

	xlen = (x[0] + 31) >> 5;
	if (xlen == 0) {
		memset(dst, 0, len);
		return;
	}
	buf = (unsigned char *)dst + len;
	k = 1;
	acc = 0;
	acc_len = 0;
	while (len != 0) {
		uint32_t w;

		w = (k <= xlen) ? x[k] : 0;
		k ++;
		if (acc_len == 0) {
			acc = w;
			acc_len = 31;
		} else {
			uint32_t z;

			z = acc | (w << acc_len);
			acc_len --;
			acc = w >> (31 - acc_len);
			if (len >= 4) {
				buf -= 4;
				len -= 4;
				br_enc32be(buf, z);
			} else {
				switch (len) {
				case 3:
					buf[-3] = (unsigned char)(z >> 16);
					/* fall through */
				case 2:
					buf[-2] = (unsigned char)(z >> 8);
					/* fall through */
				case 1:
					buf[-1] = (unsigned char)z;
					break;
				}
				return;
			}
		}
	}
}


/* ===== src/int/i31_fmont.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

/* see inner.h */
void
br_i31_from_monty(uint32_t *x, const uint32_t *m, uint32_t m0i)
{
	size_t len, u, v;

	len = (m[0] + 31) >> 5;
	for (u = 0; u < len; u ++) {
		uint32_t f;
		uint64_t cc;

		f = MUL31_lo(x[1], m0i);
		cc = 0;
		for (v = 0; v < len; v ++) {
			uint64_t z;

			z = (uint64_t)x[v + 1] + MUL31(f, m[v + 1]) + cc;
			cc = z >> 31;
			if (v != 0) {
				x[v] = (uint32_t)z & 0x7FFFFFFF;
			}
		}
		x[len] = (uint32_t)cc;
	}

	/*
	 * We may have to do an extra subtraction, but only if the
	 * value in x[] is indeed greater than or equal to that of m[],
	 * which is why we must do two calls (first call computes the
	 * carry, second call performs the subtraction only if the carry
	 * is 0).
	 */
	br_i31_sub(x, m, NOT(br_i31_sub(x, m, 0)));
}


/* ===== src/int/i31_iszero.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

/* see inner.h */
uint32_t
br_i31_iszero(const uint32_t *x)
{
	uint32_t z;
	size_t u;

	z = 0;
	for (u = (x[0] + 31) >> 5; u > 0; u --) {
		z |= x[u];
	}
	return ~(z | -z) >> 31;
}


/* ===== src/int/i31_modpow.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

/* see inner.h */
void
br_i31_modpow(uint32_t *x,
	const unsigned char *e, size_t elen,
	const uint32_t *m, uint32_t m0i, uint32_t *t1, uint32_t *t2)
{
	size_t mlen;
	uint32_t k;

	/*
	 * 'mlen' is the length of m[] expressed in bytes (including
	 * the "bit length" first field).
	 */
	mlen = ((m[0] + 63) >> 5) * sizeof m[0];

	/*
	 * Throughout the algorithm:
	 * -- t1[] is in Montgomery representation; it contains x, x^2,
	 * x^4, x^8...
	 * -- The result is accumulated, in normal representation, in
	 * the x[] array.
	 * -- t2[] is used as destination buffer for each multiplication.
	 *
	 * Note that there is no need to call br_i32_from_monty().
	 */
	memcpy(t1, x, mlen);
	br_i31_to_monty(t1, m);
	br_i31_zero(x, m[0]);
	x[1] = 1;
	for (k = 0; k < ((uint32_t)elen << 3); k ++) {
		uint32_t ctl;

		ctl = (e[elen - 1 - (k >> 3)] >> (k & 7)) & 1;
		br_i31_montymul(t2, x, t1, m, m0i);
		CCOPY(ctl, x, t2, mlen);
		br_i31_montymul(t2, t1, t1, m, m0i);
		memcpy(t1, t2, mlen);
	}
}


/* ===== src/int/i31_montmul.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

/* see inner.h */
void
br_i31_montymul(uint32_t *d, const uint32_t *x, const uint32_t *y,
	const uint32_t *m, uint32_t m0i)
{
	size_t len, len4, u, v;
	uint64_t dh;

	len = (m[0] + 31) >> 5;
	len4 = len & ~(size_t)3;
	br_i32_zero(d, m[0]);
	dh = 0;
	for (u = 0; u < len; u ++) {
		uint32_t f, xu;
		uint64_t r, zh;

		xu = x[u + 1];
		f = MUL31_lo((d[1] + MUL31_lo(x[u + 1], y[1])), m0i);

		r = 0;
		for (v = 0; v < len4; v += 4) {
			uint64_t z;

			z = (uint64_t)d[v + 1] + MUL31(xu, y[v + 1])
				+ MUL31(f, m[v + 1]) + r;
			r = z >> 31;
			d[v + 0] = (uint32_t)z & 0x7FFFFFFF;
			z = (uint64_t)d[v + 2] + MUL31(xu, y[v + 2])
				+ MUL31(f, m[v + 2]) + r;
			r = z >> 31;
			d[v + 1] = (uint32_t)z & 0x7FFFFFFF;
			z = (uint64_t)d[v + 3] + MUL31(xu, y[v + 3])
				+ MUL31(f, m[v + 3]) + r;
			r = z >> 31;
			d[v + 2] = (uint32_t)z & 0x7FFFFFFF;
			z = (uint64_t)d[v + 4] + MUL31(xu, y[v + 4])
				+ MUL31(f, m[v + 4]) + r;
			r = z >> 31;
			d[v + 3] = (uint32_t)z & 0x7FFFFFFF;
		}
		for (; v < len; v ++) {
			uint64_t z;

			z = (uint64_t)d[v + 1] + MUL31(xu, y[v + 1])
				+ MUL31(f, m[v + 1]) + r;
			r = z >> 31;
			d[v] = (uint32_t)z & 0x7FFFFFFF;
		}

		zh = dh + r;
		d[len] = (uint32_t)zh & 0x7FFFFFFF;
		dh = zh >> 31;
	}

	/*
	 * We must write back the bit length because it was overwritten in
	 * the loop (not overwriting it would require a test in the loop,
	 * which would yield bigger and slower code).
	 */
	d[0] = m[0];

	/*
	 * d[] may still be greater than m[] at that point; notably, the
	 * 'dh' word may be non-zero.
	 */
	br_i31_sub(d, m, NEQ(dh, 0) | NOT(br_i31_sub(d, m, 0)));
}


/* ===== src/int/i31_mulacc.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

/* see inner.h */
void
br_i31_mulacc(uint32_t *d, const uint32_t *a, const uint32_t *b)
{
	size_t alen, blen, u;

	alen = (a[0] + 31) >> 5;
	blen = (b[0] + 31) >> 5;
	d[0] = a[0] + b[0];
	for (u = 0; u < blen; u ++) {
		uint32_t f;
		size_t v;
		uint64_t cc;

		f = b[1 + u];
		cc = 0;
		for (v = 0; v < alen; v ++) {
			uint64_t z;

			z = (uint64_t)d[1 + u + v] + MUL31(f, a[1 + v]) + cc;
			cc = z >> 31;
			d[1 + u + v] = (uint32_t)z & 0x7FFFFFFF;
		}
		d[1 + u + alen] = (uint32_t)cc;
	}
}


/* ===== src/int/i31_muladd.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

/* see inner.h */
void
br_i31_muladd_small(uint32_t *x, uint32_t z, const uint32_t *m)
{
	uint32_t m_bitlen;
	unsigned mblr;
	size_t u, mlen;
	uint32_t a0, a1, b0, hi, g, q, tb;
	uint32_t under, over;
	uint32_t cc;

	/*
	 * We can test on the modulus bit length since we accept to
	 * leak that length.
	 */
	m_bitlen = m[0];
	if (m_bitlen == 0) {
		return;
	}
	if (m_bitlen <= 31) {
		uint32_t lo;

		hi = x[1] >> 1;
		lo = (x[1] << 31) | z;
		x[1] = br_rem(hi, lo, m[1]);
		return;
	}
	mlen = (m_bitlen + 31) >> 5;
	mblr = (unsigned)m_bitlen & 31;

	/*
	 * Principle: we estimate the quotient (x*2^31+z)/m by
	 * doing a 64/32 division with the high words.
	 *
	 * Let:
	 *   w = 2^31
	 *   a = (w*a0 + a1) * w^N + a2
	 *   b = b0 * w^N + b2
	 * such that:
	 *   0 <= a0 < w
	 *   0 <= a1 < w
	 *   0 <= a2 < w^N
	 *   w/2 <= b0 < w
	 *   0 <= b2 < w^N
	 *   a < w*b
	 * I.e. the two top words of a are a0:a1, the top word of b is
	 * b0, we ensured that b0 is "full" (high bit set), and a is
	 * such that the quotient q = a/b fits on one word (0 <= q < w).
	 *
	 * If a = b*q + r (with 0 <= r < q), we can estimate q by
	 * doing an Euclidean division on the top words:
	 *   a0*w+a1 = b0*u + v  (with 0 <= v < b0)
	 * Then the following holds:
	 *   0 <= u <= w
	 *   u-2 <= q <= u
	 */
	hi = x[mlen];
	if (mblr == 0) {
		a0 = x[mlen];
		memmove(x + 2, x + 1, (mlen - 1) * sizeof *x);
		x[1] = z;
		a1 = x[mlen];
		b0 = m[mlen];
	} else {
		a0 = ((x[mlen] << (31 - mblr)) | (x[mlen - 1] >> mblr))
			& 0x7FFFFFFF;
		memmove(x + 2, x + 1, (mlen - 1) * sizeof *x);
		x[1] = z;
		a1 = ((x[mlen] << (31 - mblr)) | (x[mlen - 1] >> mblr))
			& 0x7FFFFFFF;
		b0 = ((m[mlen] << (31 - mblr)) | (m[mlen - 1] >> mblr))
			& 0x7FFFFFFF;
	}

	/*
	 * We estimate a divisor q. If the quotient returned by br_div()
	 * is g:
	 * -- If a0 == b0 then g == 0; we want q = 0x7FFFFFFF.
	 * -- Otherwise:
	 *    -- if g == 0 then we set q = 0;
	 *    -- otherwise, we set q = g - 1.
	 * The properties described above then ensure that the true
	 * quotient is q-1, q or q+1.
	 *
	 * Take care that a0, a1 and b0 are 31-bit words, not 32-bit. We
	 * must adjust the parameters to br_div() accordingly.
	 */
	g = br_div(a0 >> 1, a1 | (a0 << 31), b0);
	q = MUX(EQ(a0, b0), 0x7FFFFFFF, MUX(EQ(g, 0), 0, g - 1));

	/*
	 * We subtract q*m from x (with the extra high word of value 'hi').
	 * Since q may be off by 1 (in either direction), we may have to
	 * add or subtract m afterwards.
	 *
	 * The 'tb' flag will be true (1) at the end of the loop if the
	 * result is greater than or equal to the modulus (not counting
	 * 'hi' or the carry).
	 */
	cc = 0;
	tb = 1;
	for (u = 1; u <= mlen; u ++) {
		uint32_t mw, zw, xw, nxw;
		uint64_t zl;

		mw = m[u];
		zl = MUL31(mw, q) + cc;
		cc = (uint32_t)(zl >> 31);
		zw = (uint32_t)zl & (uint32_t)0x7FFFFFFF;
		xw = x[u];
		nxw = xw - zw;
		cc += nxw >> 31;
		nxw &= 0x7FFFFFFF;
		x[u] = nxw;
		tb = MUX(EQ(nxw, mw), tb, GT(nxw, mw));
	}

	/*
	 * If we underestimated q, then either cc < hi (one extra bit
	 * beyond the top array word), or cc == hi and tb is true (no
	 * extra bit, but the result is not lower than the modulus). In
	 * these cases we must subtract m once.
	 *
	 * Otherwise, we may have overestimated, which will show as
	 * cc > hi (thus a negative result). Correction is adding m once.
	 */
	over = GT(cc, hi);
	under = ~over & (tb | LT(cc, hi));
	br_i31_add(x, m, over);
	br_i31_sub(x, m, under);
}


/* ===== src/int/i31_ninv31.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

/* see inner.h */
uint32_t
br_i31_ninv31(uint32_t x)
{
	uint32_t y;

	y = 2 - x;
	y *= 2 - y * x;
	y *= 2 - y * x;
	y *= 2 - y * x;
	y *= 2 - y * x;
	return MUX(x & 1, -y, 0) & 0x7FFFFFFF;
}


/* ===== src/int/i31_reduce.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

/* see inner.h */
void
br_i31_reduce(uint32_t *x, const uint32_t *a, const uint32_t *m)
{
	uint32_t m_bitlen, a_bitlen;
	size_t mlen, alen, u;

	m_bitlen = m[0];
	mlen = (m_bitlen + 31) >> 5;

	x[0] = m_bitlen;
	if (m_bitlen == 0) {
		return;
	}

	/*
	 * If the source is shorter, then simply copy all words from a[]
	 * and zero out the upper words.
	 */
	a_bitlen = a[0];
	alen = (a_bitlen + 31) >> 5;
	if (a_bitlen < m_bitlen) {
		memcpy(x + 1, a + 1, alen * sizeof *a);
		for (u = alen; u < mlen; u ++) {
			x[u + 1] = 0;
		}
		return;
	}

	/*
	 * The source length is at least equal to that of the modulus.
	 * We must thus copy N-1 words, and input the remaining words
	 * one by one.
	 */
	memcpy(x + 1, a + 2 + (alen - mlen), (mlen - 1) * sizeof *a);
	x[mlen] = 0;
	for (u = 1 + alen - mlen; u > 0; u --) {
		br_i31_muladd_small(x, a[u], m);
	}
}


/* ===== src/int/i31_rshift.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

/* see inner.h */
void
br_i31_rshift(uint32_t *x, int count)
{
	size_t u, len;
	uint32_t r;

	len = (x[0] + 31) >> 5;
	if (len == 0) {
		return;
	}
	r = x[1] >> count;
	for (u = 2; u <= len; u ++) {
		uint32_t w;

		w = x[u];
		x[u - 1] = ((w << (31 - count)) | r) & 0x7FFFFFFF;
		r = w >> count;
	}
	x[len] = r;
}


/* ===== src/int/i31_sub.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

/* see inner.h */
uint32_t
br_i31_sub(uint32_t *a, const uint32_t *b, uint32_t ctl)
{
	uint32_t cc;
	size_t u, m;

	cc = 0;
	m = (a[0] + 63) >> 5;
	for (u = 1; u < m; u ++) {
		uint32_t aw, bw, naw;

		aw = a[u];
		bw = b[u];
		naw = aw - bw - cc;
		cc = naw >> 31;
		a[u] = MUX(ctl, naw & 0x7FFFFFFF, aw);
	}
	return cc;
}


/* ===== src/int/i31_tmont.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

/* see inner.h */
void
br_i31_to_monty(uint32_t *x, const uint32_t *m)
{
	uint32_t k;

	for (k = (m[0] + 31) >> 5; k > 0; k --) {
		br_i31_muladd_small(x, 0, m);
	}
}


/* ===== src/int/i32_div32.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

/* see inner.h */
uint32_t
br_divrem(uint32_t hi, uint32_t lo, uint32_t d, uint32_t *r)
{
	// TODO: optimize this
	uint32_t q;
	uint32_t ch, cf;
	int k;

	q = 0;
	ch = EQ(hi, d);
	hi = MUX(ch, 0, hi);
	for (k = 31; k > 0; k --) {
		int j;
		uint32_t w, ctl, hi2, lo2;

		j = 32 - k;
		w = (hi << j) | (lo >> k);
		ctl = GE(w, d) | (hi >> k);
		hi2 = (w - d) >> j;
		lo2 = lo - (d << k);
		hi = MUX(ctl, hi2, hi);
		lo = MUX(ctl, lo2, lo);
		q |= ctl << k;
	}
	cf = GE(lo, d) | hi;
	q |= cf;
	*r = MUX(cf, lo - d, lo);
	return q;
}


/* ===== src/int/i15_core.c ===== */
/*
 * Copyright (c) 2017 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

/*
 * This file contains the core "big integer" functions for the i15
 * implementation, that represents integers as sequences of 15-bit
 * words.
 */

/* see inner.h */
uint32_t
br_i15_iszero(const uint16_t *x)
{
	uint32_t z;
	size_t u;

	z = 0;
	for (u = (x[0] + 15) >> 4; u > 0; u --) {
		z |= x[u];
	}
	return ~(z | -z) >> 31;
}

/* see inner.h */
uint16_t
br_i15_ninv15(uint16_t x)
{
	uint32_t y;

	y = 2 - x;
	y = MUL15(y, 2 - MUL15(x, y));
	y = MUL15(y, 2 - MUL15(x, y));
	y = MUL15(y, 2 - MUL15(x, y));
	return MUX(x & 1, -y, 0) & 0x7FFF;
}

/* see inner.h */
uint32_t
br_i15_add(uint16_t *a, const uint16_t *b, uint32_t ctl)
{
	uint32_t cc;
	size_t u, m;

	cc = 0;
	m = (a[0] + 31) >> 4;
	for (u = 1; u < m; u ++) {
		uint32_t aw, bw, naw;

		aw = a[u];
		bw = b[u];
		naw = aw + bw + cc;
		cc = naw >> 15;
		a[u] = MUX(ctl, naw & 0x7FFF, aw);
	}
	return cc;
}

/* see inner.h */
uint32_t
br_i15_sub(uint16_t *a, const uint16_t *b, uint32_t ctl)
{
	uint32_t cc;
	size_t u, m;

	cc = 0;
	m = (a[0] + 31) >> 4;
	for (u = 1; u < m; u ++) {
		uint32_t aw, bw, naw;

		aw = a[u];
		bw = b[u];
		naw = aw - bw - cc;
		cc = naw >> 31;
		a[u] = MUX(ctl, naw & 0x7FFF, aw);
	}
	return cc;
}

/*
 * Constant-time division. The divisor must not be larger than 16 bits,
 * and the quotient must fit on 17 bits.
 */
static uint32_t
divrem16(uint32_t x, uint32_t d, uint32_t *r)
{
	int i;
	uint32_t q;

	q = 0;
	d <<= 16;
	for (i = 16; i >= 0; i --) {
		uint32_t ctl;

		ctl = LE(d, x);
		q |= ctl << i;
		x -= (-ctl) & d;
		d >>= 1;
	}
	if (r != NULL) {
		*r = x;
	}
	return q;
}

/* see inner.h */
void
br_i15_muladd_small(uint16_t *x, uint16_t z, const uint16_t *m)
{
	/*
	 * Constant-time: we accept to leak the exact bit length of the
	 * modulus m.
	 */
	unsigned m_bitlen, mblr;
	size_t u, mlen;
	uint32_t hi, a0, a, b, q;
	uint32_t cc, tb, over, under;

	/*
	 * Simple case: the modulus fits on one word.
	 */
	m_bitlen = m[0];
	if (m_bitlen == 0) {
		return;
	}
	if (m_bitlen <= 15) {
		uint32_t rem;

		divrem16(((uint32_t)x[1] << 15) | z, m[1], &rem);
		x[1] = rem;
		return;
	}
	mlen = (m_bitlen + 15) >> 4;
	mblr = m_bitlen & 15;

	/*
	 * Principle: we estimate the quotient (x*2^15+z)/m by
	 * doing a 30/15 division with the high words.
	 *
	 * Let:
	 *   w = 2^15
	 *   a = (w*a0 + a1) * w^N + a2
	 *   b = b0 * w^N + b2
	 * such that:
	 *   0 <= a0 < w
	 *   0 <= a1 < w
	 *   0 <= a2 < w^N
	 *   w/2 <= b0 < w
	 *   0 <= b2 < w^N
	 *   a < w*b
	 * I.e. the two top words of a are a0:a1, the top word of b is
	 * b0, we ensured that b0 is "full" (high bit set), and a is
	 * such that the quotient q = a/b fits on one word (0 <= q < w).
	 *
	 * If a = b*q + r (with 0 <= r < q), then we can estimate q by
	 * using a division on the top words:
	 *   a0*w + a1 = b0*u + v (with 0 <= v < b0)
	 * Then the following holds:
	 *   0 <= u <= w
	 *   u-2 <= q <= u
	 */
	hi = x[mlen];
	if (mblr == 0) {
		a0 = x[mlen];
		memmove(x + 2, x + 1, (mlen - 1) * sizeof *x);
		x[1] = z;
		a = (a0 << 15) + x[mlen];
		b = m[mlen];
	} else {
		a0 = (x[mlen] << (15 - mblr)) | (x[mlen - 1] >> mblr);
		memmove(x + 2, x + 1, (mlen - 1) * sizeof *x);
		x[1] = z;
		a = (a0 << 15) | (((x[mlen] << (15 - mblr))
			| (x[mlen - 1] >> mblr)) & 0x7FFF);
		b = (m[mlen] << (15 - mblr)) | (m[mlen - 1] >> mblr);
	}
	q = divrem16(a, b, NULL);

	/*
	 * We computed an estimate for q, but the real one may be q,
	 * q-1 or q-2; moreover, the division may have returned a value
	 * 8000 or even 8001 if the two high words were identical, and
	 * we want to avoid values beyond 7FFF. We thus adjust q so
	 * that the "true" multiplier will be q+1, q or q-1, and q is
	 * in the 0000..7FFF range.
	 */
	q = MUX(EQ(b, a0), 0x7FFF, q - 1 + ((q - 1) >> 31));

	/*
	 * We subtract q*m from x (x has an extra high word of value 'hi').
	 * Since q may be off by 1 (in either direction), we may have to
	 * add or subtract m afterwards.
	 *
	 * The 'tb' flag will be true (1) at the end of the loop if the
	 * result is greater than or equal to the modulus (not counting
	 * 'hi' or the carry).
	 */
	cc = 0;
	tb = 1;
	for (u = 1; u <= mlen; u ++) {
		uint32_t mw, zl, xw, nxw;

		mw = m[u];
		zl = MUL15(mw, q) + cc;
		cc = zl >> 15;
		zl &= 0x7FFF;
		xw = x[u];
		nxw = xw - zl;
		cc += nxw >> 31;
		nxw &= 0x7FFF;
		x[u] = nxw;
		tb = MUX(EQ(nxw, mw), tb, GT(nxw, mw));
	}

	/*
	 * If we underestimated q, then either cc < hi (one extra bit
	 * beyond the top array word), or cc == hi and tb is true (no
	 * extra bit, but the result is not lower than the modulus).
	 *
	 * If we overestimated q, then cc > hi.
	 */
	over = GT(cc, hi);
	under = ~over & (tb | LT(cc, hi));
	br_i15_add(x, m, over);
	br_i15_sub(x, m, under);
}

/* see inner.h */
void
br_i15_montymul(uint16_t *d, const uint16_t *x, const uint16_t *y,
	const uint16_t *m, uint16_t m0i)
{
	size_t len, len4, u, v;
	uint32_t dh;

	len = (m[0] + 15) >> 4;
	len4 = len & ~(size_t)3;
	br_i15_zero(d, m[0]);
	dh = 0;
	for (u = 0; u < len; u ++) {
		uint32_t f, xu, r, zh;

		xu = x[u + 1];
		f = MUL15(d[1] + MUL15(x[u + 1], y[1]), m0i) & 0x7FFF;

		r = 0;
		for (v = 0; v < len4; v += 4) {
			uint32_t z;

			z = d[v + 1] + MUL15(xu, y[v + 1])
				+ MUL15(f, m[v + 1]) + r;
			r = z >> 15;
			d[v + 0] = z & 0x7FFF;
			z = d[v + 2] + MUL15(xu, y[v + 2])
				+ MUL15(f, m[v + 2]) + r;
			r = z >> 15;
			d[v + 1] = z & 0x7FFF;
			z = d[v + 3] + MUL15(xu, y[v + 3])
				+ MUL15(f, m[v + 3]) + r;
			r = z >> 15;
			d[v + 2] = z & 0x7FFF;
			z = d[v + 4] + MUL15(xu, y[v + 4])
				+ MUL15(f, m[v + 4]) + r;
			r = z >> 15;
			d[v + 3] = z & 0x7FFF;
		}
		for (; v < len; v ++) {
			uint32_t z;

			z = d[v + 1] + MUL15(xu, y[v + 1])
				+ MUL15(f, m[v + 1]) + r;
			r = z >> 15;
			d[v + 0] = z & 0x7FFF;
		}

		zh = dh + r;
		d[len] = zh & 0x7FFF;
		dh = zh >> 31;
	}

	/*
	 * Restore the bit length (it was overwritten in the loop above).
	 */
	d[0] = m[0];

	/*
	 * d[] may be greater than m[], but it is still lower than twice
	 * the modulus.
	 */
	br_i15_sub(d, m, NEQ(dh, 0) | NOT(br_i15_sub(d, m, 0)));
}

/* see inner.h */
void
br_i15_to_monty(uint16_t *x, const uint16_t *m)
{
	unsigned k;

	for (k = (m[0] + 15) >> 4; k > 0; k --) {
		br_i15_muladd_small(x, 0, m);
	}
}

/* see inner.h */
void
br_i15_modpow(uint16_t *x,
	const unsigned char *e, size_t elen,
	const uint16_t *m, uint16_t m0i, uint16_t *t1, uint16_t *t2)
{
	size_t mlen;
	unsigned k;

	mlen = ((m[0] + 31) >> 4) * sizeof m[0];
	memcpy(t1, x, mlen);
	br_i15_to_monty(t1, m);
	br_i15_zero(x, m[0]);
	x[1] = 1;
	for (k = 0; k < ((unsigned)elen << 3); k ++) {
		uint32_t ctl;

		ctl = (e[elen - 1 - (k >> 3)] >> (k & 7)) & 1;
		br_i15_montymul(t2, x, t1, m, m0i);
		CCOPY(ctl, x, t2, mlen);
		br_i15_montymul(t2, t1, t1, m, m0i);
		memcpy(t1, t2, mlen);
	}
}

/* see inner.h */
void
br_i15_encode(void *dst, size_t len, const uint16_t *x)
{
	unsigned char *buf;
	size_t u, xlen;
	uint32_t acc;
	int acc_len;

	xlen = (x[0] + 15) >> 4;
	if (xlen == 0) {
		memset(dst, 0, len);
		return;
	}
	u = 1;
	acc = 0;
	acc_len = 0;
	buf = dst;
	while (len -- > 0) {
		if (acc_len < 8) {
			if (u <= xlen) {
				acc += (uint32_t)x[u ++] << acc_len;
			}
			acc_len += 15;
		}
		buf[len] = (unsigned char)acc;
		acc >>= 8;
		acc_len -= 8;
	}
}

/* see inner.h */
uint32_t
br_i15_decode_mod(uint16_t *x, const void *src, size_t len, const uint16_t *m)
{
	/*
	 * Two-pass algorithm: in the first pass, we determine whether the
	 * value fits; in the second pass, we do the actual write.
	 *
	 * During the first pass, 'r' contains the comparison result so
	 * far:
	 *  0x00000000   value is equal to the modulus
	 *  0x00000001   value is greater than the modulus
	 *  0xFFFFFFFF   value is lower than the modulus
	 *
	 * Since we iterate starting with the least significant bytes (at
	 * the end of src[]), each new comparison overrides the previous
	 * except when the comparison yields 0 (equal).
	 *
	 * During the second pass, 'r' is either 0xFFFFFFFF (value fits)
	 * or 0x00000000 (value does not fit).
	 *
	 * We must iterate over all bytes of the source, _and_ possibly
	 * some extra virutal bytes (with value 0) so as to cover the
	 * complete modulus as well. We also add 4 such extra bytes beyond
	 * the modulus length because it then guarantees that no accumulated
	 * partial word remains to be processed.
	 */
	const unsigned char *buf;
	size_t mlen, tlen;
	int pass;
	uint32_t r;

	buf = src;
	mlen = (m[0] + 15) >> 4;
	tlen = (mlen << 1);
	if (tlen < len) {
		tlen = len;
	}
	tlen += 4;
	r = 0;
	for (pass = 0; pass < 2; pass ++) {
		size_t u, v;
		uint32_t acc;
		int acc_len;

		v = 1;
		acc = 0;
		acc_len = 0;
		for (u = 0; u < tlen; u ++) {
			uint32_t b;

			if (u < len) {
				b = buf[len - 1 - u];
			} else {
				b = 0;
			}
			acc |= (b << acc_len);
			acc_len += 8;
			if (acc_len >= 15) {
				uint32_t xw;

				xw = acc & (uint32_t)0x7FFF;
				acc_len -= 15;
				acc = b >> (8 - acc_len);
				if (v <= mlen) {
					if (pass) {
						x[v] = r & xw;
					} else {
						uint32_t cc;

						cc = (uint32_t)CMP(xw, m[v]);
						r = MUX(EQ(cc, 0), r, cc);
					}
				} else {
					if (!pass) {
						r = MUX(EQ(xw, 0), r, 1);
					}
				}
				v ++;
			}
		}

		/*
		 * When we reach this point at the end of the first pass:
		 * r is either 0, 1 or -1; we want to set r to 0 if it
		 * is equal to 0 or 1, and leave it to -1 otherwise.
		 *
		 * When we reach this point at the end of the second pass:
		 * r is either 0 or -1; we want to leave that value
		 * untouched. This is a subcase of the previous.
		 */
		r >>= 1;
		r |= (r << 1);
	}

	x[0] = m[0];
	return r & (uint32_t)1;
}


/* ===== src/int/i15_ext1.c ===== */
/*
 * Copyright (c) 2017 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

/*
 * This file contains some additional functions for "i15" big integers.
 * These functions are needed to support ECDSA.
 */

/* see inner.h */
void
br_i15_rshift(uint16_t *x, int count)
{
	size_t u, len;
	unsigned r;

	len = (x[0] + 15) >> 4;
	if (len == 0) {
		return;
	}
	r = x[1] >> count;
	for (u = 2; u <= len; u ++) {
		unsigned w;

		w = x[u];
		x[u - 1] = ((w << (15 - count)) | r) & 0x7FFF;
		r = w >> count;
	}
	x[len] = r;
}

/* see inner.h */
uint32_t
br_i15_bit_length(uint16_t *x, size_t xlen)
{
	uint32_t tw, twk;

	tw = 0;
	twk = 0;
	while (xlen -- > 0) {
		uint32_t w, c;

		c = EQ(tw, 0);
		w = x[xlen];
		tw = MUX(c, w, tw);
		twk = MUX(c, (uint32_t)xlen, twk);
	}
	return (twk << 4) + BIT_LENGTH(tw);
}

/* see inner.h */
void
br_i15_decode(uint16_t *x, const void *src, size_t len)
{
	const unsigned char *buf;
	size_t v;
	uint32_t acc;
	int acc_len;

	buf = src;
	v = 1;
	acc = 0;
	acc_len = 0;
	while (len -- > 0) {
		uint32_t b;

		b = buf[len];
		acc |= (b << acc_len);
		acc_len += 8;
		if (acc_len >= 15) {
			x[v ++] = acc & 0x7FFF;
			acc_len -= 15;
			acc >>= 15;
		}
	}
	if (acc_len != 0) {
		x[v ++] = acc;
	}
	x[0] = br_i15_bit_length(x + 1, v - 1);
}

/* see inner.h */
void
br_i15_from_monty(uint16_t *x, const uint16_t *m, uint16_t m0i)
{
	size_t len, u, v;

	len = (m[0] + 15) >> 4;
	for (u = 0; u < len; u ++) {
		uint32_t f, cc;

		f = MUL15(x[1], m0i) & 0x7FFF;
		cc = 0;
		for (v = 0; v < len; v ++) {
			uint32_t z;

			z = (uint32_t)x[v + 1] + MUL15(f, m[v + 1]) + cc;
			cc = z >> 15;
			if (v != 0) {
				x[v] = z & 0x7FFF;
			}
		}
		x[len] = cc;
	}

	/*
	 * We may have to do an extra subtraction, but only if the
	 * value in x[] is indeed greater than or equal to that of m[],
	 * which is why we must do two calls (first call computes the
	 * carry, second call performs the subtraction only if the carry
	 * is 0).
	 */
	br_i15_sub(x, m, NOT(br_i15_sub(x, m, 0)));
}


/* ===== src/int/i15_ext2.c ===== */
/*
 * Copyright (c) 2017 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

/*
 * This file contains some additional functions for "i15" big integers.
 * These functions are needed to support RSA.
 */

/* see inner.h */
void
br_i15_decode_reduce(uint16_t *x,
	const void *src, size_t len, const uint16_t *m)
{
	uint32_t m_ebitlen, m_rbitlen;
	size_t mblen, k;
	const unsigned char *buf;
	uint32_t acc;
	int acc_len;

	/*
	 * Get the encoded bit length.
	 */
	m_ebitlen = m[0];

	/*
	 * Special case for an invalid (null) modulus.
	 */
	if (m_ebitlen == 0) {
		x[0] = 0;
		return;
	}

	/*
	 * Clear the destination.
	 */
	br_i15_zero(x, m_ebitlen);

	/*
	 * First decode directly as many bytes as possible. This requires
	 * computing the actual bit length.
	 */
	m_rbitlen = m_ebitlen >> 4;
	m_rbitlen = (m_ebitlen & 15) + (m_rbitlen << 4) - m_rbitlen;
	mblen = (m_rbitlen + 7) >> 3;
	k = mblen - 1;
	if (k >= len) {
		br_i15_decode(x, src, len);
		x[0] = m_ebitlen;
		return;
	}
	buf = src;
	br_i15_decode(x, buf, k);
	x[0] = m_ebitlen;

	/*
	 * Input remaining bytes, using 15-bit words.
	 */
	acc = 0;
	acc_len = 0;
	while (k < len) {
		uint32_t v;

		v = buf[k ++];
		acc = (acc << 8) | v;
		acc_len += 8;
		if (acc_len >= 15) {
			br_i15_muladd_small(x, acc >> (acc_len - 15), m);
			acc_len -= 15;
			acc &= ~((uint32_t)-1 << acc_len);
		}
	}

	/*
	 * We may have some bits accumulated. We then perform a shift to
	 * be able to inject these bits as a full 15-bit word.
	 */
	if (acc_len != 0) {
		acc = (acc | (x[1] << acc_len)) & 0x7FFF;
		br_i15_rshift(x, 15 - acc_len);
		br_i15_muladd_small(x, acc, m);
	}
}

/* see inner.h */
void
br_i15_reduce(uint16_t *x, const uint16_t *a, const uint16_t *m)
{
	uint32_t m_bitlen, a_bitlen;
	size_t mlen, alen, u;

	m_bitlen = m[0];
	mlen = (m_bitlen + 15) >> 4;

	x[0] = m_bitlen;
	if (m_bitlen == 0) {
		return;
	}

	/*
	 * If the source is shorter, then simply copy all words from a[]
	 * and zero out the upper words.
	 */
	a_bitlen = a[0];
	alen = (a_bitlen + 15) >> 4;
	if (a_bitlen < m_bitlen) {
		memcpy(x + 1, a + 1, alen * sizeof *a);
		for (u = alen; u < mlen; u ++) {
			x[u + 1] = 0;
		}
		return;
	}

	/*
	 * The source length is at least equal to that of the modulus.
	 * We must thus copy N-1 words, and input the remaining words
	 * one by one.
	 */
	memcpy(x + 1, a + 2 + (alen - mlen), (mlen - 1) * sizeof *a);
	x[mlen] = 0;
	for (u = 1 + alen - mlen; u > 0; u --) {
		br_i15_muladd_small(x, a[u], m);
	}
}

/* see inner.h */
void
br_i15_mulacc(uint16_t *d, const uint16_t *a, const uint16_t *b)
{
	size_t alen, blen, u;

	alen = (a[0] + 15) >> 4;
	blen = (b[0] + 15) >> 4;
	d[0] = a[0] + b[0];
	for (u = 0; u < blen; u ++) {
		uint32_t f;
		size_t v;
		uint32_t cc;

		f = b[1 + u];
		cc = 0;
		for (v = 0; v < alen; v ++) {
			uint32_t z;

			z = (uint32_t)d[1 + u + v] + MUL15(f, a[1 + v]) + cc;
			cc = z >> 15;
			d[1 + u + v] = z & 0x7FFF;
		}
		d[1 + u + alen] = cc;
	}
}


/* ===== src/rsa/rsa_i31_pkcs1_vrfy.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

/* see bearssl_rsa.h */
uint32_t
br_rsa_i31_pkcs1_vrfy(const unsigned char *x, size_t xlen,
	const unsigned char *hash_oid, size_t hash_len,
	const br_rsa_public_key *pk, unsigned char *hash_out)
{
	unsigned char sig[BR_MAX_RSA_SIZE >> 3];

	if (xlen > (sizeof sig)) {
		return 0;
	}
	memcpy(sig, x, xlen);
	if (!br_rsa_i31_public(sig, xlen, pk)) {
		return 0;
	}
	return br_rsa_pkcs1_sig_unpad(sig, xlen, hash_oid, hash_len, hash_out);
}


/* ===== src/rsa/rsa_i31_pub.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

/* see bearssl_rsa.h */
uint32_t
br_rsa_i31_public(unsigned char *x, size_t xlen,
	const br_rsa_public_key *pk)
{
	const unsigned char *n;
	size_t nlen;
	uint32_t m[1 + ((BR_MAX_RSA_SIZE + 30) / 31)];
	uint32_t a[1 + ((BR_MAX_RSA_SIZE + 30) / 31)];
	uint32_t t1[1 + ((BR_MAX_RSA_SIZE + 30) / 31)];
	uint32_t t2[1 + ((BR_MAX_RSA_SIZE + 30) / 31)];
	uint32_t m0i, r;

	/*
	 * Get the actual length of the modulus, and see if it fits within
	 * our stack buffer. We also check that the length of x[] is valid.
	 */
	n = pk->n;
	nlen = pk->nlen;
	while (nlen > 0 && *n == 0) {
		n ++;
		nlen --;
	}
	if (nlen == 0 || nlen > (BR_MAX_RSA_SIZE >> 3) || xlen != nlen) {
		return 0;
	}
	br_i31_decode(m, n, nlen);
	m0i = br_i31_ninv31(m[1]);

	/*
	 * Note: if m[] is even, then m0i == 0. Otherwise, m0i must be
	 * an odd integer.
	 */
	r = m0i & 1;

	/*
	 * Decode x[] into a[]; we also check that its value is proper.
	 */
	r &= br_i31_decode_mod(a, x, xlen, m);

	/*
	 * Compute the modular exponentiation.
	 */
	br_i31_modpow(a, pk->e, pk->elen, m, m0i, t1, t2);

	/*
	 * Encode the result.
	 */
	br_i31_encode(x, xlen, a);
	return r;
}


/* ===== src/rsa/rsa_pkcs1_sig_unpad.c ===== */
/*
 * Copyright (c) 2017 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

/* see bearssl_rsa.h */
uint32_t
br_rsa_pkcs1_sig_unpad(const unsigned char *sig, size_t sig_len,
	const unsigned char *hash_oid, size_t hash_len,
	unsigned char *hash_out)
{
	static const unsigned char pad1[] = {
		0x00, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
	};

	unsigned char pad2[43];
	size_t u, x2, x3, pad_len, zlen;

	if (sig_len < 11) {
		return 0;
	}

	/*
	 * Expected format:
	 *  00 01 FF ... FF 00 30 x1 30 x2 06 x3 OID [ 05 00 ] 04 x4 HASH
	 *
	 * with the following rules:
	 *
	 *  -- Total length is that of the modulus and the signature
	 *     (this was already verified by br_rsa_i31_public()).
	 *
	 *  -- There are at least eight bytes of value 0xFF.
	 *
	 *  -- x4 is equal to the hash length (hash_len).
	 *
	 *  -- x3 is equal to the encoded OID value length (so x3 is the
	 *     first byte of hash_oid[]).
	 *
	 *  -- If the "05 00" is present, then x2 == x3 + 4; otherwise,
	 *     x2 == x3 + 2.
	 *
	 *  -- x1 == x2 + x4 + 4.
	 *
	 * So the total length after the last "FF" is either x3 + x4 + 11
	 * (with the "05 00") or x3 + x4 + 9 (without the "05 00").
	 */

	/*
	 * Check the "00 01 FF .. FF 00" with at least eight 0xFF bytes.
	 * The comparaison is valid because we made sure that the signature
	 * is at least 11 bytes long.
	 */
	if (memcmp(sig, pad1, sizeof pad1) != 0) {
		return 0;
	}
	for (u = sizeof pad1; u < sig_len; u ++) {
		if (sig[u] != 0xFF) {
			break;
		}
	}

	/*
	 * Remaining length is sig_len - u bytes (including the 00 just
	 * after the last FF). This must be equal to one of the two
	 * possible values (depending on whether the "05 00" sequence is
	 * present or not).
	 */
	if (hash_oid == NULL) {
		if (sig_len - u != hash_len + 1 || sig[u] != 0x00) {
			return 0;
		}
	} else {
		x3 = hash_oid[0];
		pad_len = x3 + 9;
		memset(pad2, 0, pad_len);
		zlen = sig_len - u - hash_len;
		if (zlen == pad_len) {
			x2 = x3 + 2;
		} else if (zlen == pad_len + 2) {
			x2 = x3 + 4;
			pad_len = zlen;
			pad2[pad_len - 4] = 0x05;
		} else {
			return 0;
		}
		pad2[1] = 0x30;
		pad2[2] = x2 + hash_len + 4;
		pad2[3] = 0x30;
		pad2[4] = x2;
		pad2[5] = 0x06;
		memcpy(pad2 + 6, hash_oid, x3 + 1);
		pad2[pad_len - 2] = 0x04;
		pad2[pad_len - 1] = hash_len;
		if (memcmp(pad2, sig + u, pad_len) != 0) {
			return 0;
		}
	}
	memcpy(hash_out, sig + sig_len - hash_len, hash_len);
	return 1;
}


/* ===== src/rsa/rsa_ssl_decrypt.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

/* see bearssl_rsa.h */
uint32_t
br_rsa_ssl_decrypt(br_rsa_private core, const br_rsa_private_key *sk,
	unsigned char *data, size_t len)
{
	uint32_t x;
	size_t u;

	/*
	 * A first check on length. Since this test works only on the
	 * buffer length, it needs not (and cannot) be constant-time.
	 */
	if (len < 59 || len != (sk->n_bitlen + 7) >> 3) {
		return 0;
	}
	x = core(data, sk);

	x &= EQ(data[0], 0x00);
	x &= EQ(data[1], 0x02);
	for (u = 2; u < (len - 49); u ++) {
		x &= NEQ(data[u], 0);
	}
	x &= EQ(data[len - 49], 0x00);
	memmove(data, data + len - 48, 48);
	return x;
}


/* ===== src/rsa/rsa_i15_pub.c ===== */
/*
 * Copyright (c) 2017 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

/* see bearssl_rsa.h */
uint32_t
br_rsa_i15_public(unsigned char *x, size_t xlen,
	const br_rsa_public_key *pk)
{
	const unsigned char *n;
	size_t nlen;
	uint16_t m[1 + ((BR_MAX_RSA_SIZE + 14) / 15)];
	uint16_t a[1 + ((BR_MAX_RSA_SIZE + 14) / 15)];
	uint16_t t1[1 + ((BR_MAX_RSA_SIZE + 14) / 15)];
	uint16_t t2[1 + ((BR_MAX_RSA_SIZE + 14) / 15)];
	uint16_t m0i;
	uint32_t r;

	/*
	 * Get the actual length of the modulus, and see if it fits within
	 * our stack buffer. We also check that the length of x[] is valid.
	 */
	n = pk->n;
	nlen = pk->nlen;
	while (nlen > 0 && *n == 0) {
		n ++;
		nlen --;
	}
	if (nlen == 0 || nlen > (BR_MAX_RSA_SIZE >> 3) || xlen != nlen) {
		return 0;
	}
	br_i15_decode(m, n, nlen);
	m0i = br_i15_ninv15(m[1]);

	/*
	 * Note: if m[] is even, then m0i == 0. Otherwise, m0i must be
	 * an odd integer.
	 */
	r = m0i & 1;

	/*
	 * Decode x[] into a[]; we also check that its value is proper.
	 */
	r &= br_i15_decode_mod(a, x, xlen, m);

	/*
	 * Compute the modular exponentiation.
	 */
	br_i15_modpow(a, pk->e, pk->elen, m, m0i, t1, t2);

	/*
	 * Encode the result.
	 */
	br_i15_encode(x, xlen, a);
	return r;
}


/* ===== src/rsa/rsa_i15_pkcs1_vrfy.c ===== */
/*
 * Copyright (c) 2017 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

/* see bearssl_rsa.h */
uint32_t
br_rsa_i15_pkcs1_vrfy(const unsigned char *x, size_t xlen,
	const unsigned char *hash_oid, size_t hash_len,
	const br_rsa_public_key *pk, unsigned char *hash_out)
{
	unsigned char sig[BR_MAX_RSA_SIZE >> 3];

	if (xlen > (sizeof sig)) {
		return 0;
	}
	memcpy(sig, x, xlen);
	if (!br_rsa_i15_public(sig, xlen, pk)) {
		return 0;
	}
	return br_rsa_pkcs1_sig_unpad(sig, xlen, hash_oid, hash_len, hash_out);
}


/* ===== src/ec/ec_prime_i31.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

/*
 * Parameters for supported curves (field modulus, and 'b' equation
 * parameter; both values use the 'i31' format, and 'b' is in Montgomery
 * representation).
 */

static const uint32_t P256_P[] = {
	0x00000108,
	0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x00000007,
	0x00000000, 0x00000000, 0x00000040, 0x7FFFFF80,
	0x000000FF
};

static const uint32_t P256_R2[] = {
	0x00000108,
	0x00014000, 0x00018000, 0x00000000, 0x7FF40000,
	0x7FEFFFFF, 0x7FF7FFFF, 0x7FAFFFFF, 0x005FFFFF,
	0x00000000
};

static const uint32_t P256_B[] = {
	0x00000108,
	0x6FEE1803, 0x6229C4BD, 0x21B139BE, 0x327150AA,
	0x3567802E, 0x3F7212ED, 0x012E4355, 0x782DD38D,
	0x0000000E
};

static const uint32_t P384_P[] = {
	0x0000018C,
	0x7FFFFFFF, 0x00000001, 0x00000000, 0x7FFFFFF8,
	0x7FFFFFEF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF,
	0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF,
	0x00000FFF
};

static const uint32_t P384_R2[] = {
	0x0000018C,
	0x00000000, 0x00000080, 0x7FFFFE00, 0x000001FF,
	0x00000800, 0x00000000, 0x7FFFE000, 0x00001FFF,
	0x00008000, 0x00008000, 0x00000000, 0x00000000,
	0x00000000
};

static const uint32_t P384_B[] = {
	0x0000018C,
	0x6E666840, 0x070D0392, 0x5D810231, 0x7651D50C,
	0x17E218D6, 0x1B192002, 0x44EFE441, 0x3A524E2B,
	0x2719BA5F, 0x41F02209, 0x36C5643E, 0x5813EFFE,
	0x000008A5
};

static const uint32_t P521_P[] = {
	0x00000219,
	0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF,
	0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF,
	0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF,
	0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF,
	0x01FFFFFF
};

static const uint32_t P521_R2[] = {
	0x00000219,
	0x00001000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000
};

static const uint32_t P521_B[] = {
	0x00000219,
	0x540FC00A, 0x228FEA35, 0x2C34F1EF, 0x67BF107A,
	0x46FC1CD5, 0x1605E9DD, 0x6937B165, 0x272A3D8F,
	0x42785586, 0x44C8C778, 0x15F3B8B4, 0x64B73366,
	0x03BA8B69, 0x0D05B42A, 0x21F929A2, 0x2C31C393,
	0x00654FAE
};

typedef struct {
	const uint32_t *p;
	const uint32_t *b;
	const uint32_t *R2;
	uint32_t p0i;
} curve_params;

static inline const curve_params *
id_to_curve(int curve)
{
	static const curve_params pp[] = {
		{ P256_P, P256_B, P256_R2, 0x00000001 },
		{ P384_P, P384_B, P384_R2, 0x00000001 },
		{ P521_P, P521_B, P521_R2, 0x00000001 }
	};

	return &pp[curve - BR_EC_secp256r1];
}

#define I31_LEN   ((BR_MAX_EC_SIZE + 61) / 31)

/*
 * Type for a point in Jacobian coordinates:
 * -- three values, x, y and z, in Montgomery representation
 * -- affine coordinates are X = x / z^2 and Y = y / z^3
 * -- for the point at infinity, z = 0
 */
typedef struct {
	uint32_t c[3][I31_LEN];
} jacobian;

/*
 * We use a custom interpreter that uses a dozen registers, and
 * only six operations:
 *    MSET(d, a)       copy a into d
 *    MADD(d, a)       d = d+a (modular)
 *    MSUB(d, a)       d = d-a (modular)
 *    MMUL(d, a, b)    d = a*b (Montgomery multiplication)
 *    MINV(d, a, b)    invert d modulo p; a and b are used as scratch registers
 *    MTZ(d)           clear return value if d = 0
 * Destination of MMUL (d) must be distinct from operands (a and b).
 * There is no such constraint for MSUB and MADD.
 *
 * Registers include the operand coordinates, and temporaries.
 */
#define MSET(d, a)      (0x0000 + ((d) << 8) + ((a) << 4))
#define MADD(d, a)      (0x1000 + ((d) << 8) + ((a) << 4))
#define MSUB(d, a)      (0x2000 + ((d) << 8) + ((a) << 4))
#define MMUL(d, a, b)   (0x3000 + ((d) << 8) + ((a) << 4) + (b))
#define MINV(d, a, b)   (0x4000 + ((d) << 8) + ((a) << 4) + (b))
#define MTZ(d)          (0x5000 + ((d) << 8))
#define ENDCODE         0

/*
 * Registers for the input operands.
 */
#define P1x    0
#define P1y    1
#define P1z    2
#define P2x    3
#define P2y    4
#define P2z    5

/*
 * Alternate names for the first input operand.
 */
#define Px     0
#define Py     1
#define Pz     2

/*
 * Temporaries.
 */
#define t1     6
#define t2     7
#define t3     8
#define t4     9
#define t5    10
#define t6    11
#define t7    12

/*
 * Extra scratch registers available when there is no second operand (e.g.
 * for "double" and "affine").
 */
#define t8     3
#define t9     4
#define t10    5

/*
 * Doubling formulas are:
 *
 *   s = 4*x*y^2
 *   m = 3*(x + z^2)*(x - z^2)
 *   x' = m^2 - 2*s
 *   y' = m*(s - x') - 8*y^4
 *   z' = 2*y*z
 *
 * If y = 0 (P has order 2) then this yields infinity (z' = 0), as it
 * should. This case should not happen anyway, because our curves have
 * prime order, and thus do not contain any point of order 2.
 *
 * If P is infinity (z = 0), then again the formulas yield infinity,
 * which is correct. Thus, this code works for all points.
 *
 * Cost: 8 multiplications
 */
static const uint16_t code_double[] = {
	/*
	 * Compute z^2 (in t1).
	 */
	MMUL(t1, Pz, Pz),

	/*
	 * Compute x-z^2 (in t2) and then x+z^2 (in t1).
	 */
	MSET(t2, Px),
	MSUB(t2, t1),
	MADD(t1, Px),

	/*
	 * Compute m = 3*(x+z^2)*(x-z^2) (in t1).
	 */
	MMUL(t3, t1, t2),
	MSET(t1, t3),
	MADD(t1, t3),
	MADD(t1, t3),

	/*
	 * Compute s = 4*x*y^2 (in t2) and 2*y^2 (in t3).
	 */
	MMUL(t3, Py, Py),
	MADD(t3, t3),
	MMUL(t2, Px, t3),
	MADD(t2, t2),

	/*
	 * Compute x' = m^2 - 2*s.
	 */
	MMUL(Px, t1, t1),
	MSUB(Px, t2),
	MSUB(Px, t2),

	/*
	 * Compute z' = 2*y*z.
	 */
	MMUL(t4, Py, Pz),
	MSET(Pz, t4),
	MADD(Pz, t4),

	/*
	 * Compute y' = m*(s - x') - 8*y^4. Note that we already have
	 * 2*y^2 in t3.
	 */
	MSUB(t2, Px),
	MMUL(Py, t1, t2),
	MMUL(t4, t3, t3),
	MSUB(Py, t4),
	MSUB(Py, t4),

	ENDCODE
};

/*
 * Addtions formulas are:
 *
 *   u1 = x1 * z2^2
 *   u2 = x2 * z1^2
 *   s1 = y1 * z2^3
 *   s2 = y2 * z1^3
 *   h = u2 - u1
 *   r = s2 - s1
 *   x3 = r^2 - h^3 - 2 * u1 * h^2
 *   y3 = r * (u1 * h^2 - x3) - s1 * h^3
 *   z3 = h * z1 * z2
 *
 * If both P1 and P2 are infinity, then z1 == 0 and z2 == 0, implying that
 * z3 == 0, so the result is correct.
 * If either of P1 or P2 is infinity, but not both, then z3 == 0, which is
 * not correct.
 * h == 0 only if u1 == u2; this happens in two cases:
 * -- if s1 == s2 then P1 and/or P2 is infinity, or P1 == P2
 * -- if s1 != s2 then P1 + P2 == infinity (but neither P1 or P2 is infinity)
 *
 * Thus, the following situations are not handled correctly:
 * -- P1 = 0 and P2 != 0
 * -- P1 != 0 and P2 = 0
 * -- P1 = P2
 * All other cases are properly computed. However, even in "incorrect"
 * situations, the three coordinates still are properly formed field
 * elements.
 *
 * The returned flag is cleared if r == 0. This happens in the following
 * cases:
 * -- Both points are on the same horizontal line (same Y coordinate).
 * -- Both points are infinity.
 * -- One point is infinity and the other is on line Y = 0.
 * The third case cannot happen with our curves (there is no valid point
 * on line Y = 0 since that would be a point of order 2). If the two
 * source points are non-infinity, then remains only the case where the
 * two points are on the same horizontal line.
 *
 * This allows us to detect the "P1 == P2" case, assuming that P1 != 0 and
 * P2 != 0:
 * -- If the returned value is not the point at infinity, then it was properly
 * computed.
 * -- Otherwise, if the returned flag is 1, then P1+P2 = 0, and the result
 * is indeed the point at infinity.
 * -- Otherwise (result is infinity, flag is 0), then P1 = P2 and we should
 * use the 'double' code.
 *
 * Cost: 16 multiplications
 */
static const uint16_t code_add[] = {
	/*
	 * Compute u1 = x1*z2^2 (in t1) and s1 = y1*z2^3 (in t3).
	 */
	MMUL(t3, P2z, P2z),
	MMUL(t1, P1x, t3),
	MMUL(t4, P2z, t3),
	MMUL(t3, P1y, t4),

	/*
	 * Compute u2 = x2*z1^2 (in t2) and s2 = y2*z1^3 (in t4).
	 */
	MMUL(t4, P1z, P1z),
	MMUL(t2, P2x, t4),
	MMUL(t5, P1z, t4),
	MMUL(t4, P2y, t5),

	/*
	 * Compute h = u2 - u1 (in t2) and r = s2 - s1 (in t4).
	 */
	MSUB(t2, t1),
	MSUB(t4, t3),

	/*
	 * Report cases where r = 0 through the returned flag.
	 */
	MTZ(t4),

	/*
	 * Compute u1*h^2 (in t6) and h^3 (in t5).
	 */
	MMUL(t7, t2, t2),
	MMUL(t6, t1, t7),
	MMUL(t5, t7, t2),

	/*
	 * Compute x3 = r^2 - h^3 - 2*u1*h^2.
	 * t1 and t7 can be used as scratch registers.
	 */
	MMUL(P1x, t4, t4),
	MSUB(P1x, t5),
	MSUB(P1x, t6),
	MSUB(P1x, t6),

	/*
	 * Compute y3 = r*(u1*h^2 - x3) - s1*h^3.
	 */
	MSUB(t6, P1x),
	MMUL(P1y, t4, t6),
	MMUL(t1, t5, t3),
	MSUB(P1y, t1),

	/*
	 * Compute z3 = h*z1*z2.
	 */
	MMUL(t1, P1z, P2z),
	MMUL(P1z, t1, t2),

	ENDCODE
};

/*
 * Check that the point is on the curve. This code snippet assumes the
 * following conventions:
 * -- Coordinates x and y have been freshly decoded in P1 (but not
 * converted to Montgomery coordinates yet).
 * -- P2x, P2y and P2z are set to, respectively, R^2, b*R and 1.
 */
static const uint16_t code_check[] = {

	/* Convert x and y to Montgomery representation. */
	MMUL(t1, P1x, P2x),
	MMUL(t2, P1y, P2x),
	MSET(P1x, t1),
	MSET(P1y, t2),

	/* Compute x^3 in t1. */
	MMUL(t2, P1x, P1x),
	MMUL(t1, P1x, t2),

	/* Subtract 3*x from t1. */
	MSUB(t1, P1x),
	MSUB(t1, P1x),
	MSUB(t1, P1x),

	/* Add b. */
	MADD(t1, P2y),

	/* Compute y^2 in t2. */
	MMUL(t2, P1y, P1y),

	/* Compare y^2 with x^3 - 3*x + b; they must match. */
	MSUB(t1, t2),
	MTZ(t1),

	/* Set z to 1 (in Montgomery representation). */
	MMUL(P1z, P2x, P2z),

	ENDCODE
};

/*
 * Conversion back to affine coordinates. This code snippet assumes that
 * the z coordinate of P2 is set to 1 (not in Montgomery representation).
 */
static const uint16_t code_affine[] = {

	/* Save z*R in t1. */
	MSET(t1, P1z),

	/* Compute z^3 in t2. */
	MMUL(t2, P1z, P1z),
	MMUL(t3, P1z, t2),
	MMUL(t2, t3, P2z),

	/* Invert to (1/z^3) in t2. */
	MINV(t2, t3, t4),

	/* Compute y. */
	MSET(t3, P1y),
	MMUL(P1y, t2, t3),

	/* Compute (1/z^2) in t3. */
	MMUL(t3, t2, t1),

	/* Compute x. */
	MSET(t2, P1x),
	MMUL(P1x, t2, t3),

	ENDCODE
};

static uint32_t
run_code(jacobian *P1, const jacobian *P2,
	const curve_params *cc, const uint16_t *code)
{
	uint32_t r;
	uint32_t t[13][I31_LEN];
	size_t u;

	r = 1;

	/*
	 * Copy the two operands in the dedicated registers.
	 */
	memcpy(t[P1x], P1->c, 3 * I31_LEN * sizeof(uint32_t));
	memcpy(t[P2x], P2->c, 3 * I31_LEN * sizeof(uint32_t));

	/*
	 * Run formulas.
	 */
	for (u = 0;; u ++) {
		unsigned op, d, a, b;

		op = code[u];
		if (op == 0) {
			break;
		}
		d = (op >> 8) & 0x0F;
		a = (op >> 4) & 0x0F;
		b = op & 0x0F;
		op >>= 12;
		switch (op) {
			uint32_t ctl;
			size_t plen;
			unsigned char tp[(BR_MAX_EC_SIZE + 7) >> 3];

		case 0:
			memcpy(t[d], t[a], I31_LEN * sizeof(uint32_t));
			break;
		case 1:
			ctl = br_i31_add(t[d], t[a], 1);
			ctl |= NOT(br_i31_sub(t[d], cc->p, 0));
			br_i31_sub(t[d], cc->p, ctl);
			break;
		case 2:
			br_i31_add(t[d], cc->p, br_i31_sub(t[d], t[a], 1));
			break;
		case 3:
			br_i31_montymul(t[d], t[a], t[b], cc->p, cc->p0i);
			break;
		case 4:
			plen = (cc->p[0] - (cc->p[0] >> 5) + 7) >> 3;
			br_i31_encode(tp, plen, cc->p);
			tp[plen - 1] -= 2;
			br_i31_modpow(t[d], tp, plen,
				cc->p, cc->p0i, t[a], t[b]);
			break;
		default:
			r &= ~br_i31_iszero(t[d]);
			break;
		}
	}

	/*
	 * Copy back result.
	 */
	memcpy(P1->c, t[P1x], 3 * I31_LEN * sizeof(uint32_t));
	return r;
}

static void
set_one(uint32_t *x, const uint32_t *p)
{
	size_t plen;

	plen = (p[0] + 63) >> 5;
	memset(x, 0, plen * sizeof *x);
	x[0] = p[0];
	x[1] = 0x00000001;
}

static void
point_zero(jacobian *P, const curve_params *cc)
{
	memset(P, 0, sizeof *P);
	P->c[0][0] = P->c[1][0] = P->c[2][0] = cc->p[0];
}

static inline void
point_double(jacobian *P, const curve_params *cc)
{
	run_code(P, P, cc, code_double);
}

static inline uint32_t
point_add(jacobian *P1, const jacobian *P2, const curve_params *cc)
{
	return run_code(P1, P2, cc, code_add);
}

static void
point_mul(jacobian *P, const unsigned char *x, size_t xlen,
	const curve_params *cc)
{
	/*
	 * We do a simple double-and-add ladder with a 2-bit window
	 * to make only one add every two doublings. We thus first
	 * precompute 2P and 3P in some local buffers.
	 *
	 * We always perform two doublings and one addition; the
	 * addition is with P, 2P and 3P and is done in a temporary
	 * array.
	 *
	 * The addition code cannot handle cases where one of the
	 * operands is infinity, which is the case at the start of the
	 * ladder. We therefore need to maintain a flag that controls
	 * this situation.
	 */
	uint32_t qz;
	jacobian P2, P3, Q, T, U;

	memcpy(&P2, P, sizeof P2);
	point_double(&P2, cc);
	memcpy(&P3, P, sizeof P3);
	point_add(&P3, &P2, cc);

	point_zero(&Q, cc);
	qz = 1;
	while (xlen -- > 0) {
		int k;

		for (k = 6; k >= 0; k -= 2) {
			uint32_t bits;
			uint32_t bnz;

			point_double(&Q, cc);
			point_double(&Q, cc);
			memcpy(&T, P, sizeof T);
			memcpy(&U, &Q, sizeof U);
			bits = (*x >> k) & (uint32_t)3;
			bnz = NEQ(bits, 0);
			CCOPY(EQ(bits, 2), &T, &P2, sizeof T);
			CCOPY(EQ(bits, 3), &T, &P3, sizeof T);
			point_add(&U, &T, cc);
			CCOPY(bnz & qz, &Q, &T, sizeof Q);
			CCOPY(bnz & ~qz, &Q, &U, sizeof Q);
			qz &= ~bnz;
		}
		x ++;
	}
	memcpy(P, &Q, sizeof Q);
}

/*
 * Decode point into Jacobian coordinates. This function does not support
 * the point at infinity. If the point is invalid then this returns 0, but
 * the coordinates are still set to properly formed field elements.
 */
static uint32_t
point_decode(jacobian *P, const void *src, size_t len, const curve_params *cc)
{
	/*
	 * Points must use uncompressed format:
	 * -- first byte is 0x04;
	 * -- coordinates X and Y use unsigned big-endian, with the same
	 *    length as the field modulus.
	 *
	 * We don't support hybrid format (uncompressed, but first byte
	 * has value 0x06 or 0x07, depending on the least significant bit
	 * of Y) because it is rather useless, and explicitly forbidden
	 * by PKIX (RFC 5480, section 2.2).
	 *
	 * We don't support compressed format either, because it is not
	 * much used in practice (there are or were patent-related
	 * concerns about point compression, which explains the lack of
	 * generalised support). Also, point compression support would
	 * need a bit more code.
	 */
	const unsigned char *buf;
	size_t plen, zlen;
	uint32_t r;
	jacobian Q;

	buf = src;
	point_zero(P, cc);
	plen = (cc->p[0] - (cc->p[0] >> 5) + 7) >> 3;
	if (len != 1 + (plen << 1)) {
		return 0;
	}
	r = br_i31_decode_mod(P->c[0], buf + 1, plen, cc->p);
	r &= br_i31_decode_mod(P->c[1], buf + 1 + plen, plen, cc->p);

	/*
	 * Check first byte.
	 */
	r &= EQ(buf[0], 0x04);
	/* obsolete
	r &= EQ(buf[0], 0x04) | (EQ(buf[0] & 0xFE, 0x06)
		& ~(uint32_t)(buf[0] ^ buf[plen << 1]));
	*/

	/*
	 * Convert coordinates and check that the point is valid.
	 */
	zlen = ((cc->p[0] + 63) >> 5) * sizeof(uint32_t);
	memcpy(Q.c[0], cc->R2, zlen);
	memcpy(Q.c[1], cc->b, zlen);
	set_one(Q.c[2], cc->p);
	r &= ~run_code(P, &Q, cc, code_check);
	return r;
}

/*
 * Encode a point. This method assumes that the point is correct and is
 * not the point at infinity. Encoded size is always 1+2*plen, where
 * plen is the field modulus length, in bytes.
 */
static void
point_encode(void *dst, const jacobian *P, const curve_params *cc)
{
	unsigned char *buf;
	uint32_t xbl;
	size_t plen;
	jacobian Q, T;

	buf = dst;
	xbl = cc->p[0];
	xbl -= (xbl >> 5);
	plen = (xbl + 7) >> 3;
	buf[0] = 0x04;
	memcpy(&Q, P, sizeof *P);
	set_one(T.c[2], cc->p);
	run_code(&Q, &T, cc, code_affine);
	br_i31_encode(buf + 1, plen, Q.c[0]);
	br_i31_encode(buf + 1 + plen, plen, Q.c[1]);
}

static const br_ec_curve_def *
id_to_curve_def(int curve)
{
	switch (curve) {
	case BR_EC_secp256r1:
		return &br_secp256r1;
	case BR_EC_secp384r1:
		return &br_secp384r1;
	case BR_EC_secp521r1:
		return &br_secp521r1;
	}
	return NULL;
}

static const unsigned char *
api_generator(int curve, size_t *len)
{
	const br_ec_curve_def *cd;

	cd = id_to_curve_def(curve);
	*len = cd->generator_len;
	return cd->generator;
}

static const unsigned char *
api_order(int curve, size_t *len)
{
	const br_ec_curve_def *cd;

	cd = id_to_curve_def(curve);
	*len = cd->order_len;
	return cd->order;
}

static uint32_t
api_mul(unsigned char *G, size_t Glen,
	const unsigned char *x, size_t xlen, int curve)
{
	uint32_t r;
	const curve_params *cc;
	jacobian P;

	cc = id_to_curve(curve);
	r = point_decode(&P, G, Glen, cc);
	point_mul(&P, x, xlen, cc);
	point_encode(G, &P, cc);
	return r;
}

static size_t
api_mulgen(unsigned char *R,
	const unsigned char *x, size_t xlen, int curve)
{
	const unsigned char *G;
	size_t Glen;

	G = api_generator(curve, &Glen);
	memcpy(R, G, Glen);
	api_mul(R, Glen, x, xlen, curve);
	return Glen;
}

static uint32_t
api_muladd(unsigned char *A, const unsigned char *B, size_t len,
	const unsigned char *x, size_t xlen,
	const unsigned char *y, size_t ylen, int curve)
{
	uint32_t r, t, z;
	const curve_params *cc;
	jacobian P, Q;

	/*
	 * TODO: see about merging the two ladders. Right now, we do
	 * two independant point multiplications, which is a bit
	 * wasteful of CPU resources (but yields short code).
	 */

	cc = id_to_curve(curve);
	r = point_decode(&P, A, len, cc);
	if (B == NULL) {
		size_t Glen;

		B = api_generator(curve, &Glen);
	}
	r &= point_decode(&Q, B, len, cc);
	point_mul(&P, x, xlen, cc);
	point_mul(&Q, y, ylen, cc);

	/*
	 * We want to compute P+Q. Since the base points A and B are distinct
	 * from infinity, and the multipliers are non-zero and lower than the
	 * curve order, then we know that P and Q are non-infinity. This
	 * leaves two special situations to test for:
	 * -- If P = Q then we must use point_double().
	 * -- If P+Q = 0 then we must report an error.
	 */
	t = point_add(&P, &Q, cc);
	point_double(&Q, cc);
	z = br_i31_iszero(P.c[2]);

	/*
	 * If z is 1 then either P+Q = 0 (t = 1) or P = Q (t = 0). So we
	 * have the following:
	 *
	 *   z = 0, t = 0   return P (normal addition)
	 *   z = 0, t = 1   return P (normal addition)
	 *   z = 1, t = 0   return Q (a 'double' case)
	 *   z = 1, t = 1   report an error (P+Q = 0)
	 */
	CCOPY(z & ~t, &P, &Q, sizeof Q);
	point_encode(A, &P, cc);
	r &= ~(z & t);

	return r;
}

/* see bearssl_ec.h */
const br_ec_impl br_ec_prime_i31 = {
	(uint32_t)0x03800000,
	&api_generator,
	&api_order,
	&api_mul,
	&api_mulgen,
	&api_muladd
};


/* ===== src/ec/ec_prime_i31_secp256r1.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

static const uint32_t P256_P[] = {
	0x00000108,
	0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x00000007,
	0x00000000, 0x00000000, 0x00000040, 0x7FFFFF80,
	0x000000FF
};

static const uint32_t P256_B[] = {
	0x00000108,
	0x6FEE1803, 0x6229C4BD, 0x21B139BE, 0x327150AA,
	0x3567802E, 0x3F7212ED, 0x012E4355, 0x782DD38D,
	0x0000000E
};

/* see inner.h */
const br_ec_prime_i31_curve br_ec_prime_i31_secp256r1 = {
	P256_P,
	P256_B,
	0x00000001
};


/* ===== src/ec/ec_prime_i31_secp384r1.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

static const uint32_t P384_P[] = {
	0x0000018C,
	0x7FFFFFFF, 0x00000001, 0x00000000, 0x7FFFFFF8,
	0x7FFFFFEF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF,
	0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF,
	0x00000FFF
};

static const uint32_t P384_B[] = {
	0x0000018C,
	0x6E666840, 0x070D0392, 0x5D810231, 0x7651D50C,
	0x17E218D6, 0x1B192002, 0x44EFE441, 0x3A524E2B,
	0x2719BA5F, 0x41F02209, 0x36C5643E, 0x5813EFFE,
	0x000008A5
};

/* see inner.h */
const br_ec_prime_i31_curve br_ec_prime_i31_secp384r1 = {
	P384_P,
	P384_B,
	0x00000001
};


/* ===== src/ec/ec_prime_i31_secp521r1.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

static const uint32_t P521_P[] = {
	0x00000219,
	0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF,
	0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF,
	0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF,
	0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF,
	0x01FFFFFF
};

static const uint32_t P521_B[] = {
	0x00000219,
	0x540FC00A, 0x228FEA35, 0x2C34F1EF, 0x67BF107A,
	0x46FC1CD5, 0x1605E9DD, 0x6937B165, 0x272A3D8F,
	0x42785586, 0x44C8C778, 0x15F3B8B4, 0x64B73366,
	0x03BA8B69, 0x0D05B42A, 0x21F929A2, 0x2C31C393,
	0x00654FAE
};

/* see inner.h */
const br_ec_prime_i31_curve br_ec_prime_i31_secp521r1 = {
	P521_P,
	P521_B,
	0x00000001
};


/* ===== src/ec/ec_secp256r1.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

static const unsigned char P256_N[] = {
	0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xBC, 0xE6, 0xFA, 0xAD, 0xA7, 0x17, 0x9E, 0x84,
	0xF3, 0xB9, 0xCA, 0xC2, 0xFC, 0x63, 0x25, 0x51
};

static const unsigned char P256_G[] = {
	0x04, 0x6B, 0x17, 0xD1, 0xF2, 0xE1, 0x2C, 0x42,
	0x47, 0xF8, 0xBC, 0xE6, 0xE5, 0x63, 0xA4, 0x40,
	0xF2, 0x77, 0x03, 0x7D, 0x81, 0x2D, 0xEB, 0x33,
	0xA0, 0xF4, 0xA1, 0x39, 0x45, 0xD8, 0x98, 0xC2,
	0x96, 0x4F, 0xE3, 0x42, 0xE2, 0xFE, 0x1A, 0x7F,
	0x9B, 0x8E, 0xE7, 0xEB, 0x4A, 0x7C, 0x0F, 0x9E,
	0x16, 0x2B, 0xCE, 0x33, 0x57, 0x6B, 0x31, 0x5E,
	0xCE, 0xCB, 0xB6, 0x40, 0x68, 0x37, 0xBF, 0x51,
	0xF5
};

/* see inner.h */
const br_ec_curve_def br_secp256r1 = {
	BR_EC_secp256r1,
	P256_N, sizeof P256_N,
	P256_G, sizeof P256_G
};


/* ===== src/ec/ec_secp384r1.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

static const unsigned char P384_N[] = {
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
	0xC7, 0x63, 0x4D, 0x81, 0xF4, 0x37, 0x2D, 0xDF, 
	0x58, 0x1A, 0x0D, 0xB2, 0x48, 0xB0, 0xA7, 0x7A, 
	0xEC, 0xEC, 0x19, 0x6A, 0xCC, 0xC5, 0x29, 0x73
};

static const unsigned char P384_G[] = {
	0x04, 0xAA, 0x87, 0xCA, 0x22, 0xBE, 0x8B, 0x05,
	0x37, 0x8E, 0xB1, 0xC7, 0x1E, 0xF3, 0x20, 0xAD,
	0x74, 0x6E, 0x1D, 0x3B, 0x62, 0x8B, 0xA7, 0x9B,
	0x98, 0x59, 0xF7, 0x41, 0xE0, 0x82, 0x54, 0x2A,
	0x38, 0x55, 0x02, 0xF2, 0x5D, 0xBF, 0x55, 0x29,
	0x6C, 0x3A, 0x54, 0x5E, 0x38, 0x72, 0x76, 0x0A,
	0xB7, 0x36, 0x17, 0xDE, 0x4A, 0x96, 0x26, 0x2C,
	0x6F, 0x5D, 0x9E, 0x98, 0xBF, 0x92, 0x92, 0xDC,
	0x29, 0xF8, 0xF4, 0x1D, 0xBD, 0x28, 0x9A, 0x14,
	0x7C, 0xE9, 0xDA, 0x31, 0x13, 0xB5, 0xF0, 0xB8,
	0xC0, 0x0A, 0x60, 0xB1, 0xCE, 0x1D, 0x7E, 0x81,
	0x9D, 0x7A, 0x43, 0x1D, 0x7C, 0x90, 0xEA, 0x0E,
	0x5F
};

/* see inner.h */
const br_ec_curve_def br_secp384r1 = {
	BR_EC_secp384r1,
	P384_N, sizeof P384_N,
	P384_G, sizeof P384_G
};


/* ===== src/ec/ec_secp521r1.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

static const unsigned char P521_N[] = {
	0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFA, 0x51, 0x86, 0x87, 0x83, 0xBF, 0x2F,
	0x96, 0x6B, 0x7F, 0xCC, 0x01, 0x48, 0xF7, 0x09,
	0xA5, 0xD0, 0x3B, 0xB5, 0xC9, 0xB8, 0x89, 0x9C,
	0x47, 0xAE, 0xBB, 0x6F, 0xB7, 0x1E, 0x91, 0x38,
	0x64, 0x09
};

static const unsigned char P521_G[] = {
	0x04, 0x00, 0xC6, 0x85, 0x8E, 0x06, 0xB7, 0x04,
	0x04, 0xE9, 0xCD, 0x9E, 0x3E, 0xCB, 0x66, 0x23,
	0x95, 0xB4, 0x42, 0x9C, 0x64, 0x81, 0x39, 0x05,
	0x3F, 0xB5, 0x21, 0xF8, 0x28, 0xAF, 0x60, 0x6B,
	0x4D, 0x3D, 0xBA, 0xA1, 0x4B, 0x5E, 0x77, 0xEF,
	0xE7, 0x59, 0x28, 0xFE, 0x1D, 0xC1, 0x27, 0xA2,
	0xFF, 0xA8, 0xDE, 0x33, 0x48, 0xB3, 0xC1, 0x85,
	0x6A, 0x42, 0x9B, 0xF9, 0x7E, 0x7E, 0x31, 0xC2,
	0xE5, 0xBD, 0x66, 0x01, 0x18, 0x39, 0x29, 0x6A,
	0x78, 0x9A, 0x3B, 0xC0, 0x04, 0x5C, 0x8A, 0x5F,
	0xB4, 0x2C, 0x7D, 0x1B, 0xD9, 0x98, 0xF5, 0x44,
	0x49, 0x57, 0x9B, 0x44, 0x68, 0x17, 0xAF, 0xBD,
	0x17, 0x27, 0x3E, 0x66, 0x2C, 0x97, 0xEE, 0x72,
	0x99, 0x5E, 0xF4, 0x26, 0x40, 0xC5, 0x50, 0xB9,
	0x01, 0x3F, 0xAD, 0x07, 0x61, 0x35, 0x3C, 0x70,
	0x86, 0xA2, 0x72, 0xC2, 0x40, 0x88, 0xBE, 0x94,
	0x76, 0x9F, 0xD1, 0x66, 0x50
};

/* see inner.h */
const br_ec_curve_def br_secp521r1 = {
	BR_EC_secp521r1,
	P521_N, sizeof P521_N,
	P521_G, sizeof P521_G
};


/* ===== src/ec/ecdsa_i31_vrfy_asn1.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

#define FIELD_LEN   ((BR_MAX_EC_SIZE + 7) >> 3)

/* see bearssl_ec.h */
uint32_t
br_ecdsa_i31_vrfy_asn1(const br_ec_impl *impl,
	const void *hash, size_t hash_len,
	const br_ec_public_key *pk,
	const void *sig, size_t sig_len)
{
	/*
	 * We use a double-sized buffer because a malformed ASN.1 signature
	 * may trigger a size expansion when converting to "raw" format.
	 */
	unsigned char rsig[(FIELD_LEN << 2) + 24];

	if (sig_len > ((sizeof rsig) >> 1)) {
		return 0;
	}
	memcpy(rsig, sig, sig_len);
	sig_len = br_ecdsa_asn1_to_raw(rsig, sig_len);
	return br_ecdsa_i31_vrfy_raw(impl, hash, hash_len, pk, rsig, sig_len);
}


/* ===== src/ec/ecdsa_i31_vrfy_raw.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

#define I31_LEN     ((BR_MAX_EC_SIZE + 61) / 31)
#define POINT_LEN   (1 + (((BR_MAX_EC_SIZE + 7) >> 3) << 1))

/* see bearssl_ec.h */
uint32_t
br_ecdsa_i31_vrfy_raw(const br_ec_impl *impl,
	const void *hash, size_t hash_len,
	const br_ec_public_key *pk,
	const void *sig, size_t sig_len)
{
	/*
	 * IMPORTANT: this code is fit only for curves with a prime
	 * order. This is needed so that modular reduction of the X
	 * coordinate of a point can be done with a simple subtraction.
	 */
	const br_ec_curve_def *cd;
	uint32_t n[I31_LEN], r[I31_LEN], s[I31_LEN], t1[I31_LEN], t2[I31_LEN];
	unsigned char tx[(BR_MAX_EC_SIZE + 7) >> 3];
	unsigned char ty[(BR_MAX_EC_SIZE + 7) >> 3];
	unsigned char eU[POINT_LEN];
	size_t nlen, rlen, ulen;
	uint32_t n0i, res;

	/*
	 * If the curve is not supported, then report an error.
	 */
	if (((impl->supported_curves >> pk->curve) & 1) == 0) {
		return 0;
	}

	/*
	 * Get the curve parameters (generator and order).
	 */
	switch (pk->curve) {
	case BR_EC_secp256r1:
		cd = &br_secp256r1;
		break;
	case BR_EC_secp384r1:
		cd = &br_secp384r1;
		break;
	case BR_EC_secp521r1:
		cd = &br_secp521r1;
		break;
	default:
		return 0;
	}

	/*
	 * Signature length must be even.
	 */
	if (sig_len & 1) {
		return 0;
	}
	rlen = sig_len >> 1;

	/*
	 * Public key point must have the proper size for this curve.
	 */
	if (pk->qlen != cd->generator_len) {
		return 0;
	}

	/*
	 * Get modulus; then decode the r and s values. They must be
	 * lower than the modulus, and s must not be null.
	 */
	nlen = cd->order_len;
	br_i31_decode(n, cd->order, nlen);
	n0i = br_i31_ninv31(n[1]);
	if (!br_i31_decode_mod(r, sig, rlen, n)) {
		return 0;
	}
	if (!br_i31_decode_mod(s, (const unsigned char *)sig + rlen, rlen, n)) {
		return 0;
	}
	if (br_i31_iszero(s)) {
		return 0;
	}

	/*
	 * Invert s. We do that with a modular exponentiation; we use
	 * the fact that for all the curves we support, the least
	 * significant byte is not 0 or 1, so we can subtract 2 without
	 * any carry to process.
	 * We also want 1/s in Montgomery representation, which can be
	 * done by converting _from_ Montgomery representation before
	 * the inversion (because (1/s)*R = 1/(s/R)).
	 */
	br_i31_from_monty(s, n, n0i);
	memcpy(tx, cd->order, nlen);
	tx[nlen - 1] -= 2;
	br_i31_modpow(s, tx, nlen, n, n0i, t1, t2);

	/*
	 * Truncate the hash to the modulus length (in bits) and reduce
	 * it modulo the curve order. The modular reduction can be done
	 * with a subtraction since the truncation already reduced the
	 * value to the modulus bit length.
	 */
	br_ecdsa_i31_bits2int(t1, hash, hash_len, n[0]);
	br_i31_sub(t1, n, br_i31_sub(t1, n, 0) ^ 1);

	/*
	 * Multiply the (truncated, reduced) hash value with 1/s, result in
	 * t2, encoded in ty.
	 */
	br_i31_montymul(t2, t1, s, n, n0i);
	br_i31_encode(ty, nlen, t2);

	/*
	 * Multiply r with 1/s, result in t1, encoded in tx.
	 */
	br_i31_montymul(t1, r, s, n, n0i);
	br_i31_encode(tx, nlen, t1);

	/*
	 * Compute the point x*Q + y*G.
	 */
	ulen = cd->generator_len;
	memcpy(eU, pk->q, ulen);
	res = impl->muladd(eU, NULL, ulen,
		tx, nlen, ty, nlen, cd->curve);

	/*
	 * Get the X coordinate, reduce modulo the curve order, and
	 * compare with the 'r' value.
	 *
	 * The modular reduction can be done with subtractions because
	 * we work with curves of prime order, so the curve order is
	 * close to the field order (Hasse's theorem).
	 */
	br_i31_zero(t1, n[0]);
	br_i31_decode(t1, &eU[1], ulen >> 1);
	t1[0] = n[0];
	br_i31_sub(t1, n, br_i31_sub(t1, n, 0) ^ 1);
	res &= ~br_i31_sub(t1, r, 1);
	res &= br_i31_iszero(t1);
	return res;
}


/* ===== src/ec/ecdsa_i31_bits.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

/* see inner.h */
void
br_ecdsa_i31_bits2int(uint32_t *x,
	const void *src, size_t len, uint32_t ebitlen)
{
	uint32_t bitlen, hbitlen;
	int sc;

	bitlen = ebitlen - (ebitlen >> 5);
	hbitlen = (uint32_t)len << 3;
	if (hbitlen > bitlen) {
		len = (bitlen + 7) >> 3;
		sc = (int)((hbitlen - bitlen) & 7);
	} else {
		sc = 0;
	}
	br_i31_zero(x, ebitlen);
	br_i31_decode(x, src, len);
	br_i31_rshift(x, sc);
	x[0] = ebitlen;
}


/* ===== src/ec/ecdsa_atr.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

/* see bearssl_ec.h */
size_t
br_ecdsa_asn1_to_raw(void *sig, size_t sig_len)
{
	/*
	 * Note: this code is a bit lenient in that it accepts a few
	 * deviations to DER with regards to minimality of encoding of
	 * lengths and integer values. These deviations are still
	 * unambiguous.
	 *
	 * Signature format is a SEQUENCE of two INTEGER values. We
	 * support only integers of less than 127 bytes each (signed
	 * encoding) so the resulting raw signature will have length
	 * at most 254 bytes.
	 */

	unsigned char *buf, *r, *s;
	size_t zlen, rlen, slen, off;
	unsigned char tmp[254];

	buf = sig;
	if (sig_len < 8) {
		return 0;
	}

	/*
	 * First byte is SEQUENCE tag.
	 */
	if (buf[0] != 0x30) {
		return 0;
	}

	/*
	 * The SEQUENCE length will be encoded over one or two bytes. We
	 * limit the total SEQUENCE contents to 255 bytes, because it
	 * makes things simpler; this is enough for subgroup orders up
	 * to 999 bits.
	 */
	zlen = buf[1];
	if (zlen > 0x80) {
		if (zlen != 0x81) {
			return 0;
		}
		zlen = buf[2];
		if (zlen != sig_len - 3) {
			return 0;
		}
		off = 3;
	} else {
		if (zlen != sig_len - 2) {
			return 0;
		}
		off = 2;
	}

	/*
	 * First INTEGER (r).
	 */
	if (buf[off ++] != 0x02) {
		return 0;
	}
	rlen = buf[off ++];
	if (rlen >= 0x80) {
		return 0;
	}
	r = buf + off;
	off += rlen;

	/*
	 * Second INTEGER (s).
	 */
	if (off + 2 > sig_len) {
		return 0;
	}
	if (buf[off ++] != 0x02) {
		return 0;
	}
	slen = buf[off ++];
	if (slen >= 0x80 || slen != sig_len - off) {
		return 0;
	}
	s = buf + off;

	/*
	 * Removing leading zeros from r and s.
	 */
	while (rlen > 0 && *r == 0) {
		rlen --;
		r ++;
	}
	while (slen > 0 && *s == 0) {
		slen --;
		s ++;
	}

	/*
	 * Compute common length for the two integers, then copy integers
	 * into the temporary buffer, and finally copy it back over the
	 * signature buffer.
	 */
	zlen = rlen > slen ? rlen : slen;
	sig_len = zlen << 1;
	memset(tmp, 0, sig_len);
	memcpy(tmp + zlen - rlen, r, rlen);
	memcpy(tmp + sig_len - slen, s, slen);
	memcpy(sig, tmp, sig_len);
	return sig_len;
}


/* ===== src/ec/ecdsa_rta.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

/*
 * Compute ASN.1 encoded length for the provided integer. The ASN.1
 * encoding is signed, so its leading bit must have value 0; it must
 * also be of minimal length (so leading bytes of value 0 must be
 * removed, except if that would contradict the rule about the sign
 * bit).
 */
static size_t
asn1_int_length(const unsigned char *x, size_t xlen)
{
	while (xlen > 0 && *x == 0) {
		x ++;
		xlen --;
	}
	if (xlen == 0 || *x >= 0x80) {
		xlen ++;
	}
	return xlen;
}

/* see bearssl_ec.h */
size_t
br_ecdsa_raw_to_asn1(void *sig, size_t sig_len)
{
	/*
	 * Internal buffer is large enough to accommodate a signature
	 * such that r and s fit on 125 bytes each (signed encoding),
	 * meaning a curve order of up to 999 bits. This is the limit
	 * that ensures "simple" length encodings.
	 */
	unsigned char *buf;
	size_t hlen, rlen, slen, zlen, off;
	unsigned char tmp[257];

	buf = sig;
	if ((sig_len & 1) != 0) {
		return 0;
	}

	/*
	 * Compute lengths for the two integers.
	 */
	hlen = sig_len >> 1;
	rlen = asn1_int_length(buf, hlen);
	slen = asn1_int_length(buf + hlen, hlen);
	if (rlen > 125 || slen > 125) {
		return 0;
	}

	/*
	 * SEQUENCE header.
	 */
	tmp[0] = 0x30;
	zlen = rlen + slen + 4;
	if (zlen >= 0x80) {
		tmp[1] = 0x81;
		tmp[2] = zlen;
		off = 3;
	} else {
		tmp[1] = zlen;
		off = 2;
	}

	/*
	 * First INTEGER (r).
	 */
	tmp[off ++] = 0x02;
	tmp[off ++] = rlen;
	if (rlen > hlen) {
		tmp[off] = 0x00;
		memcpy(tmp + off + 1, buf, hlen);
	} else {
		memcpy(tmp + off, buf + hlen - rlen, rlen);
	}
	off += rlen;

	/*
	 * Second INTEGER (s).
	 */
	tmp[off ++] = 0x02;
	tmp[off ++] = slen;
	if (slen > hlen) {
		tmp[off] = 0x00;
		memcpy(tmp + off + 1, buf + hlen, hlen);
	} else {
		memcpy(tmp + off, buf + sig_len - slen, slen);
	}
	off += slen;

	/*
	 * Return ASN.1 signature.
	 */
	memcpy(sig, tmp, off);
	return off;
}


/* ===== src/ec/ec_prime_i15.c ===== */
/*
 * Copyright (c) 2017 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

/*
 * Parameters for supported curves:
 *   - field modulus p
 *   - R^2 mod p (R = 2^(15k) for the smallest k such that R >= p)
 *   - b*R mod p (b is the second curve equation parameter)
 */

static const uint16_t P256_P[] = {
	0x0111,
	0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x003F, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x1000, 0x0000, 0x4000, 0x7FFF,
	0x7FFF, 0x0001
};

static const uint16_t P256_R2[] = {
	0x0111,
	0x0000, 0x6000, 0x0000, 0x0000, 0x0000, 0x0000, 0x7FFC, 0x7FFF,
	0x7FBF, 0x7FFF, 0x7FBF, 0x7FFF, 0x7FFF, 0x7FFF, 0x77FF, 0x7FFF,
	0x4FFF, 0x0000
};

static const uint16_t P256_B[] = {
	0x0111,
	0x770C, 0x5EEF, 0x29C4, 0x3EC4, 0x6273, 0x0486, 0x4543, 0x3993,
	0x3C01, 0x6B56, 0x212E, 0x57EE, 0x4882, 0x204B, 0x7483, 0x3C16,
	0x0187, 0x0000
};

static const uint16_t P384_P[] = {
	0x0199,
	0x7FFF, 0x7FFF, 0x0003, 0x0000, 0x0000, 0x0000, 0x7FC0, 0x7FFF,
	0x7EFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF,
	0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF,
	0x7FFF, 0x01FF
};

static const uint16_t P384_R2[] = {
	0x0199,
	0x1000, 0x0000, 0x0000, 0x7FFF, 0x7FFF, 0x0001, 0x0000, 0x0010,
	0x0000, 0x0000, 0x0000, 0x7F00, 0x7FFF, 0x01FF, 0x0000, 0x1000,
	0x0000, 0x2000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000
};

static const uint16_t P384_B[] = {
	0x0199,
	0x7333, 0x2096, 0x70D1, 0x2310, 0x3020, 0x6197, 0x1464, 0x35BB,
	0x70CA, 0x0117, 0x1920, 0x4136, 0x5FC8, 0x5713, 0x4938, 0x7DD2,
	0x4DD2, 0x4A71, 0x0220, 0x683E, 0x2C87, 0x4DB1, 0x7BFF, 0x6C09,
	0x0452, 0x0084
};

static const uint16_t P521_P[] = {
	0x022B,
	0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF,
	0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF,
	0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF,
	0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF,
	0x7FFF, 0x7FFF, 0x07FF
};

static const uint16_t P521_R2[] = {
	0x022B,
	0x0100, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000
};

static const uint16_t P521_B[] = {
	0x022B,
	0x7002, 0x6A07, 0x751A, 0x228F, 0x71EF, 0x5869, 0x20F4, 0x1EFC,
	0x7357, 0x37E0, 0x4EEC, 0x605E, 0x1652, 0x26F6, 0x31FA, 0x4A8F,
	0x6193, 0x3C2A, 0x3C42, 0x48C7, 0x3489, 0x6771, 0x4C57, 0x5CCD,
	0x2725, 0x545B, 0x503B, 0x5B42, 0x21A0, 0x2534, 0x687E, 0x70E4,
	0x1618, 0x27D7, 0x0465
};

typedef struct {
	const uint16_t *p;
	const uint16_t *b;
	const uint16_t *R2;
	uint16_t p0i;
	size_t point_len;
} curve_params;

static inline const curve_params *
id_to_curve(int curve)
{
	static const curve_params pp[] = {
		{ P256_P, P256_B, P256_R2, 0x0001,  65 },
		{ P384_P, P384_B, P384_R2, 0x0001,  97 },
		{ P521_P, P521_B, P521_R2, 0x0001, 133 }
	};

	return &pp[curve - BR_EC_secp256r1];
}

#define I15_LEN   ((BR_MAX_EC_SIZE + 29) / 15)

/*
 * Type for a point in Jacobian coordinates:
 * -- three values, x, y and z, in Montgomery representation
 * -- affine coordinates are X = x / z^2 and Y = y / z^3
 * -- for the point at infinity, z = 0
 */
typedef struct {
	uint16_t c[3][I15_LEN];
} jacobian;

/*
 * We use a custom interpreter that uses a dozen registers, and
 * only six operations:
 *    MSET(d, a)       copy a into d
 *    MADD(d, a)       d = d+a (modular)
 *    MSUB(d, a)       d = d-a (modular)
 *    MMUL(d, a, b)    d = a*b (Montgomery multiplication)
 *    MINV(d, a, b)    invert d modulo p; a and b are used as scratch registers
 *    MTZ(d)           clear return value if d = 0
 * Destination of MMUL (d) must be distinct from operands (a and b).
 * There is no such constraint for MSUB and MADD.
 *
 * Registers include the operand coordinates, and temporaries.
 */
#define MSET(d, a)      (0x0000 + ((d) << 8) + ((a) << 4))
#define MADD(d, a)      (0x1000 + ((d) << 8) + ((a) << 4))
#define MSUB(d, a)      (0x2000 + ((d) << 8) + ((a) << 4))
#define MMUL(d, a, b)   (0x3000 + ((d) << 8) + ((a) << 4) + (b))
#define MINV(d, a, b)   (0x4000 + ((d) << 8) + ((a) << 4) + (b))
#define MTZ(d)          (0x5000 + ((d) << 8))
#define ENDCODE         0

/*
 * Registers for the input operands.
 */
#define P1x    0
#define P1y    1
#define P1z    2
#define P2x    3
#define P2y    4
#define P2z    5

/*
 * Alternate names for the first input operand.
 */
#define Px     0
#define Py     1
#define Pz     2

/*
 * Temporaries.
 */
#define t1     6
#define t2     7
#define t3     8
#define t4     9
#define t5    10
#define t6    11
#define t7    12

/*
 * Extra scratch registers available when there is no second operand (e.g.
 * for "double" and "affine").
 */
#define t8     3
#define t9     4
#define t10    5

/*
 * Doubling formulas are:
 *
 *   s = 4*x*y^2
 *   m = 3*(x + z^2)*(x - z^2)
 *   x' = m^2 - 2*s
 *   y' = m*(s - x') - 8*y^4
 *   z' = 2*y*z
 *
 * If y = 0 (P has order 2) then this yields infinity (z' = 0), as it
 * should. This case should not happen anyway, because our curves have
 * prime order, and thus do not contain any point of order 2.
 *
 * If P is infinity (z = 0), then again the formulas yield infinity,
 * which is correct. Thus, this code works for all points.
 *
 * Cost: 8 multiplications
 */
static const uint16_t code_double[] = {
	/*
	 * Compute z^2 (in t1).
	 */
	MMUL(t1, Pz, Pz),

	/*
	 * Compute x-z^2 (in t2) and then x+z^2 (in t1).
	 */
	MSET(t2, Px),
	MSUB(t2, t1),
	MADD(t1, Px),

	/*
	 * Compute m = 3*(x+z^2)*(x-z^2) (in t1).
	 */
	MMUL(t3, t1, t2),
	MSET(t1, t3),
	MADD(t1, t3),
	MADD(t1, t3),

	/*
	 * Compute s = 4*x*y^2 (in t2) and 2*y^2 (in t3).
	 */
	MMUL(t3, Py, Py),
	MADD(t3, t3),
	MMUL(t2, Px, t3),
	MADD(t2, t2),

	/*
	 * Compute x' = m^2 - 2*s.
	 */
	MMUL(Px, t1, t1),
	MSUB(Px, t2),
	MSUB(Px, t2),

	/*
	 * Compute z' = 2*y*z.
	 */
	MMUL(t4, Py, Pz),
	MSET(Pz, t4),
	MADD(Pz, t4),

	/*
	 * Compute y' = m*(s - x') - 8*y^4. Note that we already have
	 * 2*y^2 in t3.
	 */
	MSUB(t2, Px),
	MMUL(Py, t1, t2),
	MMUL(t4, t3, t3),
	MSUB(Py, t4),
	MSUB(Py, t4),

	ENDCODE
};

/*
 * Addtions formulas are:
 *
 *   u1 = x1 * z2^2
 *   u2 = x2 * z1^2
 *   s1 = y1 * z2^3
 *   s2 = y2 * z1^3
 *   h = u2 - u1
 *   r = s2 - s1
 *   x3 = r^2 - h^3 - 2 * u1 * h^2
 *   y3 = r * (u1 * h^2 - x3) - s1 * h^3
 *   z3 = h * z1 * z2
 *
 * If both P1 and P2 are infinity, then z1 == 0 and z2 == 0, implying that
 * z3 == 0, so the result is correct.
 * If either of P1 or P2 is infinity, but not both, then z3 == 0, which is
 * not correct.
 * h == 0 only if u1 == u2; this happens in two cases:
 * -- if s1 == s2 then P1 and/or P2 is infinity, or P1 == P2
 * -- if s1 != s2 then P1 + P2 == infinity (but neither P1 or P2 is infinity)
 *
 * Thus, the following situations are not handled correctly:
 * -- P1 = 0 and P2 != 0
 * -- P1 != 0 and P2 = 0
 * -- P1 = P2
 * All other cases are properly computed. However, even in "incorrect"
 * situations, the three coordinates still are properly formed field
 * elements.
 *
 * The returned flag is cleared if r == 0. This happens in the following
 * cases:
 * -- Both points are on the same horizontal line (same Y coordinate).
 * -- Both points are infinity.
 * -- One point is infinity and the other is on line Y = 0.
 * The third case cannot happen with our curves (there is no valid point
 * on line Y = 0 since that would be a point of order 2). If the two
 * source points are non-infinity, then remains only the case where the
 * two points are on the same horizontal line.
 *
 * This allows us to detect the "P1 == P2" case, assuming that P1 != 0 and
 * P2 != 0:
 * -- If the returned value is not the point at infinity, then it was properly
 * computed.
 * -- Otherwise, if the returned flag is 1, then P1+P2 = 0, and the result
 * is indeed the point at infinity.
 * -- Otherwise (result is infinity, flag is 0), then P1 = P2 and we should
 * use the 'double' code.
 *
 * Cost: 16 multiplications
 */
static const uint16_t code_add[] = {
	/*
	 * Compute u1 = x1*z2^2 (in t1) and s1 = y1*z2^3 (in t3).
	 */
	MMUL(t3, P2z, P2z),
	MMUL(t1, P1x, t3),
	MMUL(t4, P2z, t3),
	MMUL(t3, P1y, t4),

	/*
	 * Compute u2 = x2*z1^2 (in t2) and s2 = y2*z1^3 (in t4).
	 */
	MMUL(t4, P1z, P1z),
	MMUL(t2, P2x, t4),
	MMUL(t5, P1z, t4),
	MMUL(t4, P2y, t5),

	/*
	 * Compute h = u2 - u1 (in t2) and r = s2 - s1 (in t4).
	 */
	MSUB(t2, t1),
	MSUB(t4, t3),

	/*
	 * Report cases where r = 0 through the returned flag.
	 */
	MTZ(t4),

	/*
	 * Compute u1*h^2 (in t6) and h^3 (in t5).
	 */
	MMUL(t7, t2, t2),
	MMUL(t6, t1, t7),
	MMUL(t5, t7, t2),

	/*
	 * Compute x3 = r^2 - h^3 - 2*u1*h^2.
	 * t1 and t7 can be used as scratch registers.
	 */
	MMUL(P1x, t4, t4),
	MSUB(P1x, t5),
	MSUB(P1x, t6),
	MSUB(P1x, t6),

	/*
	 * Compute y3 = r*(u1*h^2 - x3) - s1*h^3.
	 */
	MSUB(t6, P1x),
	MMUL(P1y, t4, t6),
	MMUL(t1, t5, t3),
	MSUB(P1y, t1),

	/*
	 * Compute z3 = h*z1*z2.
	 */
	MMUL(t1, P1z, P2z),
	MMUL(P1z, t1, t2),

	ENDCODE
};

/*
 * Check that the point is on the curve. This code snippet assumes the
 * following conventions:
 * -- Coordinates x and y have been freshly decoded in P1 (but not
 * converted to Montgomery coordinates yet).
 * -- P2x, P2y and P2z are set to, respectively, R^2, b*R and 1.
 */
static const uint16_t code_check[] = {

	/* Convert x and y to Montgomery representation. */
	MMUL(t1, P1x, P2x),
	MMUL(t2, P1y, P2x),
	MSET(P1x, t1),
	MSET(P1y, t2),

	/* Compute x^3 in t1. */
	MMUL(t2, P1x, P1x),
	MMUL(t1, P1x, t2),

	/* Subtract 3*x from t1. */
	MSUB(t1, P1x),
	MSUB(t1, P1x),
	MSUB(t1, P1x),

	/* Add b. */
	MADD(t1, P2y),

	/* Compute y^2 in t2. */
	MMUL(t2, P1y, P1y),

	/* Compare y^2 with x^3 - 3*x + b; they must match. */
	MSUB(t1, t2),
	MTZ(t1),

	/* Set z to 1 (in Montgomery representation). */
	MMUL(P1z, P2x, P2z),

	ENDCODE
};

/*
 * Conversion back to affine coordinates. This code snippet assumes that
 * the z coordinate of P2 is set to 1 (not in Montgomery representation).
 */
static const uint16_t code_affine[] = {

	/* Save z*R in t1. */
	MSET(t1, P1z),

	/* Compute z^3 in t2. */
	MMUL(t2, P1z, P1z),
	MMUL(t3, P1z, t2),
	MMUL(t2, t3, P2z),

	/* Invert to (1/z^3) in t2. */
	MINV(t2, t3, t4),

	/* Compute y. */
	MSET(t3, P1y),
	MMUL(P1y, t2, t3),

	/* Compute (1/z^2) in t3. */
	MMUL(t3, t2, t1),

	/* Compute x. */
	MSET(t2, P1x),
	MMUL(P1x, t2, t3),

	ENDCODE
};

static uint32_t
run_code(jacobian *P1, const jacobian *P2,
	const curve_params *cc, const uint16_t *code)
{
	uint32_t r;
	uint16_t t[13][I15_LEN];
	size_t u;

	r = 1;

	/*
	 * Copy the two operands in the dedicated registers.
	 */
	memcpy(t[P1x], P1->c, 3 * I15_LEN * sizeof(uint16_t));
	memcpy(t[P2x], P2->c, 3 * I15_LEN * sizeof(uint16_t));

	/*
	 * Run formulas.
	 */
	for (u = 0;; u ++) {
		unsigned op, d, a, b;

		op = code[u];
		if (op == 0) {
			break;
		}
		d = (op >> 8) & 0x0F;
		a = (op >> 4) & 0x0F;
		b = op & 0x0F;
		op >>= 12;
		switch (op) {
			uint32_t ctl;
			size_t plen;
			unsigned char tp[(BR_MAX_EC_SIZE + 7) >> 3];

		case 0:
			memcpy(t[d], t[a], I15_LEN * sizeof(uint16_t));
			break;
		case 1:
			ctl = br_i15_add(t[d], t[a], 1);
			ctl |= NOT(br_i15_sub(t[d], cc->p, 0));
			br_i15_sub(t[d], cc->p, ctl);
			break;
		case 2:
			br_i15_add(t[d], cc->p, br_i15_sub(t[d], t[a], 1));
			break;
		case 3:
			br_i15_montymul(t[d], t[a], t[b], cc->p, cc->p0i);
			break;
		case 4:
			plen = (cc->p[0] - (cc->p[0] >> 4) + 7) >> 3;
			br_i15_encode(tp, plen, cc->p);
			tp[plen - 1] -= 2;
			br_i15_modpow(t[d], tp, plen,
				cc->p, cc->p0i, t[a], t[b]);
			break;
		default:
			r &= ~br_i15_iszero(t[d]);
			break;
		}
	}

	/*
	 * Copy back result.
	 */
	memcpy(P1->c, t[P1x], 3 * I15_LEN * sizeof(uint16_t));
	return r;
}

static void
set_one(uint16_t *x, const uint16_t *p)
{
	size_t plen;

	plen = (p[0] + 31) >> 4;
	memset(x, 0, plen * sizeof *x);
	x[0] = p[0];
	x[1] = 0x0001;
}

static void
point_zero(jacobian *P, const curve_params *cc)
{
	memset(P, 0, sizeof *P);
	P->c[0][0] = P->c[1][0] = P->c[2][0] = cc->p[0];
}

static inline void
point_double(jacobian *P, const curve_params *cc)
{
	run_code(P, P, cc, code_double);
}

static inline uint32_t
point_add(jacobian *P1, const jacobian *P2, const curve_params *cc)
{
	return run_code(P1, P2, cc, code_add);
}

static void
point_mul(jacobian *P, const unsigned char *x, size_t xlen,
	const curve_params *cc)
{
	/*
	 * We do a simple double-and-add ladder with a 2-bit window
	 * to make only one add every two doublings. We thus first
	 * precompute 2P and 3P in some local buffers.
	 *
	 * We always perform two doublings and one addition; the
	 * addition is with P, 2P and 3P and is done in a temporary
	 * array.
	 *
	 * The addition code cannot handle cases where one of the
	 * operands is infinity, which is the case at the start of the
	 * ladder. We therefore need to maintain a flag that controls
	 * this situation.
	 */
	uint32_t qz;
	jacobian P2, P3, Q, T, U;

	memcpy(&P2, P, sizeof P2);
	point_double(&P2, cc);
	memcpy(&P3, P, sizeof P3);
	point_add(&P3, &P2, cc);

	point_zero(&Q, cc);
	qz = 1;
	while (xlen -- > 0) {
		int k;

		for (k = 6; k >= 0; k -= 2) {
			uint32_t bits;
			uint32_t bnz;

			point_double(&Q, cc);
			point_double(&Q, cc);
			memcpy(&T, P, sizeof T);
			memcpy(&U, &Q, sizeof U);
			bits = (*x >> k) & (uint32_t)3;
			bnz = NEQ(bits, 0);
			CCOPY(EQ(bits, 2), &T, &P2, sizeof T);
			CCOPY(EQ(bits, 3), &T, &P3, sizeof T);
			point_add(&U, &T, cc);
			CCOPY(bnz & qz, &Q, &T, sizeof Q);
			CCOPY(bnz & ~qz, &Q, &U, sizeof Q);
			qz &= ~bnz;
		}
		x ++;
	}
	memcpy(P, &Q, sizeof Q);
}

/*
 * Decode point into Jacobian coordinates. This function does not support
 * the point at infinity. If the point is invalid then this returns 0, but
 * the coordinates are still set to properly formed field elements.
 */
static uint32_t
point_decode(jacobian *P, const void *src, size_t len, const curve_params *cc)
{
	/*
	 * Points must use uncompressed format:
	 * -- first byte is 0x04;
	 * -- coordinates X and Y use unsigned big-endian, with the same
	 *    length as the field modulus.
	 *
	 * We don't support hybrid format (uncompressed, but first byte
	 * has value 0x06 or 0x07, depending on the least significant bit
	 * of Y) because it is rather useless, and explicitly forbidden
	 * by PKIX (RFC 5480, section 2.2).
	 *
	 * We don't support compressed format either, because it is not
	 * much used in practice (there are or were patent-related
	 * concerns about point compression, which explains the lack of
	 * generalised support). Also, point compression support would
	 * need a bit more code.
	 */
	const unsigned char *buf;
	size_t plen, zlen;
	uint32_t r;
	jacobian Q;

	buf = src;
	point_zero(P, cc);
	plen = (cc->p[0] - (cc->p[0] >> 4) + 7) >> 3;
	if (len != 1 + (plen << 1)) {
		return 0;
	}
	r = br_i15_decode_mod(P->c[0], buf + 1, plen, cc->p);
	r &= br_i15_decode_mod(P->c[1], buf + 1 + plen, plen, cc->p);

	/*
	 * Check first byte.
	 */
	r &= EQ(buf[0], 0x04);
	/* obsolete
	r &= EQ(buf[0], 0x04) | (EQ(buf[0] & 0xFE, 0x06)
		& ~(uint32_t)(buf[0] ^ buf[plen << 1]));
	*/

	/*
	 * Convert coordinates and check that the point is valid.
	 */
	zlen = ((cc->p[0] + 31) >> 4) * sizeof(uint16_t);
	memcpy(Q.c[0], cc->R2, zlen);
	memcpy(Q.c[1], cc->b, zlen);
	set_one(Q.c[2], cc->p);
	r &= ~run_code(P, &Q, cc, code_check);
	return r;
}

/*
 * Encode a point. This method assumes that the point is correct and is
 * not the point at infinity. Encoded size is always 1+2*plen, where
 * plen is the field modulus length, in bytes.
 */
static void
point_encode(void *dst, const jacobian *P, const curve_params *cc)
{
	unsigned char *buf;
	size_t plen;
	jacobian Q, T;

	buf = dst;
	plen = (cc->p[0] - (cc->p[0] >> 4) + 7) >> 3;
	buf[0] = 0x04;
	memcpy(&Q, P, sizeof *P);
	set_one(T.c[2], cc->p);
	run_code(&Q, &T, cc, code_affine);
	br_i15_encode(buf + 1, plen, Q.c[0]);
	br_i15_encode(buf + 1 + plen, plen, Q.c[1]);
}

static const br_ec_curve_def *
id_to_curve_def(int curve)
{
	switch (curve) {
	case BR_EC_secp256r1:
		return &br_secp256r1;
	case BR_EC_secp384r1:
		return &br_secp384r1;
	case BR_EC_secp521r1:
		return &br_secp521r1;
	}
	return NULL;
}

static const unsigned char *
api_generator(int curve, size_t *len)
{
	const br_ec_curve_def *cd;

	cd = id_to_curve_def(curve);
	*len = cd->generator_len;
	return cd->generator;
}

static const unsigned char *
api_order(int curve, size_t *len)
{
	const br_ec_curve_def *cd;

	cd = id_to_curve_def(curve);
	*len = cd->order_len;
	return cd->order;
}

static uint32_t
api_mul(unsigned char *G, size_t Glen,
	const unsigned char *x, size_t xlen, int curve)
{
	uint32_t r;
	const curve_params *cc;
	jacobian P;

	cc = id_to_curve(curve);
	r = point_decode(&P, G, Glen, cc);
	point_mul(&P, x, xlen, cc);
	if (Glen == cc->point_len) {
		point_encode(G, &P, cc);
	}
	return r;
}

static size_t
api_mulgen(unsigned char *R,
	const unsigned char *x, size_t xlen, int curve)
{
	const unsigned char *G;
	size_t Glen;

	G = api_generator(curve, &Glen);
	memcpy(R, G, Glen);
	api_mul(R, Glen, x, xlen, curve);
	return Glen;
}

static uint32_t
api_muladd(unsigned char *A, const unsigned char *B, size_t len,
	const unsigned char *x, size_t xlen,
	const unsigned char *y, size_t ylen, int curve)
{
	uint32_t r, t, z;
	const curve_params *cc;
	jacobian P, Q;

	/*
	 * TODO: see about merging the two ladders. Right now, we do
	 * two independant point multiplications, which is a bit
	 * wasteful of CPU resources (but yields short code).
	 */

	cc = id_to_curve(curve);
	r = point_decode(&P, A, len, cc);
	if (B == NULL) {
		size_t Glen;

		B = api_generator(curve, &Glen);
	}
	r &= point_decode(&Q, B, len, cc);
	point_mul(&P, x, xlen, cc);
	point_mul(&Q, y, ylen, cc);

	/*
	 * We want to compute P+Q. Since the base points A and B are distinct
	 * from infinity, and the multipliers are non-zero and lower than the
	 * curve order, then we know that P and Q are non-infinity. This
	 * leaves two special situations to test for:
	 * -- If P = Q then we must use point_double().
	 * -- If P+Q = 0 then we must report an error.
	 */
	t = point_add(&P, &Q, cc);
	point_double(&Q, cc);
	z = br_i15_iszero(P.c[2]);

	/*
	 * If z is 1 then either P+Q = 0 (t = 1) or P = Q (t = 0). So we
	 * have the following:
	 *
	 *   z = 0, t = 0   return P (normal addition)
	 *   z = 0, t = 1   return P (normal addition)
	 *   z = 1, t = 0   return Q (a 'double' case)
	 *   z = 1, t = 1   report an error (P+Q = 0)
	 */
	CCOPY(z & ~t, &P, &Q, sizeof Q);
	point_encode(A, &P, cc);
	r &= ~(z & t);

	return r;
}

/* see bearssl_ec.h */
const br_ec_impl br_ec_prime_i15 = {
	(uint32_t)0x03800000,
	&api_generator,
	&api_order,
	&api_mul,
	&api_mulgen,
	&api_muladd
};


/* ===== src/ec/ec_p256_i15.c ===== */
/*
 * Copyright (c) 2017 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

/*
 * If BR_NO_ARITH_SHIFT is undefined, or defined to 0, then we _assume_
 * that right-shifting a signed negative integer copies the sign bit
 * (arithmetic right-shift). This is "implementation-defined behaviour",
 * i.e. it is not undefined, but it may differ between compilers. Each
 * compiler is supposed to document its behaviour in that respect. GCC
 * explicitly defines that an arithmetic right shift is used. We expect
 * all other compilers to do the same, because underlying CPU offer an
 * arithmetic right shift opcode that could not be used otherwise.
 */
#if BR_NO_ARITH_SHIFT
#define ARSH(x, n)   (((uint32_t)(x) >> (n)) \
                    | ((-((uint32_t)(x) >> 31)) << (32 - (n))))
#else
#define ARSH(x, n)   ((*(int32_t *)&(x)) >> (n))
#endif

/*
 * Convert an integer from unsigned big-endian encoding to a sequence of
 * 13-bit words in little-endian order. The final "partial" word is
 * returned.
 */
static uint32_t
be8_to_le13(uint32_t *dst, const unsigned char *src, size_t len)
{
	uint32_t acc;
	int acc_len;

	acc = 0;
	acc_len = 0;
	while (len -- > 0) {
		acc |= (uint32_t)src[len] << acc_len;
		acc_len += 8;
		if (acc_len >= 13) {
			*dst ++ = acc & 0x1FFF;
			acc >>= 13;
			acc_len -= 13;
		}
	}
	return acc;
}

/*
 * Convert an integer (13-bit words, little-endian) to unsigned
 * big-endian encoding. The total encoding length is provided; all
 * the destination bytes will be filled.
 */
static void
le13_to_be8(unsigned char *dst, size_t len, const uint32_t *src)
{
	uint32_t acc;
	int acc_len;

	acc = 0;
	acc_len = 0;
	while (len -- > 0) {
		if (acc_len < 8) {
			acc |= (*src ++) << acc_len;
			acc_len += 13;
		}
		dst[len] = (unsigned char)acc;
		acc >>= 8;
		acc_len -= 8;
	}
}

/*
 * Normalise an array of words to a strict 13 bits per word. Returned
 * value is the resulting carry. The source (w) and destination (d)
 * arrays may be identical, but shall not overlap partially.
 */
static inline uint32_t
norm13(uint32_t *d, const uint32_t *w, size_t len)
{
	size_t u;
	uint32_t cc;

	cc = 0;
	for (u = 0; u < len; u ++) {
		int32_t z;

		z = w[u] + cc;
		d[u] = z & 0x1FFF;
		cc = ARSH(z, 13);
	}
	return cc;
}

/*
 * mul20() multiplies two 260-bit integers together. Each word must fit
 * on 13 bits; source operands use 20 words, destination operand
 * receives 40 words. All overlaps allowed.
 *
 * square20() computes the square of a 260-bit integer. Each word must
 * fit on 13 bits; source operand uses 20 words, destination operand
 * receives 40 words. All overlaps allowed.
 */

#if BR_SLOW_MUL15

static void
mul20(uint32_t *d, const uint32_t *a, const uint32_t *b)
{
	/*
	 * Two-level Karatsuba: turns a 20x20 multiplication into
	 * nine 5x5 multiplications. We use 13-bit words but do not
	 * propagate carries immediately, so words may expand:
	 *
	 *  - First Karatsuba decomposition turns the 20x20 mul on
	 *    13-bit words into three 10x10 muls, two on 13-bit words
	 *    and one on 14-bit words.
	 *
	 *  - Second Karatsuba decomposition further splits these into:
	 *
	 *     * four 5x5 muls on 13-bit words
	 *     * four 5x5 muls on 14-bit words
	 *     * one 5x5 mul on 15-bit words
	 *
	 * Highest word value is 8191, 16382 or 32764, for 13-bit, 14-bit
	 * or 15-bit words, respectively.
	 */
	uint32_t u[45], v[45], w[90];
	uint32_t cc;
	int i;

#define ZADD(dw, d_off, s1w, s1_off, s2w, s2_off)   do { \
		(dw)[5 * (d_off) + 0] = (s1w)[5 * (s1_off) + 0] \
			+ (s2w)[5 * (s2_off) + 0]; \
		(dw)[5 * (d_off) + 1] = (s1w)[5 * (s1_off) + 1] \
			+ (s2w)[5 * (s2_off) + 1]; \
		(dw)[5 * (d_off) + 2] = (s1w)[5 * (s1_off) + 2] \
			+ (s2w)[5 * (s2_off) + 2]; \
		(dw)[5 * (d_off) + 3] = (s1w)[5 * (s1_off) + 3] \
			+ (s2w)[5 * (s2_off) + 3]; \
		(dw)[5 * (d_off) + 4] = (s1w)[5 * (s1_off) + 4] \
			+ (s2w)[5 * (s2_off) + 4]; \
	} while (0)

#define ZADDT(dw, d_off, sw, s_off)   do { \
		(dw)[5 * (d_off) + 0] += (sw)[5 * (s_off) + 0]; \
		(dw)[5 * (d_off) + 1] += (sw)[5 * (s_off) + 1]; \
		(dw)[5 * (d_off) + 2] += (sw)[5 * (s_off) + 2]; \
		(dw)[5 * (d_off) + 3] += (sw)[5 * (s_off) + 3]; \
		(dw)[5 * (d_off) + 4] += (sw)[5 * (s_off) + 4]; \
	} while (0)

#define ZSUB2F(dw, d_off, s1w, s1_off, s2w, s2_off)   do { \
		(dw)[5 * (d_off) + 0] -= (s1w)[5 * (s1_off) + 0] \
			+ (s2w)[5 * (s2_off) + 0]; \
		(dw)[5 * (d_off) + 1] -= (s1w)[5 * (s1_off) + 1] \
			+ (s2w)[5 * (s2_off) + 1]; \
		(dw)[5 * (d_off) + 2] -= (s1w)[5 * (s1_off) + 2] \
			+ (s2w)[5 * (s2_off) + 2]; \
		(dw)[5 * (d_off) + 3] -= (s1w)[5 * (s1_off) + 3] \
			+ (s2w)[5 * (s2_off) + 3]; \
		(dw)[5 * (d_off) + 4] -= (s1w)[5 * (s1_off) + 4] \
			+ (s2w)[5 * (s2_off) + 4]; \
	} while (0)

#define CPR1(w, cprcc)   do { \
		uint32_t cprz = (w) + cprcc; \
		(w) = cprz & 0x1FFF; \
		cprcc = cprz >> 13; \
	} while (0)

#define CPR(dw, d_off)   do { \
		uint32_t cprcc; \
		cprcc = 0; \
		CPR1((dw)[(d_off) + 0], cprcc); \
		CPR1((dw)[(d_off) + 1], cprcc); \
		CPR1((dw)[(d_off) + 2], cprcc); \
		CPR1((dw)[(d_off) + 3], cprcc); \
		CPR1((dw)[(d_off) + 4], cprcc); \
		CPR1((dw)[(d_off) + 5], cprcc); \
		CPR1((dw)[(d_off) + 6], cprcc); \
		CPR1((dw)[(d_off) + 7], cprcc); \
		CPR1((dw)[(d_off) + 8], cprcc); \
		(dw)[(d_off) + 9] = cprcc; \
	} while (0)

	memcpy(u, a, 20 * sizeof *a);
	ZADD(u, 4, a, 0, a, 1);
	ZADD(u, 5, a, 2, a, 3);
	ZADD(u, 6, a, 0, a, 2);
	ZADD(u, 7, a, 1, a, 3);
	ZADD(u, 8, u, 6, u, 7);

	memcpy(v, b, 20 * sizeof *b);
	ZADD(v, 4, b, 0, b, 1);
	ZADD(v, 5, b, 2, b, 3);
	ZADD(v, 6, b, 0, b, 2);
	ZADD(v, 7, b, 1, b, 3);
	ZADD(v, 8, v, 6, v, 7);

	/*
	 * Do the eight first 8x8 muls. Source words are at most 16382
	 * each, so we can add product results together "as is" in 32-bit
	 * words.
	 */
	for (i = 0; i < 40; i += 5) {
		w[(i << 1) + 0] = MUL15(u[i + 0], v[i + 0]);
		w[(i << 1) + 1] = MUL15(u[i + 0], v[i + 1])
			+ MUL15(u[i + 1], v[i + 0]);
		w[(i << 1) + 2] = MUL15(u[i + 0], v[i + 2])
			+ MUL15(u[i + 1], v[i + 1])
			+ MUL15(u[i + 2], v[i + 0]);
		w[(i << 1) + 3] = MUL15(u[i + 0], v[i + 3])
			+ MUL15(u[i + 1], v[i + 2])
			+ MUL15(u[i + 2], v[i + 1])
			+ MUL15(u[i + 3], v[i + 0]);
		w[(i << 1) + 4] = MUL15(u[i + 0], v[i + 4])
			+ MUL15(u[i + 1], v[i + 3])
			+ MUL15(u[i + 2], v[i + 2])
			+ MUL15(u[i + 3], v[i + 1])
			+ MUL15(u[i + 4], v[i + 0]);
		w[(i << 1) + 5] = MUL15(u[i + 1], v[i + 4])
			+ MUL15(u[i + 2], v[i + 3])
			+ MUL15(u[i + 3], v[i + 2])
			+ MUL15(u[i + 4], v[i + 1]);
		w[(i << 1) + 6] = MUL15(u[i + 2], v[i + 4])
			+ MUL15(u[i + 3], v[i + 3])
			+ MUL15(u[i + 4], v[i + 2]);
		w[(i << 1) + 7] = MUL15(u[i + 3], v[i + 4])
			+ MUL15(u[i + 4], v[i + 3]);
		w[(i << 1) + 8] = MUL15(u[i + 4], v[i + 4]);
		w[(i << 1) + 9] = 0;
	}

	/*
	 * For the 9th multiplication, source words are up to 32764,
	 * so we must do some carry propagation. If we add up to
	 * 4 products and the carry is no more than 524224, then the
	 * result fits in 32 bits, and the next carry will be no more
	 * than 524224 (because 4*(32764^2)+524224 < 8192*524225).
	 *
	 * We thus just skip one of the products in the middle word,
	 * then do a carry propagation (this reduces words to 13 bits
	 * each, except possibly the last, which may use up to 17 bits
	 * or so), then add the missing product.
	 */
	w[80 + 0] = MUL15(u[40 + 0], v[40 + 0]);
	w[80 + 1] = MUL15(u[40 + 0], v[40 + 1])
		+ MUL15(u[40 + 1], v[40 + 0]);
	w[80 + 2] = MUL15(u[40 + 0], v[40 + 2])
		+ MUL15(u[40 + 1], v[40 + 1])
		+ MUL15(u[40 + 2], v[40 + 0]);
	w[80 + 3] = MUL15(u[40 + 0], v[40 + 3])
		+ MUL15(u[40 + 1], v[40 + 2])
		+ MUL15(u[40 + 2], v[40 + 1])
		+ MUL15(u[40 + 3], v[40 + 0]);
	w[80 + 4] = MUL15(u[40 + 0], v[40 + 4])
		+ MUL15(u[40 + 1], v[40 + 3])
		+ MUL15(u[40 + 2], v[40 + 2])
		+ MUL15(u[40 + 3], v[40 + 1]);
		/* + MUL15(u[40 + 4], v[40 + 0]) */
	w[80 + 5] = MUL15(u[40 + 1], v[40 + 4])
		+ MUL15(u[40 + 2], v[40 + 3])
		+ MUL15(u[40 + 3], v[40 + 2])
		+ MUL15(u[40 + 4], v[40 + 1]);
	w[80 + 6] = MUL15(u[40 + 2], v[40 + 4])
		+ MUL15(u[40 + 3], v[40 + 3])
		+ MUL15(u[40 + 4], v[40 + 2]);
	w[80 + 7] = MUL15(u[40 + 3], v[40 + 4])
		+ MUL15(u[40 + 4], v[40 + 3]);
	w[80 + 8] = MUL15(u[40 + 4], v[40 + 4]);

	CPR(w, 80);

	w[80 + 4] += MUL15(u[40 + 4], v[40 + 0]);

	/*
	 * The products on 14-bit words in slots 6 and 7 yield values
	 * up to 5*(16382^2) each, and we need to subtract two such
	 * values from the higher word. We need the subtraction to fit
	 * in a _signed_ 32-bit integer, i.e. 31 bits + a sign bit.
	 * However, 10*(16382^2) does not fit. So we must perform a
	 * bit of reduction here.
	 */
	CPR(w, 60);
	CPR(w, 70);

	/*
	 * Recompose results.
	 */

	/* 0..1*0..1 into 0..3 */
	ZSUB2F(w, 8, w, 0, w, 2);
	ZSUB2F(w, 9, w, 1, w, 3);
	ZADDT(w, 1, w, 8);
	ZADDT(w, 2, w, 9);

	/* 2..3*2..3 into 4..7 */
	ZSUB2F(w, 10, w, 4, w, 6);
	ZSUB2F(w, 11, w, 5, w, 7);
	ZADDT(w, 5, w, 10);
	ZADDT(w, 6, w, 11);

	/* (0..1+2..3)*(0..1+2..3) into 12..15 */
	ZSUB2F(w, 16, w, 12, w, 14);
	ZSUB2F(w, 17, w, 13, w, 15);
	ZADDT(w, 13, w, 16);
	ZADDT(w, 14, w, 17);

	/* first-level recomposition */
	ZSUB2F(w, 12, w, 0, w, 4);
	ZSUB2F(w, 13, w, 1, w, 5);
	ZSUB2F(w, 14, w, 2, w, 6);
	ZSUB2F(w, 15, w, 3, w, 7);
	ZADDT(w, 2, w, 12);
	ZADDT(w, 3, w, 13);
	ZADDT(w, 4, w, 14);
	ZADDT(w, 5, w, 15);

	/*
	 * Perform carry propagation to bring all words down to 13 bits.
	 */
	cc = norm13(d, w, 40);
	d[39] += (cc << 13);

#undef ZADD
#undef ZADDT
#undef ZSUB2F
#undef CPR1
#undef CPR
}

static inline void
square20(uint32_t *d, const uint32_t *a)
{
	mul20(d, a, a);
}

#else

static void
mul20(uint32_t *d, const uint32_t *a, const uint32_t *b)
{
	uint32_t t[39];

	t[ 0] = MUL15(a[ 0], b[ 0]);
	t[ 1] = MUL15(a[ 0], b[ 1])
		+ MUL15(a[ 1], b[ 0]);
	t[ 2] = MUL15(a[ 0], b[ 2])
		+ MUL15(a[ 1], b[ 1])
		+ MUL15(a[ 2], b[ 0]);
	t[ 3] = MUL15(a[ 0], b[ 3])
		+ MUL15(a[ 1], b[ 2])
		+ MUL15(a[ 2], b[ 1])
		+ MUL15(a[ 3], b[ 0]);
	t[ 4] = MUL15(a[ 0], b[ 4])
		+ MUL15(a[ 1], b[ 3])
		+ MUL15(a[ 2], b[ 2])
		+ MUL15(a[ 3], b[ 1])
		+ MUL15(a[ 4], b[ 0]);
	t[ 5] = MUL15(a[ 0], b[ 5])
		+ MUL15(a[ 1], b[ 4])
		+ MUL15(a[ 2], b[ 3])
		+ MUL15(a[ 3], b[ 2])
		+ MUL15(a[ 4], b[ 1])
		+ MUL15(a[ 5], b[ 0]);
	t[ 6] = MUL15(a[ 0], b[ 6])
		+ MUL15(a[ 1], b[ 5])
		+ MUL15(a[ 2], b[ 4])
		+ MUL15(a[ 3], b[ 3])
		+ MUL15(a[ 4], b[ 2])
		+ MUL15(a[ 5], b[ 1])
		+ MUL15(a[ 6], b[ 0]);
	t[ 7] = MUL15(a[ 0], b[ 7])
		+ MUL15(a[ 1], b[ 6])
		+ MUL15(a[ 2], b[ 5])
		+ MUL15(a[ 3], b[ 4])
		+ MUL15(a[ 4], b[ 3])
		+ MUL15(a[ 5], b[ 2])
		+ MUL15(a[ 6], b[ 1])
		+ MUL15(a[ 7], b[ 0]);
	t[ 8] = MUL15(a[ 0], b[ 8])
		+ MUL15(a[ 1], b[ 7])
		+ MUL15(a[ 2], b[ 6])
		+ MUL15(a[ 3], b[ 5])
		+ MUL15(a[ 4], b[ 4])
		+ MUL15(a[ 5], b[ 3])
		+ MUL15(a[ 6], b[ 2])
		+ MUL15(a[ 7], b[ 1])
		+ MUL15(a[ 8], b[ 0]);
	t[ 9] = MUL15(a[ 0], b[ 9])
		+ MUL15(a[ 1], b[ 8])
		+ MUL15(a[ 2], b[ 7])
		+ MUL15(a[ 3], b[ 6])
		+ MUL15(a[ 4], b[ 5])
		+ MUL15(a[ 5], b[ 4])
		+ MUL15(a[ 6], b[ 3])
		+ MUL15(a[ 7], b[ 2])
		+ MUL15(a[ 8], b[ 1])
		+ MUL15(a[ 9], b[ 0]);
	t[10] = MUL15(a[ 0], b[10])
		+ MUL15(a[ 1], b[ 9])
		+ MUL15(a[ 2], b[ 8])
		+ MUL15(a[ 3], b[ 7])
		+ MUL15(a[ 4], b[ 6])
		+ MUL15(a[ 5], b[ 5])
		+ MUL15(a[ 6], b[ 4])
		+ MUL15(a[ 7], b[ 3])
		+ MUL15(a[ 8], b[ 2])
		+ MUL15(a[ 9], b[ 1])
		+ MUL15(a[10], b[ 0]);
	t[11] = MUL15(a[ 0], b[11])
		+ MUL15(a[ 1], b[10])
		+ MUL15(a[ 2], b[ 9])
		+ MUL15(a[ 3], b[ 8])
		+ MUL15(a[ 4], b[ 7])
		+ MUL15(a[ 5], b[ 6])
		+ MUL15(a[ 6], b[ 5])
		+ MUL15(a[ 7], b[ 4])
		+ MUL15(a[ 8], b[ 3])
		+ MUL15(a[ 9], b[ 2])
		+ MUL15(a[10], b[ 1])
		+ MUL15(a[11], b[ 0]);
	t[12] = MUL15(a[ 0], b[12])
		+ MUL15(a[ 1], b[11])
		+ MUL15(a[ 2], b[10])
		+ MUL15(a[ 3], b[ 9])
		+ MUL15(a[ 4], b[ 8])
		+ MUL15(a[ 5], b[ 7])
		+ MUL15(a[ 6], b[ 6])
		+ MUL15(a[ 7], b[ 5])
		+ MUL15(a[ 8], b[ 4])
		+ MUL15(a[ 9], b[ 3])
		+ MUL15(a[10], b[ 2])
		+ MUL15(a[11], b[ 1])
		+ MUL15(a[12], b[ 0]);
	t[13] = MUL15(a[ 0], b[13])
		+ MUL15(a[ 1], b[12])
		+ MUL15(a[ 2], b[11])
		+ MUL15(a[ 3], b[10])
		+ MUL15(a[ 4], b[ 9])
		+ MUL15(a[ 5], b[ 8])
		+ MUL15(a[ 6], b[ 7])
		+ MUL15(a[ 7], b[ 6])
		+ MUL15(a[ 8], b[ 5])
		+ MUL15(a[ 9], b[ 4])
		+ MUL15(a[10], b[ 3])
		+ MUL15(a[11], b[ 2])
		+ MUL15(a[12], b[ 1])
		+ MUL15(a[13], b[ 0]);
	t[14] = MUL15(a[ 0], b[14])
		+ MUL15(a[ 1], b[13])
		+ MUL15(a[ 2], b[12])
		+ MUL15(a[ 3], b[11])
		+ MUL15(a[ 4], b[10])
		+ MUL15(a[ 5], b[ 9])
		+ MUL15(a[ 6], b[ 8])
		+ MUL15(a[ 7], b[ 7])
		+ MUL15(a[ 8], b[ 6])
		+ MUL15(a[ 9], b[ 5])
		+ MUL15(a[10], b[ 4])
		+ MUL15(a[11], b[ 3])
		+ MUL15(a[12], b[ 2])
		+ MUL15(a[13], b[ 1])
		+ MUL15(a[14], b[ 0]);
	t[15] = MUL15(a[ 0], b[15])
		+ MUL15(a[ 1], b[14])
		+ MUL15(a[ 2], b[13])
		+ MUL15(a[ 3], b[12])
		+ MUL15(a[ 4], b[11])
		+ MUL15(a[ 5], b[10])
		+ MUL15(a[ 6], b[ 9])
		+ MUL15(a[ 7], b[ 8])
		+ MUL15(a[ 8], b[ 7])
		+ MUL15(a[ 9], b[ 6])
		+ MUL15(a[10], b[ 5])
		+ MUL15(a[11], b[ 4])
		+ MUL15(a[12], b[ 3])
		+ MUL15(a[13], b[ 2])
		+ MUL15(a[14], b[ 1])
		+ MUL15(a[15], b[ 0]);
	t[16] = MUL15(a[ 0], b[16])
		+ MUL15(a[ 1], b[15])
		+ MUL15(a[ 2], b[14])
		+ MUL15(a[ 3], b[13])
		+ MUL15(a[ 4], b[12])
		+ MUL15(a[ 5], b[11])
		+ MUL15(a[ 6], b[10])
		+ MUL15(a[ 7], b[ 9])
		+ MUL15(a[ 8], b[ 8])
		+ MUL15(a[ 9], b[ 7])
		+ MUL15(a[10], b[ 6])
		+ MUL15(a[11], b[ 5])
		+ MUL15(a[12], b[ 4])
		+ MUL15(a[13], b[ 3])
		+ MUL15(a[14], b[ 2])
		+ MUL15(a[15], b[ 1])
		+ MUL15(a[16], b[ 0]);
	t[17] = MUL15(a[ 0], b[17])
		+ MUL15(a[ 1], b[16])
		+ MUL15(a[ 2], b[15])
		+ MUL15(a[ 3], b[14])
		+ MUL15(a[ 4], b[13])
		+ MUL15(a[ 5], b[12])
		+ MUL15(a[ 6], b[11])
		+ MUL15(a[ 7], b[10])
		+ MUL15(a[ 8], b[ 9])
		+ MUL15(a[ 9], b[ 8])
		+ MUL15(a[10], b[ 7])
		+ MUL15(a[11], b[ 6])
		+ MUL15(a[12], b[ 5])
		+ MUL15(a[13], b[ 4])
		+ MUL15(a[14], b[ 3])
		+ MUL15(a[15], b[ 2])
		+ MUL15(a[16], b[ 1])
		+ MUL15(a[17], b[ 0]);
	t[18] = MUL15(a[ 0], b[18])
		+ MUL15(a[ 1], b[17])
		+ MUL15(a[ 2], b[16])
		+ MUL15(a[ 3], b[15])
		+ MUL15(a[ 4], b[14])
		+ MUL15(a[ 5], b[13])
		+ MUL15(a[ 6], b[12])
		+ MUL15(a[ 7], b[11])
		+ MUL15(a[ 8], b[10])
		+ MUL15(a[ 9], b[ 9])
		+ MUL15(a[10], b[ 8])
		+ MUL15(a[11], b[ 7])
		+ MUL15(a[12], b[ 6])
		+ MUL15(a[13], b[ 5])
		+ MUL15(a[14], b[ 4])
		+ MUL15(a[15], b[ 3])
		+ MUL15(a[16], b[ 2])
		+ MUL15(a[17], b[ 1])
		+ MUL15(a[18], b[ 0]);
	t[19] = MUL15(a[ 0], b[19])
		+ MUL15(a[ 1], b[18])
		+ MUL15(a[ 2], b[17])
		+ MUL15(a[ 3], b[16])
		+ MUL15(a[ 4], b[15])
		+ MUL15(a[ 5], b[14])
		+ MUL15(a[ 6], b[13])
		+ MUL15(a[ 7], b[12])
		+ MUL15(a[ 8], b[11])
		+ MUL15(a[ 9], b[10])
		+ MUL15(a[10], b[ 9])
		+ MUL15(a[11], b[ 8])
		+ MUL15(a[12], b[ 7])
		+ MUL15(a[13], b[ 6])
		+ MUL15(a[14], b[ 5])
		+ MUL15(a[15], b[ 4])
		+ MUL15(a[16], b[ 3])
		+ MUL15(a[17], b[ 2])
		+ MUL15(a[18], b[ 1])
		+ MUL15(a[19], b[ 0]);
	t[20] = MUL15(a[ 1], b[19])
		+ MUL15(a[ 2], b[18])
		+ MUL15(a[ 3], b[17])
		+ MUL15(a[ 4], b[16])
		+ MUL15(a[ 5], b[15])
		+ MUL15(a[ 6], b[14])
		+ MUL15(a[ 7], b[13])
		+ MUL15(a[ 8], b[12])
		+ MUL15(a[ 9], b[11])
		+ MUL15(a[10], b[10])
		+ MUL15(a[11], b[ 9])
		+ MUL15(a[12], b[ 8])
		+ MUL15(a[13], b[ 7])
		+ MUL15(a[14], b[ 6])
		+ MUL15(a[15], b[ 5])
		+ MUL15(a[16], b[ 4])
		+ MUL15(a[17], b[ 3])
		+ MUL15(a[18], b[ 2])
		+ MUL15(a[19], b[ 1]);
	t[21] = MUL15(a[ 2], b[19])
		+ MUL15(a[ 3], b[18])
		+ MUL15(a[ 4], b[17])
		+ MUL15(a[ 5], b[16])
		+ MUL15(a[ 6], b[15])
		+ MUL15(a[ 7], b[14])
		+ MUL15(a[ 8], b[13])
		+ MUL15(a[ 9], b[12])
		+ MUL15(a[10], b[11])
		+ MUL15(a[11], b[10])
		+ MUL15(a[12], b[ 9])
		+ MUL15(a[13], b[ 8])
		+ MUL15(a[14], b[ 7])
		+ MUL15(a[15], b[ 6])
		+ MUL15(a[16], b[ 5])
		+ MUL15(a[17], b[ 4])
		+ MUL15(a[18], b[ 3])
		+ MUL15(a[19], b[ 2]);
	t[22] = MUL15(a[ 3], b[19])
		+ MUL15(a[ 4], b[18])
		+ MUL15(a[ 5], b[17])
		+ MUL15(a[ 6], b[16])
		+ MUL15(a[ 7], b[15])
		+ MUL15(a[ 8], b[14])
		+ MUL15(a[ 9], b[13])
		+ MUL15(a[10], b[12])
		+ MUL15(a[11], b[11])
		+ MUL15(a[12], b[10])
		+ MUL15(a[13], b[ 9])
		+ MUL15(a[14], b[ 8])
		+ MUL15(a[15], b[ 7])
		+ MUL15(a[16], b[ 6])
		+ MUL15(a[17], b[ 5])
		+ MUL15(a[18], b[ 4])
		+ MUL15(a[19], b[ 3]);
	t[23] = MUL15(a[ 4], b[19])
		+ MUL15(a[ 5], b[18])
		+ MUL15(a[ 6], b[17])
		+ MUL15(a[ 7], b[16])
		+ MUL15(a[ 8], b[15])
		+ MUL15(a[ 9], b[14])
		+ MUL15(a[10], b[13])
		+ MUL15(a[11], b[12])
		+ MUL15(a[12], b[11])
		+ MUL15(a[13], b[10])
		+ MUL15(a[14], b[ 9])
		+ MUL15(a[15], b[ 8])
		+ MUL15(a[16], b[ 7])
		+ MUL15(a[17], b[ 6])
		+ MUL15(a[18], b[ 5])
		+ MUL15(a[19], b[ 4]);
	t[24] = MUL15(a[ 5], b[19])
		+ MUL15(a[ 6], b[18])
		+ MUL15(a[ 7], b[17])
		+ MUL15(a[ 8], b[16])
		+ MUL15(a[ 9], b[15])
		+ MUL15(a[10], b[14])
		+ MUL15(a[11], b[13])
		+ MUL15(a[12], b[12])
		+ MUL15(a[13], b[11])
		+ MUL15(a[14], b[10])
		+ MUL15(a[15], b[ 9])
		+ MUL15(a[16], b[ 8])
		+ MUL15(a[17], b[ 7])
		+ MUL15(a[18], b[ 6])
		+ MUL15(a[19], b[ 5]);
	t[25] = MUL15(a[ 6], b[19])
		+ MUL15(a[ 7], b[18])
		+ MUL15(a[ 8], b[17])
		+ MUL15(a[ 9], b[16])
		+ MUL15(a[10], b[15])
		+ MUL15(a[11], b[14])
		+ MUL15(a[12], b[13])
		+ MUL15(a[13], b[12])
		+ MUL15(a[14], b[11])
		+ MUL15(a[15], b[10])
		+ MUL15(a[16], b[ 9])
		+ MUL15(a[17], b[ 8])
		+ MUL15(a[18], b[ 7])
		+ MUL15(a[19], b[ 6]);
	t[26] = MUL15(a[ 7], b[19])
		+ MUL15(a[ 8], b[18])
		+ MUL15(a[ 9], b[17])
		+ MUL15(a[10], b[16])
		+ MUL15(a[11], b[15])
		+ MUL15(a[12], b[14])
		+ MUL15(a[13], b[13])
		+ MUL15(a[14], b[12])
		+ MUL15(a[15], b[11])
		+ MUL15(a[16], b[10])
		+ MUL15(a[17], b[ 9])
		+ MUL15(a[18], b[ 8])
		+ MUL15(a[19], b[ 7]);
	t[27] = MUL15(a[ 8], b[19])
		+ MUL15(a[ 9], b[18])
		+ MUL15(a[10], b[17])
		+ MUL15(a[11], b[16])
		+ MUL15(a[12], b[15])
		+ MUL15(a[13], b[14])
		+ MUL15(a[14], b[13])
		+ MUL15(a[15], b[12])
		+ MUL15(a[16], b[11])
		+ MUL15(a[17], b[10])
		+ MUL15(a[18], b[ 9])
		+ MUL15(a[19], b[ 8]);
	t[28] = MUL15(a[ 9], b[19])
		+ MUL15(a[10], b[18])
		+ MUL15(a[11], b[17])
		+ MUL15(a[12], b[16])
		+ MUL15(a[13], b[15])
		+ MUL15(a[14], b[14])
		+ MUL15(a[15], b[13])
		+ MUL15(a[16], b[12])
		+ MUL15(a[17], b[11])
		+ MUL15(a[18], b[10])
		+ MUL15(a[19], b[ 9]);
	t[29] = MUL15(a[10], b[19])
		+ MUL15(a[11], b[18])
		+ MUL15(a[12], b[17])
		+ MUL15(a[13], b[16])
		+ MUL15(a[14], b[15])
		+ MUL15(a[15], b[14])
		+ MUL15(a[16], b[13])
		+ MUL15(a[17], b[12])
		+ MUL15(a[18], b[11])
		+ MUL15(a[19], b[10]);
	t[30] = MUL15(a[11], b[19])
		+ MUL15(a[12], b[18])
		+ MUL15(a[13], b[17])
		+ MUL15(a[14], b[16])
		+ MUL15(a[15], b[15])
		+ MUL15(a[16], b[14])
		+ MUL15(a[17], b[13])
		+ MUL15(a[18], b[12])
		+ MUL15(a[19], b[11]);
	t[31] = MUL15(a[12], b[19])
		+ MUL15(a[13], b[18])
		+ MUL15(a[14], b[17])
		+ MUL15(a[15], b[16])
		+ MUL15(a[16], b[15])
		+ MUL15(a[17], b[14])
		+ MUL15(a[18], b[13])
		+ MUL15(a[19], b[12]);
	t[32] = MUL15(a[13], b[19])
		+ MUL15(a[14], b[18])
		+ MUL15(a[15], b[17])
		+ MUL15(a[16], b[16])
		+ MUL15(a[17], b[15])
		+ MUL15(a[18], b[14])
		+ MUL15(a[19], b[13]);
	t[33] = MUL15(a[14], b[19])
		+ MUL15(a[15], b[18])
		+ MUL15(a[16], b[17])
		+ MUL15(a[17], b[16])
		+ MUL15(a[18], b[15])
		+ MUL15(a[19], b[14]);
	t[34] = MUL15(a[15], b[19])
		+ MUL15(a[16], b[18])
		+ MUL15(a[17], b[17])
		+ MUL15(a[18], b[16])
		+ MUL15(a[19], b[15]);
	t[35] = MUL15(a[16], b[19])
		+ MUL15(a[17], b[18])
		+ MUL15(a[18], b[17])
		+ MUL15(a[19], b[16]);
	t[36] = MUL15(a[17], b[19])
		+ MUL15(a[18], b[18])
		+ MUL15(a[19], b[17]);
	t[37] = MUL15(a[18], b[19])
		+ MUL15(a[19], b[18]);
	t[38] = MUL15(a[19], b[19]);
	d[39] = norm13(d, t, 39);
}

static void
square20(uint32_t *d, const uint32_t *a)
{
	uint32_t t[39];

	t[ 0] = MUL15(a[ 0], a[ 0]);
	t[ 1] = ((MUL15(a[ 0], a[ 1])) << 1);
	t[ 2] = MUL15(a[ 1], a[ 1])
		+ ((MUL15(a[ 0], a[ 2])) << 1);
	t[ 3] = ((MUL15(a[ 0], a[ 3])
		+ MUL15(a[ 1], a[ 2])) << 1);
	t[ 4] = MUL15(a[ 2], a[ 2])
		+ ((MUL15(a[ 0], a[ 4])
		+ MUL15(a[ 1], a[ 3])) << 1);
	t[ 5] = ((MUL15(a[ 0], a[ 5])
		+ MUL15(a[ 1], a[ 4])
		+ MUL15(a[ 2], a[ 3])) << 1);
	t[ 6] = MUL15(a[ 3], a[ 3])
		+ ((MUL15(a[ 0], a[ 6])
		+ MUL15(a[ 1], a[ 5])
		+ MUL15(a[ 2], a[ 4])) << 1);
	t[ 7] = ((MUL15(a[ 0], a[ 7])
		+ MUL15(a[ 1], a[ 6])
		+ MUL15(a[ 2], a[ 5])
		+ MUL15(a[ 3], a[ 4])) << 1);
	t[ 8] = MUL15(a[ 4], a[ 4])
		+ ((MUL15(a[ 0], a[ 8])
		+ MUL15(a[ 1], a[ 7])
		+ MUL15(a[ 2], a[ 6])
		+ MUL15(a[ 3], a[ 5])) << 1);
	t[ 9] = ((MUL15(a[ 0], a[ 9])
		+ MUL15(a[ 1], a[ 8])
		+ MUL15(a[ 2], a[ 7])
		+ MUL15(a[ 3], a[ 6])
		+ MUL15(a[ 4], a[ 5])) << 1);
	t[10] = MUL15(a[ 5], a[ 5])
		+ ((MUL15(a[ 0], a[10])
		+ MUL15(a[ 1], a[ 9])
		+ MUL15(a[ 2], a[ 8])
		+ MUL15(a[ 3], a[ 7])
		+ MUL15(a[ 4], a[ 6])) << 1);
	t[11] = ((MUL15(a[ 0], a[11])
		+ MUL15(a[ 1], a[10])
		+ MUL15(a[ 2], a[ 9])
		+ MUL15(a[ 3], a[ 8])
		+ MUL15(a[ 4], a[ 7])
		+ MUL15(a[ 5], a[ 6])) << 1);
	t[12] = MUL15(a[ 6], a[ 6])
		+ ((MUL15(a[ 0], a[12])
		+ MUL15(a[ 1], a[11])
		+ MUL15(a[ 2], a[10])
		+ MUL15(a[ 3], a[ 9])
		+ MUL15(a[ 4], a[ 8])
		+ MUL15(a[ 5], a[ 7])) << 1);
	t[13] = ((MUL15(a[ 0], a[13])
		+ MUL15(a[ 1], a[12])
		+ MUL15(a[ 2], a[11])
		+ MUL15(a[ 3], a[10])
		+ MUL15(a[ 4], a[ 9])
		+ MUL15(a[ 5], a[ 8])
		+ MUL15(a[ 6], a[ 7])) << 1);
	t[14] = MUL15(a[ 7], a[ 7])
		+ ((MUL15(a[ 0], a[14])
		+ MUL15(a[ 1], a[13])
		+ MUL15(a[ 2], a[12])
		+ MUL15(a[ 3], a[11])
		+ MUL15(a[ 4], a[10])
		+ MUL15(a[ 5], a[ 9])
		+ MUL15(a[ 6], a[ 8])) << 1);
	t[15] = ((MUL15(a[ 0], a[15])
		+ MUL15(a[ 1], a[14])
		+ MUL15(a[ 2], a[13])
		+ MUL15(a[ 3], a[12])
		+ MUL15(a[ 4], a[11])
		+ MUL15(a[ 5], a[10])
		+ MUL15(a[ 6], a[ 9])
		+ MUL15(a[ 7], a[ 8])) << 1);
	t[16] = MUL15(a[ 8], a[ 8])
		+ ((MUL15(a[ 0], a[16])
		+ MUL15(a[ 1], a[15])
		+ MUL15(a[ 2], a[14])
		+ MUL15(a[ 3], a[13])
		+ MUL15(a[ 4], a[12])
		+ MUL15(a[ 5], a[11])
		+ MUL15(a[ 6], a[10])
		+ MUL15(a[ 7], a[ 9])) << 1);
	t[17] = ((MUL15(a[ 0], a[17])
		+ MUL15(a[ 1], a[16])
		+ MUL15(a[ 2], a[15])
		+ MUL15(a[ 3], a[14])
		+ MUL15(a[ 4], a[13])
		+ MUL15(a[ 5], a[12])
		+ MUL15(a[ 6], a[11])
		+ MUL15(a[ 7], a[10])
		+ MUL15(a[ 8], a[ 9])) << 1);
	t[18] = MUL15(a[ 9], a[ 9])
		+ ((MUL15(a[ 0], a[18])
		+ MUL15(a[ 1], a[17])
		+ MUL15(a[ 2], a[16])
		+ MUL15(a[ 3], a[15])
		+ MUL15(a[ 4], a[14])
		+ MUL15(a[ 5], a[13])
		+ MUL15(a[ 6], a[12])
		+ MUL15(a[ 7], a[11])
		+ MUL15(a[ 8], a[10])) << 1);
	t[19] = ((MUL15(a[ 0], a[19])
		+ MUL15(a[ 1], a[18])
		+ MUL15(a[ 2], a[17])
		+ MUL15(a[ 3], a[16])
		+ MUL15(a[ 4], a[15])
		+ MUL15(a[ 5], a[14])
		+ MUL15(a[ 6], a[13])
		+ MUL15(a[ 7], a[12])
		+ MUL15(a[ 8], a[11])
		+ MUL15(a[ 9], a[10])) << 1);
	t[20] = MUL15(a[10], a[10])
		+ ((MUL15(a[ 1], a[19])
		+ MUL15(a[ 2], a[18])
		+ MUL15(a[ 3], a[17])
		+ MUL15(a[ 4], a[16])
		+ MUL15(a[ 5], a[15])
		+ MUL15(a[ 6], a[14])
		+ MUL15(a[ 7], a[13])
		+ MUL15(a[ 8], a[12])
		+ MUL15(a[ 9], a[11])) << 1);
	t[21] = ((MUL15(a[ 2], a[19])
		+ MUL15(a[ 3], a[18])
		+ MUL15(a[ 4], a[17])
		+ MUL15(a[ 5], a[16])
		+ MUL15(a[ 6], a[15])
		+ MUL15(a[ 7], a[14])
		+ MUL15(a[ 8], a[13])
		+ MUL15(a[ 9], a[12])
		+ MUL15(a[10], a[11])) << 1);
	t[22] = MUL15(a[11], a[11])
		+ ((MUL15(a[ 3], a[19])
		+ MUL15(a[ 4], a[18])
		+ MUL15(a[ 5], a[17])
		+ MUL15(a[ 6], a[16])
		+ MUL15(a[ 7], a[15])
		+ MUL15(a[ 8], a[14])
		+ MUL15(a[ 9], a[13])
		+ MUL15(a[10], a[12])) << 1);
	t[23] = ((MUL15(a[ 4], a[19])
		+ MUL15(a[ 5], a[18])
		+ MUL15(a[ 6], a[17])
		+ MUL15(a[ 7], a[16])
		+ MUL15(a[ 8], a[15])
		+ MUL15(a[ 9], a[14])
		+ MUL15(a[10], a[13])
		+ MUL15(a[11], a[12])) << 1);
	t[24] = MUL15(a[12], a[12])
		+ ((MUL15(a[ 5], a[19])
		+ MUL15(a[ 6], a[18])
		+ MUL15(a[ 7], a[17])
		+ MUL15(a[ 8], a[16])
		+ MUL15(a[ 9], a[15])
		+ MUL15(a[10], a[14])
		+ MUL15(a[11], a[13])) << 1);
	t[25] = ((MUL15(a[ 6], a[19])
		+ MUL15(a[ 7], a[18])
		+ MUL15(a[ 8], a[17])
		+ MUL15(a[ 9], a[16])
		+ MUL15(a[10], a[15])
		+ MUL15(a[11], a[14])
		+ MUL15(a[12], a[13])) << 1);
	t[26] = MUL15(a[13], a[13])
		+ ((MUL15(a[ 7], a[19])
		+ MUL15(a[ 8], a[18])
		+ MUL15(a[ 9], a[17])
		+ MUL15(a[10], a[16])
		+ MUL15(a[11], a[15])
		+ MUL15(a[12], a[14])) << 1);
	t[27] = ((MUL15(a[ 8], a[19])
		+ MUL15(a[ 9], a[18])
		+ MUL15(a[10], a[17])
		+ MUL15(a[11], a[16])
		+ MUL15(a[12], a[15])
		+ MUL15(a[13], a[14])) << 1);
	t[28] = MUL15(a[14], a[14])
		+ ((MUL15(a[ 9], a[19])
		+ MUL15(a[10], a[18])
		+ MUL15(a[11], a[17])
		+ MUL15(a[12], a[16])
		+ MUL15(a[13], a[15])) << 1);
	t[29] = ((MUL15(a[10], a[19])
		+ MUL15(a[11], a[18])
		+ MUL15(a[12], a[17])
		+ MUL15(a[13], a[16])
		+ MUL15(a[14], a[15])) << 1);
	t[30] = MUL15(a[15], a[15])
		+ ((MUL15(a[11], a[19])
		+ MUL15(a[12], a[18])
		+ MUL15(a[13], a[17])
		+ MUL15(a[14], a[16])) << 1);
	t[31] = ((MUL15(a[12], a[19])
		+ MUL15(a[13], a[18])
		+ MUL15(a[14], a[17])
		+ MUL15(a[15], a[16])) << 1);
	t[32] = MUL15(a[16], a[16])
		+ ((MUL15(a[13], a[19])
		+ MUL15(a[14], a[18])
		+ MUL15(a[15], a[17])) << 1);
	t[33] = ((MUL15(a[14], a[19])
		+ MUL15(a[15], a[18])
		+ MUL15(a[16], a[17])) << 1);
	t[34] = MUL15(a[17], a[17])
		+ ((MUL15(a[15], a[19])
		+ MUL15(a[16], a[18])) << 1);
	t[35] = ((MUL15(a[16], a[19])
		+ MUL15(a[17], a[18])) << 1);
	t[36] = MUL15(a[18], a[18])
		+ ((MUL15(a[17], a[19])) << 1);
	t[37] = ((MUL15(a[18], a[19])) << 1);
	t[38] = MUL15(a[19], a[19]);
	d[39] = norm13(d, t, 39);
}

#endif

/*
 * Modulus for field F256 (field for point coordinates in curve P-256).
 */
static const uint32_t F256[] = {
	0x1FFF, 0x1FFF, 0x1FFF, 0x1FFF, 0x1FFF, 0x1FFF, 0x1FFF, 0x001F,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0400, 0x0000,
	0x0000, 0x1FF8, 0x1FFF, 0x01FF
};

/*
 * The 'b' curve equation coefficient for P-256.
 */
static const uint32_t P256_B[] = {
	0x004B, 0x1E93, 0x0F89, 0x1C78, 0x03BC, 0x187B, 0x114E, 0x1619,
	0x1D06, 0x0328, 0x01AF, 0x0D31, 0x1557, 0x15DE, 0x1ECF, 0x127C,
	0x0A3A, 0x0EC5, 0x118D, 0x00B5
};

/*
 * Perform a "short reduction" in field F256 (field for curve P-256).
 * The source value should be less than 262 bits; on output, it will
 * be at most 257 bits, and less than twice the modulus.
 */
static void
reduce_f256(uint32_t *d)
{
	uint32_t x;

	x = d[19] >> 9;
	d[19] &= 0x01FF;
	d[17] += x << 3;
	d[14] -= x << 10;
	d[7] -= x << 5;
	d[0] += x;
	norm13(d, d, 20);
}

/*
 * Perform a "final reduction" in field F256 (field for curve P-256).
 * The source value must be less than twice the modulus. If the value
 * is not lower than the modulus, then the modulus is subtracted and
 * this function returns 1; otherwise, it leaves it untouched and it
 * returns 0.
 */
static uint32_t
reduce_final_f256(uint32_t *d)
{
	uint32_t t[20];
	uint32_t cc;
	int i;

	memcpy(t, d, sizeof t);
	cc = 0;
	for (i = 0; i < 20; i ++) {
		uint32_t w;

		w = t[i] - F256[i] - cc;
		cc = w >> 31;
		t[i] = w & 0x1FFF;
	}
	cc ^= 1;
	CCOPY(cc, d, t, sizeof t);
	return cc;
}

/*
 * Perform a multiplication of two integers modulo
 * 2^256-2^224+2^192+2^96-1 (for NIST curve P-256). Operands are arrays
 * of 20 words, each containing 13 bits of data, in little-endian order.
 * On input, upper word may be up to 13 bits (hence value up to 2^260-1);
 * on output, value fits on 257 bits and is lower than twice the modulus.
 */
static void
mul_f256(uint32_t *d, const uint32_t *a, const uint32_t *b)
{
	uint32_t t[40], cc;
	int i;

	/*
	 * Compute raw multiplication. All result words fit in 13 bits
	 * each.
	 */
	mul20(t, a, b);

	/*
	 * Modular reduction: each high word in added/subtracted where
	 * necessary.
	 *
	 * The modulus is:
	 *    p = 2^256 - 2^224 + 2^192 + 2^96 - 1
	 * Therefore:
	 *    2^256 = 2^224 - 2^192 - 2^96 + 1 mod p
	 *
	 * For a word x at bit offset n (n >= 256), we have:
	 *    x*2^n = x*2^(n-32) - x*2^(n-64)
	 *            - x*2^(n - 160) + x*2^(n-256) mod p
	 *
	 * Thus, we can nullify the high word if we reinject it at some
	 * proper emplacements.
	 */
	for (i = 39; i >= 20; i --) {
		uint32_t x;

		x = t[i];
		t[i - 2] += ARSH(x, 6);
		t[i - 3] += (x << 7) & 0x1FFF;
		t[i - 4] -= ARSH(x, 12);
		t[i - 5] -= (x << 1) & 0x1FFF;
		t[i - 12] -= ARSH(x, 4);
		t[i - 13] -= (x << 9) & 0x1FFF;
		t[i - 19] += ARSH(x, 9);
		t[i - 20] += (x << 4) & 0x1FFF;
	}

	/*
	 * Propagate carries. Since the operation above really is a
	 * truncature, followed by the addition of nonnegative values,
	 * the result will be positive. Moreover, the carry cannot
	 * exceed 5 bits (we performed 20 additions with values smaller
	 * than 256 bits).
	 */
	cc = norm13(t, t, 20);

	/*
	 * Perform modular reduction again for the bits beyond 256 (the carry
	 * and the bits 256..259). This time, we can simply inject full
	 * word values.
	 */
	cc = (cc << 4) | (t[19] >> 9);
	t[19] &= 0x01FF;
	t[17] += cc << 3;
	t[14] -= cc << 10;
	t[7] -= cc << 5;
	t[0] += cc;
	norm13(d, t, 20);
}

/*
 * Square an integer modulo 2^256-2^224+2^192+2^96-1 (for NIST curve
 * P-256). Operand is an array of 20 words, each containing 13 bits of
 * data, in little-endian order. On input, upper word may be up to 13
 * bits (hence value up to 2^260-1); on output, value fits on 257 bits
 * and is lower than twice the modulus.
 */
static void
square_f256(uint32_t *d, const uint32_t *a)
{
	uint32_t t[40], cc;
	int i;

	/*
	 * Compute raw square. All result words fit in 13 bits each.
	 */
	square20(t, a);

	/*
	 * Modular reduction: each high word in added/subtracted where
	 * necessary.
	 *
	 * The modulus is:
	 *    p = 2^256 - 2^224 + 2^192 + 2^96 - 1
	 * Therefore:
	 *    2^256 = 2^224 - 2^192 - 2^96 + 1 mod p
	 *
	 * For a word x at bit offset n (n >= 256), we have:
	 *    x*2^n = x*2^(n-32) - x*2^(n-64)
	 *            - x*2^(n - 160) + x*2^(n-256) mod p
	 *
	 * Thus, we can nullify the high word if we reinject it at some
	 * proper emplacements.
	 */
	for (i = 39; i >= 20; i --) {
		uint32_t x;

		x = t[i];
		t[i - 2] += ARSH(x, 6);
		t[i - 3] += (x << 7) & 0x1FFF;
		t[i - 4] -= ARSH(x, 12);
		t[i - 5] -= (x << 1) & 0x1FFF;
		t[i - 12] -= ARSH(x, 4);
		t[i - 13] -= (x << 9) & 0x1FFF;
		t[i - 19] += ARSH(x, 9);
		t[i - 20] += (x << 4) & 0x1FFF;
	}

	/*
	 * Propagate carries. Since the operation above really is a
	 * truncature, followed by the addition of nonnegative values,
	 * the result will be positive. Moreover, the carry cannot
	 * exceed 5 bits (we performed 20 additions with values smaller
	 * than 256 bits).
	 */
	cc = norm13(t, t, 20);

	/*
	 * Perform modular reduction again for the bits beyond 256 (the carry
	 * and the bits 256..259). This time, we can simply inject full
	 * word values.
	 */
	cc = (cc << 4) | (t[19] >> 9);
	t[19] &= 0x01FF;
	t[17] += cc << 3;
	t[14] -= cc << 10;
	t[7] -= cc << 5;
	t[0] += cc;
	norm13(d, t, 20);
}

/*
 * Jacobian coordinates for a point in P-256: affine coordinates (X,Y)
 * are such that:
 *   X = x / z^2
 *   Y = y / z^3
 * For the point at infinity, z = 0.
 * Each point thus admits many possible representations.
 *
 * Coordinates are represented in arrays of 32-bit integers, each holding
 * 13 bits of data. Values may also be slightly greater than the modulus,
 * but they will always be lower than twice the modulus.
 */
typedef struct {
	uint32_t x[20];
	uint32_t y[20];
	uint32_t z[20];
} p256_jacobian;

/*
 * Convert a point to affine coordinates:
 *  - If the point is the point at infinity, then all three coordinates
 *    are set to 0.
 *  - Otherwise, the 'z' coordinate is set to 1, and the 'x' and 'y'
 *    coordinates are the 'X' and 'Y' affine coordinates.
 * The coordinates are guaranteed to be lower than the modulus.
 */
static void
p256_to_affine(p256_jacobian *P)
{
	uint32_t t1[20], t2[20];
	int i;

	/*
	 * Invert z with a modular exponentiation: the modulus is
	 * p = 2^256 - 2^224 + 2^192 + 2^96 - 1, and the exponent is
	 * p-2. Exponent bit pattern (from high to low) is:
	 *  - 32 bits of value 1
	 *  - 31 bits of value 0
	 *  - 1 bit of value 1
	 *  - 96 bits of value 0
	 *  - 94 bits of value 1
	 *  - 1 bit of value 0
	 *  - 1 bit of value 1
	 * Thus, we precompute z^(2^31-1) to speed things up.
	 *
	 * If z = 0 (point at infinity) then the modular exponentiation
	 * will yield 0, which leads to the expected result (all three
	 * coordinates set to 0).
	 */

	/*
	 * A simple square-and-multiply for z^(2^31-1). We could save about
	 * two dozen multiplications here with an addition chain, but
	 * this would require a bit more code, and extra stack buffers.
	 */
	memcpy(t1, P->z, sizeof P->z);
	for (i = 0; i < 30; i ++) {
		square_f256(t1, t1);
		mul_f256(t1, t1, P->z);
	}

	/*
	 * Square-and-multiply. Apart from the squarings, we have a few
	 * multiplications to set bits to 1; we multiply by the original z
	 * for setting 1 bit, and by t1 for setting 31 bits.
	 */
	memcpy(t2, P->z, sizeof P->z);
	for (i = 1; i < 256; i ++) {
		square_f256(t2, t2);
		switch (i) {
		case 31:
		case 190:
		case 221:
		case 252:
			mul_f256(t2, t2, t1);
			break;
		case 63:
		case 253:
		case 255:
			mul_f256(t2, t2, P->z);
			break;
		}
	}

	/*
	 * Now that we have 1/z, multiply x by 1/z^2 and y by 1/z^3.
	 */
	mul_f256(t1, t2, t2);
	mul_f256(P->x, t1, P->x);
	mul_f256(t1, t1, t2);
	mul_f256(P->y, t1, P->y);
	reduce_final_f256(P->x);
	reduce_final_f256(P->y);

	/*
	 * Multiply z by 1/z. If z = 0, then this will yield 0, otherwise
	 * this will set z to 1.
	 */
	mul_f256(P->z, P->z, t2);
	reduce_final_f256(P->z);
}

/*
 * Double a point in P-256. This function works for all valid points,
 * including the point at infinity.
 */
static void
p256_double(p256_jacobian *Q)
{
	/*
	 * Doubling formulas are:
	 *
	 *   s = 4*x*y^2
	 *   m = 3*(x + z^2)*(x - z^2)
	 *   x' = m^2 - 2*s
	 *   y' = m*(s - x') - 8*y^4
	 *   z' = 2*y*z
	 *
	 * These formulas work for all points, including points of order 2
	 * and points at infinity:
	 *   - If y = 0 then z' = 0. But there is no such point in P-256
	 *     anyway.
	 *   - If z = 0 then z' = 0.
	 */
	uint32_t t1[20], t2[20], t3[20], t4[20];
	int i;

	/*
	 * Compute z^2 in t1.
	 */
	square_f256(t1, Q->z);

	/*
	 * Compute x-z^2 in t2 and x+z^2 in t1.
	 */
	for (i = 0; i < 20; i ++) {
		t2[i] = (F256[i] << 1) + Q->x[i] - t1[i];
		t1[i] += Q->x[i];
	}
	norm13(t1, t1, 20);
	norm13(t2, t2, 20);

	/*
	 * Compute 3*(x+z^2)*(x-z^2) in t1.
	 */
	mul_f256(t3, t1, t2);
	for (i = 0; i < 20; i ++) {
		t1[i] = MUL15(3, t3[i]);
	}
	norm13(t1, t1, 20);

	/*
	 * Compute 4*x*y^2 (in t2) and 2*y^2 (in t3).
	 */
	square_f256(t3, Q->y);
	for (i = 0; i < 20; i ++) {
		t3[i] <<= 1;
	}
	norm13(t3, t3, 20);
	mul_f256(t2, Q->x, t3);
	for (i = 0; i < 20; i ++) {
		t2[i] <<= 1;
	}
	norm13(t2, t2, 20);
	reduce_f256(t2);

	/*
	 * Compute x' = m^2 - 2*s.
	 */
	square_f256(Q->x, t1);
	for (i = 0; i < 20; i ++) {
		Q->x[i] += (F256[i] << 2) - (t2[i] << 1);
	}
	norm13(Q->x, Q->x, 20);
	reduce_f256(Q->x);

	/*
	 * Compute z' = 2*y*z.
	 */
	mul_f256(t4, Q->y, Q->z);
	for (i = 0; i < 20; i ++) {
		Q->z[i] = t4[i] << 1;
	}
	norm13(Q->z, Q->z, 20);
	reduce_f256(Q->z);

	/*
	 * Compute y' = m*(s - x') - 8*y^4. Note that we already have
	 * 2*y^2 in t3.
	 */
	for (i = 0; i < 20; i ++) {
		t2[i] += (F256[i] << 1) - Q->x[i];
	}
	norm13(t2, t2, 20);
	mul_f256(Q->y, t1, t2);
	square_f256(t4, t3);
	for (i = 0; i < 20; i ++) {
		Q->y[i] += (F256[i] << 2) - (t4[i] << 1);
	}
	norm13(Q->y, Q->y, 20);
	reduce_f256(Q->y);
}

/*
 * Add point P2 to point P1.
 *
 * This function computes the wrong result in the following cases:
 *
 *   - If P1 == 0 but P2 != 0
 *   - If P1 != 0 but P2 == 0
 *   - If P1 == P2
 *
 * In all three cases, P1 is set to the point at infinity.
 *
 * Returned value is 0 if one of the following occurs:
 *
 *   - P1 and P2 have the same Y coordinate
 *   - P1 == 0 and P2 == 0
 *   - The Y coordinate of one of the points is 0 and the other point is
 *     the point at infinity.
 *
 * The third case cannot actually happen with valid points, since a point
 * with Y == 0 is a point of order 2, and there is no point of order 2 on
 * curve P-256.
 *
 * Therefore, assuming that P1 != 0 and P2 != 0 on input, then the caller
 * can apply the following:
 *
 *   - If the result is not the point at infinity, then it is correct.
 *   - Otherwise, if the returned value is 1, then this is a case of
 *     P1+P2 == 0, so the result is indeed the point at infinity.
 *   - Otherwise, P1 == P2, so a "double" operation should have been
 *     performed.
 */
static uint32_t
p256_add(p256_jacobian *P1, const p256_jacobian *P2)
{
	/*
	 * Addtions formulas are:
	 *
	 *   u1 = x1 * z2^2
	 *   u2 = x2 * z1^2
	 *   s1 = y1 * z2^3
	 *   s2 = y2 * z1^3
	 *   h = u2 - u1
	 *   r = s2 - s1
	 *   x3 = r^2 - h^3 - 2 * u1 * h^2
	 *   y3 = r * (u1 * h^2 - x3) - s1 * h^3
	 *   z3 = h * z1 * z2
	 */
	uint32_t t1[20], t2[20], t3[20], t4[20], t5[20], t6[20], t7[20];
	uint32_t ret;
	int i;

	/*
	 * Compute u1 = x1*z2^2 (in t1) and s1 = y1*z2^3 (in t3).
	 */
	square_f256(t3, P2->z);
	mul_f256(t1, P1->x, t3);
	mul_f256(t4, P2->z, t3);
	mul_f256(t3, P1->y, t4);

	/*
	 * Compute u2 = x2*z1^2 (in t2) and s2 = y2*z1^3 (in t4).
	 */
	square_f256(t4, P1->z);
	mul_f256(t2, P2->x, t4);
	mul_f256(t5, P1->z, t4);
	mul_f256(t4, P2->y, t5);

	/*
	 * Compute h = h2 - u1 (in t2) and r = s2 - s1 (in t4).
	 * We need to test whether r is zero, so we will do some extra
	 * reduce.
	 */
	for (i = 0; i < 20; i ++) {
		t2[i] += (F256[i] << 1) - t1[i];
		t4[i] += (F256[i] << 1) - t3[i];
	}
	norm13(t2, t2, 20);
	norm13(t4, t4, 20);
	reduce_f256(t4);
	reduce_final_f256(t4);
	ret = 0;
	for (i = 0; i < 20; i ++) {
		ret |= t4[i];
	}
	ret = (ret | -ret) >> 31;

	/*
	 * Compute u1*h^2 (in t6) and h^3 (in t5);
	 */
	square_f256(t7, t2);
	mul_f256(t6, t1, t7);
	mul_f256(t5, t7, t2);

	/*
	 * Compute x3 = r^2 - h^3 - 2*u1*h^2.
	 */
	square_f256(P1->x, t4);
	for (i = 0; i < 20; i ++) {
		P1->x[i] += (F256[i] << 3) - t5[i] - (t6[i] << 1);
	}
	norm13(P1->x, P1->x, 20);
	reduce_f256(P1->x);

	/*
	 * Compute y3 = r*(u1*h^2 - x3) - s1*h^3.
	 */
	for (i = 0; i < 20; i ++) {
		t6[i] += (F256[i] << 1) - P1->x[i];
	}
	norm13(t6, t6, 20);
	mul_f256(P1->y, t4, t6);
	mul_f256(t1, t5, t3);
	for (i = 0; i < 20; i ++) {
		P1->y[i] += (F256[i] << 1) - t1[i];
	}
	norm13(P1->y, P1->y, 20);
	reduce_f256(P1->y);

	/*
	 * Compute z3 = h*z1*z2.
	 */
	mul_f256(t1, P1->z, P2->z);
	mul_f256(P1->z, t1, t2);

	return ret;
}

/*
 * Add point P2 to point P1. This is a specialised function for the
 * case when P2 is a non-zero point in affine coordinate.
 *
 * This function computes the wrong result in the following cases:
 *
 *   - If P1 == 0
 *   - If P1 == P2
 *
 * In both cases, P1 is set to the point at infinity.
 *
 * Returned value is 0 if one of the following occurs:
 *
 *   - P1 and P2 have the same Y coordinate
 *   - The Y coordinate of P2 is 0 and P1 is the point at infinity.
 *
 * The second case cannot actually happen with valid points, since a point
 * with Y == 0 is a point of order 2, and there is no point of order 2 on
 * curve P-256.
 *
 * Therefore, assuming that P1 != 0 on input, then the caller
 * can apply the following:
 *
 *   - If the result is not the point at infinity, then it is correct.
 *   - Otherwise, if the returned value is 1, then this is a case of
 *     P1+P2 == 0, so the result is indeed the point at infinity.
 *   - Otherwise, P1 == P2, so a "double" operation should have been
 *     performed.
 */
static uint32_t
p256_add_mixed(p256_jacobian *P1, const p256_jacobian *P2)
{
	/*
	 * Addtions formulas are:
	 *
	 *   u1 = x1
	 *   u2 = x2 * z1^2
	 *   s1 = y1
	 *   s2 = y2 * z1^3
	 *   h = u2 - u1
	 *   r = s2 - s1
	 *   x3 = r^2 - h^3 - 2 * u1 * h^2
	 *   y3 = r * (u1 * h^2 - x3) - s1 * h^3
	 *   z3 = h * z1
	 */
	uint32_t t1[20], t2[20], t3[20], t4[20], t5[20], t6[20], t7[20];
	uint32_t ret;
	int i;

	/*
	 * Compute u1 = x1 (in t1) and s1 = y1 (in t3).
	 */
	memcpy(t1, P1->x, sizeof t1);
	memcpy(t3, P1->y, sizeof t3);

	/*
	 * Compute u2 = x2*z1^2 (in t2) and s2 = y2*z1^3 (in t4).
	 */
	square_f256(t4, P1->z);
	mul_f256(t2, P2->x, t4);
	mul_f256(t5, P1->z, t4);
	mul_f256(t4, P2->y, t5);

	/*
	 * Compute h = h2 - u1 (in t2) and r = s2 - s1 (in t4).
	 * We need to test whether r is zero, so we will do some extra
	 * reduce.
	 */
	for (i = 0; i < 20; i ++) {
		t2[i] += (F256[i] << 1) - t1[i];
		t4[i] += (F256[i] << 1) - t3[i];
	}
	norm13(t2, t2, 20);
	norm13(t4, t4, 20);
	reduce_f256(t4);
	reduce_final_f256(t4);
	ret = 0;
	for (i = 0; i < 20; i ++) {
		ret |= t4[i];
	}
	ret = (ret | -ret) >> 31;

	/*
	 * Compute u1*h^2 (in t6) and h^3 (in t5);
	 */
	square_f256(t7, t2);
	mul_f256(t6, t1, t7);
	mul_f256(t5, t7, t2);

	/*
	 * Compute x3 = r^2 - h^3 - 2*u1*h^2.
	 */
	square_f256(P1->x, t4);
	for (i = 0; i < 20; i ++) {
		P1->x[i] += (F256[i] << 3) - t5[i] - (t6[i] << 1);
	}
	norm13(P1->x, P1->x, 20);
	reduce_f256(P1->x);

	/*
	 * Compute y3 = r*(u1*h^2 - x3) - s1*h^3.
	 */
	for (i = 0; i < 20; i ++) {
		t6[i] += (F256[i] << 1) - P1->x[i];
	}
	norm13(t6, t6, 20);
	mul_f256(P1->y, t4, t6);
	mul_f256(t1, t5, t3);
	for (i = 0; i < 20; i ++) {
		P1->y[i] += (F256[i] << 1) - t1[i];
	}
	norm13(P1->y, P1->y, 20);
	reduce_f256(P1->y);

	/*
	 * Compute z3 = h*z1*z2.
	 */
	mul_f256(P1->z, P1->z, t2);

	return ret;
}

/*
 * Decode a P-256 point. This function does not support the point at
 * infinity. Returned value is 0 if the point is invalid, 1 otherwise.
 */
static uint32_t
p256_decode(p256_jacobian *P, const void *src, size_t len)
{
	const unsigned char *buf;
	uint32_t tx[20], ty[20], t1[20], t2[20];
	uint32_t bad;
	int i;

	if (len != 65) {
		return 0;
	}
	buf = src;

	/*
	 * First byte must be 0x04 (uncompressed format). We could support
	 * "hybrid format" (first byte is 0x06 or 0x07, and encodes the
	 * least significant bit of the Y coordinate), but it is explicitly
	 * forbidden by RFC 5480 (section 2.2).
	 */
	bad = NEQ(buf[0], 0x04);

	/*
	 * Decode the coordinates, and check that they are both lower
	 * than the modulus.
	 */
	tx[19] = be8_to_le13(tx, buf + 1, 32);
	ty[19] = be8_to_le13(ty, buf + 33, 32);
	bad |= reduce_final_f256(tx);
	bad |= reduce_final_f256(ty);

	/*
	 * Check curve equation.
	 */
	square_f256(t1, tx);
	mul_f256(t1, tx, t1);
	square_f256(t2, ty);
	for (i = 0; i < 20; i ++) {
		t1[i] += (F256[i] << 3) - MUL15(3, tx[i]) + P256_B[i] - t2[i];
	}
	norm13(t1, t1, 20);
	reduce_f256(t1);
	reduce_final_f256(t1);
	for (i = 0; i < 20; i ++) {
		bad |= t1[i];
	}

	/*
	 * Copy coordinates to the point structure.
	 */
	memcpy(P->x, tx, sizeof tx);
	memcpy(P->y, ty, sizeof ty);
	memset(P->z, 0, sizeof P->z);
	P->z[0] = 1;
	return NEQ(bad, 0) ^ 1;
}

/*
 * Encode a point into a buffer. This function assumes that the point is
 * valid, in affine coordinates, and not the point at infinity.
 */
static void
p256_encode(void *dst, const p256_jacobian *P)
{
	unsigned char *buf;

	buf = dst;
	buf[0] = 0x04;
	le13_to_be8(buf + 1, 32, P->x);
	le13_to_be8(buf + 33, 32, P->y);
}

/*
 * Multiply a curve point by an integer. The integer is assumed to be
 * lower than the curve order, and the base point must not be the point
 * at infinity.
 */
static void
p256_mul(p256_jacobian *P, const unsigned char *x, size_t xlen)
{
	/*
	 * qz is a flag that is initially 1, and remains equal to 1
	 * as long as the point is the point at infinity.
	 *
	 * We use a 2-bit window to handle multiplier bits by pairs.
	 * The precomputed window really is the points P2 and P3.
	 */
	uint32_t qz;
	p256_jacobian P2, P3, Q, T, U;

	/*
	 * Compute window values.
	 */
	P2 = *P;
	p256_double(&P2);
	P3 = *P;
	p256_add(&P3, &P2);

	/*
	 * We start with Q = 0. We process multiplier bits 2 by 2.
	 */
	memset(&Q, 0, sizeof Q);
	qz = 1;
	while (xlen -- > 0) {
		int k;

		for (k = 6; k >= 0; k -= 2) {
			uint32_t bits;
			uint32_t bnz;

			p256_double(&Q);
			p256_double(&Q);
			T = *P;
			U = Q;
			bits = (*x >> k) & (uint32_t)3;
			bnz = NEQ(bits, 0);
			CCOPY(EQ(bits, 2), &T, &P2, sizeof T);
			CCOPY(EQ(bits, 3), &T, &P3, sizeof T);
			p256_add(&U, &T);
			CCOPY(bnz & qz, &Q, &T, sizeof Q);
			CCOPY(bnz & ~qz, &Q, &U, sizeof Q);
			qz &= ~bnz;
		}
		x ++;
	}
	*P = Q;
}

/*
 * Precomputed window: k*G points, where G is the curve generator, and k
 * is an integer from 1 to 15 (inclusive). The X and Y coordinates of
 * the point are encoded as 20 words of 13 bits each (little-endian
 * order); 13-bit words are then grouped 2-by-2 into 32-bit words
 * (little-endian order within each word).
 */
static const uint32_t Gwin[15][20] = {

	{ 0x04C60296, 0x02721176, 0x19D00F4A, 0x102517AC,
	  0x13B8037D, 0x0748103C, 0x1E730E56, 0x08481FE2,
	  0x0F97012C, 0x00D605F4, 0x1DFA11F5, 0x0C801A0D,
	  0x0F670CBB, 0x0AED0CC5, 0x115E0E33, 0x181F0785,
	  0x13F514A7, 0x0FF30E3B, 0x17171E1A, 0x009F18D0 },

	{ 0x1B341978, 0x16911F11, 0x0D9A1A60, 0x1C4E1FC8,
	  0x1E040969, 0x096A06B0, 0x091C0030, 0x09EF1A29,
	  0x18C40D03, 0x00F91C9E, 0x13C313D1, 0x096F0748,
	  0x011419E0, 0x1CC713A6, 0x1DD31DAD, 0x1EE80C36,
	  0x1ECD0C69, 0x1A0800A4, 0x08861B8E, 0x000E1DD5 },

	{ 0x173F1D6C, 0x02CC06F1, 0x14C21FB4, 0x043D1EB6,
	  0x0F3606B7, 0x1A971C59, 0x1BF71951, 0x01481323,
	  0x068D0633, 0x00BD12F9, 0x13EA1032, 0x136209E8,
	  0x1C1E19A7, 0x06C7013E, 0x06C10AB0, 0x14C908BB,
	  0x05830CE1, 0x1FEF18DD, 0x00620998, 0x010E0D19 },

	{ 0x18180852, 0x0604111A, 0x0B771509, 0x1B6F0156,
	  0x00181FE2, 0x1DCC0AF4, 0x16EF0659, 0x11F70E80,
	  0x11A912D0, 0x01C414D2, 0x027618C6, 0x05840FC6,
	  0x100215C4, 0x187E0C3B, 0x12771C96, 0x150C0B5D,
	  0x0FF705FD, 0x07981C67, 0x1AD20C63, 0x01C11C55 },

	{ 0x1E8113ED, 0x0A940370, 0x12920215, 0x1FA31D6F,
	  0x1F7C0C82, 0x10CD03F7, 0x02640560, 0x081A0B5E,
	  0x1BD21151, 0x00A21642, 0x0D0B0DA4, 0x0176113F,
	  0x04440D1D, 0x001A1360, 0x1068012F, 0x1F141E49,
	  0x10DF136B, 0x0E4F162B, 0x0D44104A, 0x01C1105F },

	{ 0x011411A9, 0x01551A4F, 0x0ADA0C6B, 0x01BD0EC8,
	  0x18120C74, 0x112F1778, 0x099202CB, 0x0C05124B,
	  0x195316A4, 0x01600685, 0x1E3B1FE2, 0x189014E3,
	  0x0B5E1FD7, 0x0E0311F8, 0x08E000F7, 0x174E00DE,
	  0x160702DF, 0x1B5A15BF, 0x03A11237, 0x01D01704 },

	{ 0x0C3D12A3, 0x0C501C0C, 0x17AD1300, 0x1715003F,
	  0x03F719F8, 0x18031ED8, 0x1D980667, 0x0F681896,
	  0x1B7D00BF, 0x011C14CE, 0x0FA000B4, 0x1C3501B0,
	  0x0D901C55, 0x06790C10, 0x029E0736, 0x0DEB0400,
	  0x034F183A, 0x030619B4, 0x0DEF0033, 0x00E71AC7 },

	{ 0x1B7D1393, 0x1B3B1076, 0x0BED1B4D, 0x13011F3A,
	  0x0E0E1238, 0x156A132B, 0x013A02D3, 0x160A0D01,
	  0x1CED1EE9, 0x00C5165D, 0x184C157E, 0x08141A83,
	  0x153C0DA5, 0x1ED70F9D, 0x05170D51, 0x02CF13B8,
	  0x18AE1771, 0x1B04113F, 0x05EC11E9, 0x015A16B3 },

	{ 0x04A41EE0, 0x1D1412E4, 0x1C591D79, 0x118511B7,
	  0x14F00ACB, 0x1AE31E1C, 0x049C0D51, 0x016E061E,
	  0x1DB71EDF, 0x01D41A35, 0x0E8208FA, 0x14441293,
	  0x011F1E85, 0x1D54137A, 0x026B114F, 0x151D0832,
	  0x00A50964, 0x1F9C1E1C, 0x064B12C9, 0x005409D1 },

	{ 0x062B123F, 0x0C0D0501, 0x183704C3, 0x08E31120,
	  0x0A2E0A6C, 0x14440FED, 0x090A0D1E, 0x13271964,
	  0x0B590A3A, 0x019D1D9B, 0x05780773, 0x09770A91,
	  0x0F770CA3, 0x053F19D4, 0x02C80DED, 0x1A761304,
	  0x091E0DD9, 0x15D201B8, 0x151109AA, 0x010F0198 },

	{ 0x05E101D1, 0x072314DD, 0x045F1433, 0x1A041541,
	  0x10B3142E, 0x01840736, 0x1C1B19DB, 0x098B0418,
	  0x1DBC083B, 0x007D1444, 0x01511740, 0x11DD1F3A,
	  0x04ED0E2F, 0x1B4B1A62, 0x10480D04, 0x09E911A2,
	  0x04211AFA, 0x19140893, 0x04D60CC4, 0x01210648 },

	{ 0x112703C4, 0x018B1BA1, 0x164C1D50, 0x05160BE0,
	  0x0BCC1830, 0x01CB1554, 0x13291732, 0x1B2B1918,
	  0x0DED0817, 0x00E80775, 0x0A2401D3, 0x0BFE08B3,
	  0x0E531199, 0x058616E9, 0x04770B91, 0x110F0C55,
	  0x19C11554, 0x0BFB1159, 0x03541C38, 0x000E1C2D },

	{ 0x10390C01, 0x02BB0751, 0x0AC5098E, 0x096C17AB,
	  0x03C90E28, 0x10BD18BF, 0x002E1F2D, 0x092B0986,
	  0x1BD700AC, 0x002E1F20, 0x1E3D1FD8, 0x077718BB,
	  0x06F919C4, 0x187407ED, 0x11370E14, 0x081E139C,
	  0x00481ADB, 0x14AB0289, 0x066A0EBE, 0x00C70ED6 },

	{ 0x0694120B, 0x124E1CC9, 0x0E2F0570, 0x17CF081A,
	  0x078906AC, 0x066D17CF, 0x1B3207F4, 0x0C5705E9,
	  0x10001C38, 0x00A919DE, 0x06851375, 0x0F900BD8,
	  0x080401BA, 0x0EEE0D42, 0x1B8B11EA, 0x0B4519F0,
	  0x090F18C0, 0x062E1508, 0x0DD909F4, 0x01EB067C },

	{ 0x0CDC1D5F, 0x0D1818F9, 0x07781636, 0x125B18E8,
	  0x0D7003AF, 0x13110099, 0x1D9B1899, 0x175C1EB7,
	  0x0E34171A, 0x01E01153, 0x081A0F36, 0x0B391783,
	  0x1D1F147E, 0x19CE16D7, 0x11511B21, 0x1F2C10F9,
	  0x12CA0E51, 0x05A31D39, 0x171A192E, 0x016B0E4F }
};

/*
 * Lookup one of the Gwin[] values, by index. This is constant-time.
 */
static void
lookup_Gwin(p256_jacobian *T, uint32_t idx)
{
	uint32_t xy[20];
	uint32_t k;
	size_t u;

	memset(xy, 0, sizeof xy);
	for (k = 0; k < 15; k ++) {
		uint32_t m;

		m = -EQ(idx, k + 1);
		for (u = 0; u < 20; u ++) {
			xy[u] |= m & Gwin[k][u];
		}
	}
	for (u = 0; u < 10; u ++) {
		T->x[(u << 1) + 0] = xy[u] & 0xFFFF;
		T->x[(u << 1) + 1] = xy[u] >> 16;
		T->y[(u << 1) + 0] = xy[u + 10] & 0xFFFF;
		T->y[(u << 1) + 1] = xy[u + 10] >> 16;
	}
	memset(T->z, 0, sizeof T->z);
	T->z[0] = 1;
}

/*
 * Multiply the generator by an integer. The integer is assumed non-zero
 * and lower than the curve order.
 */
static void
p256_mulgen(p256_jacobian *P, const unsigned char *x, size_t xlen)
{
	/*
	 * qz is a flag that is initially 1, and remains equal to 1
	 * as long as the point is the point at infinity.
	 *
	 * We use a 4-bit window to handle multiplier bits by groups
	 * of 4. The precomputed window is constant static data, with
	 * points in affine coordinates; we use a constant-time lookup.
	 */
	p256_jacobian Q;
	uint32_t qz;

	memset(&Q, 0, sizeof Q);
	qz = 1;
	while (xlen -- > 0) {
		int k;
		unsigned bx;

		bx = *x ++;
		for (k = 0; k < 2; k ++) {
			uint32_t bits;
			uint32_t bnz;
			p256_jacobian T, U;

			p256_double(&Q);
			p256_double(&Q);
			p256_double(&Q);
			p256_double(&Q);
			bits = (bx >> 4) & 0x0F;
			bnz = NEQ(bits, 0);
			lookup_Gwin(&T, bits);
			U = Q;
			p256_add_mixed(&U, &T);
			CCOPY(bnz & qz, &Q, &T, sizeof Q);
			CCOPY(bnz & ~qz, &Q, &U, sizeof Q);
			qz &= ~bnz;
			bx <<= 4;
		}
	}
	*P = Q;
}

static const unsigned char P256_G[] = {
	0x04, 0x6B, 0x17, 0xD1, 0xF2, 0xE1, 0x2C, 0x42, 0x47, 0xF8,
	0xBC, 0xE6, 0xE5, 0x63, 0xA4, 0x40, 0xF2, 0x77, 0x03, 0x7D,
	0x81, 0x2D, 0xEB, 0x33, 0xA0, 0xF4, 0xA1, 0x39, 0x45, 0xD8,
	0x98, 0xC2, 0x96, 0x4F, 0xE3, 0x42, 0xE2, 0xFE, 0x1A, 0x7F,
	0x9B, 0x8E, 0xE7, 0xEB, 0x4A, 0x7C, 0x0F, 0x9E, 0x16, 0x2B,
	0xCE, 0x33, 0x57, 0x6B, 0x31, 0x5E, 0xCE, 0xCB, 0xB6, 0x40,
	0x68, 0x37, 0xBF, 0x51, 0xF5
};

static const unsigned char P256_N[] = {
	0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xBC, 0xE6, 0xFA, 0xAD,
	0xA7, 0x17, 0x9E, 0x84, 0xF3, 0xB9, 0xCA, 0xC2, 0xFC, 0x63,
	0x25, 0x51
};

static const unsigned char *
api_generator(int curve, size_t *len)
{
	(void)curve;
	*len = sizeof P256_G;
	return P256_G;
}

static const unsigned char *
api_order(int curve, size_t *len)
{
	(void)curve;
	*len = sizeof P256_N;
	return P256_N;
}

static uint32_t
api_mul(unsigned char *G, size_t Glen,
	const unsigned char *x, size_t xlen, int curve)
{
	uint32_t r;
	p256_jacobian P;

	(void)curve;
	r = p256_decode(&P, G, Glen);
	p256_mul(&P, x, xlen);
	if (Glen >= 65) {
		p256_to_affine(&P);
		p256_encode(G, &P);
	}
	return r;
}

static size_t
api_mulgen(unsigned char *R,
	const unsigned char *x, size_t xlen, int curve)
{
	p256_jacobian P;

	(void)curve;
	p256_mulgen(&P, x, xlen);
	p256_to_affine(&P);
	p256_encode(R, &P);
	return 65;

	/*
	const unsigned char *G;
	size_t Glen;

	G = api_generator(curve, &Glen);
	memcpy(R, G, Glen);
	api_mul(R, Glen, x, xlen, curve);
	return Glen;
	*/
}

static uint32_t
api_muladd(unsigned char *A, const unsigned char *B, size_t len,
	const unsigned char *x, size_t xlen,
	const unsigned char *y, size_t ylen, int curve)
{
	p256_jacobian P, Q;
	uint32_t r, t, z;
	int i;

	(void)curve;
	r = p256_decode(&P, A, len);
	p256_mul(&P, x, xlen);
	if (B == NULL) {
		p256_mulgen(&Q, y, ylen);
	} else {
		r &= p256_decode(&Q, B, len);
		p256_mul(&Q, y, ylen);
	}

	/*
	 * The final addition may fail in case both points are equal.
	 */
	t = p256_add(&P, &Q);
	reduce_final_f256(P.z);
	z = 0;
	for (i = 0; i < 20; i ++) {
		z |= P.z[i];
	}
	z = EQ(z, 0);
	p256_double(&Q);

	/*
	 * If z is 1 then either P+Q = 0 (t = 1) or P = Q (t = 0). So we
	 * have the following:
	 *
	 *   z = 0, t = 0   return P (normal addition)
	 *   z = 0, t = 1   return P (normal addition)
	 *   z = 1, t = 0   return Q (a 'double' case)
	 *   z = 1, t = 1   report an error (P+Q = 0)
	 */
	CCOPY(z & ~t, &P, &Q, sizeof Q);
	p256_to_affine(&P);
	p256_encode(A, &P);
	r &= ~(z & t);
	return r;
}

/* see bearssl_ec.h */
const br_ec_impl br_ec_p256_i15 = {
	(uint32_t)0x00800000,
	&api_generator,
	&api_order,
	&api_mul,
	&api_mulgen,
	&api_muladd
};


/* ===== src/ec/ecdsa_i15_vrfy_asn1.c ===== */
/*
 * Copyright (c) 2017 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

#define FIELD_LEN   ((BR_MAX_EC_SIZE + 7) >> 3)

/* see bearssl_ec.h */
uint32_t
br_ecdsa_i15_vrfy_asn1(const br_ec_impl *impl,
	const void *hash, size_t hash_len,
	const br_ec_public_key *pk,
	const void *sig, size_t sig_len)
{
	/*
	 * We use a double-sized buffer because a malformed ASN.1 signature
	 * may trigger a size expansion when converting to "raw" format.
	 */
	unsigned char rsig[(FIELD_LEN << 2) + 24];

	if (sig_len > ((sizeof rsig) >> 1)) {
		return 0;
	}
	memcpy(rsig, sig, sig_len);
	sig_len = br_ecdsa_asn1_to_raw(rsig, sig_len);
	return br_ecdsa_i15_vrfy_raw(impl, hash, hash_len, pk, rsig, sig_len);
}


/* ===== src/ec/ecdsa_i15_vrfy_raw.c ===== */
/*
 * Copyright (c) 2017 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

#define I15_LEN     ((BR_MAX_EC_SIZE + 29) / 15)
#define POINT_LEN   (1 + (((BR_MAX_EC_SIZE + 7) >> 3) << 1))

/* see bearssl_ec.h */
uint32_t
br_ecdsa_i15_vrfy_raw(const br_ec_impl *impl,
	const void *hash, size_t hash_len,
	const br_ec_public_key *pk,
	const void *sig, size_t sig_len)
{
	/*
	 * IMPORTANT: this code is fit only for curves with a prime
	 * order. This is needed so that modular reduction of the X
	 * coordinate of a point can be done with a simple subtraction.
	 */
	const br_ec_curve_def *cd;
	uint16_t n[I15_LEN], r[I15_LEN], s[I15_LEN], t1[I15_LEN], t2[I15_LEN];
	unsigned char tx[(BR_MAX_EC_SIZE + 7) >> 3];
	unsigned char ty[(BR_MAX_EC_SIZE + 7) >> 3];
	unsigned char eU[POINT_LEN];
	size_t nlen, rlen, ulen;
	uint16_t n0i;
	uint32_t res;

	/*
	 * If the curve is not supported, then report an error.
	 */
	if (((impl->supported_curves >> pk->curve) & 1) == 0) {
		return 0;
	}

	/*
	 * Get the curve parameters (generator and order).
	 */
	switch (pk->curve) {
	case BR_EC_secp256r1:
		cd = &br_secp256r1;
		break;
	case BR_EC_secp384r1:
		cd = &br_secp384r1;
		break;
	case BR_EC_secp521r1:
		cd = &br_secp521r1;
		break;
	default:
		return 0;
	}

	/*
	 * Signature length must be even.
	 */
	if (sig_len & 1) {
		return 0;
	}
	rlen = sig_len >> 1;

	/*
	 * Public key point must have the proper size for this curve.
	 */
	if (pk->qlen != cd->generator_len) {
		return 0;
	}

	/*
	 * Get modulus; then decode the r and s values. They must be
	 * lower than the modulus, and s must not be null.
	 */
	nlen = cd->order_len;
	br_i15_decode(n, cd->order, nlen);
	n0i = br_i15_ninv15(n[1]);
	if (!br_i15_decode_mod(r, sig, rlen, n)) {
		return 0;
	}
	if (!br_i15_decode_mod(s, (const unsigned char *)sig + rlen, rlen, n)) {
		return 0;
	}
	if (br_i15_iszero(s)) {
		return 0;
	}

	/*
	 * Invert s. We do that with a modular exponentiation; we use
	 * the fact that for all the curves we support, the least
	 * significant byte is not 0 or 1, so we can subtract 2 without
	 * any carry to process.
	 * We also want 1/s in Montgomery representation, which can be
	 * done by converting _from_ Montgomery representation before
	 * the inversion (because (1/s)*R = 1/(s/R)).
	 */
	br_i15_from_monty(s, n, n0i);
	memcpy(tx, cd->order, nlen);
	tx[nlen - 1] -= 2;
	br_i15_modpow(s, tx, nlen, n, n0i, t1, t2);

	/*
	 * Truncate the hash to the modulus length (in bits) and reduce
	 * it modulo the curve order. The modular reduction can be done
	 * with a subtraction since the truncation already reduced the
	 * value to the modulus bit length.
	 */
	br_ecdsa_i15_bits2int(t1, hash, hash_len, n[0]);
	br_i15_sub(t1, n, br_i15_sub(t1, n, 0) ^ 1);

	/*
	 * Multiply the (truncated, reduced) hash value with 1/s, result in
	 * t2, encoded in ty.
	 */
	br_i15_montymul(t2, t1, s, n, n0i);
	br_i15_encode(ty, nlen, t2);

	/*
	 * Multiply r with 1/s, result in t1, encoded in tx.
	 */
	br_i15_montymul(t1, r, s, n, n0i);
	br_i15_encode(tx, nlen, t1);

	/*
	 * Compute the point x*Q + y*G.
	 */
	ulen = cd->generator_len;
	memcpy(eU, pk->q, ulen);
	res = impl->muladd(eU, NULL, ulen,
		tx, nlen, ty, nlen, cd->curve);

	/*
	 * Get the X coordinate, reduce modulo the curve order, and
	 * compare with the 'r' value.
	 *
	 * The modular reduction can be done with subtractions because
	 * we work with curves of prime order, so the curve order is
	 * close to the field order (Hasse's theorem).
	 */
	br_i15_zero(t1, n[0]);
	br_i15_decode(t1, &eU[1], ulen >> 1);
	t1[0] = n[0];
	br_i15_sub(t1, n, br_i15_sub(t1, n, 0) ^ 1);
	res &= ~br_i15_sub(t1, r, 1);
	res &= br_i15_iszero(t1);
	return res;
}


/* ===== src/ec/ecdsa_i15_bits.c ===== */
/*
 * Copyright (c) 2017 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

/* see inner.h */
void
br_ecdsa_i15_bits2int(uint16_t *x,
	const void *src, size_t len, uint32_t ebitlen)
{
	uint32_t bitlen, hbitlen;
	int sc;

	bitlen = ebitlen - (ebitlen >> 4);
	hbitlen = (uint32_t)len << 3;
	if (hbitlen > bitlen) {
		len = (bitlen + 7) >> 3;
		sc = (int)((hbitlen - bitlen) & 7);
	} else {
		sc = 0;
	}
	br_i15_zero(x, ebitlen);
	br_i15_decode(x, src, len);
	br_i15_rshift(x, sc);
	x[0] = ebitlen;
}


/* ===== src/symcipher/aes_common.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

static const uint32_t Rcon[] = {
	0x01000000, 0x02000000, 0x04000000, 0x08000000, 0x10000000, 0x20000000,
	0x40000000, 0x80000000, 0x1B000000, 0x36000000
};

#define S   br_aes_S

/* see inner.h */
const unsigned char br_aes_S[] = {
	0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6F, 0xC5, 0x30, 0x01, 0x67, 0x2B,
	0xFE, 0xD7, 0xAB, 0x76, 0xCA, 0x82, 0xC9, 0x7D, 0xFA, 0x59, 0x47, 0xF0,
	0xAD, 0xD4, 0xA2, 0xAF, 0x9C, 0xA4, 0x72, 0xC0, 0xB7, 0xFD, 0x93, 0x26,
	0x36, 0x3F, 0xF7, 0xCC, 0x34, 0xA5, 0xE5, 0xF1, 0x71, 0xD8, 0x31, 0x15,
	0x04, 0xC7, 0x23, 0xC3, 0x18, 0x96, 0x05, 0x9A, 0x07, 0x12, 0x80, 0xE2,
	0xEB, 0x27, 0xB2, 0x75, 0x09, 0x83, 0x2C, 0x1A, 0x1B, 0x6E, 0x5A, 0xA0,
	0x52, 0x3B, 0xD6, 0xB3, 0x29, 0xE3, 0x2F, 0x84, 0x53, 0xD1, 0x00, 0xED,
	0x20, 0xFC, 0xB1, 0x5B, 0x6A, 0xCB, 0xBE, 0x39, 0x4A, 0x4C, 0x58, 0xCF,
	0xD0, 0xEF, 0xAA, 0xFB, 0x43, 0x4D, 0x33, 0x85, 0x45, 0xF9, 0x02, 0x7F,
	0x50, 0x3C, 0x9F, 0xA8, 0x51, 0xA3, 0x40, 0x8F, 0x92, 0x9D, 0x38, 0xF5,
	0xBC, 0xB6, 0xDA, 0x21, 0x10, 0xFF, 0xF3, 0xD2, 0xCD, 0x0C, 0x13, 0xEC,
	0x5F, 0x97, 0x44, 0x17, 0xC4, 0xA7, 0x7E, 0x3D, 0x64, 0x5D, 0x19, 0x73,
	0x60, 0x81, 0x4F, 0xDC, 0x22, 0x2A, 0x90, 0x88, 0x46, 0xEE, 0xB8, 0x14,
	0xDE, 0x5E, 0x0B, 0xDB, 0xE0, 0x32, 0x3A, 0x0A, 0x49, 0x06, 0x24, 0x5C,
	0xC2, 0xD3, 0xAC, 0x62, 0x91, 0x95, 0xE4, 0x79, 0xE7, 0xC8, 0x37, 0x6D,
	0x8D, 0xD5, 0x4E, 0xA9, 0x6C, 0x56, 0xF4, 0xEA, 0x65, 0x7A, 0xAE, 0x08,
	0xBA, 0x78, 0x25, 0x2E, 0x1C, 0xA6, 0xB4, 0xC6, 0xE8, 0xDD, 0x74, 0x1F,
	0x4B, 0xBD, 0x8B, 0x8A, 0x70, 0x3E, 0xB5, 0x66, 0x48, 0x03, 0xF6, 0x0E,
	0x61, 0x35, 0x57, 0xB9, 0x86, 0xC1, 0x1D, 0x9E, 0xE1, 0xF8, 0x98, 0x11,
	0x69, 0xD9, 0x8E, 0x94, 0x9B, 0x1E, 0x87, 0xE9, 0xCE, 0x55, 0x28, 0xDF,
	0x8C, 0xA1, 0x89, 0x0D, 0xBF, 0xE6, 0x42, 0x68, 0x41, 0x99, 0x2D, 0x0F,
	0xB0, 0x54, 0xBB, 0x16
};

static uint32_t
SubWord(uint32_t x)
{
	return ((uint32_t)S[x >> 24] << 24)
		| ((uint32_t)S[(x >> 16) & 0xFF] << 16)
		| ((uint32_t)S[(x >> 8) & 0xFF] << 8)
		| (uint32_t)S[x & 0xFF];
}

/* see inner.h */
unsigned
br_aes_keysched(uint32_t *skey, const void *key, size_t key_len)
{
	unsigned num_rounds;
	int i, j, k, nk, nkf;

	switch (key_len) {
	case 16:
		num_rounds = 10;
		break;
	case 24:
		num_rounds = 12;
		break;
	case 32:
		num_rounds = 14;
		break;
	default:
		/* abort(); */
		return 0;
	}
	nk = (int)(key_len >> 2);
	nkf = (int)((num_rounds + 1) << 2);
	for (i = 0; i < nk; i ++) {
		skey[i] = br_dec32be((const unsigned char *)key + (i << 2));
	}
	for (i = nk, j = 0, k = 0; i < nkf; i ++) {
		uint32_t tmp;

		tmp = skey[i - 1];
		if (j == 0) {
			tmp = (tmp << 8) | (tmp >> 24);
			tmp = SubWord(tmp) ^ Rcon[k];
		} else if (nk > 6 && j == 4) {
			tmp = SubWord(tmp);
		}
		skey[i] = skey[i - nk] ^ tmp;
		if (++ j == nk) {
			j = 0;
			k ++;
		}
	}
	return num_rounds;
}


/* ===== src/symcipher/aes_ct.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

/* see inner.h */
void
br_aes_ct_bitslice_Sbox(uint32_t *q)
{
	/*
	 * This S-box implementation is a straightforward translation of
	 * the circuit described by Boyar and Peralta in "A new
	 * combinational logic minimization technique with applications
	 * to cryptology" (https://eprint.iacr.org/2009/191.pdf).
	 *
	 * Note that variables x* (input) and s* (output) are numbered
	 * in "reverse" order (x0 is the high bit, x7 is the low bit).
	 */

	uint32_t x0, x1, x2, x3, x4, x5, x6, x7;
	uint32_t y1, y2, y3, y4, y5, y6, y7, y8, y9;
	uint32_t y10, y11, y12, y13, y14, y15, y16, y17, y18, y19;
	uint32_t y20, y21;
	uint32_t z0, z1, z2, z3, z4, z5, z6, z7, z8, z9;
	uint32_t z10, z11, z12, z13, z14, z15, z16, z17;
	uint32_t t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;
	uint32_t t10, t11, t12, t13, t14, t15, t16, t17, t18, t19;
	uint32_t t20, t21, t22, t23, t24, t25, t26, t27, t28, t29;
	uint32_t t30, t31, t32, t33, t34, t35, t36, t37, t38, t39;
	uint32_t t40, t41, t42, t43, t44, t45, t46, t47, t48, t49;
	uint32_t t50, t51, t52, t53, t54, t55, t56, t57, t58, t59;
	uint32_t t60, t61, t62, t63, t64, t65, t66, t67;
	uint32_t s0, s1, s2, s3, s4, s5, s6, s7;

	x0 = q[7];
	x1 = q[6];
	x2 = q[5];
	x3 = q[4];
	x4 = q[3];
	x5 = q[2];
	x6 = q[1];
	x7 = q[0];

	/*
	 * Top linear transformation.
	 */
	y14 = x3 ^ x5;
	y13 = x0 ^ x6;
	y9 = x0 ^ x3;
	y8 = x0 ^ x5;
	t0 = x1 ^ x2;
	y1 = t0 ^ x7;
	y4 = y1 ^ x3;
	y12 = y13 ^ y14;
	y2 = y1 ^ x0;
	y5 = y1 ^ x6;
	y3 = y5 ^ y8;
	t1 = x4 ^ y12;
	y15 = t1 ^ x5;
	y20 = t1 ^ x1;
	y6 = y15 ^ x7;
	y10 = y15 ^ t0;
	y11 = y20 ^ y9;
	y7 = x7 ^ y11;
	y17 = y10 ^ y11;
	y19 = y10 ^ y8;
	y16 = t0 ^ y11;
	y21 = y13 ^ y16;
	y18 = x0 ^ y16;

	/*
	 * Non-linear section.
	 */
	t2 = y12 & y15;
	t3 = y3 & y6;
	t4 = t3 ^ t2;
	t5 = y4 & x7;
	t6 = t5 ^ t2;
	t7 = y13 & y16;
	t8 = y5 & y1;
	t9 = t8 ^ t7;
	t10 = y2 & y7;
	t11 = t10 ^ t7;
	t12 = y9 & y11;
	t13 = y14 & y17;
	t14 = t13 ^ t12;
	t15 = y8 & y10;
	t16 = t15 ^ t12;
	t17 = t4 ^ t14;
	t18 = t6 ^ t16;
	t19 = t9 ^ t14;
	t20 = t11 ^ t16;
	t21 = t17 ^ y20;
	t22 = t18 ^ y19;
	t23 = t19 ^ y21;
	t24 = t20 ^ y18;

	t25 = t21 ^ t22;
	t26 = t21 & t23;
	t27 = t24 ^ t26;
	t28 = t25 & t27;
	t29 = t28 ^ t22;
	t30 = t23 ^ t24;
	t31 = t22 ^ t26;
	t32 = t31 & t30;
	t33 = t32 ^ t24;
	t34 = t23 ^ t33;
	t35 = t27 ^ t33;
	t36 = t24 & t35;
	t37 = t36 ^ t34;
	t38 = t27 ^ t36;
	t39 = t29 & t38;
	t40 = t25 ^ t39;

	t41 = t40 ^ t37;
	t42 = t29 ^ t33;
	t43 = t29 ^ t40;
	t44 = t33 ^ t37;
	t45 = t42 ^ t41;
	z0 = t44 & y15;
	z1 = t37 & y6;
	z2 = t33 & x7;
	z3 = t43 & y16;
	z4 = t40 & y1;
	z5 = t29 & y7;
	z6 = t42 & y11;
	z7 = t45 & y17;
	z8 = t41 & y10;
	z9 = t44 & y12;
	z10 = t37 & y3;
	z11 = t33 & y4;
	z12 = t43 & y13;
	z13 = t40 & y5;
	z14 = t29 & y2;
	z15 = t42 & y9;
	z16 = t45 & y14;
	z17 = t41 & y8;

	/*
	 * Bottom linear transformation.
	 */
	t46 = z15 ^ z16;
	t47 = z10 ^ z11;
	t48 = z5 ^ z13;
	t49 = z9 ^ z10;
	t50 = z2 ^ z12;
	t51 = z2 ^ z5;
	t52 = z7 ^ z8;
	t53 = z0 ^ z3;
	t54 = z6 ^ z7;
	t55 = z16 ^ z17;
	t56 = z12 ^ t48;
	t57 = t50 ^ t53;
	t58 = z4 ^ t46;
	t59 = z3 ^ t54;
	t60 = t46 ^ t57;
	t61 = z14 ^ t57;
	t62 = t52 ^ t58;
	t63 = t49 ^ t58;
	t64 = z4 ^ t59;
	t65 = t61 ^ t62;
	t66 = z1 ^ t63;
	s0 = t59 ^ t63;
	s6 = t56 ^ ~t62;
	s7 = t48 ^ ~t60;
	t67 = t64 ^ t65;
	s3 = t53 ^ t66;
	s4 = t51 ^ t66;
	s5 = t47 ^ t65;
	s1 = t64 ^ ~s3;
	s2 = t55 ^ ~t67;

	q[7] = s0;
	q[6] = s1;
	q[5] = s2;
	q[4] = s3;
	q[3] = s4;
	q[2] = s5;
	q[1] = s6;
	q[0] = s7;
}

/* see inner.h */
void
br_aes_ct_ortho(uint32_t *q)
{
#define SWAPN(cl, ch, s, x, y)   do { \
		uint32_t a, b; \
		a = (x); \
		b = (y); \
		(x) = (a & (uint32_t)cl) | ((b & (uint32_t)cl) << (s)); \
		(y) = ((a & (uint32_t)ch) >> (s)) | (b & (uint32_t)ch); \
	} while (0)

#define SWAP2(x, y)   SWAPN(0x55555555, 0xAAAAAAAA, 1, x, y)
#define SWAP4(x, y)   SWAPN(0x33333333, 0xCCCCCCCC, 2, x, y)
#define SWAP8(x, y)   SWAPN(0x0F0F0F0F, 0xF0F0F0F0, 4, x, y)

	SWAP2(q[0], q[1]);
	SWAP2(q[2], q[3]);
	SWAP2(q[4], q[5]);
	SWAP2(q[6], q[7]);

	SWAP4(q[0], q[2]);
	SWAP4(q[1], q[3]);
	SWAP4(q[4], q[6]);
	SWAP4(q[5], q[7]);

	SWAP8(q[0], q[4]);
	SWAP8(q[1], q[5]);
	SWAP8(q[2], q[6]);
	SWAP8(q[3], q[7]);
}

static const unsigned char Rcon[] = {
	0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1B, 0x36
};

static uint32_t
sub_word(uint32_t x)
{
	uint32_t q[8];
	int i;

	for (i = 0; i < 8; i ++) {
		q[i] = x;
	}
	br_aes_ct_ortho(q);
	br_aes_ct_bitslice_Sbox(q);
	br_aes_ct_ortho(q);
	return q[0];
}

/* see inner.h */
unsigned
br_aes_ct_keysched(uint32_t *comp_skey, const void *key, size_t key_len)
{
	unsigned num_rounds;
	int i, j, k, nk, nkf;
	uint32_t tmp;
	uint32_t skey[120];

	switch (key_len) {
	case 16:
		num_rounds = 10;
		break;
	case 24:
		num_rounds = 12;
		break;
	case 32:
		num_rounds = 14;
		break;
	default:
		/* abort(); */
		return 0;
	}
	nk = (int)(key_len >> 2);
	nkf = (int)((num_rounds + 1) << 2);
	tmp = 0;
	for (i = 0; i < nk; i ++) {
		tmp = br_dec32le((const unsigned char *)key + (i << 2));
		skey[(i << 1) + 0] = tmp;
		skey[(i << 1) + 1] = tmp;
	}
	for (i = nk, j = 0, k = 0; i < nkf; i ++) {
		if (j == 0) {
			tmp = (tmp << 24) | (tmp >> 8);
			tmp = sub_word(tmp) ^ Rcon[k];
		} else if (nk > 6 && j == 4) {
			tmp = sub_word(tmp);
		}
		tmp ^= skey[(i - nk) << 1];
		skey[(i << 1) + 0] = tmp;
		skey[(i << 1) + 1] = tmp;
		if (++ j == nk) {
			j = 0;
			k ++;
		}
	}
	for (i = 0; i < nkf; i += 4) {
		br_aes_ct_ortho(skey + (i << 1));
	}
	for (i = 0, j = 0; i < nkf; i ++, j += 2) {
		comp_skey[i] = (skey[j + 0] & 0x55555555)
			| (skey[j + 1] & 0xAAAAAAAA);
	}
	return num_rounds;
}

/* see inner.h */
void
br_aes_ct_skey_expand(uint32_t *skey,
	unsigned num_rounds, const uint32_t *comp_skey)
{
	unsigned u, v, n;

	n = (num_rounds + 1) << 2;
	for (u = 0, v = 0; u < n; u ++, v += 2) {
		uint32_t x, y;

		x = y = comp_skey[u];
		x &= 0x55555555;
		skey[v + 0] = x | (x << 1);
		y &= 0xAAAAAAAA;
		skey[v + 1] = y | (y >> 1);
	}
}


/* ===== src/symcipher/aes_ct_enc.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

static inline void
add_round_key(uint32_t *q, const uint32_t *sk)
{
	q[0] ^= sk[0];
	q[1] ^= sk[1];
	q[2] ^= sk[2];
	q[3] ^= sk[3];
	q[4] ^= sk[4];
	q[5] ^= sk[5];
	q[6] ^= sk[6];
	q[7] ^= sk[7];
}

static inline void
shift_rows(uint32_t *q)
{
	int i;

	for (i = 0; i < 8; i ++) {
		uint32_t x;

		x = q[i];
		q[i] = (x & 0x000000FF)
			| ((x & 0x0000FC00) >> 2) | ((x & 0x00000300) << 6)
			| ((x & 0x00F00000) >> 4) | ((x & 0x000F0000) << 4)
			| ((x & 0xC0000000) >> 6) | ((x & 0x3F000000) << 2);
	}
}

static inline uint32_t
rotr16(uint32_t x)
{
	return (x << 16) | (x >> 16);
}

static inline void
mix_columns(uint32_t *q)
{
	uint32_t q0, q1, q2, q3, q4, q5, q6, q7;
	uint32_t r0, r1, r2, r3, r4, r5, r6, r7;

	q0 = q[0];
	q1 = q[1];
	q2 = q[2];
	q3 = q[3];
	q4 = q[4];
	q5 = q[5];
	q6 = q[6];
	q7 = q[7];
	r0 = (q0 >> 8) | (q0 << 24);
	r1 = (q1 >> 8) | (q1 << 24);
	r2 = (q2 >> 8) | (q2 << 24);
	r3 = (q3 >> 8) | (q3 << 24);
	r4 = (q4 >> 8) | (q4 << 24);
	r5 = (q5 >> 8) | (q5 << 24);
	r6 = (q6 >> 8) | (q6 << 24);
	r7 = (q7 >> 8) | (q7 << 24);

	q[0] = q7 ^ r7 ^ r0 ^ rotr16(q0 ^ r0);
	q[1] = q0 ^ r0 ^ q7 ^ r7 ^ r1 ^ rotr16(q1 ^ r1);
	q[2] = q1 ^ r1 ^ r2 ^ rotr16(q2 ^ r2);
	q[3] = q2 ^ r2 ^ q7 ^ r7 ^ r3 ^ rotr16(q3 ^ r3);
	q[4] = q3 ^ r3 ^ q7 ^ r7 ^ r4 ^ rotr16(q4 ^ r4);
	q[5] = q4 ^ r4 ^ r5 ^ rotr16(q5 ^ r5);
	q[6] = q5 ^ r5 ^ r6 ^ rotr16(q6 ^ r6);
	q[7] = q6 ^ r6 ^ r7 ^ rotr16(q7 ^ r7);
}

/* see inner.h */
void
br_aes_ct_bitslice_encrypt(unsigned num_rounds,
	const uint32_t *skey, uint32_t *q)
{
	unsigned u;

	add_round_key(q, skey);
	for (u = 1; u < num_rounds; u ++) {
		br_aes_ct_bitslice_Sbox(q);
		shift_rows(q);
		mix_columns(q);
		add_round_key(q, skey + (u << 3));
	}
	br_aes_ct_bitslice_Sbox(q);
	shift_rows(q);
	add_round_key(q, skey + (num_rounds << 3));
}


/* ===== src/symcipher/aes_ct_dec.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

/* see inner.h */
void
br_aes_ct_bitslice_invSbox(uint32_t *q)
{
	/*
	 * AES S-box is:
	 *   S(x) = A(I(x)) ^ 0x63
	 * where I() is inversion in GF(256), and A() is a linear
	 * transform (0 is formally defined to be its own inverse).
	 * Since inversion is an involution, the inverse S-box can be
	 * computed from the S-box as:
	 *   iS(x) = B(S(B(x ^ 0x63)) ^ 0x63)
	 * where B() is the inverse of A(). Indeed, for any y in GF(256):
	 *   iS(S(y)) = B(A(I(B(A(I(y)) ^ 0x63 ^ 0x63))) ^ 0x63 ^ 0x63) = y
	 *
	 * Note: we reuse the implementation of the forward S-box,
	 * instead of duplicating it here, so that total code size is
	 * lower. By merging the B() transforms into the S-box circuit
	 * we could make faster CBC decryption, but CBC decryption is
	 * already quite faster than CBC encryption because we can
	 * process two blocks in parallel.
	 */
	uint32_t q0, q1, q2, q3, q4, q5, q6, q7;

	q0 = ~q[0];
	q1 = ~q[1];
	q2 = q[2];
	q3 = q[3];
	q4 = q[4];
	q5 = ~q[5];
	q6 = ~q[6];
	q7 = q[7];
	q[7] = q1 ^ q4 ^ q6;
	q[6] = q0 ^ q3 ^ q5;
	q[5] = q7 ^ q2 ^ q4;
	q[4] = q6 ^ q1 ^ q3;
	q[3] = q5 ^ q0 ^ q2;
	q[2] = q4 ^ q7 ^ q1;
	q[1] = q3 ^ q6 ^ q0;
	q[0] = q2 ^ q5 ^ q7;

	br_aes_ct_bitslice_Sbox(q);

	q0 = ~q[0];
	q1 = ~q[1];
	q2 = q[2];
	q3 = q[3];
	q4 = q[4];
	q5 = ~q[5];
	q6 = ~q[6];
	q7 = q[7];
	q[7] = q1 ^ q4 ^ q6;
	q[6] = q0 ^ q3 ^ q5;
	q[5] = q7 ^ q2 ^ q4;
	q[4] = q6 ^ q1 ^ q3;
	q[3] = q5 ^ q0 ^ q2;
	q[2] = q4 ^ q7 ^ q1;
	q[1] = q3 ^ q6 ^ q0;
	q[0] = q2 ^ q5 ^ q7;
}

static void
add_round_key(uint32_t *q, const uint32_t *sk)
{
	int i;

	for (i = 0; i < 8; i ++) {
		q[i] ^= sk[i];
	}
}

static void
inv_shift_rows(uint32_t *q)
{
	int i;

	for (i = 0; i < 8; i ++) {
		uint32_t x;

		x = q[i];
		q[i] = (x & 0x000000FF)
			| ((x & 0x00003F00) << 2) | ((x & 0x0000C000) >> 6)
			| ((x & 0x000F0000) << 4) | ((x & 0x00F00000) >> 4)
			| ((x & 0x03000000) << 6) | ((x & 0xFC000000) >> 2);
	}
}

static inline uint32_t
rotr16(uint32_t x)
{
	return (x << 16) | (x >> 16);
}

static void
inv_mix_columns(uint32_t *q)
{
	uint32_t q0, q1, q2, q3, q4, q5, q6, q7;
	uint32_t r0, r1, r2, r3, r4, r5, r6, r7;

	q0 = q[0];
	q1 = q[1];
	q2 = q[2];
	q3 = q[3];
	q4 = q[4];
	q5 = q[5];
	q6 = q[6];
	q7 = q[7];
	r0 = (q0 >> 8) | (q0 << 24);
	r1 = (q1 >> 8) | (q1 << 24);
	r2 = (q2 >> 8) | (q2 << 24);
	r3 = (q3 >> 8) | (q3 << 24);
	r4 = (q4 >> 8) | (q4 << 24);
	r5 = (q5 >> 8) | (q5 << 24);
	r6 = (q6 >> 8) | (q6 << 24);
	r7 = (q7 >> 8) | (q7 << 24);

	q[0] = q5 ^ q6 ^ q7 ^ r0 ^ r5 ^ r7 ^ rotr16(q0 ^ q5 ^ q6 ^ r0 ^ r5);
	q[1] = q0 ^ q5 ^ r0 ^ r1 ^ r5 ^ r6 ^ r7 ^ rotr16(q1 ^ q5 ^ q7 ^ r1 ^ r5 ^ r6);
	q[2] = q0 ^ q1 ^ q6 ^ r1 ^ r2 ^ r6 ^ r7 ^ rotr16(q0 ^ q2 ^ q6 ^ r2 ^ r6 ^ r7);
	q[3] = q0 ^ q1 ^ q2 ^ q5 ^ q6 ^ r0 ^ r2 ^ r3 ^ r5 ^ rotr16(q0 ^ q1 ^ q3 ^ q5 ^ q6 ^ q7 ^ r0 ^ r3 ^ r5 ^ r7);
	q[4] = q1 ^ q2 ^ q3 ^ q5 ^ r1 ^ r3 ^ r4 ^ r5 ^ r6 ^ r7 ^ rotr16(q1 ^ q2 ^ q4 ^ q5 ^ q7 ^ r1 ^ r4 ^ r5 ^ r6);
	q[5] = q2 ^ q3 ^ q4 ^ q6 ^ r2 ^ r4 ^ r5 ^ r6 ^ r7 ^ rotr16(q2 ^ q3 ^ q5 ^ q6 ^ r2 ^ r5 ^ r6 ^ r7);
	q[6] = q3 ^ q4 ^ q5 ^ q7 ^ r3 ^ r5 ^ r6 ^ r7 ^ rotr16(q3 ^ q4 ^ q6 ^ q7 ^ r3 ^ r6 ^ r7);
	q[7] = q4 ^ q5 ^ q6 ^ r4 ^ r6 ^ r7 ^ rotr16(q4 ^ q5 ^ q7 ^ r4 ^ r7);
}

/* see inner.h */
void
br_aes_ct_bitslice_decrypt(unsigned num_rounds,
	const uint32_t *skey, uint32_t *q)
{
	unsigned u;

	add_round_key(q, skey + (num_rounds << 3));
	for (u = num_rounds - 1; u > 0; u --) {
		inv_shift_rows(q);
		br_aes_ct_bitslice_invSbox(q);
		add_round_key(q, skey + (u << 3));
		inv_mix_columns(q);
	}
	inv_shift_rows(q);
	br_aes_ct_bitslice_invSbox(q);
	add_round_key(q, skey);
}


/* ===== src/symcipher/aes_ct_ctr.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

/* see bearssl_block.h */
void
br_aes_ct_ctr_init(br_aes_ct_ctr_keys *ctx,
	const void *key, size_t len)
{
	ctx->vtable = &br_aes_ct_ctr_vtable;
	ctx->num_rounds = br_aes_ct_keysched(ctx->skey, key, len);
}

static void
xorbuf(void *dst, const void *src, size_t len)
{
	unsigned char *d;
	const unsigned char *s;

	d = dst;
	s = src;
	while (len -- > 0) {
		*d ++ ^= *s ++;
	}
}

/* see bearssl_block.h */
uint32_t
br_aes_ct_ctr_run(const br_aes_ct_ctr_keys *ctx,
	const void *iv, uint32_t cc, void *data, size_t len)
{
	unsigned char *buf;
	const unsigned char *ivbuf;
	uint32_t iv0, iv1, iv2;
	uint32_t sk_exp[120];

	br_aes_ct_skey_expand(sk_exp, ctx->num_rounds, ctx->skey);
	ivbuf = iv;
	iv0 = br_dec32le(ivbuf);
	iv1 = br_dec32le(ivbuf + 4);
	iv2 = br_dec32le(ivbuf + 8);
	buf = data;
	while (len > 0) {
		uint32_t q[8];
		unsigned char tmp[32];

		/*
		 * TODO: see if we can save on the first br_aes_ct_ortho()
		 * call, since iv0/iv1/iv2 are constant for the whole run.
		 */
		q[0] = q[1] = iv0;
		q[2] = q[3] = iv1;
		q[4] = q[5] = iv2;
		q[6] = br_swap32(cc);
		q[7] = br_swap32(cc + 1);
		br_aes_ct_ortho(q);
		br_aes_ct_bitslice_encrypt(ctx->num_rounds, sk_exp, q);
		br_aes_ct_ortho(q);
		br_enc32le(tmp, q[0]);
		br_enc32le(tmp + 4, q[2]);
		br_enc32le(tmp + 8, q[4]);
		br_enc32le(tmp + 12, q[6]);
		br_enc32le(tmp + 16, q[1]);
		br_enc32le(tmp + 20, q[3]);
		br_enc32le(tmp + 24, q[5]);
		br_enc32le(tmp + 28, q[7]);

		if (len <= 32) {
			xorbuf(buf, tmp, len);
			cc ++;
			if (len > 16) {
				cc ++;
			}
			break;
		}
		xorbuf(buf, tmp, 32);
		buf += 32;
		len -= 32;
		cc += 2;
	}
	return cc;
}

/* see bearssl_block.h */
const br_block_ctr_class br_aes_ct_ctr_vtable = {
	sizeof(br_aes_ct_ctr_keys),
	16,
	4,
	(void (*)(const br_block_ctr_class **, const void *, size_t))
		&br_aes_ct_ctr_init,
	(uint32_t (*)(const br_block_ctr_class *const *,
		const void *, uint32_t, void *, size_t))
		&br_aes_ct_ctr_run
};


/* ===== src/symcipher/aes_ct_cbcdec.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

/* see bearssl_block.h */
void
br_aes_ct_cbcdec_init(br_aes_ct_cbcdec_keys *ctx,
	const void *key, size_t len)
{
	ctx->vtable = &br_aes_ct_cbcdec_vtable;
	ctx->num_rounds = br_aes_ct_keysched(ctx->skey, key, len);
}

/* see bearssl_block.h */
void
br_aes_ct_cbcdec_run(const br_aes_ct_cbcdec_keys *ctx,
	void *iv, void *data, size_t len)
{
	unsigned char *buf, *ivbuf;
	uint32_t iv0, iv1, iv2, iv3;
	uint32_t sk_exp[120];

	br_aes_ct_skey_expand(sk_exp, ctx->num_rounds, ctx->skey);
	ivbuf = iv;
	iv0 = br_dec32le(ivbuf);
	iv1 = br_dec32le(ivbuf + 4);
	iv2 = br_dec32le(ivbuf + 8);
	iv3 = br_dec32le(ivbuf + 12);
	buf = data;
	while (len > 0) {
		uint32_t q[8], sq[8];

		q[0] = br_dec32le(buf);
		q[2] = br_dec32le(buf + 4);
		q[4] = br_dec32le(buf + 8);
		q[6] = br_dec32le(buf + 12);
		if (len >= 32) {
			q[1] = br_dec32le(buf + 16);
			q[3] = br_dec32le(buf + 20);
			q[5] = br_dec32le(buf + 24);
			q[7] = br_dec32le(buf + 28);
		} else {
			q[1] = 0;
			q[3] = 0;
			q[5] = 0;
			q[7] = 0;
		}
		memcpy(sq, q, sizeof q);
		br_aes_ct_ortho(q);
		br_aes_ct_bitslice_decrypt(ctx->num_rounds, sk_exp, q);
		br_aes_ct_ortho(q);
		br_enc32le(buf, q[0] ^ iv0);
		br_enc32le(buf + 4, q[2] ^ iv1);
		br_enc32le(buf + 8, q[4] ^ iv2);
		br_enc32le(buf + 12, q[6] ^ iv3);
		if (len < 32) {
			iv0 = sq[0];
			iv1 = sq[2];
			iv2 = sq[4];
			iv3 = sq[6];
			break;
		}
		br_enc32le(buf + 16, q[1] ^ sq[0]);
		br_enc32le(buf + 20, q[3] ^ sq[2]);
		br_enc32le(buf + 24, q[5] ^ sq[4]);
		br_enc32le(buf + 28, q[7] ^ sq[6]);
		iv0 = sq[1];
		iv1 = sq[3];
		iv2 = sq[5];
		iv3 = sq[7];
		buf += 32;
		len -= 32;
	}
	br_enc32le(ivbuf, iv0);
	br_enc32le(ivbuf + 4, iv1);
	br_enc32le(ivbuf + 8, iv2);
	br_enc32le(ivbuf + 12, iv3);
}

/* see bearssl_block.h */
const br_block_cbcdec_class br_aes_ct_cbcdec_vtable = {
	sizeof(br_aes_ct_cbcdec_keys),
	16,
	4,
	(void (*)(const br_block_cbcdec_class **, const void *, size_t))
		&br_aes_ct_cbcdec_init,
	(void (*)(const br_block_cbcdec_class *const *, void *, void *, size_t))
		&br_aes_ct_cbcdec_run
};


/* ===== src/symcipher/aes_ct_cbcenc.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

/* see bearssl_block.h */
void
br_aes_ct_cbcenc_init(br_aes_ct_cbcenc_keys *ctx,
	const void *key, size_t len)
{
	ctx->vtable = &br_aes_ct_cbcenc_vtable;
	ctx->num_rounds = br_aes_ct_keysched(ctx->skey, key, len);
}

/* see bearssl_block.h */
void
br_aes_ct_cbcenc_run(const br_aes_ct_cbcenc_keys *ctx,
	void *iv, void *data, size_t len)
{
	unsigned char *buf, *ivbuf;
	uint32_t q[8];
	uint32_t iv0, iv1, iv2, iv3;
	uint32_t sk_exp[120];

	q[1] = 0;
	q[3] = 0;
	q[5] = 0;
	q[7] = 0;
	br_aes_ct_skey_expand(sk_exp, ctx->num_rounds, ctx->skey);
	ivbuf = iv;
	iv0 = br_dec32le(ivbuf);
	iv1 = br_dec32le(ivbuf + 4);
	iv2 = br_dec32le(ivbuf + 8);
	iv3 = br_dec32le(ivbuf + 12);
	buf = data;
	while (len > 0) {
		q[0] = iv0 ^ br_dec32le(buf);
		q[2] = iv1 ^ br_dec32le(buf + 4);
		q[4] = iv2 ^ br_dec32le(buf + 8);
		q[6] = iv3 ^ br_dec32le(buf + 12);
		br_aes_ct_ortho(q);
		br_aes_ct_bitslice_encrypt(ctx->num_rounds, sk_exp, q);
		br_aes_ct_ortho(q);
		iv0 = q[0];
		iv1 = q[2];
		iv2 = q[4];
		iv3 = q[6];
		br_enc32le(buf, iv0);
		br_enc32le(buf + 4, iv1);
		br_enc32le(buf + 8, iv2);
		br_enc32le(buf + 12, iv3);
		buf += 16;
		len -= 16;
	}
	br_enc32le(ivbuf, iv0);
	br_enc32le(ivbuf + 4, iv1);
	br_enc32le(ivbuf + 8, iv2);
	br_enc32le(ivbuf + 12, iv3);
}

/* see bearssl_block.h */
const br_block_cbcenc_class br_aes_ct_cbcenc_vtable = {
	sizeof(br_aes_ct_cbcenc_keys),
	16,
	4,
	(void (*)(const br_block_cbcenc_class **, const void *, size_t))
		&br_aes_ct_cbcenc_init,
	(void (*)(const br_block_cbcenc_class *const *, void *, void *, size_t))
		&br_aes_ct_cbcenc_run
};


/* ===== src/symcipher/chacha20_ct.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

/* see bearssl_block.h */
uint32_t
br_chacha20_ct_run(const void *key,
	const void *iv, uint32_t cc, void *data, size_t len)
{
	unsigned char *buf;
	uint32_t kw[8], ivw[3];
	size_t u;

	static const uint32_t CW[] = {
		0x61707865, 0x3320646e, 0x79622d32, 0x6b206574
	};

	buf = data;
	for (u = 0; u < 8; u ++) {
		kw[u] = br_dec32le((const unsigned char *)key + (u << 2));
	}
	for (u = 0; u < 3; u ++) {
		ivw[u] = br_dec32le((const unsigned char *)iv + (u << 2));
	}
	while (len > 0) {
		uint32_t state[16];
		int i;
		size_t clen;
		unsigned char tmp[64];

		memcpy(&state[0], CW, sizeof CW);
		memcpy(&state[4], kw, sizeof kw);
		state[12] = cc;
		memcpy(&state[13], ivw, sizeof ivw);
		for (i = 0; i < 10; i ++) {

#define QROUND(a, b, c, d)   do { \
		state[a] += state[b]; \
		state[d] ^= state[a]; \
		state[d] = (state[d] << 16) | (state[d] >> 16); \
		state[c] += state[d]; \
		state[b] ^= state[c]; \
		state[b] = (state[b] << 12) | (state[b] >> 20); \
		state[a] += state[b]; \
		state[d] ^= state[a]; \
		state[d] = (state[d] <<  8) | (state[d] >> 24); \
		state[c] += state[d]; \
		state[b] ^= state[c]; \
		state[b] = (state[b] <<  7) | (state[b] >> 25); \
	} while (0)

			QROUND( 0,  4,  8, 12);
			QROUND( 1,  5,  9, 13);
			QROUND( 2,  6, 10, 14);
			QROUND( 3,  7, 11, 15);
			QROUND( 0,  5, 10, 15);
			QROUND( 1,  6, 11, 12);
			QROUND( 2,  7,  8, 13);
			QROUND( 3,  4,  9, 14);

#undef QROUND

		}
		for (u = 0; u < 4; u ++) {
			br_enc32le(&tmp[u << 2], state[u] + CW[u]);
		}
		for (u = 4; u < 12; u ++) {
			br_enc32le(&tmp[u << 2], state[u] + kw[u - 4]);
		}
		br_enc32le(&tmp[48], state[12] + cc);
		for (u = 13; u < 16; u ++) {
			br_enc32le(&tmp[u << 2], state[u] + ivw[u - 13]);
		}

		clen = len < 64 ? len : 64;
		for (u = 0; u < clen; u ++) {
			buf[u] ^= tmp[u];
		}
		buf += clen;
		len -= clen;
		cc ++;
	}
	return cc;
}


/* ===== src/symcipher/poly1305_ctmul.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

/*
 * Perform the inner processing of blocks for Poly1305. The accumulator
 * and the r key are provided as arrays of 26-bit words (these words
 * are allowed to have an extra bit, i.e. use 27 bits).
 *
 * On output, all accumulator words fit on 26 bits, except acc[1], which
 * may be slightly larger (but by a very small amount only).
 */
static void
poly1305_inner(uint32_t *acc, const uint32_t *r, const void *data, size_t len)
{
	/*
	 * Implementation notes: we split the 130-bit values into five
	 * 26-bit words. This gives us some space for carries.
	 *
	 * This code is inspired from the public-domain code available
	 * on:
	 *      https://github.com/floodyberry/poly1305-donna
	 *
	 * Since we compute modulo 2^130-5, the "upper words" become
	 * low words with a factor of 5; that is, x*2^130 = x*5 mod p.
	 */
	const unsigned char *buf;
	uint32_t a0, a1, a2, a3, a4;
	uint32_t r0, r1, r2, r3, r4;
	uint32_t u1, u2, u3, u4;

	r0 = r[0];
	r1 = r[1];
	r2 = r[2];
	r3 = r[3];
	r4 = r[4];

	u1 = r1 * 5;
	u2 = r2 * 5;
	u3 = r3 * 5;
	u4 = r4 * 5;

	a0 = acc[0];
	a1 = acc[1];
	a2 = acc[2];
	a3 = acc[3];
	a4 = acc[4];

	buf = data;
	while (len > 0) {
		uint64_t w0, w1, w2, w3, w4;
		uint64_t c;
		unsigned char tmp[16];

		/*
		 * If there is a partial block, right-pad it with zeros.
		 */
		if (len < 16) {
			memset(tmp, 0, sizeof tmp);
			memcpy(tmp, buf, len);
			buf = tmp;
			len = 16;
		}

		/*
		 * Decode next block and apply the "high bit"; that value
		 * is added to the accumulator.
		 */
		a0 += br_dec32le(buf) & 0x03FFFFFF;
		a1 += (br_dec32le(buf +  3) >> 2) & 0x03FFFFFF;
		a2 += (br_dec32le(buf +  6) >> 4) & 0x03FFFFFF;
		a3 += (br_dec32le(buf +  9) >> 6) & 0x03FFFFFF;
		a4 += (br_dec32le(buf + 12) >> 8) | 0x01000000;

		/*
		 * Compute multiplication.
		 */
#define M(x, y)   ((uint64_t)(x) * (uint64_t)(y))

		w0 = M(a0, r0) + M(a1, u4) + M(a2, u3) + M(a3, u2) + M(a4, u1);
		w1 = M(a0, r1) + M(a1, r0) + M(a2, u4) + M(a3, u3) + M(a4, u2);
		w2 = M(a0, r2) + M(a1, r1) + M(a2, r0) + M(a3, u4) + M(a4, u3);
		w3 = M(a0, r3) + M(a1, r2) + M(a2, r1) + M(a3, r0) + M(a4, u4);
		w4 = M(a0, r4) + M(a1, r3) + M(a2, r2) + M(a3, r1) + M(a4, r0);

#undef M
		/*
		 * Perform some (partial) modular reduction. This step is
		 * enough to keep values in ranges such that there won't
		 * be carry overflows. Most of the reduction was done in
		 * the multiplication step (by using the 'u*' values, and
		 * using the fact that 2^130 = -5 mod p); here we perform
		 * some carry propagation.
		 */
		c = w0 >> 26;
		a0 = (uint32_t)w0 & 0x3FFFFFF;
		w1 += c;
		c = w1 >> 26;
		a1 = (uint32_t)w1 & 0x3FFFFFF;
		w2 += c;
		c = w2 >> 26;
		a2 = (uint32_t)w2 & 0x3FFFFFF;
		w3 += c;
		c = w3 >> 26;
		a3 = (uint32_t)w3 & 0x3FFFFFF;
		w4 += c;
		c = w4 >> 26;
		a4 = (uint32_t)w4 & 0x3FFFFFF;
		a0 += (uint32_t)c * 5;
		a1 += a0 >> 26;
		a0 &= 0x3FFFFFF;

		buf += 16;
		len -= 16;
	}

	acc[0] = a0;
	acc[1] = a1;
	acc[2] = a2;
	acc[3] = a3;
	acc[4] = a4;
}

/* see bearssl_block.h */
void
br_poly1305_ctmul_run(const void *key, const void *iv,
	void *data, size_t len, const void *aad, size_t aad_len,
	void *tag, br_chacha20_run ichacha, int encrypt)
{
	unsigned char pkey[32], foot[16];
	uint32_t r[5], acc[5], cc, ctl, hi;
	uint64_t w;
	int i;

	/*
	 * Compute the MAC key. The 'r' value is the first 16 bytes of
	 * pkey[].
	 */
	memset(pkey, 0, sizeof pkey);
	ichacha(key, iv, 0, pkey, sizeof pkey);

	/*
	 * If encrypting, ChaCha20 must run first, followed by Poly1305.
	 * When decrypting, the operations are reversed.
	 */
	if (encrypt) {
		ichacha(key, iv, 1, data, len);
	}

	/*
	 * Run Poly1305. We must process the AAD, then ciphertext, then
	 * the footer (with the lengths). Note that the AAD and ciphertext
	 * are meant to be padded with zeros up to the next multiple of 16,
	 * and the length of the footer is 16 bytes as well.
	 */

	/*
	 * Decode the 'r' value into 26-bit words, with the "clamping"
	 * operation applied.
	 */
	r[0] = br_dec32le(pkey) & 0x03FFFFFF;
	r[1] = (br_dec32le(pkey +  3) >> 2) & 0x03FFFF03;
	r[2] = (br_dec32le(pkey +  6) >> 4) & 0x03FFC0FF;
	r[3] = (br_dec32le(pkey +  9) >> 6) & 0x03F03FFF;
	r[4] = (br_dec32le(pkey + 12) >> 8) & 0x000FFFFF;

	/*
	 * Accumulator is 0.
	 */
	memset(acc, 0, sizeof acc);

	/*
	 * Process the additional authenticated data, ciphertext, and
	 * footer in due order.
	 */
	br_enc64le(foot, (uint64_t)aad_len);
	br_enc64le(foot + 8, (uint64_t)len);
	poly1305_inner(acc, r, aad, aad_len);
	poly1305_inner(acc, r, data, len);
	poly1305_inner(acc, r, foot, sizeof foot);

	/*
	 * Finalise modular reduction. This is done with carry propagation
	 * and applying the '2^130 = -5 mod p' rule. Note that the output
	 * of poly1035_inner() is already mostly reduced, since only
	 * acc[1] may be (very slightly) above 2^26. A single loop back
	 * to acc[1] will be enough to make the value fit in 130 bits.
	 */
	cc = 0;
	for (i = 1; i <= 6; i ++) {
		int j;

		j = (i >= 5) ? i - 5 : i;
		acc[j] += cc;
		cc = acc[j] >> 26;
		acc[j] &= 0x03FFFFFF;
	}

	/*
	 * We may still have a value in the 2^130-5..2^130-1 range, in
	 * which case we must reduce it again. The code below selects,
	 * in constant-time, between 'acc' and 'acc-p',
	 */
	ctl = GT(acc[0], 0x03FFFFFA);
	for (i = 1; i < 5; i ++) {
		ctl &= EQ(acc[i], 0x03FFFFFF);
	}
	cc = 5;
	for (i = 0; i < 5; i ++) {
		uint32_t t;

		t = (acc[i] + cc);
		cc = t >> 26;
		t &= 0x03FFFFFF;
		acc[i] = MUX(ctl, t, acc[i]);
	}

	/*
	 * Convert back the accumulator to 32-bit words, and add the
	 * 's' value (second half of pkey[]). That addition is done
	 * modulo 2^128.
	 */
	w = (uint64_t)acc[0] + ((uint64_t)acc[1] << 26) + br_dec32le(pkey + 16);
	br_enc32le((unsigned char *)tag, (uint32_t)w);
	w = (w >> 32) + ((uint64_t)acc[2] << 20) + br_dec32le(pkey + 20);
	br_enc32le((unsigned char *)tag + 4, (uint32_t)w);
	w = (w >> 32) + ((uint64_t)acc[3] << 14) + br_dec32le(pkey + 24);
	br_enc32le((unsigned char *)tag + 8, (uint32_t)w);
	hi = (uint32_t)(w >> 32) + (acc[4] << 8) + br_dec32le(pkey + 28);
	br_enc32le((unsigned char *)tag + 12, hi);

	/*
	 * If decrypting, then ChaCha20 runs _after_ Poly1305.
	 */
	if (!encrypt) {
		ichacha(key, iv, 1, data, len);
	}
}


/* ===== src/x509/x509_decoder.c ===== */
/* Automatically generated code; do not modify directly. */

#include <stddef.h>
#include <stdint.h>

typedef struct {
	uint32_t *dp;
	uint32_t *rp;
	const unsigned char *ip;
} t0_context;

static uint32_t
t0_parse7E_unsigned(const unsigned char **p)
{
	uint32_t x;

	x = 0;
	for (;;) {
		unsigned y;

		y = *(*p) ++;
		x = (x << 7) | (uint32_t)(y & 0x7F);
		if (y < 0x80) {
			return x;
		}
	}
}

static int32_t
t0_parse7E_signed(const unsigned char **p)
{
	int neg;
	uint32_t x;

	neg = ((**p) >> 6) & 1;
	x = (uint32_t)-neg;
	for (;;) {
		unsigned y;

		y = *(*p) ++;
		x = (x << 7) | (uint32_t)(y & 0x7F);
		if (y < 0x80) {
			if (neg) {
				return -(int32_t)~x - 1;
			} else {
				return (int32_t)x;
			}
		}
	}
}

#define T0_VBYTE(x, n)   (unsigned char)((((uint32_t)(x) >> (n)) & 0x7F) | 0x80)
#define T0_FBYTE(x, n)   (unsigned char)(((uint32_t)(x) >> (n)) & 0x7F)
#define T0_SBYTE(x)      (unsigned char)((((uint32_t)(x) >> 28) + 0xF8) ^ 0xF8)
#define T0_INT1(x)       T0_FBYTE(x, 0)
#define T0_INT2(x)       T0_VBYTE(x, 7), T0_FBYTE(x, 0)
#define T0_INT3(x)       T0_VBYTE(x, 14), T0_VBYTE(x, 7), T0_FBYTE(x, 0)
#define T0_INT4(x)       T0_VBYTE(x, 21), T0_VBYTE(x, 14), T0_VBYTE(x, 7), T0_FBYTE(x, 0)
#define T0_INT5(x)       T0_SBYTE(x), T0_VBYTE(x, 21), T0_VBYTE(x, 14), T0_VBYTE(x, 7), T0_FBYTE(x, 0)

static const uint8_t t0_datablock[];


void br_x509_decoder_init_main(void *t0ctx);

void br_x509_decoder_run(void *t0ctx);



/* (already included) */





/* (already included) */

#define CTX   ((br_x509_decoder_context *)((unsigned char *)t0ctx - offsetof(br_x509_decoder_context, cpu)))
#define CONTEXT_NAME   br_x509_decoder_context

/* see bearssl_x509.h */
void
br_x509_decoder_init(br_x509_decoder_context *ctx,
	void (*append_dn)(void *ctx, const void *buf, size_t len),
	void *append_dn_ctx)
{
	memset(ctx, 0, sizeof *ctx);
	/* obsolete
	ctx->err = 0;
	ctx->hbuf = NULL;
	ctx->hlen = 0;
	*/
	ctx->append_dn = append_dn;
	ctx->append_dn_ctx = append_dn_ctx;
	ctx->cpu.dp = &ctx->dp_stack[0];
	ctx->cpu.rp = &ctx->rp_stack[0];
	br_x509_decoder_init_main(&ctx->cpu);
	br_x509_decoder_run(&ctx->cpu);
}

/* see bearssl_x509.h */
void
br_x509_decoder_push(br_x509_decoder_context *ctx,
	const void *data, size_t len)
{
	ctx->hbuf = data;
	ctx->hlen = len;
	br_x509_decoder_run(&ctx->cpu);
}



static const uint8_t t0_datablock[] = {
	0x00, 0x09, 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x01, 0x01, 0x09,
	0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x01, 0x05, 0x09, 0x2A, 0x86,
	0x48, 0x86, 0xF7, 0x0D, 0x01, 0x01, 0x0E, 0x09, 0x2A, 0x86, 0x48, 0x86,
	0xF7, 0x0D, 0x01, 0x01, 0x0B, 0x09, 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D,
	0x01, 0x01, 0x0C, 0x09, 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x01,
	0x0D, 0x07, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x02, 0x01, 0x08, 0x2A, 0x86,
	0x48, 0xCE, 0x3D, 0x03, 0x01, 0x07, 0x05, 0x2B, 0x81, 0x04, 0x00, 0x22,
	0x05, 0x2B, 0x81, 0x04, 0x00, 0x23, 0x07, 0x2A, 0x86, 0x48, 0xCE, 0x3D,
	0x04, 0x01, 0x08, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x04, 0x03, 0x01, 0x08,
	0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x04, 0x03, 0x02, 0x08, 0x2A, 0x86, 0x48,
	0xCE, 0x3D, 0x04, 0x03, 0x03, 0x08, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x04,
	0x03, 0x04, 0x00, 0x1F, 0x03, 0xFC, 0x07, 0x7F, 0x0B, 0x5E, 0x0F, 0x1F,
	0x12, 0xFE, 0x16, 0xBF, 0x1A, 0x9F, 0x1E, 0x7E, 0x22, 0x3F, 0x26, 0x1E,
	0x29, 0xDF, 0x00, 0x1F, 0x03, 0xFD, 0x07, 0x9F, 0x0B, 0x7E, 0x0F, 0x3F,
	0x13, 0x1E, 0x16, 0xDF, 0x1A, 0xBF, 0x1E, 0x9E, 0x22, 0x5F, 0x26, 0x3E,
	0x29, 0xFF, 0x03, 0x55, 0x1D, 0x13
};

static const uint8_t t0_codeblock[] = {
	0x00, 0x01, 0x00, 0x10, 0x00, 0x00, 0x01, 0x00, 0x11, 0x00, 0x00, 0x01,
	0x01, 0x09, 0x00, 0x00, 0x01, 0x01, 0x0A, 0x00, 0x00, 0x1A, 0x1A, 0x00,
	0x00, 0x01, T0_INT1(BR_ERR_X509_BAD_BOOLEAN), 0x00, 0x00, 0x01,
	T0_INT1(BR_ERR_X509_BAD_TAG_CLASS), 0x00, 0x00, 0x01,
	T0_INT1(BR_ERR_X509_BAD_TAG_VALUE), 0x00, 0x00, 0x01,
	T0_INT1(BR_ERR_X509_BAD_TIME), 0x00, 0x00, 0x01,
	T0_INT1(BR_ERR_X509_EXTRA_ELEMENT), 0x00, 0x00, 0x01,
	T0_INT1(BR_ERR_X509_INDEFINITE_LENGTH), 0x00, 0x00, 0x01,
	T0_INT1(BR_ERR_X509_INNER_TRUNC), 0x00, 0x00, 0x01,
	T0_INT1(BR_ERR_X509_LIMIT_EXCEEDED), 0x00, 0x00, 0x01,
	T0_INT1(BR_ERR_X509_NOT_CONSTRUCTED), 0x00, 0x00, 0x01,
	T0_INT1(BR_ERR_X509_NOT_PRIMITIVE), 0x00, 0x00, 0x01,
	T0_INT1(BR_ERR_X509_OVERFLOW), 0x00, 0x00, 0x01,
	T0_INT1(BR_ERR_X509_PARTIAL_BYTE), 0x00, 0x00, 0x01,
	T0_INT1(BR_ERR_X509_UNEXPECTED), 0x00, 0x00, 0x01,
	T0_INT1(BR_ERR_X509_UNSUPPORTED), 0x00, 0x00, 0x01,
	T0_INT1(BR_KEYTYPE_EC), 0x00, 0x00, 0x01, T0_INT1(BR_KEYTYPE_RSA),
	0x00, 0x00, 0x01, T0_INT2(offsetof(CONTEXT_NAME, copy_dn)), 0x00, 0x00,
	0x01, T0_INT2(offsetof(CONTEXT_NAME, decoded)), 0x00, 0x00, 0x01,
	T0_INT2(offsetof(CONTEXT_NAME, isCA)), 0x00, 0x00, 0x01,
	T0_INT2(offsetof(br_x509_decoder_context, pkey_data)), 0x01,
	T0_INT2(BR_X509_BUFSIZE_KEY), 0x00, 0x00, 0x01,
	T0_INT2(offsetof(CONTEXT_NAME, notafter_days)), 0x00, 0x00, 0x01,
	T0_INT2(offsetof(CONTEXT_NAME, notafter_seconds)), 0x00, 0x00, 0x01,
	T0_INT2(offsetof(CONTEXT_NAME, notbefore_days)), 0x00, 0x00, 0x01,
	T0_INT2(offsetof(CONTEXT_NAME, notbefore_seconds)), 0x00, 0x00, 0x01,
	T0_INT2(offsetof(CONTEXT_NAME, pad)), 0x00, 0x00, 0x01,
	T0_INT2(offsetof(CONTEXT_NAME, signer_hash_id)), 0x00, 0x00, 0x01,
	T0_INT2(offsetof(CONTEXT_NAME, signer_key_type)), 0x00, 0x00, 0x01,
	0x80, 0x45, 0x00, 0x00, 0x01, 0x80, 0x4E, 0x00, 0x00, 0x01, 0x80, 0x54,
	0x00, 0x00, 0x01, 0x81, 0x36, 0x00, 0x02, 0x03, 0x00, 0x03, 0x01, 0x1B,
	0x02, 0x01, 0x13, 0x26, 0x02, 0x00, 0x0F, 0x15, 0x00, 0x00, 0x05, 0x02,
	0x34, 0x1D, 0x00, 0x00, 0x06, 0x02, 0x35, 0x1D, 0x00, 0x00, 0x01, 0x10,
	0x4F, 0x00, 0x00, 0x11, 0x05, 0x02, 0x38, 0x1D, 0x4C, 0x00, 0x00, 0x11,
	0x05, 0x02, 0x38, 0x1D, 0x4D, 0x00, 0x00, 0x06, 0x02, 0x30, 0x1D, 0x00,
	0x00, 0x1B, 0x19, 0x01, 0x08, 0x0E, 0x26, 0x29, 0x19, 0x09, 0x00, 0x00,
	0x01, 0x30, 0x0A, 0x1B, 0x01, 0x00, 0x01, 0x09, 0x4B, 0x05, 0x02, 0x2F,
	0x1D, 0x00, 0x00, 0x20, 0x20, 0x00, 0x00, 0x01, 0x80, 0x5A, 0x00, 0x00,
	0x01, 0x80, 0x62, 0x00, 0x00, 0x01, 0x80, 0x6B, 0x00, 0x00, 0x01, 0x80,
	0x74, 0x00, 0x00, 0x01, 0x80, 0x7D, 0x00, 0x00, 0x01, 0x3D, 0x00, 0x00,
	0x20, 0x11, 0x06, 0x04, 0x2B, 0x6B, 0x7A, 0x71, 0x00, 0x04, 0x01, 0x00,
	0x3D, 0x25, 0x01, 0x00, 0x3C, 0x25, 0x01, 0x87, 0xFF, 0xFF, 0x7F, 0x6D,
	0x6D, 0x70, 0x1B, 0x01, 0x20, 0x11, 0x06, 0x11, 0x1A, 0x4C, 0x6B, 0x70,
	0x01, 0x02, 0x50, 0x6E, 0x01, 0x02, 0x12, 0x06, 0x02, 0x39, 0x1D, 0x51,
	0x70, 0x01, 0x02, 0x50, 0x6C, 0x6D, 0x7A, 0x6D, 0x7A, 0x6D, 0x65, 0x43,
	0x24, 0x42, 0x24, 0x65, 0x41, 0x24, 0x40, 0x24, 0x51, 0x01, 0x01, 0x3C,
	0x25, 0x6D, 0x7A, 0x01, 0x00, 0x3C, 0x25, 0x6D, 0x6D, 0x60, 0x05, 0x02,
	0x39, 0x1D, 0x74, 0x1C, 0x06, 0x1C, 0x7A, 0x61, 0x6D, 0x3F, 0x68, 0x03,
	0x00, 0x3F, 0x26, 0x02, 0x00, 0x09, 0x26, 0x02, 0x00, 0x0A, 0x68, 0x03,
	0x01, 0x51, 0x51, 0x02, 0x00, 0x02, 0x01, 0x18, 0x04, 0x1E, 0x5A, 0x1C,
	0x06, 0x18, 0x64, 0x03, 0x02, 0x51, 0x61, 0x1B, 0x03, 0x03, 0x1B, 0x3F,
	0x23, 0x0D, 0x06, 0x02, 0x33, 0x1D, 0x62, 0x02, 0x02, 0x02, 0x03, 0x17,
	0x04, 0x02, 0x39, 0x1D, 0x51, 0x01, 0x00, 0x3E, 0x25, 0x71, 0x01, 0x21,
	0x5B, 0x01, 0x22, 0x5B, 0x1B, 0x01, 0x23, 0x11, 0x06, 0x28, 0x1A, 0x4C,
	0x6B, 0x6D, 0x1B, 0x06, 0x1D, 0x6D, 0x60, 0x1A, 0x70, 0x1B, 0x01, 0x01,
	0x11, 0x06, 0x03, 0x63, 0x1A, 0x70, 0x01, 0x04, 0x50, 0x6B, 0x4A, 0x1C,
	0x06, 0x03, 0x5F, 0x04, 0x01, 0x7B, 0x51, 0x51, 0x04, 0x60, 0x51, 0x51,
	0x04, 0x08, 0x01, 0x7F, 0x11, 0x05, 0x02, 0x38, 0x1D, 0x1A, 0x51, 0x6D,
	0x60, 0x06, 0x80, 0x63, 0x75, 0x1C, 0x06, 0x06, 0x01, 0x02, 0x3B, 0x04,
	0x80, 0x57, 0x76, 0x1C, 0x06, 0x06, 0x01, 0x03, 0x3B, 0x04, 0x80, 0x4D,
	0x77, 0x1C, 0x06, 0x06, 0x01, 0x04, 0x3B, 0x04, 0x80, 0x43, 0x78, 0x1C,
	0x06, 0x05, 0x01, 0x05, 0x3B, 0x04, 0x3A, 0x79, 0x1C, 0x06, 0x05, 0x01,
	0x06, 0x3B, 0x04, 0x31, 0x55, 0x1C, 0x06, 0x05, 0x01, 0x02, 0x3A, 0x04,
	0x28, 0x56, 0x1C, 0x06, 0x05, 0x01, 0x03, 0x3A, 0x04, 0x1F, 0x57, 0x1C,
	0x06, 0x05, 0x01, 0x04, 0x3A, 0x04, 0x16, 0x58, 0x1C, 0x06, 0x05, 0x01,
	0x05, 0x3A, 0x04, 0x0D, 0x59, 0x1C, 0x06, 0x05, 0x01, 0x06, 0x3A, 0x04,
	0x04, 0x01, 0x00, 0x01, 0x00, 0x04, 0x04, 0x01, 0x00, 0x01, 0x00, 0x46,
	0x25, 0x45, 0x25, 0x7A, 0x61, 0x7A, 0x51, 0x1A, 0x01, 0x01, 0x3D, 0x25,
	0x73, 0x30, 0x1D, 0x00, 0x00, 0x01, 0x81, 0x06, 0x00, 0x01, 0x54, 0x0D,
	0x06, 0x02, 0x32, 0x1D, 0x1B, 0x03, 0x00, 0x0A, 0x02, 0x00, 0x00, 0x00,
	0x6D, 0x71, 0x1B, 0x01, 0x01, 0x11, 0x06, 0x08, 0x63, 0x01, 0x01, 0x15,
	0x3E, 0x25, 0x04, 0x01, 0x2B, 0x7A, 0x00, 0x00, 0x70, 0x01, 0x06, 0x50,
	0x6F, 0x00, 0x00, 0x70, 0x01, 0x03, 0x50, 0x6B, 0x72, 0x06, 0x02, 0x37,
	0x1D, 0x00, 0x00, 0x26, 0x1B, 0x06, 0x07, 0x21, 0x1B, 0x06, 0x01, 0x16,
	0x04, 0x76, 0x2B, 0x00, 0x00, 0x01, 0x01, 0x50, 0x6A, 0x01, 0x01, 0x10,
	0x06, 0x02, 0x2C, 0x1D, 0x72, 0x27, 0x00, 0x00, 0x60, 0x05, 0x02, 0x39,
	0x1D, 0x47, 0x1C, 0x06, 0x04, 0x01, 0x17, 0x04, 0x12, 0x48, 0x1C, 0x06,
	0x04, 0x01, 0x18, 0x04, 0x0A, 0x49, 0x1C, 0x06, 0x04, 0x01, 0x19, 0x04,
	0x02, 0x39, 0x1D, 0x00, 0x04, 0x70, 0x1B, 0x01, 0x17, 0x01, 0x18, 0x4B,
	0x05, 0x02, 0x2F, 0x1D, 0x01, 0x18, 0x11, 0x03, 0x00, 0x4D, 0x6B, 0x66,
	0x02, 0x00, 0x06, 0x0C, 0x01, 0x80, 0x64, 0x08, 0x03, 0x01, 0x66, 0x02,
	0x01, 0x09, 0x04, 0x0E, 0x1B, 0x01, 0x32, 0x0D, 0x06, 0x04, 0x01, 0x80,
	0x64, 0x09, 0x01, 0x8E, 0x6C, 0x09, 0x03, 0x01, 0x02, 0x01, 0x01, 0x82,
	0x6D, 0x08, 0x02, 0x01, 0x01, 0x03, 0x09, 0x01, 0x04, 0x0C, 0x09, 0x02,
	0x01, 0x01, 0x80, 0x63, 0x09, 0x01, 0x80, 0x64, 0x0C, 0x0A, 0x02, 0x01,
	0x01, 0x83, 0x0F, 0x09, 0x01, 0x83, 0x10, 0x0C, 0x09, 0x03, 0x03, 0x01,
	0x01, 0x01, 0x0C, 0x67, 0x2A, 0x01, 0x01, 0x0E, 0x02, 0x01, 0x01, 0x04,
	0x07, 0x28, 0x02, 0x01, 0x01, 0x80, 0x64, 0x07, 0x27, 0x02, 0x01, 0x01,
	0x83, 0x10, 0x07, 0x28, 0x1F, 0x15, 0x06, 0x03, 0x01, 0x18, 0x09, 0x5D,
	0x09, 0x52, 0x1B, 0x01, 0x05, 0x14, 0x02, 0x03, 0x09, 0x03, 0x03, 0x01,
	0x1F, 0x15, 0x01, 0x01, 0x26, 0x67, 0x02, 0x03, 0x09, 0x2A, 0x03, 0x03,
	0x01, 0x00, 0x01, 0x17, 0x67, 0x01, 0x9C, 0x10, 0x08, 0x03, 0x02, 0x01,
	0x00, 0x01, 0x3B, 0x67, 0x01, 0x3C, 0x08, 0x02, 0x02, 0x09, 0x03, 0x02,
	0x01, 0x00, 0x01, 0x3C, 0x67, 0x02, 0x02, 0x09, 0x03, 0x02, 0x72, 0x1B,
	0x01, 0x2E, 0x11, 0x06, 0x0D, 0x1A, 0x72, 0x1B, 0x01, 0x30, 0x01, 0x39,
	0x4B, 0x06, 0x03, 0x1A, 0x04, 0x74, 0x01, 0x80, 0x5A, 0x10, 0x06, 0x02,
	0x2F, 0x1D, 0x51, 0x02, 0x03, 0x02, 0x02, 0x00, 0x01, 0x72, 0x53, 0x01,
	0x0A, 0x08, 0x03, 0x00, 0x72, 0x53, 0x02, 0x00, 0x09, 0x00, 0x02, 0x03,
	0x00, 0x03, 0x01, 0x66, 0x1B, 0x02, 0x01, 0x02, 0x00, 0x4B, 0x05, 0x02,
	0x2F, 0x1D, 0x00, 0x00, 0x23, 0x70, 0x01, 0x02, 0x50, 0x0B, 0x69, 0x00,
	0x03, 0x1B, 0x03, 0x00, 0x03, 0x01, 0x03, 0x02, 0x6B, 0x72, 0x1B, 0x01,
	0x81, 0x00, 0x13, 0x06, 0x02, 0x36, 0x1D, 0x1B, 0x01, 0x00, 0x11, 0x06,
	0x0B, 0x1A, 0x1B, 0x05, 0x04, 0x1A, 0x01, 0x00, 0x00, 0x72, 0x04, 0x6F,
	0x02, 0x01, 0x1B, 0x05, 0x02, 0x33, 0x1D, 0x2A, 0x03, 0x01, 0x02, 0x02,
	0x25, 0x02, 0x02, 0x29, 0x03, 0x02, 0x1B, 0x06, 0x03, 0x72, 0x04, 0x68,
	0x1A, 0x02, 0x00, 0x02, 0x01, 0x0A, 0x00, 0x01, 0x72, 0x1B, 0x01, 0x81,
	0x00, 0x0D, 0x06, 0x01, 0x00, 0x01, 0x81, 0x00, 0x0A, 0x1B, 0x05, 0x02,
	0x31, 0x1D, 0x03, 0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00, 0x12, 0x06,
	0x19, 0x02, 0x00, 0x2A, 0x03, 0x00, 0x1B, 0x01, 0x83, 0xFF, 0xFF, 0x7F,
	0x12, 0x06, 0x02, 0x32, 0x1D, 0x01, 0x08, 0x0E, 0x26, 0x72, 0x23, 0x09,
	0x04, 0x60, 0x00, 0x00, 0x6A, 0x5E, 0x00, 0x00, 0x6B, 0x7A, 0x00, 0x00,
	0x70, 0x4E, 0x6B, 0x00, 0x01, 0x6B, 0x1B, 0x05, 0x02, 0x36, 0x1D, 0x72,
	0x1B, 0x01, 0x81, 0x00, 0x13, 0x06, 0x02, 0x36, 0x1D, 0x03, 0x00, 0x1B,
	0x06, 0x16, 0x72, 0x02, 0x00, 0x1B, 0x01, 0x87, 0xFF, 0xFF, 0x7F, 0x13,
	0x06, 0x02, 0x36, 0x1D, 0x01, 0x08, 0x0E, 0x09, 0x03, 0x00, 0x04, 0x67,
	0x1A, 0x02, 0x00, 0x00, 0x00, 0x6B, 0x1B, 0x01, 0x81, 0x7F, 0x12, 0x06,
	0x08, 0x7A, 0x01, 0x00, 0x44, 0x25, 0x01, 0x00, 0x00, 0x1B, 0x44, 0x25,
	0x44, 0x29, 0x62, 0x01, 0x7F, 0x00, 0x01, 0x72, 0x03, 0x00, 0x02, 0x00,
	0x01, 0x05, 0x14, 0x01, 0x01, 0x15, 0x1E, 0x02, 0x00, 0x01, 0x06, 0x14,
	0x1B, 0x01, 0x01, 0x15, 0x06, 0x02, 0x2D, 0x1D, 0x01, 0x04, 0x0E, 0x02,
	0x00, 0x01, 0x1F, 0x15, 0x1B, 0x01, 0x1F, 0x11, 0x06, 0x02, 0x2E, 0x1D,
	0x09, 0x00, 0x00, 0x1B, 0x05, 0x05, 0x01, 0x00, 0x01, 0x7F, 0x00, 0x70,
	0x00, 0x00, 0x1B, 0x05, 0x02, 0x32, 0x1D, 0x2A, 0x73, 0x00, 0x00, 0x22,
	0x1B, 0x01, 0x00, 0x13, 0x06, 0x01, 0x00, 0x1A, 0x16, 0x04, 0x74, 0x00,
	0x01, 0x01, 0x00, 0x00, 0x01, 0x0B, 0x00, 0x00, 0x01, 0x15, 0x00, 0x00,
	0x01, 0x1F, 0x00, 0x00, 0x01, 0x29, 0x00, 0x00, 0x01, 0x33, 0x00, 0x00,
	0x7B, 0x1A, 0x00, 0x00, 0x1B, 0x06, 0x07, 0x7C, 0x1B, 0x06, 0x01, 0x16,
	0x04, 0x76, 0x00, 0x00, 0x01, 0x00, 0x20, 0x21, 0x0B, 0x2B, 0x00
};

static const uint16_t t0_caddr[] = {
	0,
	5,
	10,
	15,
	20,
	24,
	28,
	32,
	36,
	40,
	44,
	48,
	52,
	56,
	60,
	64,
	68,
	72,
	76,
	80,
	84,
	88,
	93,
	98,
	103,
	111,
	116,
	121,
	126,
	131,
	136,
	141,
	146,
	151,
	156,
	161,
	166,
	181,
	187,
	193,
	198,
	206,
	214,
	220,
	231,
	246,
	250,
	255,
	260,
	265,
	270,
	275,
	279,
	289,
	620,
	625,
	639,
	659,
	666,
	678,
	692,
	707,
	740,
	960,
	974,
	991,
	1000,
	1067,
	1123,
	1127,
	1131,
	1136,
	1184,
	1210,
	1254,
	1265,
	1274,
	1287,
	1291,
	1295,
	1299,
	1303,
	1307,
	1311,
	1315,
	1327
};

#define T0_INTERPRETED   39

#define T0_ENTER(ip, rp, slot)   do { \
		const unsigned char *t0_newip; \
		uint32_t t0_lnum; \
		t0_newip = &t0_codeblock[t0_caddr[(slot) - T0_INTERPRETED]]; \
		t0_lnum = t0_parse7E_unsigned(&t0_newip); \
		(rp) += t0_lnum; \
		*((rp) ++) = (uint32_t)((ip) - &t0_codeblock[0]) + (t0_lnum << 16); \
		(ip) = t0_newip; \
	} while (0)

#define T0_DEFENTRY(name, slot) \
void \
name(void *ctx) \
{ \
	t0_context *t0ctx = ctx; \
	t0ctx->ip = &t0_codeblock[0]; \
	T0_ENTER(t0ctx->ip, t0ctx->rp, slot); \
}

T0_DEFENTRY(br_x509_decoder_init_main, 92)

#define T0_NEXT(t0ipp)   (*(*(t0ipp)) ++)

void
br_x509_decoder_run(void *t0ctx)
{
	uint32_t *dp, *rp;
	const unsigned char *ip;

#define T0_LOCAL(x)    (*(rp - 2 - (x)))
#define T0_POP()       (*-- dp)
#define T0_POPi()      (*(int32_t *)(-- dp))
#define T0_PEEK(x)     (*(dp - 1 - (x)))
#define T0_PEEKi(x)    (*(int32_t *)(dp - 1 - (x)))
#define T0_PUSH(v)     do { *dp = (v); dp ++; } while (0)
#define T0_PUSHi(v)    do { *(int32_t *)dp = (v); dp ++; } while (0)
#define T0_RPOP()      (*-- rp)
#define T0_RPOPi()     (*(int32_t *)(-- rp))
#define T0_RPUSH(v)    do { *rp = (v); rp ++; } while (0)
#define T0_RPUSHi(v)   do { *(int32_t *)rp = (v); rp ++; } while (0)
#define T0_ROLL(x)     do { \
	size_t t0len = (size_t)(x); \
	uint32_t t0tmp = *(dp - 1 - t0len); \
	memmove(dp - t0len - 1, dp - t0len, t0len * sizeof *dp); \
	*(dp - 1) = t0tmp; \
} while (0)
#define T0_SWAP()      do { \
	uint32_t t0tmp = *(dp - 2); \
	*(dp - 2) = *(dp - 1); \
	*(dp - 1) = t0tmp; \
} while (0)
#define T0_ROT()       do { \
	uint32_t t0tmp = *(dp - 3); \
	*(dp - 3) = *(dp - 2); \
	*(dp - 2) = *(dp - 1); \
	*(dp - 1) = t0tmp; \
} while (0)
#define T0_NROT()       do { \
	uint32_t t0tmp = *(dp - 1); \
	*(dp - 1) = *(dp - 2); \
	*(dp - 2) = *(dp - 3); \
	*(dp - 3) = t0tmp; \
} while (0)
#define T0_PICK(x)      do { \
	uint32_t t0depth = (x); \
	T0_PUSH(T0_PEEK(t0depth)); \
} while (0)
#define T0_CO()         do { \
	goto t0_exit; \
} while (0)
#define T0_RET()        goto t0_next

	dp = ((t0_context *)t0ctx)->dp;
	rp = ((t0_context *)t0ctx)->rp;
	ip = ((t0_context *)t0ctx)->ip;
	goto t0_next;
	for (;;) {
		uint32_t t0x;

	t0_next:
		t0x = T0_NEXT(&ip);
		if (t0x < T0_INTERPRETED) {
			switch (t0x) {
				int32_t t0off;

			case 0: /* ret */
				t0x = T0_RPOP();
				rp -= (t0x >> 16);
				t0x &= 0xFFFF;
				if (t0x == 0) {
					ip = NULL;
					goto t0_exit;
				}
				ip = &t0_codeblock[t0x];
				break;
			case 1: /* literal constant */
				T0_PUSHi(t0_parse7E_signed(&ip));
				break;
			case 2: /* read local */
				T0_PUSH(T0_LOCAL(t0_parse7E_unsigned(&ip)));
				break;
			case 3: /* write local */
				T0_LOCAL(t0_parse7E_unsigned(&ip)) = T0_POP();
				break;
			case 4: /* jump */
				t0off = t0_parse7E_signed(&ip);
				ip += t0off;
				break;
			case 5: /* jump if */
				t0off = t0_parse7E_signed(&ip);
				if (T0_POP()) {
					ip += t0off;
				}
				break;
			case 6: /* jump if not */
				t0off = t0_parse7E_signed(&ip);
				if (!T0_POP()) {
					ip += t0off;
				}
				break;
			case 7: {
				/* %25 */

	int32_t b = T0_POPi();
	int32_t a = T0_POPi();
	T0_PUSHi(a % b);

				}
				break;
			case 8: {
				/* * */

	uint32_t b = T0_POP();
	uint32_t a = T0_POP();
	T0_PUSH(a * b);

				}
				break;
			case 9: {
				/* + */

	uint32_t b = T0_POP();
	uint32_t a = T0_POP();
	T0_PUSH(a + b);

				}
				break;
			case 10: {
				/* - */

	uint32_t b = T0_POP();
	uint32_t a = T0_POP();
	T0_PUSH(a - b);

				}
				break;
			case 11: {
				/* -rot */
 T0_NROT(); 
				}
				break;
			case 12: {
				/* / */

	int32_t b = T0_POPi();
	int32_t a = T0_POPi();
	T0_PUSHi(a / b);

				}
				break;
			case 13: {
				/* < */

	int32_t b = T0_POPi();
	int32_t a = T0_POPi();
	T0_PUSH(-(uint32_t)(a < b));

				}
				break;
			case 14: {
				/* << */

	int c = (int)T0_POPi();
	uint32_t x = T0_POP();
	T0_PUSH(x << c);

				}
				break;
			case 15: {
				/* <= */

	int32_t b = T0_POPi();
	int32_t a = T0_POPi();
	T0_PUSH(-(uint32_t)(a <= b));

				}
				break;
			case 16: {
				/* <> */

	uint32_t b = T0_POP();
	uint32_t a = T0_POP();
	T0_PUSH(-(uint32_t)(a != b));

				}
				break;
			case 17: {
				/* = */

	uint32_t b = T0_POP();
	uint32_t a = T0_POP();
	T0_PUSH(-(uint32_t)(a == b));

				}
				break;
			case 18: {
				/* > */

	int32_t b = T0_POPi();
	int32_t a = T0_POPi();
	T0_PUSH(-(uint32_t)(a > b));

				}
				break;
			case 19: {
				/* >= */

	int32_t b = T0_POPi();
	int32_t a = T0_POPi();
	T0_PUSH(-(uint32_t)(a >= b));

				}
				break;
			case 20: {
				/* >> */

	int c = (int)T0_POPi();
	int32_t x = T0_POPi();
	T0_PUSHi(x >> c);

				}
				break;
			case 21: {
				/* and */

	uint32_t b = T0_POP();
	uint32_t a = T0_POP();
	T0_PUSH(a & b);

				}
				break;
			case 22: {
				/* co */
 T0_CO(); 
				}
				break;
			case 23: {
				/* copy-ec-pkey */

	size_t qlen = T0_POP();
	uint32_t curve = T0_POP();
	CTX->pkey.key_type = BR_KEYTYPE_EC;
	CTX->pkey.key.ec.curve = curve;
	CTX->pkey.key.ec.q = CTX->pkey_data;
	CTX->pkey.key.ec.qlen = qlen;

				}
				break;
			case 24: {
				/* copy-rsa-pkey */

	size_t elen = T0_POP();
	size_t nlen = T0_POP();
	CTX->pkey.key_type = BR_KEYTYPE_RSA;
	CTX->pkey.key.rsa.n = CTX->pkey_data;
	CTX->pkey.key.rsa.nlen = nlen;
	CTX->pkey.key.rsa.e = CTX->pkey_data + nlen;
	CTX->pkey.key.rsa.elen = elen;

				}
				break;
			case 25: {
				/* data-get8 */

	size_t addr = T0_POP();
	T0_PUSH(t0_datablock[addr]);

				}
				break;
			case 26: {
				/* drop */
 (void)T0_POP(); 
				}
				break;
			case 27: {
				/* dup */
 T0_PUSH(T0_PEEK(0)); 
				}
				break;
			case 28: {
				/* eqOID */

	const unsigned char *a2 = &t0_datablock[T0_POP()];
	const unsigned char *a1 = &CTX->pad[0];
	size_t len = a1[0];
	int x;
	if (len == a2[0]) {
		x = -(memcmp(a1 + 1, a2 + 1, len) == 0);
	} else {
		x = 0;
	}
	T0_PUSH((uint32_t)x);

				}
				break;
			case 29: {
				/* fail */

	CTX->err = T0_POPi();
	T0_CO();

				}
				break;
			case 30: {
				/* neg */

	uint32_t a = T0_POP();
	T0_PUSH(-a);

				}
				break;
			case 31: {
				/* or */

	uint32_t b = T0_POP();
	uint32_t a = T0_POP();
	T0_PUSH(a | b);

				}
				break;
			case 32: {
				/* over */
 T0_PUSH(T0_PEEK(1)); 
				}
				break;
			case 33: {
				/* read-blob-inner */

	uint32_t len = T0_POP();
	uint32_t addr = T0_POP();
	size_t clen = CTX->hlen;
	if (clen > len) {
		clen = (size_t)len;
	}
	if (addr != 0) {
		memcpy((unsigned char *)CTX + addr, CTX->hbuf, clen);
	}
	if (CTX->copy_dn && CTX->append_dn) {
		CTX->append_dn(CTX->append_dn_ctx, CTX->hbuf, clen);
	}
	CTX->hbuf += clen;
	CTX->hlen -= clen;
	T0_PUSH(addr + clen);
	T0_PUSH(len - clen);

				}
				break;
			case 34: {
				/* read8-low */

	if (CTX->hlen == 0) {
		T0_PUSHi(-1);
	} else {
		unsigned char x = *CTX->hbuf ++;
		if (CTX->copy_dn && CTX->append_dn) {
			CTX->append_dn(CTX->append_dn_ctx, &x, 1);
		}
		CTX->hlen --;
		T0_PUSH(x);
	}

				}
				break;
			case 35: {
				/* rot */
 T0_ROT(); 
				}
				break;
			case 36: {
				/* set32 */

	uint32_t addr = T0_POP();
	*(uint32_t *)((unsigned char *)CTX + addr) = T0_POP();

				}
				break;
			case 37: {
				/* set8 */

	uint32_t addr = T0_POP();
	*((unsigned char *)CTX + addr) = (unsigned char)T0_POP();

				}
				break;
			case 38: {
				/* swap */
 T0_SWAP(); 
				}
				break;
			}

		} else {
			T0_ENTER(ip, rp, t0x);
		}
	}
t0_exit:
	((t0_context *)t0ctx)->dp = dp;
	((t0_context *)t0ctx)->rp = rp;
	((t0_context *)t0ctx)->ip = ip;
}


/* ===== src/x509/x509_minimal.c ===== */
/* Automatically generated code; do not modify directly. */

#include <stddef.h>
#include <stdint.h>

typedef struct {
	uint32_t *dp;
	uint32_t *rp;
	const unsigned char *ip;
} t0_context;

static uint32_t
t0_parse7E_unsigned(const unsigned char **p)
{
	uint32_t x;

	x = 0;
	for (;;) {
		unsigned y;

		y = *(*p) ++;
		x = (x << 7) | (uint32_t)(y & 0x7F);
		if (y < 0x80) {
			return x;
		}
	}
}

static int32_t
t0_parse7E_signed(const unsigned char **p)
{
	int neg;
	uint32_t x;

	neg = ((**p) >> 6) & 1;
	x = (uint32_t)-neg;
	for (;;) {
		unsigned y;

		y = *(*p) ++;
		x = (x << 7) | (uint32_t)(y & 0x7F);
		if (y < 0x80) {
			if (neg) {
				return -(int32_t)~x - 1;
			} else {
				return (int32_t)x;
			}
		}
	}
}

#define T0_VBYTE(x, n)   (unsigned char)((((uint32_t)(x) >> (n)) & 0x7F) | 0x80)
#define T0_FBYTE(x, n)   (unsigned char)(((uint32_t)(x) >> (n)) & 0x7F)
#define T0_SBYTE(x)      (unsigned char)((((uint32_t)(x) >> 28) + 0xF8) ^ 0xF8)
#define T0_INT1(x)       T0_FBYTE(x, 0)
#define T0_INT2(x)       T0_VBYTE(x, 7), T0_FBYTE(x, 0)
#define T0_INT3(x)       T0_VBYTE(x, 14), T0_VBYTE(x, 7), T0_FBYTE(x, 0)
#define T0_INT4(x)       T0_VBYTE(x, 21), T0_VBYTE(x, 14), T0_VBYTE(x, 7), T0_FBYTE(x, 0)
#define T0_INT5(x)       T0_SBYTE(x), T0_VBYTE(x, 21), T0_VBYTE(x, 14), T0_VBYTE(x, 7), T0_FBYTE(x, 0)

static const uint8_t t0_datablock[];


void br_x509_minimal_init_main(void *t0ctx);

void br_x509_minimal_run(void *t0ctx);



/* (already included) */





/* (already included) */

/*
 * Implementation Notes
 * --------------------
 *
 * The C code pushes the data by chunks; all decoding is done in the
 * T0 code. The cert_length value is set to the certificate length when
 * a new certificate is started; the T0 code picks it up as outer limit,
 * and decoding functions use it to ensure that no attempt is made at
 * reading past it. The T0 code also checks that once the certificate is
 * decoded, there are no trailing bytes.
 *
 * The T0 code sets cert_length to 0 when the certificate is fully
 * decoded.
 *
 * The C code must still perform two checks:
 *
 *  -- If the certificate length is 0, then the T0 code will not be
 *  invoked at all. This invalid condition must thus be reported by the
 *  C code.
 *
 *  -- When reaching the end of certificate, the C code must verify that
 *  the certificate length has been set to 0, thereby signaling that
 *  the T0 code properly decoded a certificate.
 *
 * Processing of a chain works in the following way:
 *
 *  -- The error flag is set to a non-zero value when validation is
 *  finished. The value is either BR_ERR_X509_OK (validation is
 *  successful) or another non-zero error code. When a non-zero error
 *  code is obtained, the remaining bytes in the current certificate and
 *  the subsequent certificates (if any) are completely ignored.
 *
 *  -- Each certificate is decoded in due course, with the following
 *  "interesting points":
 *
 *     -- Start of the TBS: the multihash engine is reset and activated.
 *
 *     -- Start of the issuer DN: the secondary hash engine is started,
 *     to process the encoded issuer DN.
 *
 *     -- End of the issuer DN: the secondary hash engine is stopped. The
 *     resulting hash value is computed and then copied into the
 *     next_dn_hash[] buffer.
 *
 *     -- Start of the subject DN: the secondary hash engine is started,
 *     to process the encoded subject DN.
 *
 *     -- For the EE certificate only: the Common Name, if any, is matched
 *     against the expected server name.
 *
 *     -- End of the subject DN: the secondary hash engine is stopped. The
 *     resulting hash value is computed into the pad. It is then processed:
 *
 *        -- If this is the EE certificate, then the hash is ignored
 *        (except for direct trust processing, see later; the hash is
 *        simply left in current_dn_hash[]).
 *
 *        -- Otherwise, the hashed subject DN is compared with the saved
 *        hash value (in saved_dn_hash[]). They must match.
 *
 *     Either way, the next_dn_hash[] value is then copied into the
 *     saved_dn_hash[] value. Thus, at that point, saved_dn_hash[]
 *     contains the hash of the issuer DN for the current certificate,
 *     and current_dn_hash[] contains the hash of the subject DN for the
 *     current certificate.
 *
 *     -- Public key: it is decoded into the cert_pkey[] buffer. Unknown
 *     key types are reported at that point.
 *
 *        -- If this is the EE certificate, then the key type is compared
 *        with the expected key type (initialization parameter). The public
 *        key data is copied to ee_pkey_data[]. The key and hashed subject
 *        DN are also compared with the "direct trust" keys; if the key
 *        and DN are matched, then validation ends with a success.
 *
 *        -- Otherwise, the saved signature (cert_sig[]) is verified
 *        against the saved TBS hash (tbs_hash[]) and that freshly
 *        decoded public key. Failure here ends validation with an error.
 *
 *     -- Extensions: extension values are processed in due order.
 *
 *        -- Basic Constraints: for all certificates except EE, must be
 *        present, indicate a CA, and have a path legnth compatible with
 *        the chain length so far.
 *
 *        -- Key Usage: for the EE, if present, must allow signatures
 *        or encryption/key exchange, as required for the cipher suite.
 *        For non-EE, if present, must have the "certificate sign" bit.
 *
 *        -- Subject Alt Name: for the EE, dNSName names are matched
 *        against the server name. Ignored for non-EE.
 *
 *        -- Authority Key Identifier, Subject Key Identifier, Issuer
 *        Alt Name, Subject Directory Attributes, CRL Distribution Points
 *        Freshest CRL, Authority Info Access and Subject Info Access
 *        extensions are always ignored: they either contain only
 *        informative data, or they relate to revocation processing, which
 *        we explicitly do not support.
 *
 *        -- All other extensions are ignored if non-critical. If a
 *        critical extension other than the ones above is encountered,
 *        then a failure is reported.
 *
 *     -- End of the TBS: the multihash engine is stopped.
 *
 *     -- Signature algorithm: the signature algorithm on the
 *     certificate is decoded. A failure is reported if that algorithm
 *     is unknown. The hashed TBS corresponding to the signature hash
 *     function is computed and stored in tbs_hash[] (if not supported,
 *     then a failure is reported). The hash OID and length are stored
 *     in cert_sig_hash_oid and cert_sig_hash_len.
 *
 *     -- Signature value: the signature value is copied into the
 *     cert_sig[] array.
 *
 *     -- Certificate end: the hashed issuer DN (saved_dn_hash[]) is
 *     looked up in the trust store (CA trust anchors only); for all
 *     that match, the signature (cert_sig[]) is verified against the
 *     anchor public key (hashed TBS is in tbs_hash[]). If one of these
 *     signatures is valid, then validation ends with a success.
 *
 *  -- If the chain end is reached without obtaining a validation success,
 *  then validation is reported as failed.
 */

#ifndef BR_USE_UNIX_TIME
#if defined __unix__ || defined __linux__ \
	|| defined _POSIX_SOURCE || defined _POSIX_C_SOURCE \
	|| (defined __APPLE__ && defined __MACH__)
#define BR_USE_UNIX_TIME   1
#endif
#endif

#ifndef BR_USE_WIN32_TIME
#if defined _WIN32 || defined _WIN64
#define BR_USE_WIN32_TIME   1
#endif
#endif

#if BR_USE_UNIX_TIME
#include <time.h>
#endif

#if BR_USE_WIN32_TIME
#include <windows.h>
#endif

void br_x509_minimal_init_main(void *ctx);
void br_x509_minimal_run(void *ctx);

/* see bearssl_x509.h */
void
br_x509_minimal_init(br_x509_minimal_context *ctx,
	const br_hash_class *dn_hash_impl,
	const br_x509_trust_anchor *trust_anchors, size_t trust_anchors_num)
{
	memset(ctx, 0, sizeof *ctx);
	ctx->vtable = &br_x509_minimal_vtable;
	ctx->dn_hash_impl = dn_hash_impl;
	ctx->trust_anchors = trust_anchors;
	ctx->trust_anchors_num = trust_anchors_num;
}

static void
xm_start_chain(const br_x509_class **ctx, const char *server_name)
{
	br_x509_minimal_context *cc;
	size_t u;

	cc = (br_x509_minimal_context *)ctx;
	for (u = 0; u < cc->num_name_elts; u ++) {
		cc->name_elts[u].status = 0;
		cc->name_elts[u].buf[0] = 0;
	}
	memset(&cc->pkey, 0, sizeof cc->pkey);
	cc->num_certs = 0;
	cc->err = 0;
	cc->cpu.dp = cc->dp_stack;
	cc->cpu.rp = cc->rp_stack;
	br_x509_minimal_init_main(&cc->cpu);
	if (server_name == NULL || *server_name == 0) {
		cc->server_name = NULL;
	} else {
		cc->server_name = server_name;
	}
}

static void
xm_start_cert(const br_x509_class **ctx, uint32_t length)
{
	br_x509_minimal_context *cc;

	cc = (br_x509_minimal_context *)ctx;
	if (cc->err != 0) {
		return;
	}
	if (length == 0) {
		cc->err = BR_ERR_X509_TRUNCATED;
		return;
	}
	cc->cert_length = length;
}

static void
xm_append(const br_x509_class **ctx, const unsigned char *buf, size_t len)
{
	br_x509_minimal_context *cc;

	cc = (br_x509_minimal_context *)ctx;
	if (cc->err != 0) {
		return;
	}
	cc->hbuf = buf;
	cc->hlen = len;
	br_x509_minimal_run(&cc->cpu);
}

static void
xm_end_cert(const br_x509_class **ctx)
{
	br_x509_minimal_context *cc;

	cc = (br_x509_minimal_context *)ctx;
	if (cc->err == 0 && cc->cert_length != 0) {
		cc->err = BR_ERR_X509_TRUNCATED;
	}
	cc->num_certs ++;
}

static unsigned
xm_end_chain(const br_x509_class **ctx)
{
	br_x509_minimal_context *cc;

	cc = (br_x509_minimal_context *)ctx;
	if (cc->err == 0) {
		if (cc->num_certs == 0) {
			cc->err = BR_ERR_X509_EMPTY_CHAIN;
		} else {
			cc->err = BR_ERR_X509_NOT_TRUSTED;
		}
	} else if (cc->err == BR_ERR_X509_OK) {
		return 0;
	}
	return (unsigned)cc->err;
}

static const br_x509_pkey *
xm_get_pkey(const br_x509_class *const *ctx, unsigned *usages)
{
	br_x509_minimal_context *cc;

	cc = (br_x509_minimal_context *)ctx;
	if (cc->err == BR_ERR_X509_OK
		|| cc->err == BR_ERR_X509_NOT_TRUSTED)
	{
		if (usages != NULL) {
			*usages = cc->key_usages;
		}
		return &((br_x509_minimal_context *)ctx)->pkey;
	} else {
		return NULL;
	}
}

/* see bearssl_x509.h */
const br_x509_class br_x509_minimal_vtable = {
	sizeof(br_x509_minimal_context),
	xm_start_chain,
	xm_start_cert,
	xm_append,
	xm_end_cert,
	xm_end_chain,
	xm_get_pkey
};

#define CTX   ((br_x509_minimal_context *)((unsigned char *)t0ctx - offsetof(br_x509_minimal_context, cpu)))
#define CONTEXT_NAME   br_x509_minimal_context

#define DNHASH_LEN   ((CTX->dn_hash_impl->desc >> BR_HASHDESC_OUT_OFF) & BR_HASHDESC_OUT_MASK)

/*
 * Hash a DN (from a trust anchor) into the provided buffer. This uses the
 * DN hash implementation and context structure from the X.509 engine
 * context.
 */
static void
hash_dn(br_x509_minimal_context *ctx, const void *dn, size_t len,
	unsigned char *out)
{
	ctx->dn_hash_impl->init(&ctx->dn_hash.vtable);
	ctx->dn_hash_impl->update(&ctx->dn_hash.vtable, dn, len);
	ctx->dn_hash_impl->out(&ctx->dn_hash.vtable, out);
}

/*
 * Compare two big integers for equality. The integers use unsigned big-endian
 * encoding; extra leading bytes (of value 0) are allowed.
 */
static int
eqbigint(const unsigned char *b1, size_t len1,
	const unsigned char *b2, size_t len2)
{
	while (len1 > 0 && *b1 == 0) {
		b1 ++;
		len1 --;
	}
	while (len2 > 0 && *b2 == 0) {
		b2 ++;
		len2 --;
	}
	if (len1 != len2) {
		return 0;
	}
	return memcmp(b1, b2, len1) == 0;
}

/*
 * Verify the signature on the certificate with the provided public key.
 * This function checks the public key type with regards to the expected
 * type. Returned value is either 0 on success, or a non-zero error code.
 */
static int
verify_signature(br_x509_minimal_context *ctx, const br_x509_pkey *pk)
{
	int kt;

	kt = ctx->cert_signer_key_type;
	if ((pk->key_type & 0x0F) != kt) {
		return BR_ERR_X509_WRONG_KEY_TYPE;
	}
	switch (kt) {
		unsigned char tmp[64];

	case BR_KEYTYPE_RSA:
		if (ctx->irsa == 0) {
			return BR_ERR_X509_UNSUPPORTED;
		}
		if (!ctx->irsa(ctx->cert_sig, ctx->cert_sig_len,
			&t0_datablock[ctx->cert_sig_hash_oid],
			ctx->cert_sig_hash_len, &pk->key.rsa, tmp))
		{
			return BR_ERR_X509_BAD_SIGNATURE;
		}
		if (memcmp(ctx->tbs_hash, tmp, ctx->cert_sig_hash_len) != 0) {
			return BR_ERR_X509_BAD_SIGNATURE;
		}
		return 0;

	case BR_KEYTYPE_EC:
		if (ctx->iecdsa == 0) {
			return BR_ERR_X509_UNSUPPORTED;
		}
		if (!ctx->iecdsa(ctx->iec, ctx->tbs_hash,
			ctx->cert_sig_hash_len, &pk->key.ec,
			ctx->cert_sig, ctx->cert_sig_len))
		{
			return BR_ERR_X509_BAD_SIGNATURE;
		}
		return 0;

	default:
		return BR_ERR_X509_UNSUPPORTED;
	}
}

/*
 * Compare two strings for equality, in a case-insensitive way. This
 * function handles casing only for ASCII letters.
 */
static int
eqnocase(const void *s1, const void *s2, size_t len)
{
	const unsigned char *buf1, *buf2;

	buf1 = s1;
	buf2 = s2;
	while (len -- > 0) {
		int x1, x2;

		x1 = *buf1 ++;
		x2 = *buf2 ++;
		if (x1 >= 'A' && x1 <= 'Z') {
			x1 += 'a' - 'A';
		}
		if (x2 >= 'A' && x2 <= 'Z') {
			x2 += 'a' - 'A';
		}
		if (x1 != x2) {
			return 0;
		}
	}
	return 1;
}



static const uint8_t t0_datablock[] = {
	0x00, 0x09, 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x01, 0x01, 0x09,
	0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x01, 0x05, 0x09, 0x2A, 0x86,
	0x48, 0x86, 0xF7, 0x0D, 0x01, 0x01, 0x0E, 0x09, 0x2A, 0x86, 0x48, 0x86,
	0xF7, 0x0D, 0x01, 0x01, 0x0B, 0x09, 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D,
	0x01, 0x01, 0x0C, 0x09, 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x01,
	0x0D, 0x05, 0x2B, 0x0E, 0x03, 0x02, 0x1A, 0x09, 0x60, 0x86, 0x48, 0x01,
	0x65, 0x03, 0x04, 0x02, 0x04, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03,
	0x04, 0x02, 0x01, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02,
	0x02, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x03, 0x07,
	0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x02, 0x01, 0x08, 0x2A, 0x86, 0x48, 0xCE,
	0x3D, 0x03, 0x01, 0x07, 0x05, 0x2B, 0x81, 0x04, 0x00, 0x22, 0x05, 0x2B,
	0x81, 0x04, 0x00, 0x23, 0x07, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x04, 0x01,
	0x08, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x04, 0x03, 0x01, 0x08, 0x2A, 0x86,
	0x48, 0xCE, 0x3D, 0x04, 0x03, 0x02, 0x08, 0x2A, 0x86, 0x48, 0xCE, 0x3D,
	0x04, 0x03, 0x03, 0x08, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x04, 0x03, 0x04,
	0x03, 0x55, 0x04, 0x03, 0x00, 0x1F, 0x03, 0xFC, 0x07, 0x7F, 0x0B, 0x5E,
	0x0F, 0x1F, 0x12, 0xFE, 0x16, 0xBF, 0x1A, 0x9F, 0x1E, 0x7E, 0x22, 0x3F,
	0x26, 0x1E, 0x29, 0xDF, 0x00, 0x1F, 0x03, 0xFD, 0x07, 0x9F, 0x0B, 0x7E,
	0x0F, 0x3F, 0x13, 0x1E, 0x16, 0xDF, 0x1A, 0xBF, 0x1E, 0x9E, 0x22, 0x5F,
	0x26, 0x3E, 0x29, 0xFF, 0x03, 0x55, 0x1D, 0x13, 0x03, 0x55, 0x1D, 0x0F,
	0x03, 0x55, 0x1D, 0x11, 0x03, 0x55, 0x1D, 0x23, 0x03, 0x55, 0x1D, 0x0E,
	0x03, 0x55, 0x1D, 0x12, 0x03, 0x55, 0x1D, 0x09, 0x03, 0x55, 0x1D, 0x1F,
	0x03, 0x55, 0x1D, 0x2E, 0x08, 0x2B, 0x06, 0x01, 0x05, 0x05, 0x07, 0x01,
	0x01, 0x08, 0x2B, 0x06, 0x01, 0x05, 0x05, 0x07, 0x01, 0x0B
};

static const uint8_t t0_codeblock[] = {
	0x00, 0x01, 0x00, 0x0D, 0x00, 0x00, 0x01, 0x00, 0x10, 0x00, 0x00, 0x01,
	0x00, 0x11, 0x00, 0x00, 0x01, 0x01, 0x09, 0x00, 0x00, 0x01, 0x01, 0x0A,
	0x00, 0x00, 0x24, 0x24, 0x00, 0x00, 0x01,
	T0_INT1(BR_ERR_X509_BAD_BOOLEAN), 0x00, 0x00, 0x01,
	T0_INT1(BR_ERR_X509_BAD_DN), 0x00, 0x00, 0x01,
	T0_INT1(BR_ERR_X509_BAD_SERVER_NAME), 0x00, 0x00, 0x01,
	T0_INT1(BR_ERR_X509_BAD_TAG_CLASS), 0x00, 0x00, 0x01,
	T0_INT1(BR_ERR_X509_BAD_TAG_VALUE), 0x00, 0x00, 0x01,
	T0_INT1(BR_ERR_X509_BAD_TIME), 0x00, 0x00, 0x01,
	T0_INT1(BR_ERR_X509_CRITICAL_EXTENSION), 0x00, 0x00, 0x01,
	T0_INT1(BR_ERR_X509_DN_MISMATCH), 0x00, 0x00, 0x01,
	T0_INT1(BR_ERR_X509_EXPIRED), 0x00, 0x00, 0x01,
	T0_INT1(BR_ERR_X509_EXTRA_ELEMENT), 0x00, 0x00, 0x01,
	T0_INT1(BR_ERR_X509_FORBIDDEN_KEY_USAGE), 0x00, 0x00, 0x01,
	T0_INT1(BR_ERR_X509_INDEFINITE_LENGTH), 0x00, 0x00, 0x01,
	T0_INT1(BR_ERR_X509_INNER_TRUNC), 0x00, 0x00, 0x01,
	T0_INT1(BR_ERR_X509_LIMIT_EXCEEDED), 0x00, 0x00, 0x01,
	T0_INT1(BR_ERR_X509_NOT_CA), 0x00, 0x00, 0x01,
	T0_INT1(BR_ERR_X509_NOT_CONSTRUCTED), 0x00, 0x00, 0x01,
	T0_INT1(BR_ERR_X509_NOT_PRIMITIVE), 0x00, 0x00, 0x01,
	T0_INT1(BR_ERR_X509_OVERFLOW), 0x00, 0x00, 0x01,
	T0_INT1(BR_ERR_X509_PARTIAL_BYTE), 0x00, 0x00, 0x01,
	T0_INT1(BR_ERR_X509_UNEXPECTED), 0x00, 0x00, 0x01,
	T0_INT1(BR_ERR_X509_UNSUPPORTED), 0x00, 0x00, 0x01,
	T0_INT1(BR_ERR_X509_WEAK_PUBLIC_KEY), 0x00, 0x00, 0x01,
	T0_INT1(BR_KEYTYPE_EC), 0x00, 0x00, 0x01, T0_INT1(BR_KEYTYPE_RSA),
	0x00, 0x00, 0x01, T0_INT2(offsetof(CONTEXT_NAME, cert_length)), 0x00,
	0x00, 0x01, T0_INT2(offsetof(CONTEXT_NAME, cert_sig)), 0x00, 0x00,
	0x01, T0_INT2(offsetof(CONTEXT_NAME, cert_sig_hash_len)), 0x00, 0x00,
	0x01, T0_INT2(offsetof(CONTEXT_NAME, cert_sig_hash_oid)), 0x00, 0x00,
	0x01, T0_INT2(offsetof(CONTEXT_NAME, cert_sig_len)), 0x00, 0x00, 0x01,
	T0_INT2(offsetof(CONTEXT_NAME, cert_signer_key_type)), 0x00, 0x00,
	0x01, T0_INT2(offsetof(CONTEXT_NAME, current_dn_hash)), 0x00, 0x00,
	0x01, T0_INT2(offsetof(CONTEXT_NAME, key_usages)), 0x00, 0x00, 0x01,
	T0_INT2(offsetof(br_x509_minimal_context, pkey_data)), 0x01,
	T0_INT2(BR_X509_BUFSIZE_KEY), 0x00, 0x00, 0x01,
	T0_INT2(offsetof(CONTEXT_NAME, min_rsa_size)), 0x00, 0x00, 0x01,
	T0_INT2(offsetof(CONTEXT_NAME, next_dn_hash)), 0x00, 0x00, 0x01,
	T0_INT2(offsetof(CONTEXT_NAME, num_certs)), 0x00, 0x00, 0x01,
	T0_INT2(offsetof(CONTEXT_NAME, pad)), 0x00, 0x00, 0x01,
	T0_INT2(offsetof(CONTEXT_NAME, saved_dn_hash)), 0x00, 0x00, 0xC6, 0x71,
	0x00, 0x00, 0x01, 0x80, 0x73, 0x00, 0x00, 0x01, 0x80, 0x7C, 0x00, 0x00,
	0x01, 0x81, 0x02, 0x00, 0x00, 0x90, 0x05, 0x05, 0x34, 0x42, 0x01, 0x00,
	0x00, 0x34, 0x01, 0x0A, 0x0E, 0x09, 0x01, 0x9A, 0xFF, 0xB8, 0x00, 0x0A,
	0x00, 0x00, 0x01, 0x82, 0x0C, 0x00, 0x00, 0x01, 0x81, 0x74, 0x00, 0x00,
	0x01, 0x81, 0x68, 0x00, 0x04, 0x03, 0x00, 0x03, 0x01, 0x03, 0x02, 0x03,
	0x03, 0x02, 0x03, 0x02, 0x01, 0x11, 0x06, 0x07, 0x02, 0x02, 0x02, 0x00,
	0x0D, 0x04, 0x05, 0x02, 0x03, 0x02, 0x01, 0x0D, 0x00, 0x02, 0x03, 0x00,
	0x03, 0x01, 0x25, 0x02, 0x01, 0x13, 0x3B, 0x02, 0x00, 0x0F, 0x15, 0x00,
	0x00, 0x05, 0x02, 0x52, 0x28, 0x00, 0x00, 0x06, 0x02, 0x53, 0x28, 0x00,
	0x00, 0x01, 0x10, 0x76, 0x00, 0x00, 0x11, 0x05, 0x02, 0x56, 0x28, 0x73,
	0x00, 0x00, 0x11, 0x05, 0x02, 0x56, 0x28, 0x74, 0x00, 0x00, 0x06, 0x02,
	0x4C, 0x28, 0x00, 0x00, 0x01, 0x82, 0x04, 0x00, 0x00, 0x25, 0x20, 0x01,
	0x08, 0x0E, 0x3B, 0x40, 0x20, 0x09, 0x00, 0x09, 0x03, 0x00, 0x5B, 0x2B,
	0xAC, 0x39, 0xAC, 0xB0, 0x25, 0x01, 0x20, 0x11, 0x06, 0x11, 0x24, 0x73,
	0xAA, 0xB0, 0x01, 0x02, 0x77, 0xAD, 0x01, 0x02, 0x12, 0x06, 0x02, 0x57,
	0x28, 0x78, 0xB0, 0x01, 0x02, 0x77, 0xAB, 0xAC, 0xBF, 0x99, 0x65, 0x61,
	0x21, 0x16, 0xAC, 0xA4, 0x29, 0x69, 0x06, 0x02, 0x4B, 0x28, 0xA4, 0x29,
	0x71, 0x06, 0x02, 0x4B, 0x28, 0x78, 0x02, 0x00, 0x06, 0x05, 0x9A, 0x03,
	0x01, 0x04, 0x09, 0x99, 0x61, 0x68, 0x21, 0x27, 0x05, 0x02, 0x4A, 0x28,
	0x68, 0x65, 0x21, 0x16, 0xAC, 0xAC, 0x9B, 0x05, 0x02, 0x57, 0x28, 0xB9,
	0x26, 0x06, 0x27, 0xBF, 0xA1, 0xAC, 0x63, 0xA7, 0x03, 0x03, 0x63, 0x3B,
	0x02, 0x03, 0x09, 0x3B, 0x02, 0x03, 0x0A, 0xA7, 0x03, 0x04, 0x78, 0x64,
	0x2A, 0x01, 0x81, 0x00, 0x09, 0x02, 0x03, 0x12, 0x06, 0x02, 0x58, 0x28,
	0x78, 0x5A, 0x03, 0x02, 0x04, 0x3A, 0x87, 0x26, 0x06, 0x34, 0x9B, 0x05,
	0x02, 0x57, 0x28, 0x6A, 0x26, 0x06, 0x04, 0x01, 0x17, 0x04, 0x12, 0x6B,
	0x26, 0x06, 0x04, 0x01, 0x18, 0x04, 0x0A, 0x6C, 0x26, 0x06, 0x04, 0x01,
	0x19, 0x04, 0x02, 0x57, 0x28, 0x03, 0x05, 0x78, 0xA1, 0x25, 0x03, 0x06,
	0x25, 0x63, 0x34, 0x0D, 0x06, 0x02, 0x50, 0x28, 0xA2, 0x59, 0x03, 0x02,
	0x04, 0x02, 0x57, 0x28, 0x78, 0x02, 0x00, 0x06, 0x21, 0x02, 0x02, 0x5A,
	0x30, 0x11, 0x06, 0x08, 0x24, 0x02, 0x03, 0x02, 0x04, 0x1D, 0x04, 0x10,
	0x59, 0x30, 0x11, 0x06, 0x08, 0x24, 0x02, 0x05, 0x02, 0x06, 0x1C, 0x04,
	0x03, 0x57, 0x28, 0x24, 0x04, 0x24, 0x02, 0x02, 0x5A, 0x30, 0x11, 0x06,
	0x08, 0x24, 0x02, 0x03, 0x02, 0x04, 0x23, 0x04, 0x10, 0x59, 0x30, 0x11,
	0x06, 0x08, 0x24, 0x02, 0x05, 0x02, 0x06, 0x22, 0x04, 0x03, 0x57, 0x28,
	0x24, 0x25, 0x06, 0x01, 0x28, 0x24, 0x01, 0x00, 0x03, 0x07, 0xB1, 0x01,
	0x21, 0x8D, 0x01, 0x22, 0x8D, 0x25, 0x01, 0x23, 0x11, 0x06, 0x81, 0x17,
	0x24, 0x73, 0xAA, 0xAC, 0x25, 0x06, 0x81, 0x0B, 0x01, 0x00, 0x03, 0x08,
	0xAC, 0x9B, 0x24, 0xB0, 0x25, 0x01, 0x01, 0x11, 0x06, 0x04, 0xA3, 0x03,
	0x08, 0xB0, 0x01, 0x04, 0x77, 0xAA, 0x70, 0x26, 0x06, 0x0F, 0x02, 0x00,
	0x06, 0x03, 0xC0, 0x04, 0x05, 0x97, 0x01, 0x7F, 0x03, 0x07, 0x04, 0x80,
	0x5D, 0x8F, 0x26, 0x06, 0x06, 0x02, 0x00, 0x98, 0x04, 0x80, 0x53, 0xC2,
	0x26, 0x06, 0x10, 0x02, 0x00, 0x06, 0x09, 0x01, 0x00, 0x03, 0x01, 0x96,
	0x03, 0x01, 0x04, 0x01, 0xC0, 0x04, 0x3F, 0x6F, 0x26, 0x06, 0x03, 0xC0,
	0x04, 0x38, 0xC5, 0x26, 0x06, 0x03, 0xC0, 0x04, 0x31, 0x8E, 0x26, 0x06,
	0x03, 0xC0, 0x04, 0x2A, 0xC3, 0x26, 0x06, 0x03, 0xC0, 0x04, 0x23, 0x79,
	0x26, 0x06, 0x03, 0xC0, 0x04, 0x1C, 0x84, 0x26, 0x06, 0x03, 0xC0, 0x04,
	0x15, 0x6E, 0x26, 0x06, 0x03, 0xC0, 0x04, 0x0E, 0xC4, 0x26, 0x06, 0x03,
	0xC0, 0x04, 0x07, 0x02, 0x08, 0x06, 0x02, 0x49, 0x28, 0xC0, 0x78, 0x78,
	0x04, 0xFE, 0x71, 0x78, 0x78, 0x04, 0x08, 0x01, 0x7F, 0x11, 0x05, 0x02,
	0x56, 0x28, 0x24, 0x78, 0x3A, 0x02, 0x00, 0x06, 0x08, 0x02, 0x01, 0x3C,
	0x2F, 0x05, 0x02, 0x45, 0x28, 0x02, 0x00, 0x06, 0x01, 0x17, 0x02, 0x00,
	0x02, 0x07, 0x2F, 0x05, 0x02, 0x51, 0x28, 0xB0, 0x75, 0xAA, 0x9B, 0x06,
	0x80, 0x77, 0xBA, 0x26, 0x06, 0x07, 0x01, 0x02, 0x5A, 0x88, 0x04, 0x80,
	0x5E, 0xBB, 0x26, 0x06, 0x07, 0x01, 0x03, 0x5A, 0x89, 0x04, 0x80, 0x53,
	0xBC, 0x26, 0x06, 0x07, 0x01, 0x04, 0x5A, 0x8A, 0x04, 0x80, 0x48, 0xBD,
	0x26, 0x06, 0x06, 0x01, 0x05, 0x5A, 0x8B, 0x04, 0x3E, 0xBE, 0x26, 0x06,
	0x06, 0x01, 0x06, 0x5A, 0x8C, 0x04, 0x34, 0x7E, 0x26, 0x06, 0x06, 0x01,
	0x02, 0x59, 0x88, 0x04, 0x2A, 0x7F, 0x26, 0x06, 0x06, 0x01, 0x03, 0x59,
	0x89, 0x04, 0x20, 0x80, 0x26, 0x06, 0x06, 0x01, 0x04, 0x59, 0x8A, 0x04,
	0x16, 0x81, 0x26, 0x06, 0x06, 0x01, 0x05, 0x59, 0x8B, 0x04, 0x0C, 0x82,
	0x26, 0x06, 0x06, 0x01, 0x06, 0x59, 0x8C, 0x04, 0x02, 0x57, 0x28, 0x5E,
	0x35, 0x60, 0x37, 0x1B, 0x25, 0x05, 0x02, 0x57, 0x28, 0x5D, 0x37, 0x04,
	0x02, 0x57, 0x28, 0xBF, 0xA1, 0x25, 0x01, T0_INT2(BR_X509_BUFSIZE_SIG),
	0x12, 0x06, 0x02, 0x50, 0x28, 0x25, 0x5F, 0x35, 0x5C, 0xA2, 0x78, 0x78,
	0x01, 0x00, 0x5B, 0x36, 0x18, 0x00, 0x00, 0x01, 0x30, 0x0A, 0x25, 0x01,
	0x00, 0x01, 0x09, 0x72, 0x05, 0x02, 0x48, 0x28, 0x00, 0x00, 0x30, 0x30,
	0x00, 0x00, 0x01, 0x81, 0x08, 0x00, 0x00, 0x01, 0x81, 0x10, 0x00, 0x00,
	0x01, 0x81, 0x19, 0x00, 0x00, 0x01, 0x81, 0x22, 0x00, 0x00, 0x01, 0x81,
	0x2B, 0x00, 0x01, 0x7D, 0x01, 0x01, 0x11, 0x3B, 0x01, 0x83, 0xFD, 0x7F,
	0x11, 0x15, 0x06, 0x03, 0x3B, 0x24, 0x00, 0x3B, 0x25, 0x03, 0x00, 0x25,
	0xC7, 0x05, 0x04, 0x42, 0x01, 0x00, 0x00, 0x25, 0x01, 0x81, 0x00, 0x0D,
	0x06, 0x04, 0x94, 0x04, 0x80, 0x49, 0x25, 0x01, 0x90, 0x00, 0x0D, 0x06,
	0x0F, 0x01, 0x06, 0x14, 0x01, 0x81, 0x40, 0x2F, 0x94, 0x02, 0x00, 0x01,
	0x00, 0x95, 0x04, 0x33, 0x25, 0x01, 0x83, 0xFF, 0x7F, 0x0D, 0x06, 0x14,
	0x01, 0x0C, 0x14, 0x01, 0x81, 0x60, 0x2F, 0x94, 0x02, 0x00, 0x01, 0x06,
	0x95, 0x02, 0x00, 0x01, 0x00, 0x95, 0x04, 0x17, 0x01, 0x12, 0x14, 0x01,
	0x81, 0x70, 0x2F, 0x94, 0x02, 0x00, 0x01, 0x0C, 0x95, 0x02, 0x00, 0x01,
	0x06, 0x95, 0x02, 0x00, 0x01, 0x00, 0x95, 0x00, 0x00, 0x01, 0x82, 0x08,
	0x00, 0x00, 0x25, 0x01, 0x83, 0xB0, 0x00, 0x01, 0x83, 0xB7, 0x7F, 0x72,
	0x00, 0x00, 0x01, 0x81, 0x34, 0x00, 0x00, 0x01, 0x80, 0x6B, 0x00, 0x00,
	0x01, 0x3D, 0x00, 0x00, 0x01, 0x80, 0x43, 0x00, 0x00, 0x01, 0x80, 0x4D,
	0x00, 0x00, 0x01, 0x80, 0x57, 0x00, 0x00, 0x01, 0x80, 0x61, 0x00, 0x00,
	0x30, 0x11, 0x06, 0x04, 0x42, 0xAA, 0xBF, 0xB1, 0x00, 0x00, 0x01, 0x81,
	0x7C, 0x00, 0x00, 0x01, 0x81, 0x6C, 0x00, 0x00, 0x25, 0x01, 0x83, 0xB8,
	0x00, 0x01, 0x83, 0xBF, 0x7F, 0x72, 0x00, 0x00, 0x01, 0x30, 0x62, 0x37,
	0x01, 0x7F, 0x7B, 0x19, 0x01, 0x00, 0x7B, 0x19, 0x04, 0x7A, 0x00, 0x01,
	0x81, 0x38, 0x00, 0x01, 0x7D, 0x0D, 0x06, 0x02, 0x4F, 0x28, 0x25, 0x03,
	0x00, 0x0A, 0x02, 0x00, 0x00, 0x00, 0x30, 0x25, 0x3F, 0x3B, 0x01, 0x82,
	0x00, 0x13, 0x2F, 0x06, 0x04, 0x42, 0x01, 0x00, 0x00, 0x30, 0x67, 0x09,
	0x37, 0x40, 0x00, 0x00, 0x14, 0x01, 0x3F, 0x15, 0x01, 0x81, 0x00, 0x2F,
	0x94, 0x00, 0x02, 0x01, 0x00, 0x03, 0x00, 0xAC, 0x25, 0x06, 0x80, 0x59,
	0xB0, 0x01, 0x20, 0x30, 0x11, 0x06, 0x17, 0x24, 0x73, 0xAA, 0x9B, 0x24,
	0x01, 0x7F, 0x2E, 0x03, 0x01, 0xB0, 0x01, 0x20, 0x76, 0xAA, 0xAF, 0x02,
	0x01, 0x1F, 0x78, 0x78, 0x04, 0x38, 0x01, 0x21, 0x30, 0x11, 0x06, 0x08,
	0x24, 0x74, 0xB3, 0x01, 0x01, 0x1E, 0x04, 0x2A, 0x01, 0x22, 0x30, 0x11,
	0x06, 0x11, 0x24, 0x74, 0xB3, 0x25, 0x06, 0x06, 0x2C, 0x02, 0x00, 0x2F,
	0x03, 0x00, 0x01, 0x02, 0x1E, 0x04, 0x13, 0x01, 0x26, 0x30, 0x11, 0x06,
	0x08, 0x24, 0x74, 0xB3, 0x01, 0x06, 0x1E, 0x04, 0x05, 0x42, 0xAB, 0x01,
	0x00, 0x24, 0x04, 0xFF, 0x23, 0x78, 0x02, 0x00, 0x00, 0x00, 0xAC, 0xB1,
	0x25, 0x01, 0x01, 0x11, 0x06, 0x08, 0xA3, 0x05, 0x02, 0x51, 0x28, 0xB1,
	0x04, 0x02, 0x51, 0x28, 0x25, 0x01, 0x02, 0x11, 0x06, 0x0C, 0x24, 0x74,
	0xAD, 0x66, 0x2B, 0x41, 0x0D, 0x06, 0x02, 0x51, 0x28, 0xB1, 0x01, 0x7F,
	0x10, 0x06, 0x02, 0x56, 0x28, 0x24, 0x78, 0x00, 0x02, 0x03, 0x00, 0xB0,
	0x01, 0x03, 0x77, 0xAA, 0xB7, 0x03, 0x01, 0x02, 0x01, 0x01, 0x07, 0x12,
	0x06, 0x02, 0x56, 0x28, 0x25, 0x01, 0x00, 0x30, 0x11, 0x06, 0x05, 0x24,
	0x4D, 0x28, 0x04, 0x15, 0x01, 0x01, 0x30, 0x11, 0x06, 0x0A, 0x24, 0xB7,
	0x02, 0x01, 0x14, 0x02, 0x01, 0x0E, 0x04, 0x05, 0x24, 0xB7, 0x01, 0x00,
	0x24, 0x02, 0x00, 0x06, 0x19, 0x01, 0x00, 0x30, 0x01, 0x38, 0x15, 0x06,
	0x03, 0x01, 0x10, 0x2F, 0x3B, 0x01, 0x81, 0x40, 0x15, 0x06, 0x03, 0x01,
	0x20, 0x2F, 0x62, 0x37, 0x04, 0x07, 0x01, 0x04, 0x15, 0x05, 0x02, 0x4D,
	0x28, 0xBF, 0x00, 0x00, 0x38, 0xAC, 0xBF, 0x1A, 0x00, 0x03, 0x01, 0x00,
	0x03, 0x00, 0x38, 0xAC, 0x25, 0x06, 0x30, 0xB0, 0x01, 0x11, 0x76, 0xAA,
	0x25, 0x05, 0x02, 0x44, 0x28, 0x25, 0x06, 0x20, 0xAC, 0x9B, 0x24, 0x86,
	0x26, 0x03, 0x01, 0x01, 0x00, 0x2E, 0x03, 0x02, 0xAF, 0x25, 0x02, 0x01,
	0x15, 0x06, 0x07, 0x2C, 0x06, 0x04, 0x01, 0x7F, 0x03, 0x00, 0x02, 0x02,
	0x1F, 0x78, 0x04, 0x5D, 0x78, 0x04, 0x4D, 0x78, 0x1A, 0x02, 0x00, 0x00,
	0x00, 0xB0, 0x01, 0x06, 0x77, 0xAE, 0x00, 0x00, 0xB5, 0x85, 0x06, 0x0E,
	0x3B, 0x25, 0x05, 0x06, 0x42, 0x01, 0x00, 0x01, 0x00, 0x00, 0xB5, 0x6D,
	0x04, 0x08, 0x90, 0x06, 0x05, 0x24, 0x01, 0x00, 0x04, 0x00, 0x00, 0x00,
	0xB6, 0x85, 0x06, 0x0E, 0x3B, 0x25, 0x05, 0x06, 0x42, 0x01, 0x00, 0x01,
	0x00, 0x00, 0xB6, 0x6D, 0x04, 0x08, 0x90, 0x06, 0x05, 0x24, 0x01, 0x00,
	0x04, 0x00, 0x00, 0x00, 0xB7, 0x25, 0x01, 0x81, 0x00, 0x0D, 0x06, 0x04,
	0x00, 0x04, 0x80, 0x55, 0x25, 0x01, 0x81, 0x40, 0x0D, 0x06, 0x07, 0x24,
	0x01, 0x00, 0x00, 0x04, 0x80, 0x47, 0x25, 0x01, 0x81, 0x60, 0x0D, 0x06,
	0x0E, 0x01, 0x1F, 0x15, 0x01, 0x01, 0xA0, 0x01, 0x81, 0x00, 0x01, 0x8F,
	0x7F, 0x04, 0x32, 0x25, 0x01, 0x81, 0x70, 0x0D, 0x06, 0x0F, 0x01, 0x0F,
	0x15, 0x01, 0x02, 0xA0, 0x01, 0x90, 0x00, 0x01, 0x83, 0xFF, 0x7F, 0x04,
	0x1C, 0x25, 0x01, 0x81, 0x78, 0x0D, 0x06, 0x11, 0x01, 0x07, 0x15, 0x01,
	0x03, 0xA0, 0x01, 0x84, 0x80, 0x00, 0x01, 0x80, 0xC3, 0xFF, 0x7F, 0x04,
	0x04, 0x24, 0x01, 0x00, 0x00, 0x72, 0x05, 0x03, 0x24, 0x01, 0x00, 0x00,
	0x00, 0x3B, 0x25, 0x05, 0x06, 0x42, 0x01, 0x00, 0x01, 0x7F, 0x00, 0xB7,
	0x34, 0x25, 0x3D, 0x06, 0x03, 0x3B, 0x24, 0x00, 0x01, 0x06, 0x0E, 0x3B,
	0x25, 0x01, 0x06, 0x14, 0x01, 0x02, 0x10, 0x06, 0x04, 0x42, 0x01, 0x7F,
	0x00, 0x01, 0x3F, 0x15, 0x09, 0x00, 0x00, 0x25, 0x06, 0x06, 0x0B, 0x9F,
	0x34, 0x41, 0x04, 0x77, 0x24, 0x25, 0x00, 0x00, 0xB0, 0x01, 0x03, 0x77,
	0xAA, 0xB7, 0x06, 0x02, 0x55, 0x28, 0x00, 0x00, 0x3B, 0x25, 0x06, 0x07,
	0x31, 0x25, 0x06, 0x01, 0x19, 0x04, 0x76, 0x42, 0x00, 0x00, 0x01, 0x01,
	0x77, 0xA9, 0x01, 0x01, 0x10, 0x06, 0x02, 0x43, 0x28, 0xB7, 0x3E, 0x00,
	0x04, 0xB0, 0x25, 0x01, 0x17, 0x01, 0x18, 0x72, 0x05, 0x02, 0x48, 0x28,
	0x01, 0x18, 0x11, 0x03, 0x00, 0x74, 0xAA, 0xA5, 0x02, 0x00, 0x06, 0x0C,
	0x01, 0x80, 0x64, 0x08, 0x03, 0x01, 0xA5, 0x02, 0x01, 0x09, 0x04, 0x0E,
	0x25, 0x01, 0x32, 0x0D, 0x06, 0x04, 0x01, 0x80, 0x64, 0x09, 0x01, 0x8E,
	0x6C, 0x09, 0x03, 0x01, 0x02, 0x01, 0x01, 0x82, 0x6D, 0x08, 0x02, 0x01,
	0x01, 0x03, 0x09, 0x01, 0x04, 0x0C, 0x09, 0x02, 0x01, 0x01, 0x80, 0x63,
	0x09, 0x01, 0x80, 0x64, 0x0C, 0x0A, 0x02, 0x01, 0x01, 0x83, 0x0F, 0x09,
	0x01, 0x83, 0x10, 0x0C, 0x09, 0x03, 0x03, 0x01, 0x01, 0x01, 0x0C, 0xA6,
	0x41, 0x01, 0x01, 0x0E, 0x02, 0x01, 0x01, 0x04, 0x07, 0x3F, 0x02, 0x01,
	0x01, 0x80, 0x64, 0x07, 0x3E, 0x02, 0x01, 0x01, 0x83, 0x10, 0x07, 0x3F,
	0x2F, 0x15, 0x06, 0x03, 0x01, 0x18, 0x09, 0x92, 0x09, 0x7A, 0x25, 0x01,
	0x05, 0x14, 0x02, 0x03, 0x09, 0x03, 0x03, 0x01, 0x1F, 0x15, 0x01, 0x01,
	0x3B, 0xA6, 0x02, 0x03, 0x09, 0x41, 0x03, 0x03, 0x01, 0x00, 0x01, 0x17,
	0xA6, 0x01, 0x9C, 0x10, 0x08, 0x03, 0x02, 0x01, 0x00, 0x01, 0x3B, 0xA6,
	0x01, 0x3C, 0x08, 0x02, 0x02, 0x09, 0x03, 0x02, 0x01, 0x00, 0x01, 0x3C,
	0xA6, 0x02, 0x02, 0x09, 0x03, 0x02, 0xB7, 0x25, 0x01, 0x2E, 0x11, 0x06,
	0x0D, 0x24, 0xB7, 0x25, 0x01, 0x30, 0x01, 0x39, 0x72, 0x06, 0x03, 0x24,
	0x04, 0x74, 0x01, 0x80, 0x5A, 0x10, 0x06, 0x02, 0x48, 0x28, 0x78, 0x02,
	0x03, 0x02, 0x02, 0x00, 0x01, 0xB7, 0x7C, 0x01, 0x0A, 0x08, 0x03, 0x00,
	0xB7, 0x7C, 0x02, 0x00, 0x09, 0x00, 0x02, 0x03, 0x00, 0x03, 0x01, 0xA5,
	0x25, 0x02, 0x01, 0x02, 0x00, 0x72, 0x05, 0x02, 0x48, 0x28, 0x00, 0x00,
	0x34, 0xB0, 0x01, 0x02, 0x77, 0x0B, 0xA8, 0x00, 0x03, 0x25, 0x03, 0x00,
	0x03, 0x01, 0x03, 0x02, 0xAA, 0xB7, 0x25, 0x01, 0x81, 0x00, 0x13, 0x06,
	0x02, 0x54, 0x28, 0x25, 0x01, 0x00, 0x11, 0x06, 0x0B, 0x24, 0x25, 0x05,
	0x04, 0x24, 0x01, 0x00, 0x00, 0xB7, 0x04, 0x6F, 0x02, 0x01, 0x25, 0x05,
	0x02, 0x50, 0x28, 0x41, 0x03, 0x01, 0x02, 0x02, 0x37, 0x02, 0x02, 0x40,
	0x03, 0x02, 0x25, 0x06, 0x03, 0xB7, 0x04, 0x68, 0x24, 0x02, 0x00, 0x02,
	0x01, 0x0A, 0x00, 0x01, 0xB7, 0x25, 0x01, 0x81, 0x00, 0x0D, 0x06, 0x01,
	0x00, 0x01, 0x81, 0x00, 0x0A, 0x25, 0x05, 0x02, 0x4E, 0x28, 0x03, 0x00,
	0x01, 0x00, 0x02, 0x00, 0x01, 0x00, 0x12, 0x06, 0x19, 0x02, 0x00, 0x41,
	0x03, 0x00, 0x25, 0x01, 0x83, 0xFF, 0xFF, 0x7F, 0x12, 0x06, 0x02, 0x4F,
	0x28, 0x01, 0x08, 0x0E, 0x3B, 0xB7, 0x34, 0x09, 0x04, 0x60, 0x00, 0x00,
	0xA9, 0x93, 0x00, 0x00, 0xAA, 0xBF, 0x00, 0x00, 0xB0, 0x75, 0xAA, 0x00,
	0x01, 0xAA, 0x25, 0x05, 0x02, 0x54, 0x28, 0xB7, 0x25, 0x01, 0x81, 0x00,
	0x13, 0x06, 0x02, 0x54, 0x28, 0x03, 0x00, 0x25, 0x06, 0x16, 0xB7, 0x02,
	0x00, 0x25, 0x01, 0x87, 0xFF, 0xFF, 0x7F, 0x13, 0x06, 0x02, 0x54, 0x28,
	0x01, 0x08, 0x0E, 0x09, 0x03, 0x00, 0x04, 0x67, 0x24, 0x02, 0x00, 0x00,
	0x00, 0xAA, 0x25, 0x01, 0x81, 0x7F, 0x12, 0x06, 0x08, 0xBF, 0x01, 0x00,
	0x67, 0x37, 0x01, 0x00, 0x00, 0x25, 0x67, 0x37, 0x67, 0x40, 0xA2, 0x01,
	0x7F, 0x00, 0x00, 0xB0, 0x01, 0x0C, 0x30, 0x11, 0x06, 0x05, 0x24, 0x74,
	0xB3, 0x04, 0x3E, 0x01, 0x12, 0x30, 0x11, 0x06, 0x05, 0x24, 0x74, 0xB4,
	0x04, 0x33, 0x01, 0x13, 0x30, 0x11, 0x06, 0x05, 0x24, 0x74, 0xB4, 0x04,
	0x28, 0x01, 0x14, 0x30, 0x11, 0x06, 0x05, 0x24, 0x74, 0xB4, 0x04, 0x1D,
	0x01, 0x16, 0x30, 0x11, 0x06, 0x05, 0x24, 0x74, 0xB4, 0x04, 0x12, 0x01,
	0x1E, 0x30, 0x11, 0x06, 0x05, 0x24, 0x74, 0xB2, 0x04, 0x07, 0x42, 0xAB,
	0x01, 0x00, 0x01, 0x00, 0x24, 0x00, 0x01, 0xB7, 0x03, 0x00, 0x02, 0x00,
	0x01, 0x05, 0x14, 0x01, 0x01, 0x15, 0x2D, 0x02, 0x00, 0x01, 0x06, 0x14,
	0x25, 0x01, 0x01, 0x15, 0x06, 0x02, 0x46, 0x28, 0x01, 0x04, 0x0E, 0x02,
	0x00, 0x01, 0x1F, 0x15, 0x25, 0x01, 0x1F, 0x11, 0x06, 0x02, 0x47, 0x28,
	0x09, 0x00, 0x00, 0x25, 0x05, 0x05, 0x01, 0x00, 0x01, 0x7F, 0x00, 0xB0,
	0x00, 0x01, 0xAA, 0x25, 0x05, 0x05, 0x67, 0x37, 0x01, 0x7F, 0x00, 0x01,
	0x01, 0x03, 0x00, 0x9C, 0x25, 0x01, 0x83, 0xFF, 0x7E, 0x11, 0x06, 0x16,
	0x24, 0x25, 0x06, 0x10, 0x9D, 0x25, 0x05, 0x05, 0x24, 0xBF, 0x01, 0x00,
	0x00, 0x02, 0x00, 0x83, 0x03, 0x00, 0x04, 0x6D, 0x04, 0x1B, 0x25, 0x05,
	0x05, 0x24, 0xBF, 0x01, 0x00, 0x00, 0x02, 0x00, 0x83, 0x03, 0x00, 0x25,
	0x06, 0x0B, 0x9C, 0x25, 0x05, 0x05, 0x24, 0xBF, 0x01, 0x00, 0x00, 0x04,
	0x6D, 0x24, 0x02, 0x00, 0x25, 0x05, 0x01, 0x00, 0x41, 0x67, 0x37, 0x01,
	0x7F, 0x00, 0x01, 0xAA, 0x01, 0x01, 0x03, 0x00, 0x25, 0x06, 0x10, 0x9E,
	0x25, 0x05, 0x05, 0x24, 0xBF, 0x01, 0x00, 0x00, 0x02, 0x00, 0x83, 0x03,
	0x00, 0x04, 0x6D, 0x24, 0x02, 0x00, 0x25, 0x05, 0x01, 0x00, 0x41, 0x67,
	0x37, 0x01, 0x7F, 0x00, 0x01, 0xAA, 0x01, 0x01, 0x03, 0x00, 0x25, 0x06,
	0x10, 0xB7, 0x25, 0x05, 0x05, 0x24, 0xBF, 0x01, 0x00, 0x00, 0x02, 0x00,
	0x83, 0x03, 0x00, 0x04, 0x6D, 0x24, 0x02, 0x00, 0x25, 0x05, 0x01, 0x00,
	0x41, 0x67, 0x37, 0x01, 0x7F, 0x00, 0x00, 0xB7, 0x01, 0x08, 0x0E, 0x3B,
	0xB7, 0x34, 0x09, 0x00, 0x00, 0xB7, 0x3B, 0xB7, 0x01, 0x08, 0x0E, 0x34,
	0x09, 0x00, 0x00, 0x25, 0x05, 0x02, 0x4F, 0x28, 0x41, 0xB8, 0x00, 0x00,
	0x32, 0x25, 0x01, 0x00, 0x13, 0x06, 0x01, 0x00, 0x24, 0x19, 0x04, 0x74,
	0x00, 0x01, 0x01, 0x00, 0x00, 0x01, 0x0B, 0x00, 0x00, 0x01, 0x15, 0x00,
	0x00, 0x01, 0x1F, 0x00, 0x00, 0x01, 0x29, 0x00, 0x00, 0x01, 0x33, 0x00,
	0x00, 0xC0, 0x24, 0x00, 0x00, 0x25, 0x06, 0x07, 0xC1, 0x25, 0x06, 0x01,
	0x19, 0x04, 0x76, 0x00, 0x00, 0x01, 0x00, 0x30, 0x31, 0x0B, 0x42, 0x00,
	0x00, 0x01, 0x81, 0x70, 0x00, 0x00, 0x01, 0x82, 0x00, 0x00, 0x00, 0x01,
	0x82, 0x15, 0x00, 0x00, 0x01, 0x81, 0x78, 0x00, 0x00, 0x01, 0x03, 0x33,
	0x01, 0x03, 0x33, 0x00, 0x00, 0x25, 0x01, 0x83, 0xFB, 0x50, 0x01, 0x83,
	0xFD, 0x5F, 0x72, 0x06, 0x04, 0x24, 0x01, 0x00, 0x00, 0x25, 0x01, 0x83,
	0xB0, 0x00, 0x01, 0x83, 0xBF, 0x7F, 0x72, 0x06, 0x04, 0x24, 0x01, 0x00,
	0x00, 0x01, 0x83, 0xFF, 0x7F, 0x15, 0x01, 0x83, 0xFF, 0x7E, 0x0D, 0x00
};

static const uint16_t t0_caddr[] = {
	0,
	5,
	10,
	15,
	20,
	25,
	29,
	33,
	37,
	41,
	45,
	49,
	53,
	57,
	61,
	65,
	69,
	73,
	77,
	81,
	85,
	89,
	93,
	97,
	101,
	105,
	109,
	113,
	117,
	121,
	125,
	130,
	135,
	140,
	145,
	150,
	155,
	160,
	165,
	173,
	178,
	183,
	188,
	193,
	198,
	202,
	207,
	212,
	217,
	238,
	243,
	248,
	253,
	282,
	297,
	303,
	309,
	314,
	322,
	330,
	336,
	341,
	352,
	972,
	987,
	991,
	996,
	1001,
	1006,
	1011,
	1016,
	1130,
	1135,
	1147,
	1152,
	1157,
	1161,
	1166,
	1171,
	1176,
	1181,
	1191,
	1196,
	1201,
	1213,
	1228,
	1233,
	1247,
	1269,
	1280,
	1383,
	1430,
	1521,
	1527,
	1590,
	1597,
	1625,
	1653,
	1758,
	1800,
	1813,
	1825,
	1839,
	1854,
	2074,
	2088,
	2105,
	2114,
	2181,
	2237,
	2241,
	2245,
	2250,
	2298,
	2324,
	2400,
	2444,
	2455,
	2540,
	2578,
	2616,
	2626,
	2636,
	2645,
	2658,
	2662,
	2666,
	2670,
	2674,
	2678,
	2682,
	2686,
	2698,
	2706,
	2711,
	2716,
	2721,
	2726,
	2734
};

#define T0_INTERPRETED   61

#define T0_ENTER(ip, rp, slot)   do { \
		const unsigned char *t0_newip; \
		uint32_t t0_lnum; \
		t0_newip = &t0_codeblock[t0_caddr[(slot) - T0_INTERPRETED]]; \
		t0_lnum = t0_parse7E_unsigned(&t0_newip); \
		(rp) += t0_lnum; \
		*((rp) ++) = (uint32_t)((ip) - &t0_codeblock[0]) + (t0_lnum << 16); \
		(ip) = t0_newip; \
	} while (0)

#define T0_DEFENTRY(name, slot) \
void \
name(void *ctx) \
{ \
	t0_context *t0ctx = ctx; \
	t0ctx->ip = &t0_codeblock[0]; \
	T0_ENTER(t0ctx->ip, t0ctx->rp, slot); \
}

T0_DEFENTRY(br_x509_minimal_init_main, 145)

#define T0_NEXT(t0ipp)   (*(*(t0ipp)) ++)

void
br_x509_minimal_run(void *t0ctx)
{
	uint32_t *dp, *rp;
	const unsigned char *ip;

#define T0_LOCAL(x)    (*(rp - 2 - (x)))
#define T0_POP()       (*-- dp)
#define T0_POPi()      (*(int32_t *)(-- dp))
#define T0_PEEK(x)     (*(dp - 1 - (x)))
#define T0_PEEKi(x)    (*(int32_t *)(dp - 1 - (x)))
#define T0_PUSH(v)     do { *dp = (v); dp ++; } while (0)
#define T0_PUSHi(v)    do { *(int32_t *)dp = (v); dp ++; } while (0)
#define T0_RPOP()      (*-- rp)
#define T0_RPOPi()     (*(int32_t *)(-- rp))
#define T0_RPUSH(v)    do { *rp = (v); rp ++; } while (0)
#define T0_RPUSHi(v)   do { *(int32_t *)rp = (v); rp ++; } while (0)
#define T0_ROLL(x)     do { \
	size_t t0len = (size_t)(x); \
	uint32_t t0tmp = *(dp - 1 - t0len); \
	memmove(dp - t0len - 1, dp - t0len, t0len * sizeof *dp); \
	*(dp - 1) = t0tmp; \
} while (0)
#define T0_SWAP()      do { \
	uint32_t t0tmp = *(dp - 2); \
	*(dp - 2) = *(dp - 1); \
	*(dp - 1) = t0tmp; \
} while (0)
#define T0_ROT()       do { \
	uint32_t t0tmp = *(dp - 3); \
	*(dp - 3) = *(dp - 2); \
	*(dp - 2) = *(dp - 1); \
	*(dp - 1) = t0tmp; \
} while (0)
#define T0_NROT()       do { \
	uint32_t t0tmp = *(dp - 1); \
	*(dp - 1) = *(dp - 2); \
	*(dp - 2) = *(dp - 3); \
	*(dp - 3) = t0tmp; \
} while (0)
#define T0_PICK(x)      do { \
	uint32_t t0depth = (x); \
	T0_PUSH(T0_PEEK(t0depth)); \
} while (0)
#define T0_CO()         do { \
	goto t0_exit; \
} while (0)
#define T0_RET()        goto t0_next

	dp = ((t0_context *)t0ctx)->dp;
	rp = ((t0_context *)t0ctx)->rp;
	ip = ((t0_context *)t0ctx)->ip;
	goto t0_next;
	for (;;) {
		uint32_t t0x;

	t0_next:
		t0x = T0_NEXT(&ip);
		if (t0x < T0_INTERPRETED) {
			switch (t0x) {
				int32_t t0off;

			case 0: /* ret */
				t0x = T0_RPOP();
				rp -= (t0x >> 16);
				t0x &= 0xFFFF;
				if (t0x == 0) {
					ip = NULL;
					goto t0_exit;
				}
				ip = &t0_codeblock[t0x];
				break;
			case 1: /* literal constant */
				T0_PUSHi(t0_parse7E_signed(&ip));
				break;
			case 2: /* read local */
				T0_PUSH(T0_LOCAL(t0_parse7E_unsigned(&ip)));
				break;
			case 3: /* write local */
				T0_LOCAL(t0_parse7E_unsigned(&ip)) = T0_POP();
				break;
			case 4: /* jump */
				t0off = t0_parse7E_signed(&ip);
				ip += t0off;
				break;
			case 5: /* jump if */
				t0off = t0_parse7E_signed(&ip);
				if (T0_POP()) {
					ip += t0off;
				}
				break;
			case 6: /* jump if not */
				t0off = t0_parse7E_signed(&ip);
				if (!T0_POP()) {
					ip += t0off;
				}
				break;
			case 7: {
				/* %25 */

	int32_t b = T0_POPi();
	int32_t a = T0_POPi();
	T0_PUSHi(a % b);

				}
				break;
			case 8: {
				/* * */

	uint32_t b = T0_POP();
	uint32_t a = T0_POP();
	T0_PUSH(a * b);

				}
				break;
			case 9: {
				/* + */

	uint32_t b = T0_POP();
	uint32_t a = T0_POP();
	T0_PUSH(a + b);

				}
				break;
			case 10: {
				/* - */

	uint32_t b = T0_POP();
	uint32_t a = T0_POP();
	T0_PUSH(a - b);

				}
				break;
			case 11: {
				/* -rot */
 T0_NROT(); 
				}
				break;
			case 12: {
				/* / */

	int32_t b = T0_POPi();
	int32_t a = T0_POPi();
	T0_PUSHi(a / b);

				}
				break;
			case 13: {
				/* < */

	int32_t b = T0_POPi();
	int32_t a = T0_POPi();
	T0_PUSH(-(uint32_t)(a < b));

				}
				break;
			case 14: {
				/* << */

	int c = (int)T0_POPi();
	uint32_t x = T0_POP();
	T0_PUSH(x << c);

				}
				break;
			case 15: {
				/* <= */

	int32_t b = T0_POPi();
	int32_t a = T0_POPi();
	T0_PUSH(-(uint32_t)(a <= b));

				}
				break;
			case 16: {
				/* <> */

	uint32_t b = T0_POP();
	uint32_t a = T0_POP();
	T0_PUSH(-(uint32_t)(a != b));

				}
				break;
			case 17: {
				/* = */

	uint32_t b = T0_POP();
	uint32_t a = T0_POP();
	T0_PUSH(-(uint32_t)(a == b));

				}
				break;
			case 18: {
				/* > */

	int32_t b = T0_POPi();
	int32_t a = T0_POPi();
	T0_PUSH(-(uint32_t)(a > b));

				}
				break;
			case 19: {
				/* >= */

	int32_t b = T0_POPi();
	int32_t a = T0_POPi();
	T0_PUSH(-(uint32_t)(a >= b));

				}
				break;
			case 20: {
				/* >> */

	int c = (int)T0_POPi();
	int32_t x = T0_POPi();
	T0_PUSHi(x >> c);

				}
				break;
			case 21: {
				/* and */

	uint32_t b = T0_POP();
	uint32_t a = T0_POP();
	T0_PUSH(a & b);

				}
				break;
			case 22: {
				/* blobcopy */

	size_t len = T0_POP();
	unsigned char *src = (unsigned char *)CTX + T0_POP();
	unsigned char *dst = (unsigned char *)CTX + T0_POP();
	memcpy(dst, src, len);

				}
				break;
			case 23: {
				/* check-direct-trust */

	size_t u;

	for (u = 0; u < CTX->trust_anchors_num; u ++) {
		const br_x509_trust_anchor *ta;
		unsigned char hashed_DN[64];
		int kt;

		ta = &CTX->trust_anchors[u];
		if (ta->flags & BR_X509_TA_CA) {
			continue;
		}
		hash_dn(CTX, ta->dn.data, ta->dn.len, hashed_DN);
		if (memcmp(hashed_DN, CTX->current_dn_hash, DNHASH_LEN)) {
			continue;
		}
		kt = CTX->pkey.key_type;
		if ((ta->pkey.key_type & 0x0F) != kt) {
			continue;
		}
		switch (kt) {

		case BR_KEYTYPE_RSA:
			if (!eqbigint(CTX->pkey.key.rsa.n,
				CTX->pkey.key.rsa.nlen,
				ta->pkey.key.rsa.n,
				ta->pkey.key.rsa.nlen)
				|| !eqbigint(CTX->pkey.key.rsa.e,
				CTX->pkey.key.rsa.elen,
				ta->pkey.key.rsa.e,
				ta->pkey.key.rsa.elen))
			{
				continue;
			}
			break;

		case BR_KEYTYPE_EC:
			if (CTX->pkey.key.ec.curve != ta->pkey.key.ec.curve
				|| CTX->pkey.key.ec.qlen != ta->pkey.key.ec.qlen
				|| memcmp(CTX->pkey.key.ec.q,
					ta->pkey.key.ec.q,
					ta->pkey.key.ec.qlen) != 0)
			{
				continue;
			}
			break;

		default:
			continue;
		}

		/*
		 * Direct trust match!
		 */
		CTX->err = BR_ERR_X509_OK;
		T0_CO();
	}

				}
				break;
			case 24: {
				/* check-trust-anchor-CA */

	size_t u;

	for (u = 0; u < CTX->trust_anchors_num; u ++) {
		const br_x509_trust_anchor *ta;
		unsigned char hashed_DN[64];

		ta = &CTX->trust_anchors[u];
		if (!(ta->flags & BR_X509_TA_CA)) {
			continue;
		}
		hash_dn(CTX, ta->dn.data, ta->dn.len, hashed_DN);
		if (memcmp(hashed_DN, CTX->saved_dn_hash, DNHASH_LEN)) {
			continue;
		}
		if (verify_signature(CTX, &ta->pkey) == 0) {
			CTX->err = BR_ERR_X509_OK;
			T0_CO();
		}
	}

				}
				break;
			case 25: {
				/* co */
 T0_CO(); 
				}
				break;
			case 26: {
				/* compute-dn-hash */

	CTX->dn_hash_impl->out(&CTX->dn_hash.vtable, CTX->current_dn_hash);
	CTX->do_dn_hash = 0;

				}
				break;
			case 27: {
				/* compute-tbs-hash */

	int id = T0_POPi();
	size_t len;
	len = br_multihash_out(&CTX->mhash, id, CTX->tbs_hash);
	T0_PUSH(len);

				}
				break;
			case 28: {
				/* copy-ee-ec-pkey */

	size_t qlen = T0_POP();
	uint32_t curve = T0_POP();
	memcpy(CTX->ee_pkey_data, CTX->pkey_data, qlen);
	CTX->pkey.key_type = BR_KEYTYPE_EC;
	CTX->pkey.key.ec.curve = curve;
	CTX->pkey.key.ec.q = CTX->ee_pkey_data;
	CTX->pkey.key.ec.qlen = qlen;

				}
				break;
			case 29: {
				/* copy-ee-rsa-pkey */

	size_t elen = T0_POP();
	size_t nlen = T0_POP();
	memcpy(CTX->ee_pkey_data, CTX->pkey_data, nlen + elen);
	CTX->pkey.key_type = BR_KEYTYPE_RSA;
	CTX->pkey.key.rsa.n = CTX->ee_pkey_data;
	CTX->pkey.key.rsa.nlen = nlen;
	CTX->pkey.key.rsa.e = CTX->ee_pkey_data + nlen;
	CTX->pkey.key.rsa.elen = elen;

				}
				break;
			case 30: {
				/* copy-name-SAN */

	unsigned tag = T0_POP();
	unsigned ok = T0_POP();
	size_t u, len;

	len = CTX->pad[0];
	for (u = 0; u < CTX->num_name_elts; u ++) {
		br_name_element *ne;

		ne = &CTX->name_elts[u];
		if (ne->status == 0 && ne->oid[0] == 0 && ne->oid[1] == tag) {
			if (ok && ne->len > len) {
				memcpy(ne->buf, CTX->pad + 1, len);
				ne->buf[len] = 0;
				ne->status = 1;
			} else {
				ne->status = -1;
			}
			break;
		}
	}

				}
				break;
			case 31: {
				/* copy-name-element */

	size_t len;
	int32_t off = T0_POPi();
	int ok = T0_POPi();

	if (off >= 0) {
		br_name_element *ne = &CTX->name_elts[off];

		if (ok) {
			len = CTX->pad[0];
			if (len < ne->len) {
				memcpy(ne->buf, CTX->pad + 1, len);
				ne->buf[len] = 0;
				ne->status = 1;
			} else {
				ne->status = -1;
			}
		} else {
			ne->status = -1;
		}
	}

				}
				break;
			case 32: {
				/* data-get8 */

	size_t addr = T0_POP();
	T0_PUSH(t0_datablock[addr]);

				}
				break;
			case 33: {
				/* dn-hash-length */

	T0_PUSH(DNHASH_LEN);

				}
				break;
			case 34: {
				/* do-ecdsa-vrfy */

	size_t qlen = T0_POP();
	int curve = T0_POP();
	br_x509_pkey pk;

	pk.key_type = BR_KEYTYPE_EC;
	pk.key.ec.curve = curve;
	pk.key.ec.q = CTX->pkey_data;
	pk.key.ec.qlen = qlen;
	T0_PUSH(verify_signature(CTX, &pk));

				}
				break;
			case 35: {
				/* do-rsa-vrfy */

	size_t elen = T0_POP();
	size_t nlen = T0_POP();
	br_x509_pkey pk;

	pk.key_type = BR_KEYTYPE_RSA;
	pk.key.rsa.n = CTX->pkey_data;
	pk.key.rsa.nlen = nlen;
	pk.key.rsa.e = CTX->pkey_data + nlen;
	pk.key.rsa.elen = elen;
	T0_PUSH(verify_signature(CTX, &pk));

				}
				break;
			case 36: {
				/* drop */
 (void)T0_POP(); 
				}
				break;
			case 37: {
				/* dup */
 T0_PUSH(T0_PEEK(0)); 
				}
				break;
			case 38: {
				/* eqOID */

	const unsigned char *a2 = &t0_datablock[T0_POP()];
	const unsigned char *a1 = &CTX->pad[0];
	size_t len = a1[0];
	int x;
	if (len == a2[0]) {
		x = -(memcmp(a1 + 1, a2 + 1, len) == 0);
	} else {
		x = 0;
	}
	T0_PUSH((uint32_t)x);

				}
				break;
			case 39: {
				/* eqblob */

	size_t len = T0_POP();
	const unsigned char *a2 = (const unsigned char *)CTX + T0_POP();
	const unsigned char *a1 = (const unsigned char *)CTX + T0_POP();
	T0_PUSHi(-(memcmp(a1, a2, len) == 0));

				}
				break;
			case 40: {
				/* fail */

	CTX->err = T0_POPi();
	T0_CO();

				}
				break;
			case 41: {
				/* get-system-date */

	if (CTX->days == 0 && CTX->seconds == 0) {
#if BR_USE_UNIX_TIME
		time_t x = time(NULL);

		T0_PUSH((uint32_t)(x / 86400) + 719528);
		T0_PUSH((uint32_t)(x % 86400));
#elif BR_USE_WIN32_TIME
		FILETIME ft;
		uint64_t x;

		GetSystemTimeAsFileTime(&ft);
		x = ((uint64_t)ft.dwHighDateTime << 32)
			+ (uint64_t)ft.dwLowDateTime;
		x = (x / 10000000);
		T0_PUSH((uint32_t)(x / 86400) + 584754);
		T0_PUSH((uint32_t)(x % 86400));
#else
		CTX->err = BR_ERR_X509_TIME_UNKNOWN;
		T0_CO();
#endif
	} else {
		T0_PUSH(CTX->days);
		T0_PUSH(CTX->seconds);
	}

				}
				break;
			case 42: {
				/* get16 */

	uint32_t addr = T0_POP();
	T0_PUSH(*(uint16_t *)((unsigned char *)CTX + addr));

				}
				break;
			case 43: {
				/* get32 */

	uint32_t addr = T0_POP();
	T0_PUSH(*(uint32_t *)((unsigned char *)CTX + addr));

				}
				break;
			case 44: {
				/* match-server-name */

	size_t n1, n2;

	if (CTX->server_name == NULL) {
		T0_PUSH(0);
		T0_RET();
	}
	n1 = strlen(CTX->server_name);
	n2 = CTX->pad[0];
	if (n1 == n2 && eqnocase(&CTX->pad[1], CTX->server_name, n1)) {
		T0_PUSHi(-1);
		T0_RET();
	}
	if (n2 >= 2 && CTX->pad[1] == '*' && CTX->pad[2] == '.') {
		size_t u;

		u = 0;
		while (u < n1 && CTX->server_name[u] != '.') {
			u ++;
		}
		u ++;
		n1 -= u;
		if ((n2 - 2) == n1
			&& eqnocase(&CTX->pad[3], CTX->server_name + u, n1))
		{
			T0_PUSHi(-1);
			T0_RET();
		}
	}
	T0_PUSH(0);

				}
				break;
			case 45: {
				/* neg */

	uint32_t a = T0_POP();
	T0_PUSH(-a);

				}
				break;
			case 46: {
				/* offset-name-element */

	unsigned san = T0_POP();
	size_t u;

	for (u = 0; u < CTX->num_name_elts; u ++) {
		if (CTX->name_elts[u].status == 0) {
			const unsigned char *oid;
			size_t len, off;

			oid = CTX->name_elts[u].oid;
			if (san) {
				if (oid[0] != 0 || oid[1] != 0) {
					continue;
				}
				off = 2;
			} else {
				off = 0;
			}
			len = oid[off];
			if (len != 0 && len == CTX->pad[0]
				&& memcmp(oid + off + 1,
					CTX->pad + 1, len) == 0)
			{
				T0_PUSH(u);
				T0_RET();
			}
		}
	}
	T0_PUSHi(-1);

				}
				break;
			case 47: {
				/* or */

	uint32_t b = T0_POP();
	uint32_t a = T0_POP();
	T0_PUSH(a | b);

				}
				break;
			case 48: {
				/* over */
 T0_PUSH(T0_PEEK(1)); 
				}
				break;
			case 49: {
				/* read-blob-inner */

	uint32_t len = T0_POP();
	uint32_t addr = T0_POP();
	size_t clen = CTX->hlen;
	if (clen > len) {
		clen = (size_t)len;
	}
	if (addr != 0) {
		memcpy((unsigned char *)CTX + addr, CTX->hbuf, clen);
	}
	if (CTX->do_mhash) {
		br_multihash_update(&CTX->mhash, CTX->hbuf, clen);
	}
	if (CTX->do_dn_hash) {
		CTX->dn_hash_impl->update(
			&CTX->dn_hash.vtable, CTX->hbuf, clen);
	}
	CTX->hbuf += clen;
	CTX->hlen -= clen;
	T0_PUSH(addr + clen);
	T0_PUSH(len - clen);

				}
				break;
			case 50: {
				/* read8-low */

	if (CTX->hlen == 0) {
		T0_PUSHi(-1);
	} else {
		unsigned char x = *CTX->hbuf ++;
		if (CTX->do_mhash) {
			br_multihash_update(&CTX->mhash, &x, 1);
		}
		if (CTX->do_dn_hash) {
			CTX->dn_hash_impl->update(&CTX->dn_hash.vtable, &x, 1);
		}
		CTX->hlen --;
		T0_PUSH(x);
	}

				}
				break;
			case 51: {
				/* roll */
 T0_ROLL(T0_POP()); 
				}
				break;
			case 52: {
				/* rot */
 T0_ROT(); 
				}
				break;
			case 53: {
				/* set16 */

	uint32_t addr = T0_POP();
	*(uint16_t *)((unsigned char *)CTX + addr) = T0_POP();

				}
				break;
			case 54: {
				/* set32 */

	uint32_t addr = T0_POP();
	*(uint32_t *)((unsigned char *)CTX + addr) = T0_POP();

				}
				break;
			case 55: {
				/* set8 */

	uint32_t addr = T0_POP();
	*((unsigned char *)CTX + addr) = (unsigned char)T0_POP();

				}
				break;
			case 56: {
				/* start-dn-hash */

	CTX->dn_hash_impl->init(&CTX->dn_hash.vtable);
	CTX->do_dn_hash = 1;

				}
				break;
			case 57: {
				/* start-tbs-hash */

	br_multihash_init(&CTX->mhash);
	CTX->do_mhash = 1;

				}
				break;
			case 58: {
				/* stop-tbs-hash */

	CTX->do_mhash = 0;

				}
				break;
			case 59: {
				/* swap */
 T0_SWAP(); 
				}
				break;
			case 60: {
				/* zero-server-name */

	T0_PUSHi(-(CTX->server_name == NULL));

				}
				break;
			}

		} else {
			T0_ENTER(ip, rp, t0x);
		}
	}
t0_exit:
	((t0_context *)t0ctx)->dp = dp;
	((t0_context *)t0ctx)->rp = rp;
	((t0_context *)t0ctx)->ip = ip;
}


/* ===== src/x509/x509_minimal_full.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

/* see bearssl_x509.h */
void
br_x509_minimal_init_full(br_x509_minimal_context *xc,
	const br_x509_trust_anchor *trust_anchors, size_t trust_anchors_num)
{
	/*
	 * All hash functions are activated.
	 * Note: the X.509 validation engine will nonetheless refuse to
	 * validate signatures that use MD5 as hash function.
	 */
	static const br_hash_class *hashes[] = {
		&br_md5_vtable,
		&br_sha1_vtable,
		&br_sha224_vtable,
		&br_sha256_vtable,
		&br_sha384_vtable,
		&br_sha512_vtable
	};

	int id;

	br_x509_minimal_init(xc, &br_sha256_vtable,
		trust_anchors, trust_anchors_num);
	br_x509_minimal_set_rsa(xc, &br_rsa_i31_pkcs1_vrfy);
	br_x509_minimal_set_ecdsa(xc,
		&br_ec_prime_i31, &br_ecdsa_i31_vrfy_asn1);
	for (id = br_md5_ID; id <= br_sha512_ID; id ++) {
		const br_hash_class *hc;

		hc = hashes[id - 1];
		br_x509_minimal_set_hash(xc, id, hc);
	}
}


/* ===== src/x509/x509_knownkey.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

/* see bearssl_x509.h */
void
br_x509_knownkey_init_rsa(br_x509_knownkey_context *ctx,
	const br_rsa_public_key *pk, unsigned usages)
{
	ctx->vtable = &br_x509_knownkey_vtable;
	ctx->pkey.key_type = BR_KEYTYPE_RSA;
	ctx->pkey.key.rsa = *pk;
	ctx->usages = usages;
}

/* see bearssl_x509.h */
void
br_x509_knownkey_init_ec(br_x509_knownkey_context *ctx,
	const br_ec_public_key *pk, unsigned usages)
{
	ctx->vtable = &br_x509_knownkey_vtable;
	ctx->pkey.key_type = BR_KEYTYPE_EC;
	ctx->pkey.key.ec = *pk;
	ctx->usages = usages;
}

static void
kk_start_chain(const br_x509_class **ctx, const char *server_name)
{
	(void)ctx;
	(void)server_name;
}

static void
kk_start_cert(const br_x509_class **ctx, uint32_t length)
{
	(void)ctx;
	(void)length;
}

static void
kk_append(const br_x509_class **ctx, const unsigned char *buf, size_t len)
{
	(void)ctx;
	(void)buf;
	(void)len;
}

static void
kk_end_cert(const br_x509_class **ctx)
{
	(void)ctx;
}

static unsigned
kk_end_chain(const br_x509_class **ctx)
{
	(void)ctx;
	return 0;
}

static const br_x509_pkey *
kk_get_pkey(const br_x509_class *const *ctx, unsigned *usages)
{
	const br_x509_knownkey_context *xc;

	xc = (const br_x509_knownkey_context *)ctx;
	if (usages != NULL) {
		*usages = xc->usages;
	}
	return &xc->pkey;
}

/* see bearssl_x509.h */
const br_x509_class br_x509_knownkey_vtable = {
	sizeof(br_x509_knownkey_context),
	kk_start_chain,
	kk_start_cert,
	kk_append,
	kk_end_cert,
	kk_end_chain,
	kk_get_pkey
};


/* ===== src/x509/skey_decoder.c ===== */
/* Automatically generated code; do not modify directly. */

#include <stddef.h>
#include <stdint.h>

typedef struct {
	uint32_t *dp;
	uint32_t *rp;
	const unsigned char *ip;
} t0_context;

static uint32_t
t0_parse7E_unsigned(const unsigned char **p)
{
	uint32_t x;

	x = 0;
	for (;;) {
		unsigned y;

		y = *(*p) ++;
		x = (x << 7) | (uint32_t)(y & 0x7F);
		if (y < 0x80) {
			return x;
		}
	}
}

static int32_t
t0_parse7E_signed(const unsigned char **p)
{
	int neg;
	uint32_t x;

	neg = ((**p) >> 6) & 1;
	x = (uint32_t)-neg;
	for (;;) {
		unsigned y;

		y = *(*p) ++;
		x = (x << 7) | (uint32_t)(y & 0x7F);
		if (y < 0x80) {
			if (neg) {
				return -(int32_t)~x - 1;
			} else {
				return (int32_t)x;
			}
		}
	}
}

#define T0_VBYTE(x, n)   (unsigned char)((((uint32_t)(x) >> (n)) & 0x7F) | 0x80)
#define T0_FBYTE(x, n)   (unsigned char)(((uint32_t)(x) >> (n)) & 0x7F)
#define T0_SBYTE(x)      (unsigned char)((((uint32_t)(x) >> 28) + 0xF8) ^ 0xF8)
#define T0_INT1(x)       T0_FBYTE(x, 0)
#define T0_INT2(x)       T0_VBYTE(x, 7), T0_FBYTE(x, 0)
#define T0_INT3(x)       T0_VBYTE(x, 14), T0_VBYTE(x, 7), T0_FBYTE(x, 0)
#define T0_INT4(x)       T0_VBYTE(x, 21), T0_VBYTE(x, 14), T0_VBYTE(x, 7), T0_FBYTE(x, 0)
#define T0_INT5(x)       T0_SBYTE(x), T0_VBYTE(x, 21), T0_VBYTE(x, 14), T0_VBYTE(x, 7), T0_FBYTE(x, 0)

static const uint8_t t0_datablock[];


void br_skey_decoder_init_main(void *t0ctx);

void br_skey_decoder_run(void *t0ctx);



/* (already included) */





/* (already included) */

#define CTX   ((br_skey_decoder_context *)((unsigned char *)t0ctx - offsetof(br_skey_decoder_context, cpu)))
#define CONTEXT_NAME   br_skey_decoder_context

/* see bearssl_x509.h */
void
br_skey_decoder_init(br_skey_decoder_context *ctx)
{
	memset(ctx, 0, sizeof *ctx);
	ctx->cpu.dp = &ctx->dp_stack[0];
	ctx->cpu.rp = &ctx->rp_stack[0];
	br_skey_decoder_init_main(&ctx->cpu);
	br_skey_decoder_run(&ctx->cpu);
}

/* see bearssl_x509.h */
void
br_skey_decoder_push(br_skey_decoder_context *ctx,
	const void *data, size_t len)
{
	ctx->hbuf = data;
	ctx->hlen = len;
	br_skey_decoder_run(&ctx->cpu);
}



static const uint8_t t0_datablock[] = {
	0x00, 0x09, 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x01, 0x01, 0x07,
	0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x02, 0x01, 0x08, 0x2A, 0x86, 0x48, 0xCE,
	0x3D, 0x03, 0x01, 0x07, 0x05, 0x2B, 0x81, 0x04, 0x00, 0x22, 0x05, 0x2B,
	0x81, 0x04, 0x00, 0x23
};

static const uint8_t t0_codeblock[] = {
	0x00, 0x01, 0x01, 0x07, 0x00, 0x00, 0x01, 0x01, 0x08, 0x00, 0x00, 0x13,
	0x13, 0x00, 0x00, 0x01, T0_INT1(BR_ERR_X509_BAD_TAG_CLASS), 0x00, 0x00,
	0x01, T0_INT1(BR_ERR_X509_BAD_TAG_VALUE), 0x00, 0x00, 0x01,
	T0_INT1(BR_ERR_X509_EXTRA_ELEMENT), 0x00, 0x00, 0x01,
	T0_INT1(BR_ERR_X509_INDEFINITE_LENGTH), 0x00, 0x00, 0x01,
	T0_INT1(BR_ERR_X509_INNER_TRUNC), 0x00, 0x00, 0x01,
	T0_INT1(BR_ERR_X509_INVALID_VALUE), 0x00, 0x00, 0x01,
	T0_INT1(BR_ERR_X509_LIMIT_EXCEEDED), 0x00, 0x00, 0x01,
	T0_INT1(BR_ERR_X509_NOT_CONSTRUCTED), 0x00, 0x00, 0x01,
	T0_INT1(BR_ERR_X509_NOT_PRIMITIVE), 0x00, 0x00, 0x01,
	T0_INT1(BR_ERR_X509_OVERFLOW), 0x00, 0x00, 0x01,
	T0_INT1(BR_ERR_X509_UNEXPECTED), 0x00, 0x00, 0x01,
	T0_INT1(BR_ERR_X509_UNSUPPORTED), 0x00, 0x00, 0x01,
	T0_INT1(BR_KEYTYPE_EC), 0x00, 0x00, 0x01, T0_INT1(BR_KEYTYPE_RSA),
	0x00, 0x00, 0x01, T0_INT2(offsetof(CONTEXT_NAME, key_data)), 0x00,
	0x00, 0x01, T0_INT2(offsetof(CONTEXT_NAME, key_type)), 0x00, 0x00,
	0x33, 0x48, 0x00, 0x00, 0x01, T0_INT2(offsetof(CONTEXT_NAME, pad)),
	0x00, 0x00, 0x01, 0x13, 0x00, 0x00, 0x01, 0x1C, 0x00, 0x00, 0x01, 0x22,
	0x00, 0x00, 0x05, 0x02, 0x2C, 0x16, 0x00, 0x00, 0x06, 0x02, 0x2D, 0x16,
	0x00, 0x00, 0x01, 0x10, 0x3D, 0x00, 0x00, 0x0D, 0x05, 0x02, 0x2F, 0x16,
	0x3A, 0x00, 0x00, 0x0D, 0x05, 0x02, 0x2F, 0x16, 0x3B, 0x00, 0x00, 0x06,
	0x02, 0x27, 0x16, 0x00, 0x01, 0x03, 0x00, 0x54, 0x57, 0x01, 0x02, 0x3E,
	0x55, 0x23, 0x06, 0x02, 0x30, 0x16, 0x57, 0x01, 0x04, 0x3E, 0x02, 0x00,
	0x41, 0x3F, 0x00, 0x02, 0x03, 0x00, 0x53, 0x14, 0x14, 0x03, 0x01, 0x48,
	0x0E, 0x06, 0x02, 0x30, 0x16, 0x33, 0x4C, 0x58, 0x01, 0x7F, 0x19, 0x0D,
	0x06, 0x04, 0x13, 0x13, 0x04, 0x29, 0x01, 0x20, 0x19, 0x0D, 0x06, 0x16,
	0x13, 0x3A, 0x53, 0x4D, 0x02, 0x00, 0x06, 0x09, 0x02, 0x00, 0x0C, 0x06,
	0x02, 0x2A, 0x16, 0x04, 0x02, 0x03, 0x00, 0x3F, 0x04, 0x0D, 0x01, 0x21,
	0x19, 0x0D, 0x06, 0x04, 0x13, 0x3A, 0x04, 0x03, 0x30, 0x16, 0x13, 0x5D,
	0x02, 0x00, 0x05, 0x02, 0x30, 0x16, 0x02, 0x00, 0x02, 0x01, 0x1D, 0x00,
	0x02, 0x53, 0x4B, 0x05, 0x02, 0x30, 0x16, 0x5B, 0x15, 0x06, 0x07, 0x5D,
	0x01, 0x7F, 0x03, 0x01, 0x04, 0x16, 0x46, 0x15, 0x06, 0x10, 0x01, 0x00,
	0x03, 0x01, 0x14, 0x06, 0x03, 0x4D, 0x04, 0x02, 0x01, 0x00, 0x03, 0x00,
	0x04, 0x02, 0x30, 0x16, 0x3F, 0x57, 0x01, 0x04, 0x3E, 0x53, 0x02, 0x01,
	0x06, 0x03, 0x43, 0x04, 0x03, 0x02, 0x00, 0x40, 0x3F, 0x5D, 0x02, 0x01,
	0x06, 0x03, 0x32, 0x04, 0x01, 0x31, 0x00, 0x00, 0x54, 0x57, 0x01, 0x02,
	0x3E, 0x55, 0x06, 0x02, 0x30, 0x16, 0x57, 0x01, 0x02, 0x3E, 0x44, 0x3F,
	0x00, 0x07, 0x35, 0x50, 0x14, 0x05, 0x02, 0x2F, 0x16, 0x23, 0x01, 0x03,
	0x0B, 0x33, 0x17, 0x47, 0x07, 0x03, 0x00, 0x4F, 0x4F, 0x35, 0x4E, 0x14,
	0x14, 0x03, 0x01, 0x03, 0x02, 0x51, 0x14, 0x03, 0x03, 0x02, 0x02, 0x07,
	0x14, 0x03, 0x02, 0x51, 0x14, 0x03, 0x04, 0x02, 0x02, 0x07, 0x14, 0x03,
	0x02, 0x51, 0x14, 0x03, 0x05, 0x02, 0x02, 0x07, 0x14, 0x03, 0x02, 0x51,
	0x03, 0x06, 0x02, 0x00, 0x02, 0x01, 0x02, 0x03, 0x02, 0x04, 0x02, 0x05,
	0x02, 0x06, 0x1E, 0x00, 0x00, 0x19, 0x19, 0x00, 0x00, 0x01, 0x0B, 0x00,
	0x00, 0x01, 0x00, 0x20, 0x14, 0x06, 0x08, 0x01, 0x01, 0x21, 0x20, 0x22,
	0x20, 0x04, 0x75, 0x13, 0x00, 0x00, 0x01,
	T0_INT2(3 * BR_X509_BUFSIZE_KEY), 0x00, 0x01, 0x01, 0x87, 0xFF, 0xFF,
	0x7F, 0x54, 0x57, 0x01, 0x02, 0x3E, 0x55, 0x01, 0x01, 0x0E, 0x06, 0x02,
	0x30, 0x16, 0x57, 0x01, 0x02, 0x19, 0x0D, 0x06, 0x06, 0x13, 0x3B, 0x44,
	0x32, 0x04, 0x1C, 0x01, 0x04, 0x19, 0x0D, 0x06, 0x08, 0x13, 0x3B, 0x01,
	0x00, 0x41, 0x31, 0x04, 0x0E, 0x01, 0x10, 0x19, 0x0D, 0x06, 0x05, 0x13,
	0x3A, 0x42, 0x04, 0x03, 0x30, 0x16, 0x13, 0x03, 0x00, 0x3F, 0x02, 0x00,
	0x34, 0x1F, 0x5A, 0x27, 0x16, 0x00, 0x01, 0x45, 0x0A, 0x06, 0x02, 0x29,
	0x16, 0x14, 0x03, 0x00, 0x08, 0x02, 0x00, 0x00, 0x00, 0x57, 0x01, 0x06,
	0x3E, 0x56, 0x00, 0x00, 0x20, 0x14, 0x06, 0x07, 0x1A, 0x14, 0x06, 0x01,
	0x12, 0x04, 0x76, 0x24, 0x00, 0x00, 0x4B, 0x05, 0x02, 0x30, 0x16, 0x37,
	0x15, 0x06, 0x04, 0x01, 0x17, 0x04, 0x12, 0x38, 0x15, 0x06, 0x04, 0x01,
	0x18, 0x04, 0x0A, 0x39, 0x15, 0x06, 0x04, 0x01, 0x19, 0x04, 0x02, 0x30,
	0x16, 0x00, 0x00, 0x1C, 0x57, 0x01, 0x02, 0x3E, 0x09, 0x50, 0x00, 0x00,
	0x35, 0x4E, 0x13, 0x00, 0x03, 0x14, 0x03, 0x00, 0x03, 0x01, 0x03, 0x02,
	0x53, 0x59, 0x14, 0x01, 0x81, 0x00, 0x0F, 0x06, 0x02, 0x2E, 0x16, 0x14,
	0x01, 0x00, 0x0D, 0x06, 0x0B, 0x13, 0x14, 0x05, 0x04, 0x13, 0x01, 0x00,
	0x00, 0x59, 0x04, 0x6F, 0x02, 0x01, 0x14, 0x05, 0x02, 0x2B, 0x16, 0x23,
	0x03, 0x01, 0x02, 0x02, 0x1F, 0x02, 0x02, 0x22, 0x03, 0x02, 0x14, 0x06,
	0x03, 0x59, 0x04, 0x68, 0x13, 0x02, 0x00, 0x02, 0x01, 0x08, 0x00, 0x00,
	0x14, 0x35, 0x1C, 0x08, 0x20, 0x1C, 0x07, 0x20, 0x4E, 0x00, 0x01, 0x59,
	0x14, 0x01, 0x81, 0x00, 0x0A, 0x06, 0x01, 0x00, 0x01, 0x81, 0x00, 0x08,
	0x14, 0x05, 0x02, 0x28, 0x16, 0x03, 0x00, 0x01, 0x00, 0x02, 0x00, 0x01,
	0x00, 0x0E, 0x06, 0x19, 0x02, 0x00, 0x23, 0x03, 0x00, 0x14, 0x01, 0x83,
	0xFF, 0xFF, 0x7F, 0x0E, 0x06, 0x02, 0x29, 0x16, 0x01, 0x08, 0x0B, 0x20,
	0x59, 0x1C, 0x07, 0x04, 0x60, 0x00, 0x00, 0x52, 0x4A, 0x00, 0x00, 0x57,
	0x3C, 0x53, 0x00, 0x01, 0x53, 0x14, 0x05, 0x02, 0x2E, 0x16, 0x59, 0x14,
	0x01, 0x81, 0x00, 0x0F, 0x06, 0x02, 0x2E, 0x16, 0x03, 0x00, 0x14, 0x06,
	0x16, 0x59, 0x02, 0x00, 0x14, 0x01, 0x87, 0xFF, 0xFF, 0x7F, 0x0F, 0x06,
	0x02, 0x2E, 0x16, 0x01, 0x08, 0x0B, 0x07, 0x03, 0x00, 0x04, 0x67, 0x13,
	0x02, 0x00, 0x00, 0x00, 0x53, 0x14, 0x01, 0x81, 0x7F, 0x0E, 0x06, 0x08,
	0x5C, 0x01, 0x00, 0x36, 0x1F, 0x01, 0x00, 0x00, 0x14, 0x36, 0x1F, 0x36,
	0x22, 0x4C, 0x01, 0x7F, 0x00, 0x01, 0x59, 0x03, 0x00, 0x02, 0x00, 0x01,
	0x05, 0x10, 0x01, 0x01, 0x11, 0x18, 0x02, 0x00, 0x01, 0x06, 0x10, 0x14,
	0x01, 0x01, 0x11, 0x06, 0x02, 0x25, 0x16, 0x01, 0x04, 0x0B, 0x02, 0x00,
	0x01, 0x1F, 0x11, 0x14, 0x01, 0x1F, 0x0D, 0x06, 0x02, 0x26, 0x16, 0x07,
	0x00, 0x00, 0x14, 0x05, 0x05, 0x01, 0x00, 0x01, 0x7F, 0x00, 0x57, 0x00,
	0x00, 0x14, 0x05, 0x02, 0x29, 0x16, 0x23, 0x5A, 0x00, 0x00, 0x1B, 0x14,
	0x01, 0x00, 0x0F, 0x06, 0x01, 0x00, 0x13, 0x12, 0x04, 0x74, 0x00, 0x01,
	0x01, 0x00, 0x00, 0x5D, 0x13, 0x00, 0x00, 0x14, 0x06, 0x07, 0x5E, 0x14,
	0x06, 0x01, 0x12, 0x04, 0x76, 0x00, 0x00, 0x01, 0x00, 0x19, 0x1A, 0x09,
	0x24, 0x00
};

static const uint16_t t0_caddr[] = {
	0,
	5,
	10,
	14,
	18,
	22,
	26,
	30,
	34,
	38,
	42,
	46,
	50,
	54,
	58,
	62,
	66,
	70,
	75,
	80,
	84,
	89,
	93,
	97,
	101,
	107,
	113,
	118,
	126,
	134,
	140,
	163,
	244,
	311,
	329,
	404,
	408,
	412,
	429,
	434,
	505,
	519,
	526,
	540,
	573,
	582,
	587,
	654,
	665,
	721,
	725,
	730,
	778,
	804,
	848,
	859,
	868,
	881,
	885,
	889,
	901
};

#define T0_INTERPRETED   34

#define T0_ENTER(ip, rp, slot)   do { \
		const unsigned char *t0_newip; \
		uint32_t t0_lnum; \
		t0_newip = &t0_codeblock[t0_caddr[(slot) - T0_INTERPRETED]]; \
		t0_lnum = t0_parse7E_unsigned(&t0_newip); \
		(rp) += t0_lnum; \
		*((rp) ++) = (uint32_t)((ip) - &t0_codeblock[0]) + (t0_lnum << 16); \
		(ip) = t0_newip; \
	} while (0)

#define T0_DEFENTRY(name, slot) \
void \
name(void *ctx) \
{ \
	t0_context *t0ctx = ctx; \
	t0ctx->ip = &t0_codeblock[0]; \
	T0_ENTER(t0ctx->ip, t0ctx->rp, slot); \
}

T0_DEFENTRY(br_skey_decoder_init_main, 73)

#define T0_NEXT(t0ipp)   (*(*(t0ipp)) ++)

void
br_skey_decoder_run(void *t0ctx)
{
	uint32_t *dp, *rp;
	const unsigned char *ip;

#define T0_LOCAL(x)    (*(rp - 2 - (x)))
#define T0_POP()       (*-- dp)
#define T0_POPi()      (*(int32_t *)(-- dp))
#define T0_PEEK(x)     (*(dp - 1 - (x)))
#define T0_PEEKi(x)    (*(int32_t *)(dp - 1 - (x)))
#define T0_PUSH(v)     do { *dp = (v); dp ++; } while (0)
#define T0_PUSHi(v)    do { *(int32_t *)dp = (v); dp ++; } while (0)
#define T0_RPOP()      (*-- rp)
#define T0_RPOPi()     (*(int32_t *)(-- rp))
#define T0_RPUSH(v)    do { *rp = (v); rp ++; } while (0)
#define T0_RPUSHi(v)   do { *(int32_t *)rp = (v); rp ++; } while (0)
#define T0_ROLL(x)     do { \
	size_t t0len = (size_t)(x); \
	uint32_t t0tmp = *(dp - 1 - t0len); \
	memmove(dp - t0len - 1, dp - t0len, t0len * sizeof *dp); \
	*(dp - 1) = t0tmp; \
} while (0)
#define T0_SWAP()      do { \
	uint32_t t0tmp = *(dp - 2); \
	*(dp - 2) = *(dp - 1); \
	*(dp - 1) = t0tmp; \
} while (0)
#define T0_ROT()       do { \
	uint32_t t0tmp = *(dp - 3); \
	*(dp - 3) = *(dp - 2); \
	*(dp - 2) = *(dp - 1); \
	*(dp - 1) = t0tmp; \
} while (0)
#define T0_NROT()       do { \
	uint32_t t0tmp = *(dp - 1); \
	*(dp - 1) = *(dp - 2); \
	*(dp - 2) = *(dp - 3); \
	*(dp - 3) = t0tmp; \
} while (0)
#define T0_PICK(x)      do { \
	uint32_t t0depth = (x); \
	T0_PUSH(T0_PEEK(t0depth)); \
} while (0)
#define T0_CO()         do { \
	goto t0_exit; \
} while (0)
#define T0_RET()        goto t0_next

	dp = ((t0_context *)t0ctx)->dp;
	rp = ((t0_context *)t0ctx)->rp;
	ip = ((t0_context *)t0ctx)->ip;
	goto t0_next;
	for (;;) {
		uint32_t t0x;

	t0_next:
		t0x = T0_NEXT(&ip);
		if (t0x < T0_INTERPRETED) {
			switch (t0x) {
				int32_t t0off;

			case 0: /* ret */
				t0x = T0_RPOP();
				rp -= (t0x >> 16);
				t0x &= 0xFFFF;
				if (t0x == 0) {
					ip = NULL;
					goto t0_exit;
				}
				ip = &t0_codeblock[t0x];
				break;
			case 1: /* literal constant */
				T0_PUSHi(t0_parse7E_signed(&ip));
				break;
			case 2: /* read local */
				T0_PUSH(T0_LOCAL(t0_parse7E_unsigned(&ip)));
				break;
			case 3: /* write local */
				T0_LOCAL(t0_parse7E_unsigned(&ip)) = T0_POP();
				break;
			case 4: /* jump */
				t0off = t0_parse7E_signed(&ip);
				ip += t0off;
				break;
			case 5: /* jump if */
				t0off = t0_parse7E_signed(&ip);
				if (T0_POP()) {
					ip += t0off;
				}
				break;
			case 6: /* jump if not */
				t0off = t0_parse7E_signed(&ip);
				if (!T0_POP()) {
					ip += t0off;
				}
				break;
			case 7: {
				/* + */

	uint32_t b = T0_POP();
	uint32_t a = T0_POP();
	T0_PUSH(a + b);

				}
				break;
			case 8: {
				/* - */

	uint32_t b = T0_POP();
	uint32_t a = T0_POP();
	T0_PUSH(a - b);

				}
				break;
			case 9: {
				/* -rot */
 T0_NROT(); 
				}
				break;
			case 10: {
				/* < */

	int32_t b = T0_POPi();
	int32_t a = T0_POPi();
	T0_PUSH(-(uint32_t)(a < b));

				}
				break;
			case 11: {
				/* << */

	int c = (int)T0_POPi();
	uint32_t x = T0_POP();
	T0_PUSH(x << c);

				}
				break;
			case 12: {
				/* <> */

	uint32_t b = T0_POP();
	uint32_t a = T0_POP();
	T0_PUSH(-(uint32_t)(a != b));

				}
				break;
			case 13: {
				/* = */

	uint32_t b = T0_POP();
	uint32_t a = T0_POP();
	T0_PUSH(-(uint32_t)(a == b));

				}
				break;
			case 14: {
				/* > */

	int32_t b = T0_POPi();
	int32_t a = T0_POPi();
	T0_PUSH(-(uint32_t)(a > b));

				}
				break;
			case 15: {
				/* >= */

	int32_t b = T0_POPi();
	int32_t a = T0_POPi();
	T0_PUSH(-(uint32_t)(a >= b));

				}
				break;
			case 16: {
				/* >> */

	int c = (int)T0_POPi();
	int32_t x = T0_POPi();
	T0_PUSHi(x >> c);

				}
				break;
			case 17: {
				/* and */

	uint32_t b = T0_POP();
	uint32_t a = T0_POP();
	T0_PUSH(a & b);

				}
				break;
			case 18: {
				/* co */
 T0_CO(); 
				}
				break;
			case 19: {
				/* drop */
 (void)T0_POP(); 
				}
				break;
			case 20: {
				/* dup */
 T0_PUSH(T0_PEEK(0)); 
				}
				break;
			case 21: {
				/* eqOID */

	const unsigned char *a2 = &t0_datablock[T0_POP()];
	const unsigned char *a1 = &CTX->pad[0];
	size_t len = a1[0];
	int x;
	if (len == a2[0]) {
		x = -(memcmp(a1 + 1, a2 + 1, len) == 0);
	} else {
		x = 0;
	}
	T0_PUSH((uint32_t)x);

				}
				break;
			case 22: {
				/* fail */

	CTX->err = T0_POPi();
	T0_CO();

				}
				break;
			case 23: {
				/* get8 */

	uint32_t addr = T0_POP();
	T0_PUSH(*((unsigned char *)CTX + addr));

				}
				break;
			case 24: {
				/* neg */

	uint32_t a = T0_POP();
	T0_PUSH(-a);

				}
				break;
			case 25: {
				/* over */
 T0_PUSH(T0_PEEK(1)); 
				}
				break;
			case 26: {
				/* read-blob-inner */

	uint32_t len = T0_POP();
	uint32_t addr = T0_POP();
	size_t clen = CTX->hlen;
	if (clen > len) {
		clen = (size_t)len;
	}
	if (addr != 0) {
		memcpy((unsigned char *)CTX + addr, CTX->hbuf, clen);
	}
	CTX->hbuf += clen;
	CTX->hlen -= clen;
	T0_PUSH(addr + clen);
	T0_PUSH(len - clen);

				}
				break;
			case 27: {
				/* read8-low */

	if (CTX->hlen == 0) {
		T0_PUSHi(-1);
	} else {
		CTX->hlen --;
		T0_PUSH(*CTX->hbuf ++);
	}

				}
				break;
			case 28: {
				/* rot */
 T0_ROT(); 
				}
				break;
			case 29: {
				/* set-ec-key */

	size_t xlen = T0_POP();
	uint32_t curve = T0_POP();
	CTX->key.ec.curve = curve;
	CTX->key.ec.x = CTX->key_data;
	CTX->key.ec.xlen = xlen;

				}
				break;
			case 30: {
				/* set-rsa-key */

	size_t iqlen = T0_POP();
	size_t dqlen = T0_POP();
	size_t dplen = T0_POP();
	size_t qlen = T0_POP();
	size_t plen = T0_POP();
	uint32_t n_bitlen = T0_POP();
	size_t off;

	CTX->key.rsa.n_bitlen = n_bitlen;
	CTX->key.rsa.p = CTX->key_data;
	CTX->key.rsa.plen = plen;
	off = plen;
	CTX->key.rsa.q = CTX->key_data + off;
	CTX->key.rsa.qlen = qlen;
	off += qlen;
	CTX->key.rsa.dp = CTX->key_data + off;
	CTX->key.rsa.dplen = dplen;
	off += dplen;
	CTX->key.rsa.dq = CTX->key_data + off;
	CTX->key.rsa.dqlen = dqlen;
	off += dqlen;
	CTX->key.rsa.iq = CTX->key_data + off;
	CTX->key.rsa.iqlen = iqlen;

				}
				break;
			case 31: {
				/* set8 */

	uint32_t addr = T0_POP();
	*((unsigned char *)CTX + addr) = (unsigned char)T0_POP();

				}
				break;
			case 32: {
				/* swap */
 T0_SWAP(); 
				}
				break;
			case 33: {
				/* u>> */

	int c = (int)T0_POPi();
	uint32_t x = T0_POP();
	T0_PUSH(x >> c);

				}
				break;
			}

		} else {
			T0_ENTER(ip, rp, t0x);
		}
	}
t0_exit:
	((t0_context *)t0ctx)->dp = dp;
	((t0_context *)t0ctx)->rp = rp;
	((t0_context *)t0ctx)->ip = ip;
}


/* ===== src/ssl/prf.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

/* see inner.h */
void
br_tls_phash(void *dst, size_t len,
	const br_hash_class *dig,
	const void *secret, size_t secret_len,
	const char *label, const void *seed, size_t seed_len)
{
	unsigned char *buf;
	unsigned char tmp[64], a[64];
	br_hmac_key_context kc;
	br_hmac_context hc;
	size_t label_len, hlen;

	if (len == 0) {
		return;
	}
	buf = dst;
	for (label_len = 0; label[label_len]; label_len ++);
	hlen = br_digest_size(dig);
	br_hmac_key_init(&kc, dig, secret, secret_len);
	br_hmac_init(&hc, &kc, 0);
	br_hmac_update(&hc, label, label_len);
	br_hmac_update(&hc, seed, seed_len);
	br_hmac_out(&hc, a);
	for (;;) {
		size_t u;

		br_hmac_init(&hc, &kc, 0);
		br_hmac_update(&hc, a, hlen);
		br_hmac_update(&hc, label, label_len);
		br_hmac_update(&hc, seed, seed_len);
		br_hmac_out(&hc, tmp);
		for (u = 0; u < hlen && u < len; u ++) {
			buf[u] ^= tmp[u];
		}
		buf += u;
		len -= u;
		if (len == 0) {
			return;
		}
		br_hmac_init(&hc, &kc, 0);
		br_hmac_update(&hc, a, hlen);
		br_hmac_out(&hc, a);
	}
}


/* ===== src/ssl/prf_md5sha1.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

/* see bearssl.h */
void
br_tls10_prf(void *dst, size_t len,
	const void *secret, size_t secret_len,
	const char *label, const void *seed, size_t seed_len)
{
	const unsigned char *s1;
	size_t slen;

	s1 = secret;
	slen = (secret_len + 1) >> 1;
	memset(dst, 0, len);
	br_tls_phash(dst, len, &br_md5_vtable,
		s1, slen, label, seed, seed_len);
	br_tls_phash(dst, len, &br_sha1_vtable,
		s1 + secret_len - slen, slen, label, seed, seed_len);
}


/* ===== src/ssl/prf_sha256.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

/* see bearssl.h */
void
br_tls12_sha256_prf(void *dst, size_t len,
	const void *secret, size_t secret_len,
	const char *label, const void *seed, size_t seed_len)
{
	memset(dst, 0, len);
	br_tls_phash(dst, len, &br_sha256_vtable,
		secret, secret_len, label, seed, seed_len);
}


/* ===== src/ssl/prf_sha384.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

/* see bearssl.h */
void
br_tls12_sha384_prf(void *dst, size_t len,
	const void *secret, size_t secret_len,
	const char *label, const void *seed, size_t seed_len)
{
	memset(dst, 0, len);
	br_tls_phash(dst, len, &br_sha384_vtable,
		secret, secret_len, label, seed, seed_len);
}


/* ===== src/ssl/ssl_engine.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

/*
 * If BR_USE_URANDOM is not defined, then try to autodetect its presence
 * through compiler macros.
 */
#ifndef BR_USE_URANDOM

/*
 * Macro values documented on:
 *    https://sourceforge.net/p/predef/wiki/OperatingSystems/
 *
 * Only the most common systems have been included here for now. This
 * should be enriched later on.
 */
#if defined _AIX \
	|| defined __ANDROID__ \
	|| defined __FreeBSD__ \
	|| defined __NetBSD__ \
	|| defined __OpenBSD__ \
	|| defined __DragonFly__ \
	|| defined __linux__ \
	|| (defined __sun && (defined __SVR4 || defined __svr4__)) \
	|| (defined __APPLE__ && defined __MACH__)
#define BR_USE_URANDOM   1
#endif

#endif

/*
 * If BR_USE_WIN32_RAND is not defined, perform autodetection here.
 */
#ifndef BR_USE_WIN32_RAND

#if defined _WIN32 || defined _WIN64
#define BR_USE_WIN32_RAND   1
#endif

#endif

#if BR_USE_URANDOM
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#endif

#if BR_USE_WIN32_RAND
#include <windows.h>
#include <wincrypt.h>
#pragma comment(lib, "advapi32")
#endif

/* ==================================================================== */
/*
 * This part of the file does the low-level record management.
 */

/*
 * IMPLEMENTATION NOTES
 * ====================
 *
 * In this file, we designate by "input" (and the "i" letter) the "recv"
 * operations: incoming records from the peer, from which payload data
 * is obtained, and must be extracted by the application (or the SSL
 * handshake engine). Similarly, "output" (and the "o" letter) is for
 * "send": payload data injected by the application (and SSL handshake
 * engine), to be wrapped into records, that are then conveyed to the
 * peer over the transport medium.
 *
 * The input and output buffers may be distinct or shared. When
 * shared, input and output cannot occur concurrently; the caller
 * must make sure that it never needs to output data while input
 * data has been received. In practice, a shared buffer prevents
 * pipelining of HTTP requests, or similar protocols; however, a
 * shared buffer saves RAM.
 *
 * The input buffer is pointed to by 'ibuf' and has size 'ibuf_len';
 * the output buffer is pointed to by 'obuf' and has size 'obuf_len'.
 * From the size of these buffers is derived the maximum fragment
 * length, which will be honoured upon sending records; regardless of
 * that length, incoming records will be processed as long as they
 * fit in the input buffer, and their length still complies with the
 * protocol specification (maximum plaintext payload length is 16384
 * bytes).
 *
 * Three registers are used to manage buffering in ibuf, called ixa,
 * ixb and ixc. Similarly, three registers are used to manage buffering
 * in obuf, called oxa, oxb and oxc.
 *
 *
 * At any time, the engine is in one of the following modes:
 * -- Failed mode: an error occurs, no I/O can happen.
 * -- Input mode: the engine can either receive record bytes from the
 * transport layer, or it has some buffered payload bytes to yield.
 * -- Output mode: the engine can either receive payload bytes, or it
 * has some record bytes to send to the transport layer.
 * -- Input/Output mode: both input and output modes are active. When
 * the buffer is shared, this can happen only when the buffer is empty
 * (no buffered payload bytes or record bytes in either direction).
 *
 *
 * Failed mode:
 * ------------
 *
 * I/O failed for some reason (invalid received data, not enough room
 * for the next record...). No I/O may ever occur again for this context,
 * until an explicit reset is performed. This mode, and the error code,
 * are also used for protocol errors, especially handshake errors.
 *
 *
 * Input mode:
 * -----------
 *
 *  ixa   index within ibuf[] for the currently read data
 *  ixb   maximum index within ibuf[] for the currently read data
 *  ixc   number of bytes not yet received for the current record
 * 
 * -- When ixa == ixb, there is no available data for readers. When
 * ixa != ixb, there is available data and it starts at offset ixa.
 *
 * -- When waiting for the next record header, ixa and ixb are equal
 * and contain a value ranging from 0 to 4; ixc is equal to 5-ixa.
 *
 * -- When the header has been received, record data is obtained. The
 * ixc field records how many bytes are still needed to reach the
 * end of the current record.
 *
 *    ** If encryption is active, then ixa and ixb are kept equal, and
 *    point to the end of the currently received record bytes. When
 *    ixc reaches 0, decryption/MAC is applied, and ixa and ixb are
 *    adjusted.
 *
 *    ** If encryption is not active, then ixa and ixb are distinct
 *    and data can be read right away. Additional record data is
 *    obtained only when ixa == ixb.
 *
 * Note: in input mode and no encryption, records larger than the buffer
 * size are allowed. When encryption is active, the complete record must
 * fit within the buffer, since it cannot be decrypted/MACed until it
 * has been completely received.
 *
 * -- When receiving the next record header, 'version_in' contains the
 * expected input version (0 if not expecting a specific version); on
 * mismatch, the mode switches to 'failed'.
 *
 * -- When the header has been received, 'version_in' contains the received
 * version. It is up to the caller to check and adjust the 'version_in' field
 * to implement the required semantics.
 *
 * -- The 'record_type_in' field is updated with the incoming record type
 * when the next record header has been received.
 *
 *
 * Output mode:
 * ------------
 *
 *  oxa   index within obuf[] for the currently accumulated data
 *  oxb   maximum index within obuf[] for record data
 *  oxc   pointer for start of record data, and for record sending
 *
 * -- When oxa != oxb, more data can be accumulated into the current
 * record; when oxa == oxb, a closed record is being sent.
 *
 * -- When accumulating data, oxc points to the start of the data.
 *
 * -- During record sending, oxa (and oxb) point to the next record byte
 * to send, and oxc indicates the end of the current record.
 *
 * Note: sent records must fit within the buffer, since the header is
 * adjusted only when the complete record has been assembled.
 *
 * -- The 'version_out' and 'record_type_out' fields are used to build the
 * record header when the mode is switched to 'sending'.
 *
 *
 * Modes:
 * ------
 *
 * The state register iomode contains one of the following values:
 *
 *  BR_IO_FAILED   I/O failed
 *  BR_IO_IN       input mode
 *  BR_IO_OUT      output mode
 *  BR_IO_INOUT    input/output mode
 *
 * Whether encryption is active on incoming records is indicated by the
 * incrypt flag. For outgoing records, there is no such flag; "encryption"
 * is always considered active, but initially uses functions that do not
 * encrypt anything. The 'incrypt' flag is needed because when there is
 * no active encryption, records larger than the I/O buffer are accepted.
 *
 * Note: we do not support no-encryption modes (MAC only).
 *
 * TODO: implement GCM support
 *
 *
 * Misc:
 * -----
 *
 * 'max_frag_len' is the maximum plaintext size for an outgoing record.
 * By default, it is set to the maximum value that fits in the provided
 * buffers, in the following list: 512, 1024, 2048, 4096, 16384. The
 * caller may change it if needed, but the new value MUST still fit in
 * the buffers, and it MUST be one of the list above for compatibility
 * with the Maximum Fragment Length extension.
 *
 * For incoming records, only the total buffer length and current
 * encryption mode impact the maximum length for incoming records. The
 * 'max_frag_len' value is still adjusted so that records up to that
 * length can be both received and sent.
 *
 *
 * Offsets and lengths:
 * --------------------
 *
 * When sending fragments with TLS-1.1+, the maximum overhead is:
 *   5 bytes for the record header
 *   16 bytes for the explicit IV
 *   48 bytes for the MAC (HMAC/SHA-384)
 *   16 bytes for the padding (AES)
 * so a total of 85 extra bytes. Note that we support block cipher sizes
 * up to 16 bytes (AES) and HMAC output sizes up to 48 bytes (SHA-384).
 *
 * With TLS-1.0 and CBC mode, we apply a 1/n-1 split, for a maximum
 * overhead of:
 *   5 bytes for the first record header
 *   32 bytes for the first record payload (AES-CBC + HMAC/SHA-1)
 *   5 bytes for the second record header
 *   20 bytes for the MAC (HMAC/SHA-1)
 *   16 bytes for the padding (AES)
 *   -1 byte to account for the payload byte in the first record
 * so a total of 77 extra bytes at most, less than the 85 bytes above.
 * Note that with TLS-1.0, the MAC is HMAC with either MD5 or SHA-1, but
 * no other hash function.
 *
 * The implementation does not try to send larger records when the current
 * encryption mode has less overhead.
 *
 * Maximum input record overhead is:
 *   5 bytes for the record header
 *   16 bytes for the explicit IV (TLS-1.1+)
 *   48 bytes for the MAC (HMAC/SHA-384)
 *   256 bytes for the padding
 * so a total of 325 extra bytes.
 *
 * When receiving the next record header, it is written into the buffer
 * bytes 0 to 4 (inclusive). Record data is always written into buf[]
 * starting at offset 5. When encryption is active, the plaintext data
 * may start at a larger offset (e.g. because of an explicit IV).
 */

#define MAX_OUT_OVERHEAD    85
#define MAX_IN_OVERHEAD    325

/* see inner.h */
void
br_ssl_engine_fail(br_ssl_engine_context *rc, int err)
{
	if (rc->iomode != BR_IO_FAILED) {
		rc->iomode = BR_IO_FAILED;
		rc->err = err;
	}
}

/*
 * Adjust registers for a new incoming record.
 */
static void
make_ready_in(br_ssl_engine_context *rc)
{
	rc->ixa = rc->ixb = 0;
	rc->ixc = 5;
	if (rc->iomode == BR_IO_IN) {
		rc->iomode = BR_IO_INOUT;
	}
}

/*
 * Adjust registers for a new outgoing record.
 */
static void
make_ready_out(br_ssl_engine_context *rc)
{
	size_t a, b;

	a = 5;
	b = rc->obuf_len - a;
	rc->out.vtable->max_plaintext(&rc->out.vtable, &a, &b);
	if ((b - a) > rc->max_frag_len) {
		b = a + rc->max_frag_len;
	}
	rc->oxa = a;
	rc->oxb = b;
	rc->oxc = a;
	if (rc->iomode == BR_IO_OUT) {
		rc->iomode = BR_IO_INOUT;
	}
}

/* see inner.h */
void
br_ssl_engine_new_max_frag_len(br_ssl_engine_context *rc, unsigned max_frag_len)
{
	size_t nxb;

	rc->max_frag_len = max_frag_len;
	nxb = rc->oxc + max_frag_len;
	if (rc->oxa < rc->oxb && rc->oxb > nxb && rc->oxa < nxb) {
		rc->oxb = nxb;
	}
}

/* see bearssl_ssl.h */
void
br_ssl_engine_set_buffer(br_ssl_engine_context *rc,
	void *buf, size_t buf_len, int bidi)
{
	if (buf == NULL) {
		br_ssl_engine_set_buffers_bidi(rc, NULL, 0, NULL, 0);
	} else {
		/*
		 * In bidirectional mode, we want to maximise input
		 * buffer size, since we support arbitrary fragmentation
		 * when sending, but the peer will not necessarily
		 * comply to any low fragment length (in particular if
		 * we are the server, because the maximum fragment
		 * length extension is under client control).
		 *
		 * We keep a minimum size of 512 bytes for the plaintext
		 * of our outgoing records.
		 *
		 * br_ssl_engine_set_buffers_bidi() will compute the maximum
		 * fragment length for outgoing records by using the minimum
		 * of allocated spaces for both input and output records,
		 * rounded down to a standard length.
		 */
		if (bidi) {
			size_t w;

			if (buf_len < (512 + MAX_IN_OVERHEAD
				+ 512 + MAX_OUT_OVERHEAD))
			{
				rc->iomode = BR_IO_FAILED;
				rc->err = BR_ERR_BAD_PARAM;
				return;
			} else if (buf_len < (16384 + MAX_IN_OVERHEAD
				+ 512 + MAX_OUT_OVERHEAD))
			{
				w = 512 + MAX_OUT_OVERHEAD;
			} else {
				w = buf_len - (16384 + MAX_IN_OVERHEAD);
			}
			br_ssl_engine_set_buffers_bidi(rc,
				buf, buf_len - w,
				(unsigned char *)buf + w, w);
		} else {
			br_ssl_engine_set_buffers_bidi(rc,
				buf, buf_len, NULL, 0);
		}
	}
}

/* see bearssl_ssl.h */
void
br_ssl_engine_set_buffers_bidi(br_ssl_engine_context *rc,
	void *ibuf, size_t ibuf_len, void *obuf, size_t obuf_len)
{
	rc->iomode = BR_IO_INOUT;
	rc->incrypt = 0;
	rc->err = BR_ERR_OK;
	rc->version_in = 0;
	rc->record_type_in = 0;
	rc->version_out = 0;
	rc->record_type_out = 0;
	if (ibuf == NULL) {
		if (rc->ibuf == NULL) {
			br_ssl_engine_fail(rc, BR_ERR_BAD_PARAM);
		}
	} else {
		unsigned u;

		rc->ibuf = ibuf;
		rc->ibuf_len = ibuf_len;
		if (obuf == NULL) {
			obuf = ibuf;
			obuf_len = ibuf_len;
		}
		rc->obuf = obuf;
		rc->obuf_len = obuf_len;

		/*
		 * Compute the maximum fragment length, that fits for
		 * both incoming and outgoing records. This length will
		 * be used in fragment length negotiation, so we must
		 * honour it both ways. Regardless, larger incoming
		 * records will be accepted, as long as they fit in the
		 * actual buffer size.
		 */
		for (u = 14; u >= 9; u --) {
			size_t flen;

			flen = (size_t)1 << u;
			if (obuf_len >= flen + MAX_OUT_OVERHEAD
				&& ibuf_len >= flen + MAX_IN_OVERHEAD)
			{
				break;
			}
		}
		if (u == 8) {
			br_ssl_engine_fail(rc, BR_ERR_BAD_PARAM);
			return;
		} else if (u == 13) {
			u = 12;
		}
		rc->max_frag_len = (size_t)1 << u;
		rc->log_max_frag_len = u;
		rc->peer_log_max_frag_len = 0;
	}
	rc->out.vtable = &br_sslrec_out_clear_vtable;
	make_ready_in(rc);
	make_ready_out(rc);
}

/*
 * Clear buffers in both directions.
 */
static void
engine_clearbuf(br_ssl_engine_context *rc)
{
	make_ready_in(rc);
	make_ready_out(rc);
}

/* see inner.h */
int
br_ssl_engine_init_rand(br_ssl_engine_context *cc)
{
	/*
	 * TODO: use getrandom() on Linux systems, with a fallback to
	 * opening /dev/urandom if that system call fails.
	 *
	 * Use similar OS facilities on other OS (getentropy() on OpenBSD,
	 * specialized sysctl on NetBSD and FreeBSD...).
	 */
#if BR_USE_URANDOM
	if (!cc->rng_os_rand_done) {
		int f;

		f = open("/dev/urandom", O_RDONLY);
		if (f >= 0) {
			unsigned char tmp[32];
			size_t u;

			for (u = 0; u < sizeof tmp;) {
				ssize_t len;

				len = read(f, tmp + u, (sizeof tmp) - u);
				if (len < 0) {
					if (errno == EINTR) {
						continue;
					}
					break;
				}
				u += (size_t)len;
			}
			close(f);
			if (u == sizeof tmp) {
				br_ssl_engine_inject_entropy(cc, tmp, u);
				cc->rng_os_rand_done = 1;
			}
		}
	}
#elif BR_USE_WIN32_RAND
	if (!cc->rng_os_rand_done) {
		HCRYPTPROV hp;

		if (CryptAcquireContextW(&hp, 0, 0, PROV_RSA_FULL,
			CRYPT_VERIFYCONTEXT | CRYPT_SILENT))
		{
			BYTE buf[32];

			if (CryptGenRandom(hp, sizeof buf, buf)) {
				br_ssl_engine_inject_entropy(cc,
					buf, sizeof buf);
				cc->rng_os_rand_done = 1;
			}
			CryptReleaseContext(hp, 0);
		}
	}
#endif

	if (!cc->rng_init_done) {
		br_ssl_engine_fail(cc, BR_ERR_NO_RANDOM);
		return 0;
	}
	return 1;
}

/* see bearssl_ssl.h */
void
br_ssl_engine_inject_entropy(br_ssl_engine_context *cc,
	const void *data, size_t len)
{
	if (cc->rng_init_done) {
		br_hmac_drbg_update(&cc->rng, data, len);
	} else {
		/*
		 * If using TLS-1.2, then SHA-256 or SHA-384 must be
		 * present (or both); we prefer SHA-256 which is faster
		 * for 32-bit systems.
		 *
		 * If using TLS-1.0 or 1.1 then SHA-1 must be present.
		 *
		 * Though HMAC_DRBG/SHA-1 is, as far as we know, as safe
		 * as these things can be, we still prefer the SHA-2
		 * functions over SHA-1, if only for public relations
		 * (known theoretical weaknesses of SHA-1 with regards to
		 * collisions are mostly irrelevant here, but they still
		 * make people nervous).
		 */
		const br_hash_class *h;

		h = br_multihash_getimpl(&cc->mhash, br_sha256_ID);
		if (!h) {
			h = br_multihash_getimpl(&cc->mhash, br_sha384_ID);
			if (!h) {
				h = br_multihash_getimpl(&cc->mhash,
					br_sha1_ID);
				if (!h) {
					br_ssl_engine_fail(cc,
						BR_ERR_BAD_STATE);
					return;
				}
			}
		}
		br_hmac_drbg_init(&cc->rng, h, data, len);
		cc->rng_init_done = 1;
	}
}

/*
 * We define a few internal functions that implement the low-level engine
 * API for I/O; the external API (br_ssl_engine_sendapp_buf() and similar
 * functions) is built upon these function, with special processing for
 * records which are not of type "application data".
 *
 *   recvrec_buf, recvrec_ack     receives bytes from transport medium
 *   sendrec_buf, sendrec_ack     send bytes to transport medium
 *   recvpld_buf, recvpld_ack     receives payload data from engine
 *   sendpld_buf, sendpld_ack     send payload data to engine
 */

static unsigned char *
recvrec_buf(const br_ssl_engine_context *rc, size_t *len)
{
	if (rc->shutdown_recv) {
		*len = 0;
		return NULL;
	}

	/*
	 * Bytes from the transport can be injected only if the mode is
	 * compatible (in or in/out), and ixa == ixb; ixc then contains
	 * the number of bytes that are still expected (but it may
	 * exceed our buffer size).
	 *
	 * We cannot get "stuck" here (buffer is full, but still more
	 * data is expected) because oversized records are detected when
	 * their header is processed.
	 */
	switch (rc->iomode) {
	case BR_IO_IN:
	case BR_IO_INOUT:
		if (rc->ixa == rc->ixb) {
			size_t z;

			z = rc->ixc;
			if (z > rc->ibuf_len - rc->ixa) {
				z = rc->ibuf_len - rc->ixa;
			}
			*len = z;
			return rc->ibuf + rc->ixa;
		}
		break;
	}
	*len = 0;
	return NULL;
}

static void
recvrec_ack(br_ssl_engine_context *rc, size_t len)
{
	unsigned char *pbuf;
	size_t pbuf_len;

	/*
	 * Adjust state if necessary (for a shared input/output buffer):
	 * we got some incoming bytes, so we cannot (temporarily) handle
	 * outgoing data.
	 */
	if (rc->iomode == BR_IO_INOUT && rc->ibuf == rc->obuf) {
		rc->iomode = BR_IO_IN;
	}

	/*
	 * Adjust data pointers.
	 */
	rc->ixb = (rc->ixa += len);
	rc->ixc -= len;

	/*
	 * If we are receiving a header and did not fully obtained it
	 * yet, then just wait for the next bytes.
	 */
	if (rc->ixa < 5) {
		return;
	}

	/*
	 * If we just obtained a full header, process it.
	 */
	if (rc->ixa == 5) {
		unsigned version;
		unsigned rlen;

		/*
		 * Get record type and version. We support only versions
		 * 3.x (if the version major number does not match, then
		 * we suppose that the record format is too alien for us
		 * to process it).
		 *
		 * Note: right now, we reject clients that try to send
		 * a ClientHello in a format compatible with SSL-2.0. It
		 * is unclear whether this will ever be supported; and
		 * if we want to support it, then this might be done in
		 * in the server-specific code, not here.
		 */
		rc->record_type_in = rc->ibuf[0];
		version = br_dec16be(rc->ibuf + 1);
		if ((version >> 8) != 3) {
			br_ssl_engine_fail(rc, BR_ERR_UNSUPPORTED_VERSION);
			return;
		}

		/*
		 * We ensure that successive records have the same
		 * version. The handshake code must check and adjust the
		 * variables when necessary to accommodate the protocol
		 * negotiation details.
		 */
		if (rc->version_in != 0 && rc->version_in != version) {
			br_ssl_engine_fail(rc, BR_ERR_BAD_VERSION);
			return;
		}
		rc->version_in = version;

		/*
		 * Decode record length. We must check that the length
		 * is valid (relatively to the current encryption mode)
		 * and also (if encryption is active) that the record
		 * will fit in our buffer.
		 *
		 * When no encryption is active, we can process records
		 * by chunks, and thus accept any record up to the
		 * maximum allowed plaintext length (16384 bytes).
		 */
		rlen = br_dec16be(rc->ibuf + 3);
		if (rc->incrypt) {
			if (!rc->in.vtable->check_length(
				&rc->in.vtable, rlen))
			{
				br_ssl_engine_fail(rc, BR_ERR_BAD_LENGTH);
				return;
			}
			if (rlen > (rc->ibuf_len - 5)) {
				br_ssl_engine_fail(rc, BR_ERR_TOO_LARGE);
				return;
			}
		} else {
			if (rlen > 16384) {
				br_ssl_engine_fail(rc, BR_ERR_BAD_LENGTH);
				return;
			}
		}

		/*
		 * If the record is completely empty then we must switch
		 * to a new record. Note that, in that case, we
		 * completely ignore the record type, which is fitting
		 * since we received no actual data of that type.
		 *
		 * A completely empty record is technically allowed as
		 * long as encryption/MAC is not active, i.e. before
		 * completion of the first handshake. It it still weird;
		 * it might conceptually be useful as a heartbeat or
		 * keep-alive mechanism while some lengthy operation is
		 * going on, e.g. interaction with a human user.
		 */
		if (rlen == 0) {
			make_ready_in(rc);
		} else {
			rc->ixa = rc->ixb = 5;
			rc->ixc = rlen;
		}
		return;
	}

	/*
	 * If there is no active encryption, then the data can be read
	 * right away. Note that we do not receive bytes from the
	 * transport medium when we still have payload bytes to be
	 * acknowledged.
	 */
	if (!rc->incrypt) {
		rc->ixa = 5;
		return;
	}

	/*
	 * Since encryption is active, we must wait for a full record
	 * before processing it.
	 */
	if (rc->ixc != 0) {
		return;
	}

	/*
	 * We got the full record. Decrypt it.
	 */
	pbuf_len = rc->ixa - 5;
	pbuf = rc->in.vtable->decrypt(&rc->in.vtable,
		rc->record_type_in, rc->version_in, rc->ibuf + 5, &pbuf_len);
	if (pbuf == 0) {
		br_ssl_engine_fail(rc, BR_ERR_BAD_MAC);
		return;
	}
	rc->ixa = (size_t)(pbuf - rc->ibuf);
	rc->ixb = rc->ixa + pbuf_len;

	/*
	 * Decryption may have yielded an empty record, in which case
	 * we get back to "ready" state immediately.
	 */
	if (rc->ixa == rc->ixb) {
		make_ready_in(rc);
	}
}

/* see inner.h */
int
br_ssl_engine_recvrec_finished(const br_ssl_engine_context *rc)
{
	switch (rc->iomode) {
	case BR_IO_IN:
	case BR_IO_INOUT:
		return rc->ixc == 0 || rc->ixa < 5;
	default:
		return 1;
	}
}

static unsigned char *
recvpld_buf(const br_ssl_engine_context *rc, size_t *len)
{
	/*
	 * There is payload data to be read only if the mode is
	 * compatible, and ixa != ixb.
	 */
	switch (rc->iomode) {
	case BR_IO_IN:
	case BR_IO_INOUT:
		*len = rc->ixb - rc->ixa;
		return (*len == 0) ? NULL : (rc->ibuf + rc->ixa);
	default:
		*len = 0;
		return NULL;
	}
}

static void
recvpld_ack(br_ssl_engine_context *rc, size_t len)
{
	rc->ixa += len;

	/*
	 * If we read all the available data, then we either expect
	 * the remainder of the current record (if the current record
	 * was not finished; this may happen when encryption is not
	 * active), or go to "ready" state.
	 */
	if (rc->ixa == rc->ixb) {
		if (rc->ixc == 0) {
			make_ready_in(rc);
		} else {
			rc->ixa = rc->ixb = 5;
		}
	}
}

static unsigned char *
sendpld_buf(const br_ssl_engine_context *rc, size_t *len)
{
	/*
	 * Payload data can be injected only if the current mode is
	 * compatible, and oxa != oxb.
	 */
	switch (rc->iomode) {
	case BR_IO_OUT:
	case BR_IO_INOUT:
		*len = rc->oxb - rc->oxa;
		return (*len == 0) ? NULL : (rc->obuf + rc->oxa);
	default:
		*len = 0;
		return NULL;
	}
}

/*
 * If some payload bytes have been accumulated, then wrap them into
 * an outgoing record. Otherwise, this function does nothing, unless
 * 'force' is non-zero, in which case an empty record is assembled.
 *
 * The caller must take care not to invoke this function if the engine
 * is not currently ready to receive payload bytes to send.
 */
static void
sendpld_flush(br_ssl_engine_context *rc, int force)
{
	size_t xlen;
	unsigned char *buf;

	if (rc->oxa == rc->oxb) {
		return;
	}
	xlen = rc->oxa - rc->oxc;
	if (xlen == 0 && !force) {
		return;
	}
	buf = rc->out.vtable->encrypt(&rc->out.vtable,
		rc->record_type_out, rc->version_out,
		rc->obuf + rc->oxc, &xlen);
	rc->oxb = rc->oxa = (size_t)(buf - rc->obuf);
	rc->oxc = rc->oxa + xlen;
}

static void
sendpld_ack(br_ssl_engine_context *rc, size_t len)
{
	/*
	 * If using a shared buffer, then we may have to modify the
	 * current mode.
	 */
	if (rc->iomode == BR_IO_INOUT && rc->ibuf == rc->obuf) {
		rc->iomode = BR_IO_OUT;
	}
	rc->oxa += len;
	if (rc->oxa >= rc->oxb) {
		/*
		 * Set oxb to one more than oxa so that sendpld_flush()
		 * does not mistakingly believe that a record is
		 * already prepared and being sent.
		 */
		rc->oxb = rc->oxa + 1;
		sendpld_flush(rc, 0);
	}
}

static unsigned char *
sendrec_buf(const br_ssl_engine_context *rc, size_t *len)
{
	/*
	 * When still gathering payload bytes, oxc points to the start
	 * of the record data, so oxc <= oxa. However, when a full
	 * record has been completed, oxc points to the end of the record,
	 * so oxc > oxa.
	 */
	switch (rc->iomode) {
	case BR_IO_OUT:
	case BR_IO_INOUT:
		if (rc->oxc > rc->oxa) {
			*len = rc->oxc - rc->oxa;
			return rc->obuf + rc->oxa;
		}
		break;
	}
	*len = 0;
	return NULL;
}

static void
sendrec_ack(br_ssl_engine_context *rc, size_t len)
{
	rc->oxb = (rc->oxa += len);
	if (rc->oxa == rc->oxc) {
		make_ready_out(rc);
	}
}

/*
 * Test whether there is some buffered outgoing record that still must
 * sent.
 */
static inline int
has_rec_tosend(const br_ssl_engine_context *rc)
{
	return rc->oxa == rc->oxb && rc->oxa != rc->oxc;
}

/*
 * The "no encryption" mode has no overhead. It limits the payload size
 * to the maximum size allowed by the standard (16384 bytes); the caller
 * is responsible for possibly enforcing a smaller fragment length.
 */
static void
clear_max_plaintext(const br_sslrec_out_clear_context *cc,
	size_t *start, size_t *end)
{
	size_t len;

	(void)cc;
	len = *end - *start;
	if (len > 16384) {
		*end = *start + 16384;
	}
}

/*
 * In "no encryption" mode, encryption is trivial (a no-operation) so
 * we just have to encode the header.
 */
static unsigned char *
clear_encrypt(br_sslrec_out_clear_context *cc,
	int record_type, unsigned version, void *data, size_t *data_len)
{
	unsigned char *buf;

	(void)cc;
	buf = (unsigned char *)data - 5;
	buf[0] = record_type;
	br_enc16be(buf + 1, version);
	br_enc16be(buf + 3, *data_len);
	*data_len += 5;
	return buf;
}

/* see bearssl_ssl.h */
const br_sslrec_out_class br_sslrec_out_clear_vtable = {
	sizeof(br_sslrec_out_clear_context),
	(void (*)(const br_sslrec_out_class *const *, size_t *, size_t *))
		&clear_max_plaintext,
	(unsigned char *(*)(const br_sslrec_out_class **,
		int, unsigned, void *, size_t *))
		&clear_encrypt
};

/* ==================================================================== */
/*
 * In this part of the file, we handle the various record types, and
 * communications with the handshake processor.
 */

/*
 * IMPLEMENTATION NOTES
 * ====================
 *
 * The handshake processor is written in T0 and runs as a coroutine.
 * It receives the contents of all records except application data, and
 * is responsible for producing the contents of all records except
 * application data.
 *
 * A state flag is maintained, which specifies whether application data
 * is acceptable or not. When it is set:
 *
 * -- Application data can be injected as payload data (provided that
 *    the output buffer is ready for that).
 *
 * -- Incoming application data records are accepted, and yield data
 *    that the caller may retrieve.
 *
 * When the flag is cleared, application data is not accepted from the
 * application, and incoming application data records trigger an error.
 *
 *
 * Records of type handshake, alert or change-cipher-spec are handled
 * by the handshake processor. The handshake processor is written in T0
 * and runs as a coroutine; it gets invoked whenever one of the following
 * situations is reached:
 *
 * -- An incoming record has type handshake, alert or change-cipher-spec,
 *    and yields data that can be read (zero-length records are thus
 *    ignored).
 *
 * -- An outgoing record has just finished being sent, and the "application
 *    data" flag is cleared.
 *
 * -- The caller wishes to perform a close (call to br_ssl_engine_close()).
 *
 * -- The caller wishes to perform a renegotiation (call to
 *    br_ssl_engine_renegotiate()).
 *
 * Whenever the handshake processor is entered, access to the payload
 * buffers is provided, along with some information about explicit
 * closures or renegotiations.
 */

/* see bearssl_ssl.h */
void
br_ssl_engine_set_suites(br_ssl_engine_context *cc,
	const uint16_t *suites, size_t suites_num)
{
	if ((suites_num * sizeof *suites) > sizeof cc->suites_buf) {
		br_ssl_engine_fail(cc, BR_ERR_BAD_PARAM);
		return;
	}
	memcpy(cc->suites_buf, suites, suites_num * sizeof *suites);
	cc->suites_num = suites_num;
}

/*
 * Give control to handshake processor. 'action' is 1 for a close,
 * 2 for a renegotiation, or 0 for a jump due to I/O completion.
 */
static void
jump_handshake(br_ssl_engine_context *cc, int action)
{
	/*
	 * We use a loop because the handshake processor actions may
	 * allow for more actions; namely, if the processor reads all
	 * input data, then it may allow for output data to be produced,
	 * in case of a shared in/out buffer.
	 */
	for (;;) {
		size_t hlen_in, hlen_out;

		/*
		 * Get input buffer. We do not want to provide
		 * application data to the handshake processor (we could
		 * get called with an explicit close or renegotiation
		 * while there is application data ready to be read).
		 */
		cc->hbuf_in = recvpld_buf(cc, &hlen_in);
		if (cc->hbuf_in != NULL
			&& cc->record_type_in == BR_SSL_APPLICATION_DATA)
		{
			hlen_in = 0;
		}

		/*
		 * Get output buffer. The handshake processor never
		 * leaves an unfinished outgoing record, so if there is
		 * buffered output, then it MUST be some application
		 * data, so the processor cannot write to it.
		 */
		cc->saved_hbuf_out = cc->hbuf_out = sendpld_buf(cc, &hlen_out);
		if (cc->hbuf_out != NULL && br_ssl_engine_has_pld_to_send(cc)) {
			hlen_out = 0;
		}

		/*
		 * Note: hlen_in and hlen_out can be both non-zero only if
		 * the input and output buffers are disjoint. Thus, we can
		 * offer both buffers to the handshake code.
		 */

		cc->hlen_in = hlen_in;
		cc->hlen_out = hlen_out;
		cc->action = action;
		cc->hsrun(&cc->cpu);
		if (br_ssl_engine_closed(cc)) {
			return;
		}
		if (cc->hbuf_out != cc->saved_hbuf_out) {
			sendpld_ack(cc, cc->hbuf_out - cc->saved_hbuf_out);
		}
		if (hlen_in != cc->hlen_in) {
			recvpld_ack(cc, hlen_in - cc->hlen_in);
			if (cc->hlen_in == 0) {
				/*
				 * We read all data bytes, which may have
				 * released the output buffer in case it
				 * is shared with the input buffer, and
				 * the handshake code might be waiting for
				 * that.
				 */
				action = 0;
				continue;
			}
		}
		break;
	}
}

/* see inner.h */
void
br_ssl_engine_flush_record(br_ssl_engine_context *cc)
{
	if (cc->hbuf_out != cc->saved_hbuf_out) {
		sendpld_ack(cc, cc->hbuf_out - cc->saved_hbuf_out);
	}
	if (br_ssl_engine_has_pld_to_send(cc)) {
		sendpld_flush(cc, 0);
	}
	cc->saved_hbuf_out = cc->hbuf_out = sendpld_buf(cc, &cc->hlen_out);
}

/* see bearssl_ssl.h */
unsigned char *
br_ssl_engine_sendapp_buf(const br_ssl_engine_context *cc, size_t *len)
{
	if (!cc->application_data) {
		*len = 0;
		return NULL;
	}
	return sendpld_buf(cc, len);
}

/* see bearssl_ssl.h */
void
br_ssl_engine_sendapp_ack(br_ssl_engine_context *cc, size_t len)
{
	sendpld_ack(cc, len);
}

/* see bearssl_ssl.h */
unsigned char *
br_ssl_engine_recvapp_buf(const br_ssl_engine_context *cc, size_t *len)
{
	if (!cc->application_data
		|| cc->record_type_in != BR_SSL_APPLICATION_DATA)
	{
		*len = 0;
		return NULL;
	}
	return recvpld_buf(cc, len);
}

/* see bearssl_ssl.h */
void
br_ssl_engine_recvapp_ack(br_ssl_engine_context *cc, size_t len)
{
	recvpld_ack(cc, len);
}

/* see bearssl_ssl.h */
unsigned char *
br_ssl_engine_sendrec_buf(const br_ssl_engine_context *cc, size_t *len)
{
	return sendrec_buf(cc, len);
}

/* see bearssl_ssl.h */
void
br_ssl_engine_sendrec_ack(br_ssl_engine_context *cc, size_t len)
{
	sendrec_ack(cc, len);
	if (len != 0 && !has_rec_tosend(cc)
		&& (cc->record_type_out != BR_SSL_APPLICATION_DATA
		|| cc->application_data == 0))
	{
		jump_handshake(cc, 0);
	}
}

/* see bearssl_ssl.h */
unsigned char *
br_ssl_engine_recvrec_buf(const br_ssl_engine_context *cc, size_t *len)
{
	return recvrec_buf(cc, len);
}

/* see bearssl_ssl.h */
void
br_ssl_engine_recvrec_ack(br_ssl_engine_context *cc, size_t len)
{
	unsigned char *buf;

	recvrec_ack(cc, len);
	if (br_ssl_engine_closed(cc)) {
		return;
	}

	/*
	 * We just received some bytes from the peer. This may have
	 * yielded some payload bytes, in which case we must process
	 * them according to the record type.
	 */
	buf = recvpld_buf(cc, &len);
	if (buf != NULL) {
		switch (cc->record_type_in) {
		case BR_SSL_CHANGE_CIPHER_SPEC:
		case BR_SSL_ALERT:
		case BR_SSL_HANDSHAKE:
			jump_handshake(cc, 0);
			break;
		case BR_SSL_APPLICATION_DATA:
			if (cc->application_data) {
				break;
			}
			/* Fall through */
		default:
			br_ssl_engine_fail(cc, BR_ERR_UNEXPECTED);
			break;
		}
	}
}

/* see bearssl_ssl.h */
void
br_ssl_engine_close(br_ssl_engine_context *cc)
{
	if (!br_ssl_engine_closed(cc)) {
		jump_handshake(cc, 1);
	}
}

/* see bearssl_ssl.h */
int
br_ssl_engine_renegotiate(br_ssl_engine_context *cc)
{
	if (br_ssl_engine_closed(cc) || cc->reneg == 1
		|| (cc->flags & BR_OPT_NO_RENEGOTIATION) != 0)
	{
		return 0;
	}
	jump_handshake(cc, 2);
	return 1;
}

/* see bearssl.h */
unsigned
br_ssl_engine_current_state(const br_ssl_engine_context *cc)
{
	unsigned s;
	size_t len;

	if (br_ssl_engine_closed(cc)) {
		return BR_SSL_CLOSED;
	}

	s = 0;
	if (br_ssl_engine_sendrec_buf(cc, &len) != NULL) {
		s |= BR_SSL_SENDREC;
	}
	if (br_ssl_engine_recvrec_buf(cc, &len) != NULL) {
		s |= BR_SSL_RECVREC;
	}
	if (br_ssl_engine_sendapp_buf(cc, &len) != NULL) {
		s |= BR_SSL_SENDAPP;
	}
	if (br_ssl_engine_recvapp_buf(cc, &len) != NULL) {
		s |= BR_SSL_RECVAPP;
	}
	return s;
}

/* see bearssl_ssl.h */
void
br_ssl_engine_flush(br_ssl_engine_context *cc, int force)
{
	if (!br_ssl_engine_closed(cc) && cc->application_data) {
		sendpld_flush(cc, force);
	}
}

/* see inner.h */
void
br_ssl_engine_hs_reset(br_ssl_engine_context *cc,
	void (*hsinit)(void *), void (*hsrun)(void *))
{
	engine_clearbuf(cc);
	cc->cpu.dp = cc->dp_stack;
	cc->cpu.rp = cc->rp_stack;
	hsinit(&cc->cpu);
	cc->hsrun = hsrun;
	cc->shutdown_recv = 0;
	cc->application_data = 0;
	jump_handshake(cc, 0);
}

/* see inner.h */
br_tls_prf_impl
br_ssl_engine_get_PRF(br_ssl_engine_context *cc, int prf_id)
{
	if (cc->session.version >= BR_TLS12) {
		if (prf_id == br_sha384_ID) {
			return cc->prf_sha384;
		} else {
			return cc->prf_sha256;
		}
	} else {
		return cc->prf10;
	}
}

/* see inner.h */
void
br_ssl_engine_compute_master(br_ssl_engine_context *cc,
	int prf_id, const void *pms, size_t pms_len)
{
	br_tls_prf_impl iprf;
	unsigned char seed[64];

	iprf = br_ssl_engine_get_PRF(cc, prf_id);
	memcpy(seed, cc->client_random, 32);
	memcpy(seed + 32, cc->server_random, 32);
	iprf(cc->session.master_secret, sizeof cc->session.master_secret,
		pms, pms_len, "master secret", seed, sizeof seed);
}

/*
 * Compute key block.
 */
static void
compute_key_block(br_ssl_engine_context *cc, int prf_id,
	size_t half_len, unsigned char *kb)
{
	br_tls_prf_impl iprf;
	unsigned char seed[64];

	iprf = br_ssl_engine_get_PRF(cc, prf_id);
	memcpy(seed, cc->server_random, 32);
	memcpy(seed + 32, cc->client_random, 32);
	iprf(kb, half_len << 1,
		cc->session.master_secret, sizeof cc->session.master_secret,
		"key expansion", seed, sizeof seed);
}

/* see inner.h */
void
br_ssl_engine_switch_cbc_in(br_ssl_engine_context *cc,
	int is_client, int prf_id, int mac_id,
	const br_block_cbcdec_class *bc_impl, size_t cipher_key_len)
{
	unsigned char kb[192];
	unsigned char *cipher_key, *mac_key, *iv;
	const br_hash_class *imh;
	size_t mac_key_len, mac_out_len, iv_len;

	imh = br_ssl_engine_get_hash(cc, mac_id);
	mac_out_len = (imh->desc >> BR_HASHDESC_OUT_OFF) & BR_HASHDESC_OUT_MASK;
	mac_key_len = mac_out_len;

	/*
	 * TLS 1.1+ uses per-record explicit IV, so no IV to generate here.
	 */
	if (cc->session.version >= BR_TLS11) {
		iv_len = 0;
	} else {
		iv_len = bc_impl->block_size;
	}
	compute_key_block(cc, prf_id,
		mac_key_len + cipher_key_len + iv_len, kb);
	if (is_client) {
		mac_key = &kb[mac_key_len];
		cipher_key = &kb[(mac_key_len << 1) + cipher_key_len];
		iv = &kb[((mac_key_len + cipher_key_len) << 1) + iv_len];
	} else {
		mac_key = &kb[0];
		cipher_key = &kb[mac_key_len << 1];
		iv = &kb[(mac_key_len + cipher_key_len) << 1];
	}
	if (iv_len == 0) {
		iv = NULL;
	}
	cc->icbc_in->init(&cc->in.cbc.vtable,
		bc_impl, cipher_key, cipher_key_len,
		imh, mac_key, mac_key_len, mac_out_len, iv);
	cc->incrypt = 1;
}

/* see inner.h */
void
br_ssl_engine_switch_cbc_out(br_ssl_engine_context *cc,
	int is_client, int prf_id, int mac_id,
	const br_block_cbcenc_class *bc_impl, size_t cipher_key_len)
{
	unsigned char kb[192];
	unsigned char *cipher_key, *mac_key, *iv;
	const br_hash_class *imh;
	size_t mac_key_len, mac_out_len, iv_len;

	imh = br_ssl_engine_get_hash(cc, mac_id);
	mac_out_len = (imh->desc >> BR_HASHDESC_OUT_OFF) & BR_HASHDESC_OUT_MASK;
	mac_key_len = mac_out_len;

	/*
	 * TLS 1.1+ uses per-record explicit IV, so no IV to generate here.
	 */
	if (cc->session.version >= BR_TLS11) {
		iv_len = 0;
	} else {
		iv_len = bc_impl->block_size;
	}
	compute_key_block(cc, prf_id,
		mac_key_len + cipher_key_len + iv_len, kb);
	if (is_client) {
		mac_key = &kb[0];
		cipher_key = &kb[mac_key_len << 1];
		iv = &kb[(mac_key_len + cipher_key_len) << 1];
	} else {
		mac_key = &kb[mac_key_len];
		cipher_key = &kb[(mac_key_len << 1) + cipher_key_len];
		iv = &kb[((mac_key_len + cipher_key_len) << 1) + iv_len];
	}
	if (iv_len == 0) {
		iv = NULL;
	}
	cc->icbc_out->init(&cc->out.cbc.vtable,
		bc_impl, cipher_key, cipher_key_len,
		imh, mac_key, mac_key_len, mac_out_len, iv);
}

/* see inner.h */
void
br_ssl_engine_switch_gcm_in(br_ssl_engine_context *cc,
	int is_client, int prf_id,
	const br_block_ctr_class *bc_impl, size_t cipher_key_len)
{
	unsigned char kb[72];
	unsigned char *cipher_key, *iv;

	compute_key_block(cc, prf_id, cipher_key_len + 4, kb);
	if (is_client) {
		cipher_key = &kb[cipher_key_len];
		iv = &kb[(cipher_key_len << 1) + 4];
	} else {
		cipher_key = &kb[0];
		iv = &kb[cipher_key_len << 1];
	}
	cc->igcm_in->init(&cc->in.gcm.vtable.in,
		bc_impl, cipher_key, cipher_key_len, cc->ighash, iv);
	cc->incrypt = 1;
}

/* see inner.h */
void
br_ssl_engine_switch_gcm_out(br_ssl_engine_context *cc,
	int is_client, int prf_id,
	const br_block_ctr_class *bc_impl, size_t cipher_key_len)
{
	unsigned char kb[72];
	unsigned char *cipher_key, *iv;

	compute_key_block(cc, prf_id, cipher_key_len + 4, kb);
	if (is_client) {
		cipher_key = &kb[0];
		iv = &kb[cipher_key_len << 1];
	} else {
		cipher_key = &kb[cipher_key_len];
		iv = &kb[(cipher_key_len << 1) + 4];
	}
	cc->igcm_out->init(&cc->out.gcm.vtable.out,
		bc_impl, cipher_key, cipher_key_len, cc->ighash, iv);
}

/* see inner.h */
void
br_ssl_engine_switch_chapol_in(br_ssl_engine_context *cc,
	int is_client, int prf_id)
{
	unsigned char kb[88];
	unsigned char *cipher_key, *iv;

	compute_key_block(cc, prf_id, 44, kb);
	if (is_client) {
		cipher_key = &kb[32];
		iv = &kb[76];
	} else {
		cipher_key = &kb[0];
		iv = &kb[64];
	}
	cc->ichapol_in->init(&cc->in.chapol.vtable.in,
		cc->ichacha, cc->ipoly, cipher_key, iv);
	cc->incrypt = 1;
}

/* see inner.h */
void
br_ssl_engine_switch_chapol_out(br_ssl_engine_context *cc,
	int is_client, int prf_id)
{
	unsigned char kb[88];
	unsigned char *cipher_key, *iv;

	compute_key_block(cc, prf_id, 44, kb);
	if (is_client) {
		cipher_key = &kb[0];
		iv = &kb[64];
	} else {
		cipher_key = &kb[32];
		iv = &kb[76];
	}
	cc->ichapol_out->init(&cc->out.chapol.vtable.out,
		cc->ichacha, cc->ipoly, cipher_key, iv);
}


/* ===== src/ssl/ssl_client.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

/* see bearssl_ssl.h */
void
br_ssl_client_zero(br_ssl_client_context *cc)
{
	/*
	 * For really standard C, we should explicitly set to NULL all
	 * pointers, and 0 all other fields. However, on all our target
	 * architectures, a direct memset() will work, be faster, and
	 * use a lot less code.
	 */
	memset(cc, 0, sizeof *cc);
}

/* see bearssl_ssl.h */
int
br_ssl_client_reset(br_ssl_client_context *cc,
	const char *server_name, int resume_session)
{
	size_t n;

	br_ssl_engine_set_buffer(&cc->eng, NULL, 0, 0);
	cc->eng.version_out = cc->eng.version_min;
	if (!resume_session) {
		br_ssl_client_forget_session(cc);
	}
	if (!br_ssl_engine_init_rand(&cc->eng)) {
		return 0;
	}

	/*
	 * We always set back the "reneg" flag to 0 because we use it
	 * to distinguish between first handshake and renegotiation.
	 * Note that "renegotiation" and "session resumption" are two
	 * different things.
	 */
	cc->eng.reneg = 0;

	if (server_name == NULL) {
		cc->eng.server_name[0] = 0;
	} else {
		n = strlen(server_name) + 1;
		if (n > sizeof cc->eng.server_name) {
			br_ssl_engine_fail(&cc->eng, BR_ERR_BAD_PARAM);
			return 0;
		}
		memcpy(cc->eng.server_name, server_name, n);
	}

	br_ssl_engine_hs_reset(&cc->eng,
		br_ssl_hs_client_init_main, br_ssl_hs_client_run);
	return br_ssl_engine_last_error(&cc->eng) == BR_ERR_OK;
}


/* ===== src/ssl/ssl_client_full.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

/* see bearssl_ssl.h */
void
br_ssl_client_init_full(br_ssl_client_context *cc,
	br_x509_minimal_context *xc,
	const br_x509_trust_anchor *trust_anchors, size_t trust_anchors_num)
{
	/*
	 * The "full" profile supports all implemented cipher suites.
	 *
	 * Rationale for suite order, from most important to least
	 * important rule:
	 *
	 * -- Don't use 3DES if AES or ChaCha20 is available.
	 * -- Try to have Forward Secrecy (ECDHE suite) if possible.
	 * -- When not using Forward Secrecy, ECDH key exchange is
	 *    better than RSA key exchange (slightly more expensive on the
	 *    client, but much cheaper on the server, and it implies smaller
	 *    messages).
	 * -- ChaCha20+Poly1305 is better than AES/GCM (faster, smaller code).
	 * -- GCM is better than CBC.
	 * -- AES-128 is preferred over AES-256 (AES-128 is already
	 *    strong enough, and AES-256 is 40% more expensive).
	 */
	static const uint16_t suites[] = {
		BR_TLS_ECDHE_ECDSA_WITH_CHACHA20_POLY1305_SHA256,
		BR_TLS_ECDHE_RSA_WITH_CHACHA20_POLY1305_SHA256,
		BR_TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256,
		BR_TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256,
		BR_TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384,
		BR_TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384,
		BR_TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256,
		BR_TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256,
		BR_TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA384,
		BR_TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA384,
		BR_TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA,
		BR_TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA,
		BR_TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA,
		BR_TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA,
		BR_TLS_ECDH_ECDSA_WITH_AES_128_GCM_SHA256,
		BR_TLS_ECDH_RSA_WITH_AES_128_GCM_SHA256,
		BR_TLS_ECDH_ECDSA_WITH_AES_256_GCM_SHA384,
		BR_TLS_ECDH_RSA_WITH_AES_256_GCM_SHA384,
		BR_TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA256,
		BR_TLS_ECDH_RSA_WITH_AES_128_CBC_SHA256,
		BR_TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA384,
		BR_TLS_ECDH_RSA_WITH_AES_256_CBC_SHA384,
		BR_TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA,
		BR_TLS_ECDH_RSA_WITH_AES_128_CBC_SHA,
		BR_TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA,
		BR_TLS_ECDH_RSA_WITH_AES_256_CBC_SHA,
		BR_TLS_RSA_WITH_AES_128_GCM_SHA256,
		BR_TLS_RSA_WITH_AES_256_GCM_SHA384,
		BR_TLS_RSA_WITH_AES_128_CBC_SHA256,
		BR_TLS_RSA_WITH_AES_256_CBC_SHA256,
		BR_TLS_RSA_WITH_AES_128_CBC_SHA,
		BR_TLS_RSA_WITH_AES_256_CBC_SHA,
		BR_TLS_ECDHE_ECDSA_WITH_3DES_EDE_CBC_SHA,
		BR_TLS_ECDHE_RSA_WITH_3DES_EDE_CBC_SHA,
		BR_TLS_ECDH_ECDSA_WITH_3DES_EDE_CBC_SHA,
		BR_TLS_ECDH_RSA_WITH_3DES_EDE_CBC_SHA,
		BR_TLS_RSA_WITH_3DES_EDE_CBC_SHA
	};

	/*
	 * All hash functions are activated.
	 * Note: the X.509 validation engine will nonetheless refuse to
	 * validate signatures that use MD5 as hash function.
	 */
	static const br_hash_class *hashes[] = {
		&br_md5_vtable,
		&br_sha1_vtable,
		&br_sha224_vtable,
		&br_sha256_vtable,
		&br_sha384_vtable,
		&br_sha512_vtable
	};

	int id;

	/*
	 * Reset client context and set supported versions from TLS-1.0
	 * to TLS-1.2 (inclusive).
	 */
	br_ssl_client_zero(cc);
	br_ssl_engine_set_versions(&cc->eng, BR_TLS10, BR_TLS12);

	/*
	 * X.509 engine uses SHA-256 to hash certificate DN (for
	 * comparisons).
	 */
	br_x509_minimal_init(xc, &br_sha256_vtable,
		trust_anchors, trust_anchors_num);

	/*
	 * Set suites and asymmetric crypto implementations. We use the
	 * "i31" code for RSA (it is somewhat faster than the "i32"
	 * implementation).
	 * TODO: change that when better implementations are made available.
	 */
	br_ssl_engine_set_suites(&cc->eng, suites,
		(sizeof suites) / (sizeof suites[0]));
	br_ssl_client_set_rsapub(cc, &br_rsa_i31_public);
	br_ssl_engine_set_rsavrfy(&cc->eng, &br_rsa_i31_pkcs1_vrfy);
	br_ssl_engine_set_ec(&cc->eng, &br_ec_prime_i31);
	br_ssl_engine_set_ecdsa(&cc->eng, &br_ecdsa_i31_vrfy_asn1);
	br_x509_minimal_set_rsa(xc, &br_rsa_i31_pkcs1_vrfy);
	br_x509_minimal_set_ecdsa(xc,
		&br_ec_prime_i31, &br_ecdsa_i31_vrfy_asn1);

	/*
	 * Set supported hash functions, for the SSL engine and for the
	 * X.509 engine.
	 */
	for (id = br_md5_ID; id <= br_sha512_ID; id ++) {
		const br_hash_class *hc;

		hc = hashes[id - 1];
		br_ssl_engine_set_hash(&cc->eng, id, hc);
		br_x509_minimal_set_hash(xc, id, hc);
	}

	/*
	 * Link the X.509 engine in the SSL engine.
	 */
	br_ssl_engine_set_x509(&cc->eng, &xc->vtable);

	/*
	 * Set the PRF implementations.
	 */
	br_ssl_engine_set_prf10(&cc->eng, &br_tls10_prf);
	br_ssl_engine_set_prf_sha256(&cc->eng, &br_tls12_sha256_prf);
	br_ssl_engine_set_prf_sha384(&cc->eng, &br_tls12_sha384_prf);

	/*
	 * Symmetric encryption. We use the "constant-time"
	 * implementations, which are the safest.
	 *
	 * On architectures detected as "64-bit", use the 64-bit
	 * versions (aes_ct64, ghash_ctmul64).
	 */
#if BR_64
	br_ssl_engine_set_aes_cbc(&cc->eng,
		&br_aes_ct64_cbcenc_vtable,
		&br_aes_ct64_cbcdec_vtable);
	br_ssl_engine_set_aes_ctr(&cc->eng,
		&br_aes_ct64_ctr_vtable);
	br_ssl_engine_set_ghash(&cc->eng,
		&br_ghash_ctmul64);
#else
	br_ssl_engine_set_aes_cbc(&cc->eng,
		&br_aes_ct_cbcenc_vtable,
		&br_aes_ct_cbcdec_vtable);
	br_ssl_engine_set_aes_ctr(&cc->eng,
		&br_aes_ct_ctr_vtable);
	br_ssl_engine_set_ghash(&cc->eng,
		&br_ghash_ctmul);
#endif
	br_ssl_engine_set_des_cbc(&cc->eng,
		&br_des_ct_cbcenc_vtable,
		&br_des_ct_cbcdec_vtable);
	br_ssl_engine_set_chacha20(&cc->eng,
		&br_chacha20_ct_run);
	br_ssl_engine_set_poly1305(&cc->eng,
		&br_poly1305_ctmul_run);

	/*
	 * Set the SSL record engines (CBC, GCM).
	 */
	br_ssl_engine_set_cbc(&cc->eng,
		&br_sslrec_in_cbc_vtable,
		&br_sslrec_out_cbc_vtable);
	br_ssl_engine_set_gcm(&cc->eng,
		&br_sslrec_in_gcm_vtable,
		&br_sslrec_out_gcm_vtable);
	br_ssl_engine_set_chapol(&cc->eng,
		&br_sslrec_in_chapol_vtable,
		&br_sslrec_out_chapol_vtable);
}


/* ===== src/ssl/ssl_hs_client.c ===== */
/* Automatically generated code; do not modify directly. */

#include <stddef.h>
#include <stdint.h>

typedef struct {
	uint32_t *dp;
	uint32_t *rp;
	const unsigned char *ip;
} t0_context;

static uint32_t
t0_parse7E_unsigned(const unsigned char **p)
{
	uint32_t x;

	x = 0;
	for (;;) {
		unsigned y;

		y = *(*p) ++;
		x = (x << 7) | (uint32_t)(y & 0x7F);
		if (y < 0x80) {
			return x;
		}
	}
}

static int32_t
t0_parse7E_signed(const unsigned char **p)
{
	int neg;
	uint32_t x;

	neg = ((**p) >> 6) & 1;
	x = (uint32_t)-neg;
	for (;;) {
		unsigned y;

		y = *(*p) ++;
		x = (x << 7) | (uint32_t)(y & 0x7F);
		if (y < 0x80) {
			if (neg) {
				return -(int32_t)~x - 1;
			} else {
				return (int32_t)x;
			}
		}
	}
}

#define T0_VBYTE(x, n)   (unsigned char)((((uint32_t)(x) >> (n)) & 0x7F) | 0x80)
#define T0_FBYTE(x, n)   (unsigned char)(((uint32_t)(x) >> (n)) & 0x7F)
#define T0_SBYTE(x)      (unsigned char)((((uint32_t)(x) >> 28) + 0xF8) ^ 0xF8)
#define T0_INT1(x)       T0_FBYTE(x, 0)
#define T0_INT2(x)       T0_VBYTE(x, 7), T0_FBYTE(x, 0)
#define T0_INT3(x)       T0_VBYTE(x, 14), T0_VBYTE(x, 7), T0_FBYTE(x, 0)
#define T0_INT4(x)       T0_VBYTE(x, 21), T0_VBYTE(x, 14), T0_VBYTE(x, 7), T0_FBYTE(x, 0)
#define T0_INT5(x)       T0_SBYTE(x), T0_VBYTE(x, 21), T0_VBYTE(x, 14), T0_VBYTE(x, 7), T0_FBYTE(x, 0)

static const uint8_t t0_datablock[];


void br_ssl_hs_client_init_main(void *t0ctx);

void br_ssl_hs_client_run(void *t0ctx);



#include <stddef.h>
#include <string.h>

/* (already included) */

/*
 * This macro evaluates to a pointer to the current engine context.
 */
#define ENG  ((br_ssl_engine_context *)((unsigned char *)t0ctx - offsetof(br_ssl_engine_context, cpu)))





/*
 * This macro evaluates to a pointer to the client context, under that
 * specific name. It must be noted that since the engine context is the
 * first field of the br_ssl_client_context structure ('eng'), then
 * pointers values of both types are interchangeable, modulo an
 * appropriate cast. This also means that "adresses" computed as offsets
 * within the structure work for both kinds of context.
 */
#define CTX  ((br_ssl_client_context *)ENG)

/*
 * Generate the pre-master secret for RSA key exchange, and encrypt it
 * with the server's public key. Returned value is either the encrypted
 * data length (in bytes), or -x on error, with 'x' being an error code.
 *
 * This code assumes that the public key has been already verified (it
 * was properly obtained by the X.509 engine, and it has the right type,
 * i.e. it is of type RSA and suitable for encryption).
 */
static int
make_pms_rsa(br_ssl_client_context *ctx, int prf_id)
{
	const br_x509_class **xc;
	const br_x509_pkey *pk;
	const unsigned char *n;
	unsigned char *pms;
	size_t nlen, u;

	xc = ctx->eng.x509ctx;
	pk = (*xc)->get_pkey(xc, NULL);

	/*
	 * Compute actual RSA key length, in case there are leading zeros.
	 */
	n = pk->key.rsa.n;
	nlen = pk->key.rsa.nlen;
	while (nlen > 0 && *n == 0) {
		n ++;
		nlen --;
	}

	/*
	 * We need at least 59 bytes (48 bytes for pre-master secret, and
	 * 11 bytes for the PKCS#1 type 2 padding). Note that the X.509
	 * minimal engine normally blocks RSA keys shorter than 128 bytes,
	 * so this is mostly for public keys provided explicitly by the
	 * caller.
	 */
	if (nlen < 59) {
		return -BR_ERR_X509_WEAK_PUBLIC_KEY;
	}
	if (nlen > sizeof ctx->eng.pad) {
		return -BR_ERR_LIMIT_EXCEEDED;
	}

	/*
	 * Make PMS.
	 */
	pms = ctx->eng.pad + nlen - 48;
	br_enc16be(pms, ctx->eng.version_max);
	br_hmac_drbg_generate(&ctx->eng.rng, pms + 2, 46);
	br_ssl_engine_compute_master(&ctx->eng, prf_id, pms, 48);

	/*
	 * Apply PKCS#1 type 2 padding.
	 */
	ctx->eng.pad[0] = 0x00;
	ctx->eng.pad[1] = 0x02;
	ctx->eng.pad[nlen - 49] = 0x00;
	br_hmac_drbg_generate(&ctx->eng.rng, ctx->eng.pad + 2, nlen - 51);
	for (u = 2; u < nlen - 49; u ++) {
		while (ctx->eng.pad[u] == 0) {
			br_hmac_drbg_generate(&ctx->eng.rng,
				&ctx->eng.pad[u], 1);
		}
	}

	/*
	 * Compute RSA encryption.
	 */
	if (!ctx->irsapub(ctx->eng.pad, nlen, &pk->key.rsa)) {
		return -BR_ERR_LIMIT_EXCEEDED;
	}
	return (int)nlen;
}

/*
 * OID for hash functions in RSA signatures.
 */
static const unsigned char HASH_OID_SHA1[] = {
	0x05, 0x2B, 0x0E, 0x03, 0x02, 0x1A
};

static const unsigned char HASH_OID_SHA224[] = {
	0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x04
};

static const unsigned char HASH_OID_SHA256[] = {
	0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01
};

static const unsigned char HASH_OID_SHA384[] = {
	0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x02
};

static const unsigned char HASH_OID_SHA512[] = {
	0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x03
};

static const unsigned char *HASH_OID[] = {
	HASH_OID_SHA1,
	HASH_OID_SHA224,
	HASH_OID_SHA256,
	HASH_OID_SHA384,
	HASH_OID_SHA512
};

/*
 * Check the RSA signature on the ServerKeyExchange message.
 *
 *   hash      hash function ID (2 to 6), or 0 for MD5+SHA-1 (with RSA only)
 *   use_rsa   non-zero for RSA signature, zero for ECDSA
 *   sig_len   signature length (in bytes); signature value is in the pad
 *
 * Returned value is 0 on success, or an error code.
 */
static int
verify_SKE_sig(br_ssl_client_context *ctx,
	int hash, int use_rsa, size_t sig_len)
{
	const br_x509_class **xc;
	const br_x509_pkey *pk;
	br_multihash_context mhc;
	unsigned char hv[64], head[4];
	size_t hv_len;

	xc = ctx->eng.x509ctx;
	pk = (*xc)->get_pkey(xc, NULL);
	br_multihash_zero(&mhc);
	br_multihash_copyimpl(&mhc, &ctx->eng.mhash);
	br_multihash_init(&mhc);
	br_multihash_update(&mhc,
		ctx->eng.client_random, sizeof ctx->eng.client_random);
	br_multihash_update(&mhc,
		ctx->eng.server_random, sizeof ctx->eng.server_random);
	head[0] = 3;
	head[1] = 0;
	head[2] = ctx->eng.ecdhe_curve;
	head[3] = ctx->eng.ecdhe_point_len;
	br_multihash_update(&mhc, head, sizeof head);
	br_multihash_update(&mhc,
		ctx->eng.ecdhe_point, ctx->eng.ecdhe_point_len);
	if (hash) {
		hv_len = br_multihash_out(&mhc, hash, hv);
		if (hv_len == 0) {
			return BR_ERR_INVALID_ALGORITHM;
		}
	} else {
		if (!br_multihash_out(&mhc, br_md5_ID, hv)
			|| !br_multihash_out(&mhc, br_sha1_ID, hv + 16))
		{
			return BR_ERR_INVALID_ALGORITHM;
		}
		hv_len = 36;
	}
	if (use_rsa) {
		unsigned char tmp[64];
		const unsigned char *hash_oid;

		if (hash) {
			hash_oid = HASH_OID[hash - 2];
		} else {
			hash_oid = NULL;
		}
		if (!ctx->eng.irsavrfy(ctx->eng.pad, sig_len,
			hash_oid, hv_len, &pk->key.rsa, tmp)
			|| memcmp(tmp, hv, hv_len) != 0)
		{
			return BR_ERR_BAD_SIGNATURE;
		}
	} else {
		if (!ctx->eng.iecdsa(ctx->eng.iec, hv, hv_len, &pk->key.ec,
			ctx->eng.pad, sig_len))
		{
			return BR_ERR_BAD_SIGNATURE;
		}
	}
	return 0;
}

/*
 * Perform client-side ECDH (or ECDHE). The point that should be sent to
 * the server is written in the pad; returned value is either the point
 * length (in bytes), or -x on error, with 'x' being an error code.
 *
 * The point _from_ the server is taken from ecdhe_point[] if 'ecdhe'
 * is non-zero, or from the X.509 engine context if 'ecdhe' is zero
 * (for static ECDH).
 */
static int
make_pms_ecdh(br_ssl_client_context *ctx, unsigned ecdhe, int prf_id)
{
	int curve;
	unsigned char key[66], point[133];
	const unsigned char *generator, *order, *point_src;
	size_t glen, olen, point_len;
	unsigned char mask;

	if (ecdhe) {
		curve = ctx->eng.ecdhe_curve;
		point_src = ctx->eng.ecdhe_point;
		point_len = ctx->eng.ecdhe_point_len;
	} else {
		const br_x509_class **xc;
		const br_x509_pkey *pk;

		xc = ctx->eng.x509ctx;
		pk = (*xc)->get_pkey(xc, NULL);
		curve = pk->key.ec.curve;
		point_src = pk->key.ec.q;
		point_len = pk->key.ec.qlen;
	}
	if ((ctx->eng.iec->supported_curves & ((uint32_t)1 << curve)) == 0) {
		return -BR_ERR_INVALID_ALGORITHM;
	}

	/*
	 * We need to generate our key, as a non-zero random value which
	 * is lower than the curve order, in a "large enough" range. We
	 * force top bit to 0 and bottom bit to 1, which guarantees that
	 * the value is in the proper range.
	 */
	order = ctx->eng.iec->order(curve, &olen);
	mask = 0xFF;
	while (mask >= order[0]) {
		mask >>= 1;
	}
	br_hmac_drbg_generate(&ctx->eng.rng, key, olen);
	key[0] &= mask;
	key[olen - 1] |= 0x01;

	/*
	 * Compute the common ECDH point, whose X coordinate is the
	 * pre-master secret.
	 */
	generator = ctx->eng.iec->generator(curve, &glen);
	if (glen != point_len) {
		return -BR_ERR_INVALID_ALGORITHM;
	}

	memcpy(point, point_src, glen);
	if (!ctx->eng.iec->mul(point, glen, key, olen, curve)) {
		return -BR_ERR_INVALID_ALGORITHM;
	}

	/*
	 * The pre-master secret is the X coordinate.
	 */
	br_ssl_engine_compute_master(&ctx->eng, prf_id, point + 1, glen >> 1);

	ctx->eng.iec->mulgen(point, key, olen, curve);
	memcpy(ctx->eng.pad, point, glen);
	return (int)glen;
}

/*
 * Perform full static ECDH. This occurs only in the context of client
 * authentication with certificates: the server uses an EC public key,
 * the cipher suite is of type ECDH (not ECDHE), the server requested a
 * client certificate and accepts static ECDH, the client has a
 * certificate with an EC public key in the same curve, and accepts
 * static ECDH as well.
 *
 * Returned value is 0 on success, -1 on error.
 */
static int
make_pms_static_ecdh(br_ssl_client_context *ctx, int prf_id)
{
	unsigned char point[133];
	size_t point_len;
	const br_x509_class **xc;
	const br_x509_pkey *pk;

	xc = ctx->eng.x509ctx;
	pk = (*xc)->get_pkey(xc, NULL);
	point_len = pk->key.ec.qlen;
	if (point_len > sizeof point) {
		return -1;
	}
	memcpy(point, pk->key.ec.q, point_len);
	if (!(*ctx->client_auth_vtable)->do_keyx(
		ctx->client_auth_vtable, point, point_len))
	{
		return -1;
	}
	br_ssl_engine_compute_master(&ctx->eng,
		prf_id, point + 1, point_len >> 1);
	return 0;
}

/*
 * Compute the client-side signature. This is invoked only when a
 * signature-based client authentication was selected. The computed
 * signature is in the pad; its length (in bytes) is returned. On
 * error, 0 is returned.
 */
static size_t
make_client_sign(br_ssl_client_context *ctx)
{
	size_t hv_len;

	/*
	 * Compute hash of handshake messages so far. This "cannot" fail
	 * because the list of supported hash functions provided to the
	 * client certificate handler was trimmed to include only the
	 * hash functions that the multi-hasher supports.
	 */
	if (ctx->hash_id) {
		hv_len = br_multihash_out(&ctx->eng.mhash,
			ctx->hash_id, ctx->eng.pad);
	} else {
		br_multihash_out(&ctx->eng.mhash,
			br_md5_ID, ctx->eng.pad);
		br_multihash_out(&ctx->eng.mhash,
			br_sha1_ID, ctx->eng.pad + 16);
		hv_len = 36;
	}
	return (*ctx->client_auth_vtable)->do_sign(
		ctx->client_auth_vtable, ctx->hash_id, hv_len,
		ctx->eng.pad, sizeof ctx->eng.pad);
}



static const uint8_t t0_datablock[] = {
	0x00, 0x00, 0x0A, 0x00, 0x24, 0x00, 0x2F, 0x01, 0x24, 0x00, 0x35, 0x02,
	0x24, 0x00, 0x3C, 0x01, 0x44, 0x00, 0x3D, 0x02, 0x44, 0x00, 0x9C, 0x03,
	0x04, 0x00, 0x9D, 0x04, 0x05, 0xC0, 0x03, 0x40, 0x24, 0xC0, 0x04, 0x41,
	0x24, 0xC0, 0x05, 0x42, 0x24, 0xC0, 0x08, 0x20, 0x24, 0xC0, 0x09, 0x21,
	0x24, 0xC0, 0x0A, 0x22, 0x24, 0xC0, 0x0D, 0x30, 0x24, 0xC0, 0x0E, 0x31,
	0x24, 0xC0, 0x0F, 0x32, 0x24, 0xC0, 0x12, 0x10, 0x24, 0xC0, 0x13, 0x11,
	0x24, 0xC0, 0x14, 0x12, 0x24, 0xC0, 0x23, 0x21, 0x44, 0xC0, 0x24, 0x22,
	0x55, 0xC0, 0x25, 0x41, 0x44, 0xC0, 0x26, 0x42, 0x55, 0xC0, 0x27, 0x11,
	0x44, 0xC0, 0x28, 0x12, 0x55, 0xC0, 0x29, 0x31, 0x44, 0xC0, 0x2A, 0x32,
	0x55, 0xC0, 0x2B, 0x23, 0x04, 0xC0, 0x2C, 0x24, 0x05, 0xC0, 0x2D, 0x43,
	0x04, 0xC0, 0x2E, 0x44, 0x05, 0xC0, 0x2F, 0x13, 0x04, 0xC0, 0x30, 0x14,
	0x05, 0xC0, 0x31, 0x33, 0x04, 0xC0, 0x32, 0x34, 0x05, 0xCC, 0xA8, 0x15,
	0x04, 0xCC, 0xA9, 0x25, 0x04, 0x00, 0x00
};

static const uint8_t t0_codeblock[] = {
	0x00, 0x01, 0x00, 0x0A, 0x00, 0x00, 0x01, 0x00, 0x0D, 0x00, 0x00, 0x01,
	0x00, 0x0E, 0x00, 0x00, 0x01, 0x00, 0x0F, 0x00, 0x00, 0x01, 0x01, 0x08,
	0x00, 0x00, 0x01, 0x01, 0x09, 0x00, 0x00, 0x01, 0x02, 0x08, 0x00, 0x00,
	0x01, 0x02, 0x09, 0x00, 0x00, 0x25, 0x25, 0x00, 0x00, 0x01,
	T0_INT1(BR_ERR_BAD_CCS), 0x00, 0x00, 0x01,
	T0_INT1(BR_ERR_BAD_CIPHER_SUITE), 0x00, 0x00, 0x01,
	T0_INT1(BR_ERR_BAD_COMPRESSION), 0x00, 0x00, 0x01,
	T0_INT1(BR_ERR_BAD_FINISHED), 0x00, 0x00, 0x01,
	T0_INT1(BR_ERR_BAD_FRAGLEN), 0x00, 0x00, 0x01,
	T0_INT1(BR_ERR_BAD_HANDSHAKE), 0x00, 0x00, 0x01,
	T0_INT1(BR_ERR_BAD_HELLO_DONE), 0x00, 0x00, 0x01,
	T0_INT1(BR_ERR_BAD_PARAM), 0x00, 0x00, 0x01,
	T0_INT1(BR_ERR_BAD_SECRENEG), 0x00, 0x00, 0x01,
	T0_INT1(BR_ERR_BAD_SNI), 0x00, 0x00, 0x01, T0_INT1(BR_ERR_BAD_VERSION),
	0x00, 0x00, 0x01, T0_INT1(BR_ERR_EXTRA_EXTENSION), 0x00, 0x00, 0x01,
	T0_INT1(BR_ERR_INVALID_ALGORITHM), 0x00, 0x00, 0x01,
	T0_INT1(BR_ERR_LIMIT_EXCEEDED), 0x00, 0x00, 0x01, T0_INT1(BR_ERR_OK),
	0x00, 0x00, 0x01, T0_INT1(BR_ERR_OVERSIZED_ID), 0x00, 0x00, 0x01,
	T0_INT1(BR_ERR_RESUME_MISMATCH), 0x00, 0x00, 0x01,
	T0_INT1(BR_ERR_UNEXPECTED), 0x00, 0x00, 0x01,
	T0_INT1(BR_ERR_UNSUPPORTED_VERSION), 0x00, 0x00, 0x01,
	T0_INT1(BR_ERR_WRONG_KEY_USAGE), 0x00, 0x00, 0x01,
	T0_INT2(offsetof(br_ssl_engine_context, action)), 0x00, 0x00, 0x01,
	T0_INT2(offsetof(br_ssl_engine_context, alert)), 0x00, 0x00, 0x01,
	T0_INT2(offsetof(br_ssl_engine_context, application_data)), 0x00, 0x00,
	0x01, T0_INT2(offsetof(br_ssl_client_context, auth_type)), 0x00, 0x00,
	0x01,
	T0_INT2(offsetof(br_ssl_engine_context, session) + offsetof(br_ssl_session_parameters, cipher_suite)),
	0x00, 0x00, 0x01,
	T0_INT2(offsetof(br_ssl_engine_context, client_random)), 0x00, 0x00,
	0x01, T0_INT2(offsetof(br_ssl_engine_context, close_received)), 0x00,
	0x00, 0x01, T0_INT2(offsetof(br_ssl_engine_context, ecdhe_curve)),
	0x00, 0x00, 0x01,
	T0_INT2(offsetof(br_ssl_engine_context, ecdhe_point)), 0x00, 0x00,
	0x01, T0_INT2(offsetof(br_ssl_engine_context, ecdhe_point_len)), 0x00,
	0x00, 0x01, T0_INT2(offsetof(br_ssl_engine_context, flags)), 0x00,
	0x00, 0x01, T0_INT2(offsetof(br_ssl_client_context, hash_id)), 0x00,
	0x00, 0x01, T0_INT2(offsetof(br_ssl_client_context, hashes)), 0x00,
	0x00, 0x01, T0_INT2(offsetof(br_ssl_engine_context, log_max_frag_len)),
	0x00, 0x00, 0x01,
	T0_INT2(offsetof(br_ssl_client_context, min_clienthello_len)), 0x00,
	0x00, 0x01, T0_INT2(offsetof(br_ssl_engine_context, pad)), 0x00, 0x00,
	0x01, T0_INT2(offsetof(br_ssl_engine_context, protocol_names_num)),
	0x00, 0x00, 0x01,
	T0_INT2(offsetof(br_ssl_engine_context, record_type_in)), 0x00, 0x00,
	0x01, T0_INT2(offsetof(br_ssl_engine_context, record_type_out)), 0x00,
	0x00, 0x01, T0_INT2(offsetof(br_ssl_engine_context, reneg)), 0x00,
	0x00, 0x01, T0_INT2(offsetof(br_ssl_engine_context, saved_finished)),
	0x00, 0x00, 0x01,
	T0_INT2(offsetof(br_ssl_engine_context, selected_protocol)), 0x00,
	0x00, 0x01, T0_INT2(offsetof(br_ssl_engine_context, server_name)),
	0x00, 0x00, 0x01,
	T0_INT2(offsetof(br_ssl_engine_context, server_random)), 0x00, 0x00,
	0x01,
	T0_INT2(offsetof(br_ssl_engine_context, session) + offsetof(br_ssl_session_parameters, session_id)),
	0x00, 0x00, 0x01,
	T0_INT2(offsetof(br_ssl_engine_context, session) + offsetof(br_ssl_session_parameters, session_id_len)),
	0x00, 0x00, 0x01,
	T0_INT2(offsetof(br_ssl_engine_context, shutdown_recv)), 0x00, 0x00,
	0x01, T0_INT2(offsetof(br_ssl_engine_context, suites_buf)), 0x00, 0x00,
	0x01, T0_INT2(offsetof(br_ssl_engine_context, suites_num)), 0x00, 0x00,
	0x01,
	T0_INT2(offsetof(br_ssl_engine_context, session) + offsetof(br_ssl_session_parameters, version)),
	0x00, 0x00, 0x01, T0_INT2(offsetof(br_ssl_engine_context, version_in)),
	0x00, 0x00, 0x01,
	T0_INT2(offsetof(br_ssl_engine_context, version_max)), 0x00, 0x00,
	0x01, T0_INT2(offsetof(br_ssl_engine_context, version_min)), 0x00,
	0x00, 0x01, T0_INT2(offsetof(br_ssl_engine_context, version_out)),
	0x00, 0x00, 0x09, 0x26, 0x55, 0x06, 0x02, 0x65, 0x28, 0x00, 0x00, 0x06,
	0x08, 0x2C, 0x0E, 0x05, 0x02, 0x6E, 0x28, 0x04, 0x01, 0x3C, 0x00, 0x00,
	0x01, 0x01, 0x00, 0x01, 0x03, 0x00, 0x96, 0x26, 0x5B, 0x43, 0x9A, 0x26,
	0x05, 0x04, 0x5D, 0x01, 0x00, 0x00, 0x02, 0x00, 0x0E, 0x06, 0x02, 0x9A,
	0x00, 0x5B, 0x04, 0x6B, 0x00, 0x06, 0x02, 0x65, 0x28, 0x00, 0x00, 0x26,
	0x86, 0x43, 0x05, 0x03, 0x01, 0x0C, 0x08, 0x43, 0x76, 0x2C, 0xA8, 0x1C,
	0x81, 0x01, 0x0C, 0x31, 0x00, 0x00, 0x26, 0x1F, 0x01, 0x08, 0x0B, 0x43,
	0x59, 0x1F, 0x08, 0x00, 0x01, 0x03, 0x00, 0x01, 0x00, 0x74, 0x3D, 0x29,
	0x1A, 0x36, 0x06, 0x07, 0x02, 0x00, 0xCB, 0x03, 0x00, 0x04, 0x75, 0x01,
	0x00, 0xC2, 0x02, 0x00, 0x26, 0x1A, 0x17, 0x06, 0x02, 0x6C, 0x28, 0xCB,
	0x04, 0x76, 0x01, 0x01, 0x00, 0x74, 0x3D, 0x01, 0x16, 0x84, 0x3D, 0x01,
	0x00, 0x87, 0x3C, 0x34, 0xD1, 0x29, 0xB1, 0x06, 0x09, 0x01, 0x7F, 0xAC,
	0x01, 0x7F, 0xCE, 0x04, 0x80, 0x53, 0xAE, 0x76, 0x2C, 0x9E, 0x01,
	T0_INT1(BR_KEYTYPE_SIGN), 0x17, 0x06, 0x01, 0xB2, 0xB5, 0x26, 0x01,
	0x0D, 0x0E, 0x06, 0x07, 0x25, 0xB4, 0xB5, 0x01, 0x7F, 0x04, 0x02, 0x01,
	0x00, 0x03, 0x00, 0x01, 0x0E, 0x0E, 0x05, 0x02, 0x6F, 0x28, 0x06, 0x02,
	0x64, 0x28, 0x33, 0x06, 0x02, 0x6F, 0x28, 0x02, 0x00, 0x06, 0x1C, 0xCF,
	0x7D, 0x2E, 0x01, 0x81, 0x7F, 0x0E, 0x06, 0x0D, 0x25, 0x01, 0x10, 0xDA,
	0x01, 0x00, 0xD9, 0x76, 0x2C, 0xA8, 0x24, 0x04, 0x04, 0xD2, 0x06, 0x01,
	0xD0, 0x04, 0x01, 0xD2, 0x01, 0x7F, 0xCE, 0x01, 0x7F, 0xAC, 0x01, 0x01,
	0x74, 0x3D, 0x01, 0x17, 0x84, 0x3D, 0x00, 0x00, 0x38, 0x38, 0x00, 0x00,
	0x97, 0x01, 0x0C, 0x11, 0x01, 0x00, 0x38, 0x0E, 0x06, 0x05, 0x25, 0x01,
	T0_INT1(BR_KEYTYPE_RSA | BR_KEYTYPE_KEYX), 0x04, 0x30, 0x01, 0x01,
	0x38, 0x0E, 0x06, 0x05, 0x25, 0x01,
	T0_INT1(BR_KEYTYPE_RSA | BR_KEYTYPE_SIGN), 0x04, 0x25, 0x01, 0x02,
	0x38, 0x0E, 0x06, 0x05, 0x25, 0x01,
	T0_INT1(BR_KEYTYPE_EC  | BR_KEYTYPE_SIGN), 0x04, 0x1A, 0x01, 0x03,
	0x38, 0x0E, 0x06, 0x05, 0x25, 0x01,
	T0_INT1(BR_KEYTYPE_EC  | BR_KEYTYPE_KEYX), 0x04, 0x0F, 0x01, 0x04,
	0x38, 0x0E, 0x06, 0x05, 0x25, 0x01,
	T0_INT1(BR_KEYTYPE_EC  | BR_KEYTYPE_KEYX), 0x04, 0x04, 0x01, 0x00,
	0x43, 0x25, 0x00, 0x00, 0x7F, 0x2E, 0x01, 0x0E, 0x0E, 0x06, 0x04, 0x01,
	0x00, 0x04, 0x02, 0x01, 0x05, 0x00, 0x00, 0x3F, 0x06, 0x04, 0x01, 0x06,
	0x04, 0x02, 0x01, 0x00, 0x00, 0x00, 0x85, 0x2E, 0x26, 0x06, 0x08, 0x01,
	0x01, 0x09, 0x01, 0x11, 0x07, 0x04, 0x03, 0x25, 0x01, 0x05, 0x00, 0x01,
	0x40, 0x03, 0x00, 0x25, 0x01, 0x00, 0x42, 0x06, 0x03, 0x02, 0x00, 0x08,
	0x41, 0x06, 0x03, 0x02, 0x00, 0x08, 0x26, 0x06, 0x06, 0x01, 0x01, 0x0B,
	0x01, 0x06, 0x08, 0x00, 0x00, 0x88, 0x3E, 0x26, 0x06, 0x03, 0x01, 0x09,
	0x08, 0x00, 0x01, 0x3F, 0x26, 0x06, 0x1E, 0x01, 0x00, 0x03, 0x00, 0x26,
	0x06, 0x0E, 0x26, 0x01, 0x01, 0x17, 0x02, 0x00, 0x08, 0x03, 0x00, 0x01,
	0x01, 0x11, 0x04, 0x6F, 0x25, 0x02, 0x00, 0x01, 0x01, 0x0B, 0x01, 0x06,
	0x08, 0x00, 0x00, 0x7C, 0x2D, 0x43, 0x11, 0x01, 0x01, 0x17, 0x35, 0x00,
	0x00, 0x9C, 0xCA, 0x26, 0x01, 0x07, 0x17, 0x01, 0x00, 0x38, 0x0E, 0x06,
	0x09, 0x25, 0x01, 0x10, 0x17, 0x06, 0x01, 0x9C, 0x04, 0x2D, 0x01, 0x01,
	0x38, 0x0E, 0x06, 0x24, 0x25, 0x25, 0x01, 0x00, 0x74, 0x3D, 0xB0, 0x85,
	0x2E, 0x01, 0x01, 0x0E, 0x01, 0x01, 0xA5, 0x37, 0x06, 0x0F, 0x29, 0x1A,
	0x36, 0x06, 0x04, 0xCA, 0x25, 0x04, 0x78, 0x01, 0x80, 0x64, 0xC2, 0x04,
	0x01, 0x9C, 0x04, 0x03, 0x6F, 0x28, 0x25, 0x04, 0xFF, 0x3C, 0x01, 0x26,
	0x03, 0x00, 0x09, 0x26, 0x55, 0x06, 0x02, 0x65, 0x28, 0x02, 0x00, 0x00,
	0x00, 0x97, 0x01, 0x0F, 0x17, 0x00, 0x00, 0x73, 0x2E, 0x01, 0x00, 0x38,
	0x0E, 0x06, 0x10, 0x25, 0x26, 0x01, 0x01, 0x0D, 0x06, 0x03, 0x25, 0x01,
	0x02, 0x73, 0x3D, 0x01, 0x00, 0x04, 0x22, 0x01, 0x01, 0x38, 0x0E, 0x06,
	0x15, 0x25, 0x01, 0x00, 0x73, 0x3D, 0x26, 0x01, 0x80, 0x64, 0x0E, 0x06,
	0x05, 0x01, 0x82, 0x00, 0x08, 0x28, 0x57, 0x00, 0x04, 0x07, 0x25, 0x01,
	0x82, 0x00, 0x08, 0x28, 0x25, 0x00, 0x00, 0x01, 0x00, 0x2F, 0x06, 0x05,
	0x3A, 0xA9, 0x37, 0x04, 0x78, 0x26, 0x06, 0x04, 0x01, 0x01, 0x8C, 0x3D,
	0x00, 0x01, 0xBC, 0xA7, 0xBC, 0xA7, 0xBE, 0x81, 0x43, 0x26, 0x03, 0x00,
	0xB3, 0x98, 0x98, 0x02, 0x00, 0x4A, 0x26, 0x55, 0x06, 0x0A, 0x01, 0x03,
	0xA5, 0x06, 0x02, 0x6F, 0x28, 0x25, 0x04, 0x03, 0x59, 0x87, 0x3C, 0x00,
	0x00, 0x2F, 0x06, 0x0B, 0x83, 0x2E, 0x01, 0x14, 0x0D, 0x06, 0x02, 0x6F,
	0x28, 0x04, 0x11, 0xCA, 0x01, 0x07, 0x17, 0x26, 0x01, 0x02, 0x0D, 0x06,
	0x06, 0x06, 0x02, 0x6F, 0x28, 0x04, 0x70, 0x25, 0xBF, 0x01, 0x01, 0x0D,
	0x33, 0x37, 0x06, 0x02, 0x5E, 0x28, 0x26, 0x01, 0x01, 0xC5, 0x36, 0xAF,
	0x00, 0x01, 0xB5, 0x01, 0x0B, 0x0E, 0x05, 0x02, 0x6F, 0x28, 0x26, 0x01,
	0x03, 0x0E, 0x06, 0x08, 0xBD, 0x06, 0x02, 0x65, 0x28, 0x43, 0x25, 0x00,
	0x43, 0x54, 0xBD, 0xA7, 0x26, 0x06, 0x23, 0xBD, 0xA7, 0x26, 0x53, 0x26,
	0x06, 0x18, 0x26, 0x01, 0x82, 0x00, 0x0F, 0x06, 0x05, 0x01, 0x82, 0x00,
	0x04, 0x01, 0x26, 0x03, 0x00, 0x81, 0x02, 0x00, 0xB3, 0x02, 0x00, 0x50,
	0x04, 0x65, 0x98, 0x51, 0x04, 0x5A, 0x98, 0x98, 0x52, 0x26, 0x06, 0x02,
	0x35, 0x00, 0x25, 0x2B, 0x00, 0x00, 0x76, 0x2C, 0x9E, 0x01, 0x7F, 0xAD,
	0x26, 0x55, 0x06, 0x02, 0x35, 0x28, 0x26, 0x05, 0x02, 0x6F, 0x28, 0x38,
	0x17, 0x0D, 0x06, 0x02, 0x71, 0x28, 0x3B, 0x00, 0x00, 0x99, 0xB5, 0x01,
	0x14, 0x0D, 0x06, 0x02, 0x6F, 0x28, 0x81, 0x01, 0x0C, 0x08, 0x01, 0x0C,
	0xB3, 0x98, 0x81, 0x26, 0x01, 0x0C, 0x08, 0x01, 0x0C, 0x30, 0x05, 0x02,
	0x61, 0x28, 0x00, 0x00, 0xB6, 0x06, 0x02, 0x6F, 0x28, 0x06, 0x02, 0x63,
	0x28, 0x00, 0x0A, 0xB5, 0x01, 0x02, 0x0E, 0x05, 0x02, 0x6F, 0x28, 0xBC,
	0x03, 0x00, 0x02, 0x00, 0x92, 0x2C, 0x0A, 0x02, 0x00, 0x91, 0x2C, 0x0F,
	0x37, 0x06, 0x02, 0x70, 0x28, 0x02, 0x00, 0x90, 0x2C, 0x0D, 0x06, 0x02,
	0x68, 0x28, 0x02, 0x00, 0x93, 0x3C, 0x89, 0x01, 0x20, 0xB3, 0x01, 0x00,
	0x03, 0x01, 0xBE, 0x03, 0x02, 0x02, 0x02, 0x01, 0x20, 0x0F, 0x06, 0x02,
	0x6D, 0x28, 0x81, 0x02, 0x02, 0xB3, 0x02, 0x02, 0x8B, 0x2E, 0x0E, 0x02,
	0x02, 0x01, 0x00, 0x0F, 0x17, 0x06, 0x0B, 0x8A, 0x81, 0x02, 0x02, 0x30,
	0x06, 0x04, 0x01, 0x7F, 0x03, 0x01, 0x8A, 0x81, 0x02, 0x02, 0x31, 0x02,
	0x02, 0x8B, 0x3D, 0x02, 0x00, 0x8F, 0x02, 0x01, 0x95, 0xBC, 0x26, 0xC0,
	0x55, 0x06, 0x02, 0x5F, 0x28, 0x76, 0x02, 0x01, 0x95, 0xBE, 0x06, 0x02,
	0x60, 0x28, 0x26, 0x06, 0x81, 0x45, 0xBC, 0xA7, 0xA3, 0x03, 0x03, 0xA1,
	0x03, 0x04, 0x9F, 0x03, 0x05, 0xA2, 0x03, 0x06, 0xA4, 0x03, 0x07, 0xA0,
	0x03, 0x08, 0x27, 0x03, 0x09, 0x26, 0x06, 0x81, 0x18, 0xBC, 0x01, 0x00,
	0x38, 0x0E, 0x06, 0x0F, 0x25, 0x02, 0x03, 0x05, 0x02, 0x69, 0x28, 0x01,
	0x00, 0x03, 0x03, 0xBB, 0x04, 0x80, 0x7F, 0x01, 0x01, 0x38, 0x0E, 0x06,
	0x0F, 0x25, 0x02, 0x05, 0x05, 0x02, 0x69, 0x28, 0x01, 0x00, 0x03, 0x05,
	0xB9, 0x04, 0x80, 0x6A, 0x01, 0x83, 0xFE, 0x01, 0x38, 0x0E, 0x06, 0x0F,
	0x25, 0x02, 0x04, 0x05, 0x02, 0x69, 0x28, 0x01, 0x00, 0x03, 0x04, 0xBA,
	0x04, 0x80, 0x53, 0x01, 0x0D, 0x38, 0x0E, 0x06, 0x0E, 0x25, 0x02, 0x06,
	0x05, 0x02, 0x69, 0x28, 0x01, 0x00, 0x03, 0x06, 0xB7, 0x04, 0x3F, 0x01,
	0x0A, 0x38, 0x0E, 0x06, 0x0E, 0x25, 0x02, 0x07, 0x05, 0x02, 0x69, 0x28,
	0x01, 0x00, 0x03, 0x07, 0xB7, 0x04, 0x2B, 0x01, 0x0B, 0x38, 0x0E, 0x06,
	0x0E, 0x25, 0x02, 0x08, 0x05, 0x02, 0x69, 0x28, 0x01, 0x00, 0x03, 0x08,
	0xB7, 0x04, 0x17, 0x01, 0x10, 0x38, 0x0E, 0x06, 0x0E, 0x25, 0x02, 0x09,
	0x05, 0x02, 0x69, 0x28, 0x01, 0x00, 0x03, 0x09, 0xAB, 0x04, 0x03, 0x69,
	0x28, 0x25, 0x04, 0xFE, 0x64, 0x02, 0x04, 0x06, 0x0D, 0x02, 0x04, 0x01,
	0x05, 0x0F, 0x06, 0x02, 0x66, 0x28, 0x01, 0x01, 0x85, 0x3D, 0x98, 0x98,
	0x02, 0x01, 0x00, 0x04, 0xB5, 0x01, 0x0C, 0x0E, 0x05, 0x02, 0x6F, 0x28,
	0xBE, 0x01, 0x03, 0x0E, 0x05, 0x02, 0x6A, 0x28, 0xBC, 0x26, 0x79, 0x3D,
	0x26, 0x01, 0x20, 0x10, 0x06, 0x02, 0x6A, 0x28, 0x3F, 0x43, 0x11, 0x01,
	0x01, 0x17, 0x05, 0x02, 0x6A, 0x28, 0xBE, 0x26, 0x01, 0x81, 0x05, 0x0F,
	0x06, 0x02, 0x6A, 0x28, 0x26, 0x7B, 0x3D, 0x7A, 0x43, 0xB3, 0x8F, 0x2C,
	0x01, 0x86, 0x03, 0x10, 0x03, 0x00, 0x76, 0x2C, 0xC8, 0x03, 0x01, 0x01,
	0x02, 0x03, 0x02, 0x02, 0x00, 0x06, 0x21, 0xBE, 0x26, 0x26, 0x01, 0x02,
	0x0A, 0x43, 0x01, 0x06, 0x0F, 0x37, 0x06, 0x02, 0x6A, 0x28, 0x03, 0x02,
	0xBE, 0x02, 0x01, 0x01, 0x01, 0x0B, 0x01, 0x03, 0x08, 0x0E, 0x05, 0x02,
	0x6A, 0x28, 0x04, 0x08, 0x02, 0x01, 0x06, 0x04, 0x01, 0x00, 0x03, 0x02,
	0xBC, 0x26, 0x03, 0x03, 0x26, 0x01, 0x84, 0x00, 0x0F, 0x06, 0x02, 0x6B,
	0x28, 0x81, 0x43, 0xB3, 0x02, 0x02, 0x02, 0x01, 0x02, 0x03, 0x4D, 0x26,
	0x06, 0x01, 0x28, 0x25, 0x98, 0x00, 0x02, 0x03, 0x00, 0x03, 0x01, 0x02,
	0x00, 0x94, 0x02, 0x01, 0x02, 0x00, 0x39, 0x26, 0x01, 0x00, 0x0E, 0x06,
	0x02, 0x5D, 0x00, 0xCC, 0x04, 0x74, 0x02, 0x01, 0x00, 0x03, 0x00, 0xBE,
	0xA7, 0x26, 0x06, 0x80, 0x43, 0xBE, 0x01, 0x01, 0x38, 0x0E, 0x06, 0x06,
	0x25, 0x01, 0x81, 0x7F, 0x04, 0x2E, 0x01, 0x80, 0x40, 0x38, 0x0E, 0x06,
	0x07, 0x25, 0x01, 0x83, 0xFE, 0x00, 0x04, 0x20, 0x01, 0x80, 0x41, 0x38,
	0x0E, 0x06, 0x07, 0x25, 0x01, 0x84, 0x80, 0x00, 0x04, 0x12, 0x01, 0x80,
	0x42, 0x38, 0x0E, 0x06, 0x07, 0x25, 0x01, 0x88, 0x80, 0x00, 0x04, 0x04,
	0x01, 0x00, 0x43, 0x25, 0x02, 0x00, 0x37, 0x03, 0x00, 0x04, 0xFF, 0x39,
	0x98, 0x76, 0x2C, 0xC6, 0x05, 0x09, 0x02, 0x00, 0x01, 0x83, 0xFF, 0x7F,
	0x17, 0x03, 0x00, 0x8F, 0x2C, 0x01, 0x86, 0x03, 0x10, 0x06, 0x3A, 0xB8,
	0x26, 0x7E, 0x3C, 0x40, 0x25, 0x26, 0x01, 0x08, 0x0B, 0x37, 0x01, 0x8C,
	0x80, 0x00, 0x37, 0x17, 0x02, 0x00, 0x17, 0x02, 0x00, 0x01, 0x8C, 0x80,
	0x00, 0x17, 0x06, 0x19, 0x26, 0x01, 0x81, 0x7F, 0x17, 0x06, 0x05, 0x01,
	0x84, 0x80, 0x00, 0x37, 0x26, 0x01, 0x83, 0xFE, 0x00, 0x17, 0x06, 0x05,
	0x01, 0x88, 0x80, 0x00, 0x37, 0x03, 0x00, 0x04, 0x09, 0x02, 0x00, 0x01,
	0x8C, 0x88, 0x01, 0x17, 0x03, 0x00, 0x16, 0xBC, 0xA7, 0x26, 0x06, 0x23,
	0xBC, 0xA7, 0x26, 0x15, 0x26, 0x06, 0x18, 0x26, 0x01, 0x82, 0x00, 0x0F,
	0x06, 0x05, 0x01, 0x82, 0x00, 0x04, 0x01, 0x26, 0x03, 0x01, 0x81, 0x02,
	0x01, 0xB3, 0x02, 0x01, 0x12, 0x04, 0x65, 0x98, 0x13, 0x04, 0x5A, 0x98,
	0x14, 0x98, 0x02, 0x00, 0x2A, 0x00, 0x00, 0xB6, 0x26, 0x57, 0x06, 0x07,
	0x25, 0x06, 0x02, 0x63, 0x28, 0x04, 0x74, 0x00, 0x00, 0xBF, 0x01, 0x03,
	0xBD, 0x43, 0x25, 0x43, 0x00, 0x00, 0xBC, 0xC3, 0x00, 0x03, 0x01, 0x00,
	0x03, 0x00, 0xBC, 0xA7, 0x26, 0x06, 0x32, 0xBE, 0x03, 0x01, 0xBE, 0x03,
	0x02, 0x02, 0x01, 0x01, 0x02, 0x10, 0x02, 0x01, 0x01, 0x06, 0x0C, 0x17,
	0x02, 0x02, 0x01, 0x01, 0x0E, 0x02, 0x02, 0x01, 0x03, 0x0E, 0x37, 0x17,
	0x06, 0x11, 0x02, 0x00, 0x01, 0x01, 0x02, 0x02, 0x5A, 0x01, 0x02, 0x0B,
	0x02, 0x01, 0x08, 0x0B, 0x37, 0x03, 0x00, 0x04, 0x4B, 0x98, 0x02, 0x00,
	0x00, 0x00, 0xBC, 0x01, 0x01, 0x0E, 0x05, 0x02, 0x62, 0x28, 0xBE, 0x01,
	0x08, 0x08, 0x7F, 0x2E, 0x0E, 0x05, 0x02, 0x62, 0x28, 0x00, 0x00, 0xBC,
	0x85, 0x2E, 0x05, 0x15, 0x01, 0x01, 0x0E, 0x05, 0x02, 0x66, 0x28, 0xBE,
	0x01, 0x00, 0x0E, 0x05, 0x02, 0x66, 0x28, 0x01, 0x02, 0x85, 0x3D, 0x04,
	0x1C, 0x01, 0x19, 0x0E, 0x05, 0x02, 0x66, 0x28, 0xBE, 0x01, 0x18, 0x0E,
	0x05, 0x02, 0x66, 0x28, 0x81, 0x01, 0x18, 0xB3, 0x86, 0x81, 0x01, 0x18,
	0x30, 0x05, 0x02, 0x66, 0x28, 0x00, 0x00, 0xBC, 0x06, 0x02, 0x67, 0x28,
	0x00, 0x00, 0x01, 0x02, 0x94, 0xBF, 0x01, 0x08, 0x0B, 0xBF, 0x08, 0x00,
	0x00, 0x01, 0x03, 0x94, 0xBF, 0x01, 0x08, 0x0B, 0xBF, 0x08, 0x01, 0x08,
	0x0B, 0xBF, 0x08, 0x00, 0x00, 0x01, 0x01, 0x94, 0xBF, 0x00, 0x00, 0x3A,
	0x26, 0x55, 0x05, 0x01, 0x00, 0x25, 0xCC, 0x04, 0x76, 0x02, 0x03, 0x00,
	0x8E, 0x2E, 0x03, 0x01, 0x01, 0x00, 0x26, 0x02, 0x01, 0x0A, 0x06, 0x10,
	0x26, 0x01, 0x01, 0x0B, 0x8D, 0x08, 0x2C, 0x02, 0x00, 0x0E, 0x06, 0x01,
	0x00, 0x59, 0x04, 0x6A, 0x25, 0x01, 0x7F, 0x00, 0x00, 0x01, 0x15, 0x84,
	0x3D, 0x43, 0x4F, 0x25, 0x4F, 0x25, 0x29, 0x00, 0x00, 0x01, 0x01, 0x43,
	0xC1, 0x00, 0x00, 0x43, 0x38, 0x94, 0x43, 0x26, 0x06, 0x05, 0xBF, 0x25,
	0x5A, 0x04, 0x78, 0x25, 0x00, 0x00, 0x26, 0x01, 0x81, 0xAC, 0x00, 0x0E,
	0x06, 0x04, 0x25, 0x01, 0x7F, 0x00, 0x97, 0x56, 0x00, 0x02, 0x03, 0x00,
	0x76, 0x2C, 0x97, 0x03, 0x01, 0x02, 0x01, 0x01, 0x0F, 0x17, 0x02, 0x01,
	0x01, 0x04, 0x11, 0x01, 0x0F, 0x17, 0x02, 0x01, 0x01, 0x08, 0x11, 0x01,
	0x0F, 0x17, 0x01, 0x00, 0x38, 0x0E, 0x06, 0x10, 0x25, 0x01, 0x00, 0x01,
	0x18, 0x02, 0x00, 0x06, 0x03, 0x46, 0x04, 0x01, 0x47, 0x04, 0x80, 0x68,
	0x01, 0x01, 0x38, 0x0E, 0x06, 0x10, 0x25, 0x01, 0x01, 0x01, 0x10, 0x02,
	0x00, 0x06, 0x03, 0x46, 0x04, 0x01, 0x47, 0x04, 0x80, 0x52, 0x01, 0x02,
	0x38, 0x0E, 0x06, 0x0F, 0x25, 0x01, 0x01, 0x01, 0x20, 0x02, 0x00, 0x06,
	0x03, 0x46, 0x04, 0x01, 0x47, 0x04, 0x3D, 0x01, 0x03, 0x38, 0x0E, 0x06,
	0x0E, 0x25, 0x25, 0x01, 0x10, 0x02, 0x00, 0x06, 0x03, 0x44, 0x04, 0x01,
	0x45, 0x04, 0x29, 0x01, 0x04, 0x38, 0x0E, 0x06, 0x0E, 0x25, 0x25, 0x01,
	0x20, 0x02, 0x00, 0x06, 0x03, 0x44, 0x04, 0x01, 0x45, 0x04, 0x15, 0x01,
	0x05, 0x38, 0x0E, 0x06, 0x0C, 0x25, 0x25, 0x02, 0x00, 0x06, 0x03, 0x48,
	0x04, 0x01, 0x49, 0x04, 0x03, 0x65, 0x28, 0x25, 0x00, 0x00, 0x97, 0x01,
	0x0C, 0x11, 0x01, 0x02, 0x0F, 0x00, 0x00, 0x97, 0x01, 0x0C, 0x11, 0x26,
	0x58, 0x43, 0x01, 0x03, 0x0A, 0x17, 0x00, 0x00, 0x97, 0x01, 0x0C, 0x11,
	0x01, 0x01, 0x0E, 0x00, 0x00, 0x97, 0x01, 0x0C, 0x11, 0x57, 0x00, 0x00,
	0x1B, 0x01, 0x00, 0x72, 0x2E, 0x26, 0x06, 0x1F, 0x01, 0x01, 0x38, 0x0E,
	0x06, 0x06, 0x25, 0x01, 0x00, 0x9B, 0x04, 0x11, 0x01, 0x02, 0x38, 0x0E,
	0x06, 0x0A, 0x25, 0x74, 0x2E, 0x06, 0x03, 0x01, 0x10, 0x37, 0x04, 0x01,
	0x25, 0x04, 0x01, 0x25, 0x78, 0x2E, 0x05, 0x33, 0x2F, 0x06, 0x30, 0x83,
	0x2E, 0x01, 0x14, 0x38, 0x0E, 0x06, 0x06, 0x25, 0x01, 0x02, 0x37, 0x04,
	0x22, 0x01, 0x15, 0x38, 0x0E, 0x06, 0x09, 0x25, 0xAA, 0x06, 0x03, 0x01,
	0x7F, 0x9B, 0x04, 0x13, 0x01, 0x16, 0x38, 0x0E, 0x06, 0x06, 0x25, 0x01,
	0x01, 0x37, 0x04, 0x07, 0x25, 0x01, 0x04, 0x37, 0x01, 0x00, 0x25, 0x1A,
	0x06, 0x03, 0x01, 0x08, 0x37, 0x00, 0x00, 0x1B, 0x26, 0x05, 0x0F, 0x2F,
	0x06, 0x0C, 0x83, 0x2E, 0x01, 0x15, 0x0E, 0x06, 0x04, 0x25, 0xAA, 0x04,
	0x01, 0x20, 0x00, 0x00, 0xCA, 0x01, 0x07, 0x17, 0x01, 0x01, 0x0F, 0x06,
	0x02, 0x6F, 0x28, 0x00, 0x01, 0x03, 0x00, 0x29, 0x1A, 0x06, 0x05, 0x02,
	0x00, 0x84, 0x3D, 0x00, 0xCA, 0x25, 0x04, 0x74, 0x00, 0x01, 0x14, 0xCD,
	0x01, 0x01, 0xDA, 0x29, 0x26, 0x01, 0x00, 0xC5, 0x01, 0x16, 0xCD, 0xD3,
	0x29, 0x00, 0x00, 0x01, 0x0B, 0xDA, 0x4B, 0x26, 0x26, 0x01, 0x03, 0x08,
	0xD9, 0xD9, 0x18, 0x26, 0x55, 0x06, 0x02, 0x25, 0x00, 0xD9, 0x1D, 0x26,
	0x06, 0x05, 0x81, 0x43, 0xD4, 0x04, 0x77, 0x25, 0x04, 0x6C, 0x00, 0x21,
	0x01, 0x0F, 0xDA, 0x26, 0x8F, 0x2C, 0x01, 0x86, 0x03, 0x10, 0x06, 0x0C,
	0x01, 0x04, 0x08, 0xD9, 0x7D, 0x2E, 0xDA, 0x75, 0x2E, 0xDA, 0x04, 0x02,
	0x5B, 0xD9, 0x26, 0xD8, 0x81, 0x43, 0xD4, 0x00, 0x02, 0xA1, 0xA3, 0x08,
	0x9F, 0x08, 0xA2, 0x08, 0xA4, 0x08, 0xA0, 0x08, 0x27, 0x08, 0x03, 0x00,
	0x01, 0x01, 0xDA, 0x01, 0x27, 0x8B, 0x2E, 0x08, 0x8E, 0x2E, 0x01, 0x01,
	0x0B, 0x08, 0x02, 0x00, 0x06, 0x04, 0x5B, 0x02, 0x00, 0x08, 0x80, 0x2C,
	0x38, 0x09, 0x26, 0x58, 0x06, 0x24, 0x02, 0x00, 0x05, 0x04, 0x43, 0x5B,
	0x43, 0x5C, 0x01, 0x04, 0x09, 0x26, 0x55, 0x06, 0x03, 0x25, 0x01, 0x00,
	0x26, 0x01, 0x04, 0x08, 0x02, 0x00, 0x08, 0x03, 0x00, 0x43, 0x01, 0x04,
	0x08, 0x38, 0x08, 0x43, 0x04, 0x03, 0x25, 0x01, 0x7F, 0x03, 0x01, 0xD9,
	0x91, 0x2C, 0xD8, 0x77, 0x01, 0x04, 0x19, 0x77, 0x01, 0x04, 0x08, 0x01,
	0x1C, 0x32, 0x77, 0x01, 0x20, 0xD4, 0x8A, 0x8B, 0x2E, 0xD6, 0x8E, 0x2E,
	0x26, 0x01, 0x01, 0x0B, 0xD8, 0x8D, 0x43, 0x26, 0x06, 0x0F, 0x5A, 0x38,
	0x2C, 0x26, 0xC4, 0x05, 0x02, 0x5F, 0x28, 0xD8, 0x43, 0x5B, 0x43, 0x04,
	0x6E, 0x5D, 0x01, 0x01, 0xDA, 0x01, 0x00, 0xDA, 0x02, 0x00, 0x06, 0x81,
	0x46, 0x02, 0x00, 0xD8, 0xA1, 0x06, 0x0E, 0x01, 0x83, 0xFE, 0x01, 0xD8,
	0x86, 0xA1, 0x01, 0x04, 0x09, 0x26, 0xD8, 0x5A, 0xD6, 0xA3, 0x06, 0x16,
	0x01, 0x00, 0xD8, 0x88, 0xA3, 0x01, 0x04, 0x09, 0x26, 0xD8, 0x01, 0x02,
	0x09, 0x26, 0xD8, 0x01, 0x00, 0xDA, 0x01, 0x03, 0x09, 0xD5, 0x9F, 0x06,
	0x0C, 0x01, 0x01, 0xD8, 0x01, 0x01, 0xD8, 0x7F, 0x2E, 0x01, 0x08, 0x09,
	0xDA, 0xA2, 0x06, 0x19, 0x01, 0x0D, 0xD8, 0xA2, 0x01, 0x04, 0x09, 0x26,
	0xD8, 0x01, 0x02, 0x09, 0xD8, 0x41, 0x06, 0x03, 0x01, 0x03, 0xD7, 0x42,
	0x06, 0x03, 0x01, 0x01, 0xD7, 0xA4, 0x26, 0x06, 0x22, 0x01, 0x0A, 0xD8,
	0x01, 0x04, 0x09, 0x26, 0xD8, 0x5C, 0xD8, 0x3F, 0x01, 0x00, 0x26, 0x01,
	0x20, 0x0A, 0x06, 0x0C, 0x9D, 0x11, 0x01, 0x01, 0x17, 0x06, 0x02, 0x26,
	0xD8, 0x59, 0x04, 0x6E, 0x5D, 0x04, 0x01, 0x25, 0xA0, 0x06, 0x0A, 0x01,
	0x0B, 0xD8, 0x01, 0x02, 0xD8, 0x01, 0x82, 0x00, 0xD8, 0x27, 0x26, 0x06,
	0x1F, 0x01, 0x10, 0xD8, 0x01, 0x04, 0x09, 0x26, 0xD8, 0x5C, 0xD8, 0x82,
	0x2C, 0x01, 0x00, 0x9D, 0x0F, 0x06, 0x0A, 0x26, 0x1E, 0x26, 0xDA, 0x81,
	0x43, 0xD4, 0x59, 0x04, 0x72, 0x5D, 0x04, 0x01, 0x25, 0x02, 0x01, 0x55,
	0x05, 0x11, 0x01, 0x15, 0xD8, 0x02, 0x01, 0x26, 0xD8, 0x26, 0x06, 0x06,
	0x5A, 0x01, 0x00, 0xDA, 0x04, 0x77, 0x25, 0x00, 0x00, 0x01, 0x10, 0xDA,
	0x76, 0x2C, 0x26, 0xC9, 0x06, 0x0C, 0xA8, 0x23, 0x26, 0x5B, 0xD9, 0x26,
	0xD8, 0x81, 0x43, 0xD4, 0x04, 0x0D, 0x26, 0xC7, 0x43, 0xA8, 0x22, 0x26,
	0x59, 0xD9, 0x26, 0xDA, 0x81, 0x43, 0xD4, 0x00, 0x00, 0x99, 0x01, 0x14,
	0xDA, 0x01, 0x0C, 0xD9, 0x81, 0x01, 0x0C, 0xD4, 0x00, 0x00, 0x4E, 0x26,
	0x01, 0x00, 0x0E, 0x06, 0x02, 0x5D, 0x00, 0xCA, 0x25, 0x04, 0x73, 0x00,
	0x26, 0xD8, 0xD4, 0x00, 0x00, 0x26, 0xDA, 0xD4, 0x00, 0x01, 0x03, 0x00,
	0x40, 0x25, 0x26, 0x01, 0x10, 0x17, 0x06, 0x06, 0x01, 0x04, 0xDA, 0x02,
	0x00, 0xDA, 0x26, 0x01, 0x08, 0x17, 0x06, 0x06, 0x01, 0x03, 0xDA, 0x02,
	0x00, 0xDA, 0x26, 0x01, 0x20, 0x17, 0x06, 0x06, 0x01, 0x05, 0xDA, 0x02,
	0x00, 0xDA, 0x26, 0x01, 0x80, 0x40, 0x17, 0x06, 0x06, 0x01, 0x06, 0xDA,
	0x02, 0x00, 0xDA, 0x01, 0x04, 0x17, 0x06, 0x06, 0x01, 0x02, 0xDA, 0x02,
	0x00, 0xDA, 0x00, 0x00, 0x26, 0x01, 0x08, 0x4C, 0xDA, 0xDA, 0x00, 0x00,
	0x26, 0x01, 0x10, 0x4C, 0xDA, 0xD8, 0x00, 0x00, 0x26, 0x4F, 0x06, 0x02,
	0x25, 0x00, 0xCA, 0x25, 0x04, 0x76
};

static const uint16_t t0_caddr[] = {
	0,
	5,
	10,
	15,
	20,
	25,
	30,
	35,
	40,
	44,
	48,
	52,
	56,
	60,
	64,
	68,
	72,
	76,
	80,
	84,
	88,
	92,
	96,
	100,
	104,
	108,
	112,
	116,
	120,
	124,
	129,
	134,
	139,
	144,
	149,
	154,
	159,
	164,
	169,
	174,
	179,
	184,
	189,
	194,
	199,
	204,
	209,
	214,
	219,
	224,
	229,
	234,
	239,
	244,
	249,
	254,
	259,
	264,
	269,
	274,
	279,
	284,
	289,
	294,
	303,
	316,
	320,
	345,
	351,
	370,
	381,
	415,
	535,
	539,
	604,
	619,
	630,
	648,
	677,
	687,
	723,
	733,
	803,
	817,
	823,
	883,
	902,
	937,
	986,
	1062,
	1089,
	1120,
	1131,
	1456,
	1603,
	1627,
	1843,
	1857,
	1866,
	1870,
	1934,
	1955,
	2011,
	2018,
	2029,
	2045,
	2051,
	2062,
	2097,
	2109,
	2115,
	2130,
	2146,
	2302,
	2311,
	2324,
	2333,
	2340,
	2443,
	2464,
	2477,
	2493,
	2511,
	2543,
	2577,
	2925,
	2961,
	2974,
	2988,
	2993,
	2998,
	3064,
	3072,
	3080
};

#define T0_INTERPRETED   85

#define T0_ENTER(ip, rp, slot)   do { \
		const unsigned char *t0_newip; \
		uint32_t t0_lnum; \
		t0_newip = &t0_codeblock[t0_caddr[(slot) - T0_INTERPRETED]]; \
		t0_lnum = t0_parse7E_unsigned(&t0_newip); \
		(rp) += t0_lnum; \
		*((rp) ++) = (uint32_t)((ip) - &t0_codeblock[0]) + (t0_lnum << 16); \
		(ip) = t0_newip; \
	} while (0)

#define T0_DEFENTRY(name, slot) \
void \
name(void *ctx) \
{ \
	t0_context *t0ctx = ctx; \
	t0ctx->ip = &t0_codeblock[0]; \
	T0_ENTER(t0ctx->ip, t0ctx->rp, slot); \
}

T0_DEFENTRY(br_ssl_hs_client_init_main, 166)

#define T0_NEXT(t0ipp)   (*(*(t0ipp)) ++)

void
br_ssl_hs_client_run(void *t0ctx)
{
	uint32_t *dp, *rp;
	const unsigned char *ip;

#define T0_LOCAL(x)    (*(rp - 2 - (x)))
#define T0_POP()       (*-- dp)
#define T0_POPi()      (*(int32_t *)(-- dp))
#define T0_PEEK(x)     (*(dp - 1 - (x)))
#define T0_PEEKi(x)    (*(int32_t *)(dp - 1 - (x)))
#define T0_PUSH(v)     do { *dp = (v); dp ++; } while (0)
#define T0_PUSHi(v)    do { *(int32_t *)dp = (v); dp ++; } while (0)
#define T0_RPOP()      (*-- rp)
#define T0_RPOPi()     (*(int32_t *)(-- rp))
#define T0_RPUSH(v)    do { *rp = (v); rp ++; } while (0)
#define T0_RPUSHi(v)   do { *(int32_t *)rp = (v); rp ++; } while (0)
#define T0_ROLL(x)     do { \
	size_t t0len = (size_t)(x); \
	uint32_t t0tmp = *(dp - 1 - t0len); \
	memmove(dp - t0len - 1, dp - t0len, t0len * sizeof *dp); \
	*(dp - 1) = t0tmp; \
} while (0)
#define T0_SWAP()      do { \
	uint32_t t0tmp = *(dp - 2); \
	*(dp - 2) = *(dp - 1); \
	*(dp - 1) = t0tmp; \
} while (0)
#define T0_ROT()       do { \
	uint32_t t0tmp = *(dp - 3); \
	*(dp - 3) = *(dp - 2); \
	*(dp - 2) = *(dp - 1); \
	*(dp - 1) = t0tmp; \
} while (0)
#define T0_NROT()       do { \
	uint32_t t0tmp = *(dp - 1); \
	*(dp - 1) = *(dp - 2); \
	*(dp - 2) = *(dp - 3); \
	*(dp - 3) = t0tmp; \
} while (0)
#define T0_PICK(x)      do { \
	uint32_t t0depth = (x); \
	T0_PUSH(T0_PEEK(t0depth)); \
} while (0)
#define T0_CO()         do { \
	goto t0_exit; \
} while (0)
#define T0_RET()        goto t0_next

	dp = ((t0_context *)t0ctx)->dp;
	rp = ((t0_context *)t0ctx)->rp;
	ip = ((t0_context *)t0ctx)->ip;
	goto t0_next;
	for (;;) {
		uint32_t t0x;

	t0_next:
		t0x = T0_NEXT(&ip);
		if (t0x < T0_INTERPRETED) {
			switch (t0x) {
				int32_t t0off;

			case 0: /* ret */
				t0x = T0_RPOP();
				rp -= (t0x >> 16);
				t0x &= 0xFFFF;
				if (t0x == 0) {
					ip = NULL;
					goto t0_exit;
				}
				ip = &t0_codeblock[t0x];
				break;
			case 1: /* literal constant */
				T0_PUSHi(t0_parse7E_signed(&ip));
				break;
			case 2: /* read local */
				T0_PUSH(T0_LOCAL(t0_parse7E_unsigned(&ip)));
				break;
			case 3: /* write local */
				T0_LOCAL(t0_parse7E_unsigned(&ip)) = T0_POP();
				break;
			case 4: /* jump */
				t0off = t0_parse7E_signed(&ip);
				ip += t0off;
				break;
			case 5: /* jump if */
				t0off = t0_parse7E_signed(&ip);
				if (T0_POP()) {
					ip += t0off;
				}
				break;
			case 6: /* jump if not */
				t0off = t0_parse7E_signed(&ip);
				if (!T0_POP()) {
					ip += t0off;
				}
				break;
			case 7: {
				/* * */

	uint32_t b = T0_POP();
	uint32_t a = T0_POP();
	T0_PUSH(a * b);

				}
				break;
			case 8: {
				/* + */

	uint32_t b = T0_POP();
	uint32_t a = T0_POP();
	T0_PUSH(a + b);

				}
				break;
			case 9: {
				/* - */

	uint32_t b = T0_POP();
	uint32_t a = T0_POP();
	T0_PUSH(a - b);

				}
				break;
			case 10: {
				/* < */

	int32_t b = T0_POPi();
	int32_t a = T0_POPi();
	T0_PUSH(-(uint32_t)(a < b));

				}
				break;
			case 11: {
				/* << */

	int c = (int)T0_POPi();
	uint32_t x = T0_POP();
	T0_PUSH(x << c);

				}
				break;
			case 12: {
				/* <= */

	int32_t b = T0_POPi();
	int32_t a = T0_POPi();
	T0_PUSH(-(uint32_t)(a <= b));

				}
				break;
			case 13: {
				/* <> */

	uint32_t b = T0_POP();
	uint32_t a = T0_POP();
	T0_PUSH(-(uint32_t)(a != b));

				}
				break;
			case 14: {
				/* = */

	uint32_t b = T0_POP();
	uint32_t a = T0_POP();
	T0_PUSH(-(uint32_t)(a == b));

				}
				break;
			case 15: {
				/* > */

	int32_t b = T0_POPi();
	int32_t a = T0_POPi();
	T0_PUSH(-(uint32_t)(a > b));

				}
				break;
			case 16: {
				/* >= */

	int32_t b = T0_POPi();
	int32_t a = T0_POPi();
	T0_PUSH(-(uint32_t)(a >= b));

				}
				break;
			case 17: {
				/* >> */

	int c = (int)T0_POPi();
	int32_t x = T0_POPi();
	T0_PUSHi(x >> c);

				}
				break;
			case 18: {
				/* anchor-dn-append-name */

	size_t len;

	len = T0_POP();
	if (CTX->client_auth_vtable != NULL) {
		(*CTX->client_auth_vtable)->append_name(
			CTX->client_auth_vtable, ENG->pad, len);
	}

				}
				break;
			case 19: {
				/* anchor-dn-end-name */

	if (CTX->client_auth_vtable != NULL) {
		(*CTX->client_auth_vtable)->end_name(
			CTX->client_auth_vtable);
	}

				}
				break;
			case 20: {
				/* anchor-dn-end-name-list */

	if (CTX->client_auth_vtable != NULL) {
		(*CTX->client_auth_vtable)->end_name_list(
			CTX->client_auth_vtable);
	}

				}
				break;
			case 21: {
				/* anchor-dn-start-name */

	size_t len;

	len = T0_POP();
	if (CTX->client_auth_vtable != NULL) {
		(*CTX->client_auth_vtable)->start_name(
			CTX->client_auth_vtable, len);
	}

				}
				break;
			case 22: {
				/* anchor-dn-start-name-list */

	if (CTX->client_auth_vtable != NULL) {
		(*CTX->client_auth_vtable)->start_name_list(
			CTX->client_auth_vtable);
	}

				}
				break;
			case 23: {
				/* and */

	uint32_t b = T0_POP();
	uint32_t a = T0_POP();
	T0_PUSH(a & b);

				}
				break;
			case 24: {
				/* begin-cert */

	if (ENG->chain_len == 0) {
		T0_PUSHi(-1);
	} else {
		ENG->cert_cur = ENG->chain->data;
		ENG->cert_len = ENG->chain->data_len;
		ENG->chain ++;
		ENG->chain_len --;
		T0_PUSH(ENG->cert_len);
	}

				}
				break;
			case 25: {
				/* bzero */

	size_t len = (size_t)T0_POP();
	void *addr = (unsigned char *)ENG + (size_t)T0_POP();
	memset(addr, 0, len);

				}
				break;
			case 26: {
				/* can-output? */

	T0_PUSHi(-(ENG->hlen_out > 0));

				}
				break;
			case 27: {
				/* co */
 T0_CO(); 
				}
				break;
			case 28: {
				/* compute-Finished-inner */

	int prf_id = T0_POP();
	int from_client = T0_POPi();
	unsigned char seed[48];
	size_t seed_len;

	br_tls_prf_impl prf = br_ssl_engine_get_PRF(ENG, prf_id);
	if (ENG->session.version >= BR_TLS12) {
		seed_len = br_multihash_out(&ENG->mhash, prf_id, seed);
	} else {
		br_multihash_out(&ENG->mhash, br_md5_ID, seed);
		br_multihash_out(&ENG->mhash, br_sha1_ID, seed + 16);
		seed_len = 36;
	}
	prf(ENG->pad, 12, ENG->session.master_secret,
		sizeof ENG->session.master_secret,
		from_client ? "client finished" : "server finished",
		seed, seed_len);

				}
				break;
			case 29: {
				/* copy-cert-chunk */

	size_t clen;

	clen = ENG->cert_len;
	if (clen > sizeof ENG->pad) {
		clen = sizeof ENG->pad;
	}
	memcpy(ENG->pad, ENG->cert_cur, clen);
	ENG->cert_cur += clen;
	ENG->cert_len -= clen;
	T0_PUSH(clen);

				}
				break;
			case 30: {
				/* copy-protocol-name */

	size_t idx = T0_POP();
	size_t len = strlen(ENG->protocol_names[idx]);
	memcpy(ENG->pad, ENG->protocol_names[idx], len);
	T0_PUSH(len);

				}
				break;
			case 31: {
				/* data-get8 */

	size_t addr = T0_POP();
	T0_PUSH(t0_datablock[addr]);

				}
				break;
			case 32: {
				/* discard-input */

	ENG->hlen_in = 0;

				}
				break;
			case 33: {
				/* do-client-sign */

	size_t sig_len;

	sig_len = make_client_sign(CTX);
	if (sig_len == 0) {
		br_ssl_engine_fail(ENG, BR_ERR_INVALID_ALGORITHM);
		T0_CO();
	}
	T0_PUSH(sig_len);

				}
				break;
			case 34: {
				/* do-ecdh */

	unsigned prf_id = T0_POP();
	unsigned ecdhe = T0_POP();
	int x;

	x = make_pms_ecdh(CTX, ecdhe, prf_id);
	if (x < 0) {
		br_ssl_engine_fail(ENG, -x);
		T0_CO();
	} else {
		T0_PUSH(x);
	}

				}
				break;
			case 35: {
				/* do-rsa-encrypt */

	int x;

	x = make_pms_rsa(CTX, T0_POP());
	if (x < 0) {
		br_ssl_engine_fail(ENG, -x);
		T0_CO();
	} else {
		T0_PUSH(x);
	}

				}
				break;
			case 36: {
				/* do-static-ecdh */

	unsigned prf_id = T0_POP();

	if (make_pms_static_ecdh(CTX, prf_id) < 0) {
		br_ssl_engine_fail(ENG, BR_ERR_INVALID_ALGORITHM);
		T0_CO();
	}

				}
				break;
			case 37: {
				/* drop */
 (void)T0_POP(); 
				}
				break;
			case 38: {
				/* dup */
 T0_PUSH(T0_PEEK(0)); 
				}
				break;
			case 39: {
				/* ext-ALPN-length */

	size_t u, len;

	if (ENG->protocol_names_num == 0) {
		T0_PUSH(0);
		T0_RET();
	}
	len = 6;
	for (u = 0; u < ENG->protocol_names_num; u ++) {
		len += 1 + strlen(ENG->protocol_names[u]);
	}
	T0_PUSH(len);

				}
				break;
			case 40: {
				/* fail */

	br_ssl_engine_fail(ENG, (int)T0_POPi());
	T0_CO();

				}
				break;
			case 41: {
				/* flush-record */

	br_ssl_engine_flush_record(ENG);

				}
				break;
			case 42: {
				/* get-client-chain */

	uint32_t auth_types;

	auth_types = T0_POP();
	if (CTX->client_auth_vtable != NULL) {
		br_ssl_client_certificate ux;

		(*CTX->client_auth_vtable)->choose(CTX->client_auth_vtable,
			CTX, auth_types, &ux);
		CTX->auth_type = (unsigned char)ux.auth_type;
		CTX->hash_id = (unsigned char)ux.hash_id;
		ENG->chain = ux.chain;
		ENG->chain_len = ux.chain_len;
	} else {
		CTX->hash_id = 0;
		ENG->chain_len = 0;
	}

				}
				break;
			case 43: {
				/* get-key-type-usages */

	const br_x509_class *xc;
	const br_x509_pkey *pk;
	unsigned usages;

	xc = *(ENG->x509ctx);
	pk = xc->get_pkey(ENG->x509ctx, &usages);
	if (pk == NULL) {
		T0_PUSH(0);
	} else {
		T0_PUSH(pk->key_type | usages);
	}

				}
				break;
			case 44: {
				/* get16 */

	size_t addr = (size_t)T0_POP();
	T0_PUSH(*(uint16_t *)((unsigned char *)ENG + addr));

				}
				break;
			case 45: {
				/* get32 */

	size_t addr = (size_t)T0_POP();
	T0_PUSH(*(uint32_t *)((unsigned char *)ENG + addr));

				}
				break;
			case 46: {
				/* get8 */

	size_t addr = (size_t)T0_POP();
	T0_PUSH(*((unsigned char *)ENG + addr));

				}
				break;
			case 47: {
				/* has-input? */

	T0_PUSHi(-(ENG->hlen_in != 0));

				}
				break;
			case 48: {
				/* memcmp */

	size_t len = (size_t)T0_POP();
	void *addr2 = (unsigned char *)ENG + (size_t)T0_POP();
	void *addr1 = (unsigned char *)ENG + (size_t)T0_POP();
	int x = memcmp(addr1, addr2, len);
	T0_PUSH((uint32_t)-(x == 0));

				}
				break;
			case 49: {
				/* memcpy */

	size_t len = (size_t)T0_POP();
	void *src = (unsigned char *)ENG + (size_t)T0_POP();
	void *dst = (unsigned char *)ENG + (size_t)T0_POP();
	memcpy(dst, src, len);

				}
				break;
			case 50: {
				/* mkrand */

	size_t len = (size_t)T0_POP();
	void *addr = (unsigned char *)ENG + (size_t)T0_POP();
	br_hmac_drbg_generate(&ENG->rng, addr, len);

				}
				break;
			case 51: {
				/* more-incoming-bytes? */

	T0_PUSHi(ENG->hlen_in != 0 || !br_ssl_engine_recvrec_finished(ENG));

				}
				break;
			case 52: {
				/* multihash-init */

	br_multihash_init(&ENG->mhash);

				}
				break;
			case 53: {
				/* neg */

	uint32_t a = T0_POP();
	T0_PUSH(-a);

				}
				break;
			case 54: {
				/* not */

	uint32_t a = T0_POP();
	T0_PUSH(~a);

				}
				break;
			case 55: {
				/* or */

	uint32_t b = T0_POP();
	uint32_t a = T0_POP();
	T0_PUSH(a | b);

				}
				break;
			case 56: {
				/* over */
 T0_PUSH(T0_PEEK(1)); 
				}
				break;
			case 57: {
				/* read-chunk-native */

	size_t clen = ENG->hlen_in;
	if (clen > 0) {
		uint32_t addr, len;

		len = T0_POP();
		addr = T0_POP();
		if ((size_t)len < clen) {
			clen = (size_t)len;
		}
		memcpy((unsigned char *)ENG + addr, ENG->hbuf_in, clen);
		if (ENG->record_type_in == BR_SSL_HANDSHAKE) {
			br_multihash_update(&ENG->mhash, ENG->hbuf_in, clen);
		}
		T0_PUSH(addr + (uint32_t)clen);
		T0_PUSH(len - (uint32_t)clen);
		ENG->hbuf_in += clen;
		ENG->hlen_in -= clen;
	}

				}
				break;
			case 58: {
				/* read8-native */

	if (ENG->hlen_in > 0) {
		unsigned char x;

		x = *ENG->hbuf_in ++;
		if (ENG->record_type_in == BR_SSL_HANDSHAKE) {
			br_multihash_update(&ENG->mhash, &x, 1);
		}
		T0_PUSH(x);
		ENG->hlen_in --;
	} else {
		T0_PUSHi(-1);
	}

				}
				break;
			case 59: {
				/* set-server-curve */

	const br_x509_class *xc;
	const br_x509_pkey *pk;

	xc = *(ENG->x509ctx);
	pk = xc->get_pkey(ENG->x509ctx, NULL);
	CTX->server_curve =
		(pk->key_type == BR_KEYTYPE_EC) ? pk->key.ec.curve : 0;

				}
				break;
			case 60: {
				/* set16 */

	size_t addr = (size_t)T0_POP();
	*(uint16_t *)((unsigned char *)ENG + addr) = (uint16_t)T0_POP();

				}
				break;
			case 61: {
				/* set8 */

	size_t addr = (size_t)T0_POP();
	*((unsigned char *)ENG + addr) = (unsigned char)T0_POP();

				}
				break;
			case 62: {
				/* strlen */

	void *str = (unsigned char *)ENG + (size_t)T0_POP();
	T0_PUSH((uint32_t)strlen(str));

				}
				break;
			case 63: {
				/* supported-curves */

	uint32_t x = ENG->iec == NULL ? 0 : ENG->iec->supported_curves;
	T0_PUSH(x);

				}
				break;
			case 64: {
				/* supported-hash-functions */

	int i;
	unsigned x, num;

	x = 0;
	num = 0;
	for (i = br_sha1_ID; i <= br_sha512_ID; i ++) {
		if (br_multihash_getimpl(&ENG->mhash, i)) {
			x |= 1U << i;
			num ++;
		}
	}
	T0_PUSH(x);
	T0_PUSH(num);

				}
				break;
			case 65: {
				/* supports-ecdsa? */

	T0_PUSHi(-(ENG->iecdsa != 0));

				}
				break;
			case 66: {
				/* supports-rsa-sign? */

	T0_PUSHi(-(ENG->irsavrfy != 0));

				}
				break;
			case 67: {
				/* swap */
 T0_SWAP(); 
				}
				break;
			case 68: {
				/* switch-aesgcm-in */

	int is_client, prf_id;
	unsigned cipher_key_len;

	cipher_key_len = T0_POP();
	prf_id = T0_POP();
	is_client = T0_POP();
	br_ssl_engine_switch_gcm_in(ENG, is_client, prf_id,
		ENG->iaes_ctr, cipher_key_len);

				}
				break;
			case 69: {
				/* switch-aesgcm-out */

	int is_client, prf_id;
	unsigned cipher_key_len;

	cipher_key_len = T0_POP();
	prf_id = T0_POP();
	is_client = T0_POP();
	br_ssl_engine_switch_gcm_out(ENG, is_client, prf_id,
		ENG->iaes_ctr, cipher_key_len);

				}
				break;
			case 70: {
				/* switch-cbc-in */

	int is_client, prf_id, mac_id, aes;
	unsigned cipher_key_len;

	cipher_key_len = T0_POP();
	aes = T0_POP();
	mac_id = T0_POP();
	prf_id = T0_POP();
	is_client = T0_POP();
	br_ssl_engine_switch_cbc_in(ENG, is_client, prf_id, mac_id,
		aes ? ENG->iaes_cbcdec : ENG->ides_cbcdec, cipher_key_len);

				}
				break;
			case 71: {
				/* switch-cbc-out */

	int is_client, prf_id, mac_id, aes;
	unsigned cipher_key_len;

	cipher_key_len = T0_POP();
	aes = T0_POP();
	mac_id = T0_POP();
	prf_id = T0_POP();
	is_client = T0_POP();
	br_ssl_engine_switch_cbc_out(ENG, is_client, prf_id, mac_id,
		aes ? ENG->iaes_cbcenc : ENG->ides_cbcenc, cipher_key_len);

				}
				break;
			case 72: {
				/* switch-chapol-in */

	int is_client, prf_id;

	prf_id = T0_POP();
	is_client = T0_POP();
	br_ssl_engine_switch_chapol_in(ENG, is_client, prf_id);

				}
				break;
			case 73: {
				/* switch-chapol-out */

	int is_client, prf_id;

	prf_id = T0_POP();
	is_client = T0_POP();
	br_ssl_engine_switch_chapol_out(ENG, is_client, prf_id);

				}
				break;
			case 74: {
				/* test-protocol-name */

	size_t len = T0_POP();
	size_t u;

	for (u = 0; u < ENG->protocol_names_num; u ++) {
		const char *name;

		name = ENG->protocol_names[u];
		if (len == strlen(name) && memcmp(ENG->pad, name, len) == 0) {
			T0_PUSH(u);
			T0_RET();
		}
	}
	T0_PUSHi(-1);

				}
				break;
			case 75: {
				/* total-chain-length */

	size_t u;
	uint32_t total;

	total = 0;
	for (u = 0; u < ENG->chain_len; u ++) {
		total += 3 + (uint32_t)ENG->chain[u].data_len;
	}
	T0_PUSH(total);

				}
				break;
			case 76: {
				/* u>> */

	int c = (int)T0_POPi();
	uint32_t x = T0_POP();
	T0_PUSH(x >> c);

				}
				break;
			case 77: {
				/* verify-SKE-sig */

	size_t sig_len = T0_POP();
	int use_rsa = T0_POPi();
	int hash = T0_POPi();

	T0_PUSH(verify_SKE_sig(CTX, hash, use_rsa, sig_len));

				}
				break;
			case 78: {
				/* write-blob-chunk */

	size_t clen = ENG->hlen_out;
	if (clen > 0) {
		uint32_t addr, len;

		len = T0_POP();
		addr = T0_POP();
		if ((size_t)len < clen) {
			clen = (size_t)len;
		}
		memcpy(ENG->hbuf_out, (unsigned char *)ENG + addr, clen);
		if (ENG->record_type_out == BR_SSL_HANDSHAKE) {
			br_multihash_update(&ENG->mhash, ENG->hbuf_out, clen);
		}
		T0_PUSH(addr + (uint32_t)clen);
		T0_PUSH(len - (uint32_t)clen);
		ENG->hbuf_out += clen;
		ENG->hlen_out -= clen;
	}

				}
				break;
			case 79: {
				/* write8-native */

	unsigned char x;

	x = (unsigned char)T0_POP();
	if (ENG->hlen_out > 0) {
		if (ENG->record_type_out == BR_SSL_HANDSHAKE) {
			br_multihash_update(&ENG->mhash, &x, 1);
		}
		*ENG->hbuf_out ++ = x;
		ENG->hlen_out --;
		T0_PUSHi(-1);
	} else {
		T0_PUSHi(0);
	}

				}
				break;
			case 80: {
				/* x509-append */

	const br_x509_class *xc;
	size_t len;

	xc = *(ENG->x509ctx);
	len = T0_POP();
	xc->append(ENG->x509ctx, ENG->pad, len);

				}
				break;
			case 81: {
				/* x509-end-cert */

	const br_x509_class *xc;

	xc = *(ENG->x509ctx);
	xc->end_cert(ENG->x509ctx);

				}
				break;
			case 82: {
				/* x509-end-chain */

	const br_x509_class *xc;

	xc = *(ENG->x509ctx);
	T0_PUSH(xc->end_chain(ENG->x509ctx));

				}
				break;
			case 83: {
				/* x509-start-cert */

	const br_x509_class *xc;

	xc = *(ENG->x509ctx);
	xc->start_cert(ENG->x509ctx, T0_POP());

				}
				break;
			case 84: {
				/* x509-start-chain */

	const br_x509_class *xc;
	uint32_t bc;

	bc = T0_POP();
	xc = *(ENG->x509ctx);
	xc->start_chain(ENG->x509ctx, bc ? ENG->server_name : NULL);

				}
				break;
			}

		} else {
			T0_ENTER(ip, rp, t0x);
		}
	}
t0_exit:
	((t0_context *)t0ctx)->dp = dp;
	((t0_context *)t0ctx)->rp = rp;
	((t0_context *)t0ctx)->ip = ip;
}


/* ===== src/ssl/ssl_hashes.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

/* see inner.h */
int
br_ssl_choose_hash(unsigned bf)
{
	static const unsigned char pref[] = {
		br_sha256_ID, br_sha384_ID, br_sha512_ID,
		br_sha224_ID, br_sha1_ID
	};
	size_t u;

	for (u = 0; u < sizeof pref; u ++) {
		int x;

		x = pref[u];
		if ((bf >> x) & 1) {
			return x;
		}
	}
	return 0;
}


/* ===== src/ssl/ssl_io.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

/* see bearssl_ssl.h */
void
br_sslio_init(br_sslio_context *ctx,
	br_ssl_engine_context *engine,
	int (*low_read)(void *read_context,
		unsigned char *data, size_t len),
	void *read_context,
	int (*low_write)(void *write_context,
		const unsigned char *data, size_t len),
	void *write_context)
{
	ctx->engine = engine;
	ctx->low_read = low_read;
	ctx->read_context = read_context;
	ctx->low_write = low_write;
	ctx->write_context = write_context;
}

/*
 * Run the engine, until the specified target state is achieved, or
 * an error occurs. The target state is SENDAPP, RECVAPP, or the
 * combination of both (the combination matches either). When a match is
 * achieved, this function returns 0. On error, it returns -1.
 */
static int
run_until(br_sslio_context *ctx, unsigned target)
{
	for (;;) {
		unsigned state;

		state = br_ssl_engine_current_state(ctx->engine);
		if (state & BR_SSL_CLOSED) {
			return -1;
		}

		/*
		 * If there is some record data to send, do it. This takes
		 * precedence over everything else.
		 */
		if (state & BR_SSL_SENDREC) {
			unsigned char *buf;
			size_t len;
			int wlen;

			buf = br_ssl_engine_sendrec_buf(ctx->engine, &len);
			wlen = ctx->low_write(ctx->write_context, buf, len);
			if (wlen < 0) {
				/*
				 * If we received a close_notify and we
				 * still send something, then we have our
				 * own response close_notify to send, and
				 * the peer is allowed by RFC 5246 not to
				 * wait for it.
				 */
				if (!ctx->engine->shutdown_recv) {
					br_ssl_engine_fail(
						ctx->engine, BR_ERR_IO);
				}
				return -1;
			}
			if (wlen > 0) {
				br_ssl_engine_sendrec_ack(ctx->engine, wlen);
			}
			continue;
		}

		/*
		 * If we reached our target, then we are finished.
		 */
		if (state & target) {
			return 0;
		}

		/*
		 * If some application data must be read, and we did not
		 * exit, then this means that we are trying to write data,
		 * and that's not possible until the application data is
		 * read. This may happen if using a shared in/out buffer,
		 * and the underlying protocol is not strictly half-duplex.
		 * This is unrecoverable here, so we report an error.
		 */
		if (state & BR_SSL_RECVAPP) {
			return -1;
		}

		/*
		 * If we reached that point, then either we are trying
		 * to read data and there is some, or the engine is stuck
		 * until a new record is obtained.
		 */
		if (state & BR_SSL_RECVREC) {
			unsigned char *buf;
			size_t len;
			int rlen;

			buf = br_ssl_engine_recvrec_buf(ctx->engine, &len);
			rlen = ctx->low_read(ctx->read_context, buf, len);
			if (rlen < 0) {
				br_ssl_engine_fail(ctx->engine, BR_ERR_IO);
				return -1;
			}
			if (rlen > 0) {
				br_ssl_engine_recvrec_ack(ctx->engine, rlen);
			}
			continue;
		}

		/*
		 * We can reach that point if the target RECVAPP, and
		 * the state contains SENDAPP only. This may happen with
		 * a shared in/out buffer. In that case, we must flush
		 * the buffered data to "make room" for a new incoming
		 * record.
		 */
		br_ssl_engine_flush(ctx->engine, 0);
	}
}

/* see bearssl_ssl.h */
int
br_sslio_read(br_sslio_context *ctx, void *dst, size_t len)
{
	unsigned char *buf;
	size_t alen;

	if (len == 0) {
		return 0;
	}
	if (run_until(ctx, BR_SSL_RECVAPP) < 0) {
		return -1;
	}
	buf = br_ssl_engine_recvapp_buf(ctx->engine, &alen);
	if (alen > len) {
		alen = len;
	}
	memcpy(dst, buf, alen);
	br_ssl_engine_recvapp_ack(ctx->engine, alen);
	return (int)alen;
}

/* see bearssl_ssl.h */
int
br_sslio_read_all(br_sslio_context *ctx, void *dst, size_t len)
{
	unsigned char *buf;

	buf = dst;
	while (len > 0) {
		int rlen;

		rlen = br_sslio_read(ctx, buf, len);
		if (rlen < 0) {
			return -1;
		}
		buf += rlen;
		len -= (size_t)rlen;
	}
	return 0;
}

/* see bearssl_ssl.h */
int
br_sslio_write(br_sslio_context *ctx, const void *src, size_t len)
{
	unsigned char *buf;
	size_t alen;

	if (len == 0) {
		return 0;
	}
	if (run_until(ctx, BR_SSL_SENDAPP) < 0) {
		return -1;
	}
	buf = br_ssl_engine_sendapp_buf(ctx->engine, &alen);
	if (alen > len) {
		alen = len;
	}
	memcpy(buf, src, alen);
	br_ssl_engine_sendapp_ack(ctx->engine, alen);
	return (int)alen;
}

/* see bearssl_ssl.h */
int
br_sslio_write_all(br_sslio_context *ctx, const void *src, size_t len)
{
	const unsigned char *buf;

	buf = src;
	while (len > 0) {
		int wlen;

		wlen = br_sslio_write(ctx, buf, len);
		if (wlen < 0) {
			return -1;
		}
		buf += wlen;
		len -= (size_t)wlen;
	}
	return 0;
}

/* see bearssl_ssl.h */
int
br_sslio_flush(br_sslio_context *ctx)
{
	/*
	 * We trigger a flush. We know the data is gone when there is
	 * no longer any record data to send, and we can either read
	 * or write application data. The call to run_until() does the
	 * job because it ensures that any assembled record data is
	 * first sent down the wire before considering anything else.
	 */
	br_ssl_engine_flush(ctx->engine, 0);
	return run_until(ctx, BR_SSL_SENDAPP | BR_SSL_RECVAPP);
}

/* see bearssl_ssl.h */
int
br_sslio_close(br_sslio_context *ctx)
{
	br_ssl_engine_close(ctx->engine);
	while (br_ssl_engine_current_state(ctx->engine) != BR_SSL_CLOSED) {
		/*
		 * Discard any incoming application data.
		 */
		size_t len;

		run_until(ctx, BR_SSL_RECVAPP);
		if (br_ssl_engine_recvapp_buf(ctx->engine, &len) != NULL) {
			br_ssl_engine_recvapp_ack(ctx->engine, len);
		}
	}
	return br_ssl_engine_last_error(ctx->engine) == BR_ERR_OK;
}


/* ===== src/ssl/ssl_rec_gcm.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

/*
 * GCM initialisation. This does everything except setting the vtable,
 * which depends on whether this is a context for encrypting or for
 * decrypting.
 */
static void
gen_gcm_init(br_sslrec_gcm_context *cc,
	const br_block_ctr_class *bc_impl,
	const void *key, size_t key_len,
	br_ghash gh_impl,
	const void *iv)
{
	unsigned char tmp[12];

	cc->seq = 0;
	bc_impl->init(&cc->bc.vtable, key, key_len);
	cc->gh = gh_impl;
	memcpy(cc->iv, iv, sizeof cc->iv);
	memset(cc->h, 0, sizeof cc->h);
	memset(tmp, 0, sizeof tmp);
	bc_impl->run(&cc->bc.vtable, tmp, 0, cc->h, sizeof cc->h);
}

static void
in_gcm_init(br_sslrec_gcm_context *cc,
	const br_block_ctr_class *bc_impl,
	const void *key, size_t key_len,
	br_ghash gh_impl,
	const void *iv)
{
	cc->vtable.in = &br_sslrec_in_gcm_vtable;
	gen_gcm_init(cc, bc_impl, key, key_len, gh_impl, iv);
}

static int
gcm_check_length(const br_sslrec_gcm_context *cc, size_t rlen)
{
	/*
	 * GCM adds a fixed overhead:
	 *   8 bytes for the nonce_explicit (before the ciphertext)
	 *  16 bytes for the authentication tag (after the ciphertext)
	 */
	(void)cc;
	return rlen >= 24 && rlen <= (16384 + 24);
}

/*
 * Compute the authentication tag. The value written in 'tag' must still
 * be CTR-encrypted.
 */
static void
do_tag(br_sslrec_gcm_context *cc,
	int record_type, unsigned version,
	void *data, size_t len, void *tag)
{
	unsigned char header[13];
	unsigned char footer[16];

	/*
	 * Compute authentication tag. Three elements must be injected in
	 * sequence, each possibly 0-padded to reach a length multiple
	 * of the block size: the 13-byte header (sequence number, record
	 * type, protocol version, record length), the cipher text, and
	 * the word containing the encodings of the bit lengths of the two
	 * other elements.
	 */
	br_enc64be(header, cc->seq ++);
	header[8] = (unsigned char)record_type;
	br_enc16be(header + 9, version);
	br_enc16be(header + 11, len);
	br_enc64be(footer, (uint64_t)(sizeof header) << 3);
	br_enc64be(footer + 8, (uint64_t)len << 3);
	memset(tag, 0, 16);
	cc->gh(tag, cc->h, header, sizeof header);
	cc->gh(tag, cc->h, data, len);
	cc->gh(tag, cc->h, footer, sizeof footer);
}

/*
 * Do CTR encryption. This also does CTR encryption of a single block at
 * address 'xortag' with the counter value appropriate for the final
 * processing of the authentication tag.
 */
static void
do_ctr(br_sslrec_gcm_context *cc, const void *nonce, void *data, size_t len,
	void *xortag)
{
	unsigned char iv[12];

	memcpy(iv, cc->iv, 4);
	memcpy(iv + 4, nonce, 8);
	cc->bc.vtable->run(&cc->bc.vtable, iv, 2, data, len);
	cc->bc.vtable->run(&cc->bc.vtable, iv, 1, xortag, 16);
}

static unsigned char *
gcm_decrypt(br_sslrec_gcm_context *cc,
	int record_type, unsigned version, void *data, size_t *data_len)
{
	unsigned char *buf;
	size_t len, u;
	uint32_t bad;
	unsigned char tag[16];

	buf = (unsigned char *)data + 8;
	len = *data_len - 24;
	do_tag(cc, record_type, version, buf, len, tag);
	do_ctr(cc, data, buf, len, tag);

	/*
	 * Compare the computed tag with the value from the record. It
	 * is possibly useless to do a constant-time comparison here,
	 * but it does not hurt.
	 */
	bad = 0;
	for (u = 0; u < 16; u ++) {
		bad |= tag[u] ^ buf[len + u];
	}
	if (bad) {
		return NULL;
	}
	*data_len = len;
	return buf;
}

/* see bearssl_ssl.h */
const br_sslrec_in_gcm_class br_sslrec_in_gcm_vtable = {
	{
		sizeof(br_sslrec_gcm_context),
		(int (*)(const br_sslrec_in_class *const *, size_t))
			&gcm_check_length,
		(unsigned char *(*)(const br_sslrec_in_class **,
			int, unsigned, void *, size_t *))
			&gcm_decrypt
	},
	(void (*)(const br_sslrec_in_gcm_class **,
		const br_block_ctr_class *, const void *, size_t,
		br_ghash, const void *))
		&in_gcm_init
};

static void
out_gcm_init(br_sslrec_gcm_context *cc,
	const br_block_ctr_class *bc_impl,
	const void *key, size_t key_len,
	br_ghash gh_impl,
	const void *iv)
{
	cc->vtable.out = &br_sslrec_out_gcm_vtable;
	gen_gcm_init(cc, bc_impl, key, key_len, gh_impl, iv);
}

static void
gcm_max_plaintext(const br_sslrec_gcm_context *cc,
	size_t *start, size_t *end)
{
	size_t len;

	(void)cc;
	*start += 8;
	len = *end - *start - 16;
	if (len > 16384) {
		len = 16384;
	}
	*end = *start + len;
}

static unsigned char *
gcm_encrypt(br_sslrec_gcm_context *cc,
	int record_type, unsigned version, void *data, size_t *data_len)
{
	unsigned char *buf;
	size_t u, len;
	unsigned char tmp[16];

	buf = (unsigned char *)data;
	len = *data_len;
	memset(tmp, 0, sizeof tmp);
	br_enc64be(buf - 8, cc->seq);
	do_ctr(cc, buf - 8, buf, len, tmp);
	do_tag(cc, record_type, version, buf, len, buf + len);
	for (u = 0; u < 16; u ++) {
		buf[len + u] ^= tmp[u];
	}
	len += 24;
	buf -= 13;
	buf[0] = (unsigned char)record_type;
	br_enc16be(buf + 1, version);
	br_enc16be(buf + 3, len);
	*data_len = len + 5;
	return buf;
}

/* see bearssl_ssl.h */
const br_sslrec_out_gcm_class br_sslrec_out_gcm_vtable = {
	{
		sizeof(br_sslrec_gcm_context),
		(void (*)(const br_sslrec_out_class *const *,
			size_t *, size_t *))
			&gcm_max_plaintext,
		(unsigned char *(*)(const br_sslrec_out_class **,
			int, unsigned, void *, size_t *))
			&gcm_encrypt
	},
	(void (*)(const br_sslrec_out_gcm_class **,
		const br_block_ctr_class *, const void *, size_t,
		br_ghash, const void *))
		&out_gcm_init
};


/* ===== src/ssl/ssl_rec_cbc.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

static void
in_cbc_init(br_sslrec_in_cbc_context *cc,
	const br_block_cbcdec_class *bc_impl,
	const void *bc_key, size_t bc_key_len,
	const br_hash_class *dig_impl,
	const void *mac_key, size_t mac_key_len, size_t mac_out_len,
	const void *iv)
{
	cc->vtable = &br_sslrec_in_cbc_vtable;
	cc->seq = 0;
	bc_impl->init(&cc->bc.vtable, bc_key, bc_key_len);
	br_hmac_key_init(&cc->mac, dig_impl, mac_key, mac_key_len);
	cc->mac_len = mac_out_len;
	if (iv == NULL) {
		memset(cc->iv, 0, sizeof cc->iv);
		cc->explicit_IV = 1;
	} else {
		memcpy(cc->iv, iv, bc_impl->block_size);
		cc->explicit_IV = 0;
	}
}

static int
cbc_check_length(const br_sslrec_in_cbc_context *cc, size_t rlen)
{
	/*
	 * Plaintext size: at most 16384 bytes
	 * Padding: at most 256 bytes
	 * MAC: mac_len extra bytes
	 * TLS 1.1+: each record has an explicit IV
	 *
	 * Minimum length includes at least one byte of padding, and the
	 * MAC.
	 *
	 * Total length must be a multiple of the block size.
	 */
	size_t blen;
	size_t min_len, max_len;

	blen = cc->bc.vtable->block_size;
	min_len = (blen + cc->mac_len) & ~(blen - 1);
	max_len = (16384 + 256 + cc->mac_len) & ~(blen - 1);
	if (cc->explicit_IV) {
		min_len += blen;
		max_len += blen;
	}
	return min_len <= rlen && rlen <= max_len;
}

/*
 * Rotate array buf[] of length 'len' to the left (towards low indices)
 * by 'num' bytes if ctl is 1; otherwise, leave it unchanged. This is
 * constant-time. 'num' MUST be lower than 'len'. 'len' MUST be lower
 * than or equal to 64.
 */
static void
cond_rotate(uint32_t ctl, unsigned char *buf, size_t len, size_t num)
{
	unsigned char tmp[64];
	size_t u, v;

	for (u = 0, v = num; u < len; u ++) {
		tmp[u] = MUX(ctl, buf[v], buf[u]);
		if (++ v == len) {
			v = 0;
		}
	}
	memcpy(buf, tmp, len);
}

static unsigned char *
cbc_decrypt(br_sslrec_in_cbc_context *cc,
	int record_type, unsigned version, void *data, size_t *data_len)
{
	/*
	 * We represent all lengths on 32-bit integers, because:
	 * -- SSL record lengths always fit in 32 bits;
	 * -- our constant-time primitives operate on 32-bit integers.
	 */
	unsigned char *buf;
	uint32_t u, v, len, blen, min_len, max_len;
	uint32_t good, pad_len, rot_count, len_withmac, len_nomac;
	unsigned char tmp1[64], tmp2[64];
	int i;
	br_hmac_context hc;

	buf = data;
	len = *data_len;
	blen = cc->bc.vtable->block_size;

	/*
	 * Decrypt data, and skip the explicit IV (if applicable). Note
	 * that the total length is supposed to have been verified by
	 * the caller. If there is an explicit IV, then we actually
	 * "decrypt" it using the implicit IV (from previous record),
	 * which is useless but harmless.
	 */
	cc->bc.vtable->run(&cc->bc.vtable, cc->iv, data, len);
	if (cc->explicit_IV) {
		buf += blen;
		len -= blen;
	}

	/*
	 * Compute minimum and maximum length of plaintext + MAC. These
	 * lengths can be inferred from the outside: they are not secret.
	 */
	min_len = (cc->mac_len + 256 < len) ? len - 256 : cc->mac_len;
	max_len = len - 1;

	/*
	 * Use the last decrypted byte to compute the actual payload
	 * length. Take care not to underflow (we use unsigned types).
	 */
	pad_len = buf[max_len];
	good = LE(pad_len, (uint32_t)(max_len - min_len));
	len = MUX(good, (uint32_t)(max_len - pad_len), min_len);

	/*
	 * Check padding contents: all padding bytes must be equal to
	 * the value of pad_len.
	 */
	for (u = min_len; u < max_len; u ++) {
		good &= LT(u, len) | EQ(buf[u], pad_len);
	}

	/*
	 * Extract the MAC value. This is done in one pass, but results
	 * in a "rotated" MAC value depending on where it actually
	 * occurs. The 'rot_count' value is set to the offset of the
	 * first MAC byte within tmp1[].
	 *
	 * min_len and max_len are also adjusted to the minimum and
	 * maximum lengths of the plaintext alone (without the MAC).
	 */
	len_withmac = (uint32_t)len;
	len_nomac = len_withmac - cc->mac_len;
	min_len -= cc->mac_len;
	rot_count = 0;
	memset(tmp1, 0, cc->mac_len);
	v = 0;
	for (u = min_len; u < max_len; u ++) {
		tmp1[v] |= MUX(GE(u, len_nomac) & LT(u, len_withmac),
			buf[u], 0x00);
		rot_count = MUX(EQ(u, len_nomac), v, rot_count);
		if (++ v == cc->mac_len) {
			v = 0;
		}
	}
	max_len -= cc->mac_len;

	/*
	 * Rotate back the MAC value. The loop below does the constant-time
	 * rotation in time n*log n for a MAC output of length n. We assume
	 * that the MAC output length is no more than 64 bytes, so the
	 * rotation count fits on 6 bits.
	 */
	for (i = 5; i >= 0; i --) {
		uint32_t rc;

		rc = (uint32_t)1 << i;
		cond_rotate(rot_count >> i, tmp1, cc->mac_len, rc);
		rot_count &= ~rc;
	}

	/*
	 * Recompute the HMAC value. The input is the concatenation of
	 * the sequence number (8 bytes), the record header (5 bytes),
	 * and the payload.
	 *
	 * At that point, min_len is the minimum plaintext length, but
	 * max_len still includes the MAC length.
	 */
	br_enc64be(tmp2, cc->seq ++);
	tmp2[8] = (unsigned char)record_type;
	br_enc16be(tmp2 + 9, version);
	br_enc16be(tmp2 + 11, len_nomac);
	br_hmac_init(&hc, &cc->mac, cc->mac_len);
	br_hmac_update(&hc, tmp2, 13);
	br_hmac_outCT(&hc, buf, len_nomac, min_len, max_len, tmp2);

	/*
	 * Compare the extracted and recomputed MAC values.
	 */
	for (u = 0; u < cc->mac_len; u ++) {
		good &= EQ0(tmp1[u] ^ tmp2[u]);
	}

	/*
	 * Check that the plaintext length is valid. The previous
	 * check was on the encrypted length, but the padding may have
	 * turned shorter than expected.
	 *
	 * Once this final test is done, the critical "constant-time"
	 * section ends and we can make conditional jumps again.
	 */
	good &= LE(len_nomac, 16384);

	if (!good) {
		return 0;
	}
	*data_len = len_nomac;
	return buf;
}

/* see bearssl_ssl.h */
const br_sslrec_in_cbc_class br_sslrec_in_cbc_vtable = {
	{
		sizeof(br_sslrec_in_cbc_context),
		(int (*)(const br_sslrec_in_class *const *, size_t))
			&cbc_check_length,
		(unsigned char *(*)(const br_sslrec_in_class **,
			int, unsigned, void *, size_t *))
			&cbc_decrypt
	},
	(void (*)(const br_sslrec_in_cbc_class **,
		const br_block_cbcdec_class *, const void *, size_t,
		const br_hash_class *, const void *, size_t, size_t,
		const void *))
		&in_cbc_init
};

/*
 * For CBC output:
 *
 * -- With TLS 1.1+, there is an explicit IV. Generation method uses
 * HMAC, computed over the current sequence number, and the current MAC
 * key. The resulting value is truncated to the size of a block, and
 * added at the head of the plaintext; it will get encrypted along with
 * the data. This custom generation mechanism is "safe" under the
 * assumption that HMAC behaves like a random oracle; since the MAC for
 * a record is computed over the concatenation of the sequence number,
 * the record header and the plaintext, the HMAC-for-IV will not collide
 * with the normal HMAC.
 *
 * -- With TLS 1.0, for application data, we want to enforce a 1/n-1
 * split, as a countermeasure against chosen-plaintext attacks. We thus
 * need to leave some room in the buffer for that extra record.
 */

static void
out_cbc_init(br_sslrec_out_cbc_context *cc,
	const br_block_cbcenc_class *bc_impl,
	const void *bc_key, size_t bc_key_len,
	const br_hash_class *dig_impl,
	const void *mac_key, size_t mac_key_len, size_t mac_out_len,
	const void *iv)
{
	cc->vtable = &br_sslrec_out_cbc_vtable;
	cc->seq = 0;
	bc_impl->init(&cc->bc.vtable, bc_key, bc_key_len);
	br_hmac_key_init(&cc->mac, dig_impl, mac_key, mac_key_len);
	cc->mac_len = mac_out_len;
	if (iv == NULL) {
		memset(cc->iv, 0, sizeof cc->iv);
		cc->explicit_IV = 1;
	} else {
		memcpy(cc->iv, iv, bc_impl->block_size);
		cc->explicit_IV = 0;
	}
}

static void
cbc_max_plaintext(const br_sslrec_out_cbc_context *cc,
	size_t *start, size_t *end)
{
	size_t blen, len;

	blen = cc->bc.vtable->block_size;
	if (cc->explicit_IV) {
		*start += blen;
	} else {
		*start += 4 + ((cc->mac_len + blen + 1) & ~(blen - 1));
	}
	len = (*end - *start) & ~(blen - 1);
	len -= 1 + cc->mac_len;
	if (len > 16384) {
		len = 16384;
	}
	*end = *start + len;
}

static unsigned char *
cbc_encrypt(br_sslrec_out_cbc_context *cc,
	int record_type, unsigned version, void *data, size_t *data_len)
{
	unsigned char *buf, *rbuf;
	size_t len, blen, plen;
	unsigned char tmp[13];
	br_hmac_context hc;

	buf = data;
	len = *data_len;
	blen = cc->bc.vtable->block_size;

	/*
	 * If using TLS 1.0, with more than one byte of plaintext, and
	 * the record is application data, then we need to compute
	 * a "split". We do not perform the split on other record types
	 * because it turned out that some existing, deployed
	 * implementations of SSL/TLS do not tolerate the splitting of
	 * some message types (in particular the Finished message).
	 *
	 * If using TLS 1.1+, then there is an explicit IV. We produce
	 * that IV by adding an extra initial plaintext block, whose
	 * value is computed with HMAC over the record sequence number.
	 */
	if (cc->explicit_IV) {
		/*
		 * We use here the fact that all the HMAC variants we
		 * support can produce at least 16 bytes, while all the
		 * block ciphers we support have blocks of no more than
		 * 16 bytes. Thus, we can always truncate the HMAC output
		 * down to the block size.
		 */
		br_enc64be(tmp, cc->seq);
		br_hmac_init(&hc, &cc->mac, blen);
		br_hmac_update(&hc, tmp, 8);
		br_hmac_out(&hc, buf - blen);
		rbuf = buf - blen - 5;
	} else {
		if (len > 1 && record_type == BR_SSL_APPLICATION_DATA) {
			/*
			 * To do the split, we use a recursive invocation;
			 * since we only give one byte to the inner call,
			 * the recursion stops there.
			 *
			 * We need to compute the exact size of the extra
			 * record, so that the two resulting records end up
			 * being sequential in RAM.
			 *
			 * We use here the fact that cbc_max_plaintext()
			 * adjusted the start offset to leave room for the
			 * initial fragment.
			 */
			size_t xlen;

			rbuf = buf - 4
				- ((cc->mac_len + blen + 1) & ~(blen - 1));
			rbuf[0] = buf[0];
			xlen = 1;
			rbuf = cbc_encrypt(cc, record_type,
				version, rbuf, &xlen);
			buf ++;
			len --;
		} else {
			rbuf = buf - 5;
		}
	}

	/*
	 * Compute MAC.
	 */
	br_enc64be(tmp, cc->seq ++);
	tmp[8] = record_type;
	br_enc16be(tmp + 9, version);
	br_enc16be(tmp + 11, len);
	br_hmac_init(&hc, &cc->mac, cc->mac_len);
	br_hmac_update(&hc, tmp, 13);
	br_hmac_update(&hc, buf, len);
	br_hmac_out(&hc, buf + len);
	len += cc->mac_len;

	/*
	 * Add padding.
	 */
	plen = blen - (len & (blen - 1));
	memset(buf + len, (unsigned)plen - 1, plen);
	len += plen;

	/*
	 * If an explicit IV is used, the corresponding extra block was
	 * already put in place earlier; we just have to account for it
	 * here.
	 */
	if (cc->explicit_IV) {
		buf -= blen;
		len += blen;
	}

	/*
	 * Encrypt the whole thing. If there is an explicit IV, we also
	 * encrypt it, which is fine (encryption of a uniformly random
	 * block is still a uniformly random block).
	 */
	cc->bc.vtable->run(&cc->bc.vtable, cc->iv, buf, len);

	/*
	 * Add the header and return.
	 */
	buf[-5] = record_type;
	br_enc16be(buf - 4, version);
	br_enc16be(buf - 2, len);
	*data_len = (size_t)((buf + len) - rbuf);
	return rbuf;
}

/* see bearssl_ssl.h */
const br_sslrec_out_cbc_class br_sslrec_out_cbc_vtable = {
	{
		sizeof(br_sslrec_out_cbc_context),
		(void (*)(const br_sslrec_out_class *const *,
			size_t *, size_t *))
			&cbc_max_plaintext,
		(unsigned char *(*)(const br_sslrec_out_class **,
			int, unsigned, void *, size_t *))
			&cbc_encrypt
	},
	(void (*)(const br_sslrec_out_cbc_class **,
		const br_block_cbcenc_class *, const void *, size_t,
		const br_hash_class *, const void *, size_t, size_t,
		const void *))
		&out_cbc_init
};


/* ===== src/ssl/ssl_rec_chapol.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

static void
gen_chapol_init(br_sslrec_chapol_context *cc,
	br_chacha20_run ichacha, br_poly1305_run ipoly,
	const void *key, const void *iv)
{
	cc->seq = 0;
	cc->ichacha = ichacha;
	cc->ipoly = ipoly;
	memcpy(cc->key, key, sizeof cc->key);
	memcpy(cc->iv, iv, sizeof cc->iv);
}

static void
gen_chapol_process(br_sslrec_chapol_context *cc,
	int record_type, unsigned version, void *data, size_t len,
	void *tag, int encrypt)
{
	unsigned char header[13];
	unsigned char nonce[12];
	uint64_t seq;
	size_t u;

	seq = cc->seq ++;
	br_enc64be(header, seq);
	header[8] = (unsigned char)record_type;
	br_enc16be(header + 9, version);
	br_enc16be(header + 11, len);
	memcpy(nonce, cc->iv, 12);
	for (u = 0; u < 8; u ++) {
		nonce[11 - u] ^= (unsigned char)seq;
		seq >>= 8;
	}
	cc->ipoly(cc->key, nonce, data, len, header, sizeof header,
		tag, cc->ichacha, encrypt);
}

static void
in_chapol_init(br_sslrec_chapol_context *cc,
	br_chacha20_run ichacha, br_poly1305_run ipoly,
	const void *key, const void *iv)
{
	cc->vtable.in = &br_sslrec_in_chapol_vtable;
	gen_chapol_init(cc, ichacha, ipoly, key, iv);
}

static int
chapol_check_length(const br_sslrec_chapol_context *cc, size_t rlen)
{
	/*
	 * Overhead is just the authentication tag (16 bytes).
	 */
	(void)cc;
	return rlen >= 16 && rlen <= (16384 + 16);
}

static unsigned char *
chapol_decrypt(br_sslrec_chapol_context *cc,
	int record_type, unsigned version, void *data, size_t *data_len)
{
	unsigned char *buf;
	size_t u, len;
	unsigned char tag[16];
	unsigned bad;

	buf = data;
	len = *data_len - 16;
	gen_chapol_process(cc, record_type, version, buf, len, tag, 0);
	bad = 0;
	for (u = 0; u < 16; u ++) {
		bad |= tag[u] ^ buf[len + u];
	}
	if (bad) {
		return NULL;
	}
	*data_len = len;
	return buf;
}

/* see bearssl_ssl.h */
const br_sslrec_in_chapol_class br_sslrec_in_chapol_vtable = {
	{
		sizeof(br_sslrec_chapol_context),
		(int (*)(const br_sslrec_in_class *const *, size_t))
			&chapol_check_length,
		(unsigned char *(*)(const br_sslrec_in_class **,
			int, unsigned, void *, size_t *))
			&chapol_decrypt
	},
	(void (*)(const br_sslrec_in_chapol_class **,
		br_chacha20_run, br_poly1305_run,
		const void *, const void *))
		&in_chapol_init
};

static void
out_chapol_init(br_sslrec_chapol_context *cc,
	br_chacha20_run ichacha, br_poly1305_run ipoly,
	const void *key, const void *iv)
{
	cc->vtable.out = &br_sslrec_out_chapol_vtable;
	gen_chapol_init(cc, ichacha, ipoly, key, iv);
}

static void
chapol_max_plaintext(const br_sslrec_chapol_context *cc,
	size_t *start, size_t *end)
{
	size_t len;

	(void)cc;
	len = *end - *start - 16;
	if (len > 16384) {
		len = 16384;
	}
	*end = *start + len;
}

static unsigned char *
chapol_encrypt(br_sslrec_chapol_context *cc,
	int record_type, unsigned version, void *data, size_t *data_len)
{
	unsigned char *buf;
	size_t len;

	buf = data;
	len = *data_len;
	gen_chapol_process(cc, record_type, version, buf, len, buf + len, 1);
	buf -= 5;
	buf[0] = (unsigned char)record_type;
	br_enc16be(buf + 1, version);
	br_enc16be(buf + 3, len + 16);
	*data_len = len + 21;
	return buf;
}

/* see bearssl_ssl.h */
const br_sslrec_out_chapol_class br_sslrec_out_chapol_vtable = {
	{
		sizeof(br_sslrec_chapol_context),
		(void (*)(const br_sslrec_out_class *const *,
			size_t *, size_t *))
			&chapol_max_plaintext,
		(unsigned char *(*)(const br_sslrec_out_class **,
			int, unsigned, void *, size_t *))
			&chapol_encrypt
	},
	(void (*)(const br_sslrec_out_chapol_class **,
		br_chacha20_run, br_poly1305_run,
		const void *, const void *))
		&out_chapol_init
};


/* ===== src/ssl/ssl_lru.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

/*
 * Each entry consists in a fixed number of bytes. Entries are concatenated
 * in the store block. "Addresses" are really offsets in the block,
 * expressed over 32 bits (so the cache may have size at most 4 GB, which
 * "ought to be enough for everyone"). The "null address" is 0xFFFFFFFF.
 * Note that since the storage block alignment is in no way guaranted, we
 * perform only accesses that can handle unaligned data.
 *
 * Two concurrent data structures are maintained:
 *
 * -- Entries are organised in a doubly-linked list; saved entries are added
 * at the head, and loaded entries are moved to the head. Eviction uses
 * the list tail (this is the LRU algorithm).
 *
 * -- Entries are indexed with a binary tree: all left descendants of a
 * node have a lower session ID (in lexicographic order), while all
 * right descendants have a higher session ID. The tree is balanced.
 *
 * Entry format:
 *
 *   session ID          32 bytes
 *   master secret       48 bytes
 *   protocol version    2 bytes (big endian)
 *   cipher suite        2 bytes (big endian)
 *   list prev           4 bytes (big endian)
 *   list next           4 bytes (big endian)
 *   tree left child     4 bytes (big endian)
 *   tree right child    4 bytes (big endian)
 *   tree node colour    1 byte (0 = red, 1 = black)
 *
 * We need to keep the tree balanced because an attacker could make
 * handshakes, selecting some specific sessions (by reusing them) to
 * try to make us make an imbalanced tree that makes lookups expensive
 * (a denial-of-service attack that would persist as long as the cache
 * remains, i.e. even after the attacker made all his connections).
 * To do that, we replace the session ID (or the start of the session ID)
 * with a HMAC value computed over the replaced part; the hash function
 * implementation and the key are obtained from the server context upon
 * first save() call.
 */
#define SESSION_ID_LEN       32
#define MASTER_SECRET_LEN    48

#define SESSION_ID_OFF        0
#define MASTER_SECRET_OFF    32
#define VERSION_OFF          80
#define CIPHER_SUITE_OFF     82
#define LIST_PREV_OFF        84
#define LIST_NEXT_OFF        88
#define TREE_LEFT_OFF        92
#define TREE_RIGHT_OFF       96

#define LRU_ENTRY_LEN       100

#define ADDR_NULL   ((uint32_t)-1)

#define GETSET(name, off) \
static inline uint32_t get_ ## name(br_ssl_session_cache_lru *cc, uint32_t x) \
{ \
	return br_dec32be(cc->store + x + (off)); \
} \
static inline void set_ ## name(br_ssl_session_cache_lru *cc, \
	uint32_t x, uint32_t val) \
{ \
	br_enc32be(cc->store + x + (off), val); \
}

GETSET(prev, LIST_PREV_OFF)
GETSET(next, LIST_NEXT_OFF)
GETSET(left, TREE_LEFT_OFF)
GETSET(right, TREE_RIGHT_OFF)

/*
 * Transform the session ID by replacing the first N bytes with a HMAC
 * value computed over these bytes, using the random key K (the HMAC
 * value is truncated if needed). HMAC will use the same hash function
 * as the DRBG in the SSL server context, so with SHA-256, SHA-384,
 * or SHA-1, depending on what is available.
 *
 * The risk of collision is considered too small to be a concern; and
 * the impact of a collision is low (the handshake won't succeed). This
 * risk is much lower than any transmission error, which would lead to
 * the same consequences.
 */
static void
mask_id(br_ssl_session_cache_lru *cc,
	const unsigned char *src, unsigned char *dst)
{
	br_hmac_key_context hkc;
	br_hmac_context hc;

	memcpy(dst, src, SESSION_ID_LEN);
	br_hmac_key_init(&hkc, cc->hash, cc->index_key, sizeof cc->index_key);
	br_hmac_init(&hc, &hkc, SESSION_ID_LEN);
	br_hmac_update(&hc, src, SESSION_ID_LEN);
	br_hmac_out(&hc, dst);
}

/*
 * Find a node by ID. Returned value is the node address, or ADDR_NULL if
 * the node is not found.
 *
 * If addr_link is not NULL, then '*addr_link' is set to the address of the
 * last followed link. If the found node is the root, then '*addr_link' is
 * set to ADDR_NULL.
 */
static uint32_t
find_node(br_ssl_session_cache_lru *cc, const unsigned char *id,
	uint32_t *addr_link)
{
	uint32_t x, y;

	x = cc->root;
	y = ADDR_NULL;
	while (x != ADDR_NULL) {
		int r;

		r = memcmp(id, cc->store + x + SESSION_ID_OFF, SESSION_ID_LEN);
		if (r < 0) {
			y = x + TREE_LEFT_OFF;
			x = get_left(cc, x);
		} else if (r == 0) {
			if (addr_link != NULL) {
				*addr_link = y;
			}
			return x;
		} else {
			y = x + TREE_RIGHT_OFF;
			x = get_right(cc, x);
		}
	}
	if (addr_link != NULL) {
		*addr_link = y;
	}
	return ADDR_NULL;
}

/*
 * For node x, find its replacement upon removal.
 *
 *  -- If node x has no child, then this returns ADDR_NULL.
 *  -- Otherwise, if node x has a left child, then the replacement is the
 *     rightmost left-descendent.
 *  -- Otherwise, the replacement is the leftmost right-descendent.
 *
 * If a node is returned, then '*al' is set to the address of the field
 * that points to that node.
 */
static uint32_t
find_replacement_node(br_ssl_session_cache_lru *cc, uint32_t x, uint32_t *al)
{
	uint32_t y1, y2;

	y1 = get_left(cc, x);
	if (y1 != ADDR_NULL) {
		y2 = x + TREE_LEFT_OFF;
		for (;;) {
			uint32_t z;

			z = get_right(cc, y1);
			if (z == ADDR_NULL) {
				*al = y2;
				return y1;
			}
			y2 = y1 + TREE_RIGHT_OFF;
			y1 = z;
		}
	}
	y1 = get_right(cc, x);
	if (y1 != ADDR_NULL) {
		y2 = x + TREE_RIGHT_OFF;
		for (;;) {
			uint32_t z;

			z = get_left(cc, y1);
			if (z == ADDR_NULL) {
				*al = y2;
				return y1;
			}
			y2 = y1 + TREE_LEFT_OFF;
			y1 = z;
		}
	}
	*al = ADDR_NULL;
	return ADDR_NULL;
}

static inline void
set_link(br_ssl_session_cache_lru *cc, uint32_t alx, uint32_t x)
{
	if (alx == ADDR_NULL) {
		cc->root = x;
	} else {
		br_enc32be(cc->store + alx, x);
	}
}

static void
remove_node(br_ssl_session_cache_lru *cc, uint32_t x)
{
	uint32_t alx, y, aly;

	/*
	 * Find node back and its ancestor link.
	 */
	find_node(cc, cc->store + x + SESSION_ID_OFF, &alx);

	/*
	 * Find replacement node.
	 */
	y = find_replacement_node(cc, x, &aly);

	/*
	 * Unlink replacement node.
	 */
	set_link(cc, aly, ADDR_NULL);

	/*
	 * Link the replacement node in its new place.
	 */
	set_link(cc, alx, y);
}

static void
lru_save(const br_ssl_session_cache_class **ctx,
	br_ssl_server_context *server_ctx,
	const br_ssl_session_parameters *params)
{
	br_ssl_session_cache_lru *cc;
	unsigned char id[SESSION_ID_LEN];
	uint32_t x, alx;

	cc = (br_ssl_session_cache_lru *)ctx;

	/*
	 * If the buffer is too small, we don't record anything. This
	 * test avoids problems in subsequent code.
	 */
	if (cc->store_len < LRU_ENTRY_LEN) {
		return;
	}

	/*
	 * Upon the first save in a session cache instance, we obtain
	 * a random key for our indexing.
	 */
	if (!cc->init_done) {
		br_hmac_drbg_generate(&server_ctx->eng.rng,
			cc->index_key, sizeof cc->index_key);
		cc->hash = br_hmac_drbg_get_hash(&server_ctx->eng.rng);
		cc->init_done = 1;
	}
	mask_id(cc, params->session_id, id);

	/*
	 * Look for the node in the tree. If the same ID is already used,
	 * then reject it. This is a collision event, which should be
	 * exceedingly rare.
	 * Note: we do NOT record the emplacement here, because the
	 * removal of an entry may change the tree topology.
	 */
	if (find_node(cc, id, NULL) != ADDR_NULL) {
		return;
	}

	/*
	 * Find some room for the new parameters. If the cache is not
	 * full yet, add it to the end of the area and bump the pointer up.
	 * Otherwise, evict the list tail entry. Note that we already
	 * filtered out the case of a ridiculously small buffer that
	 * cannot hold any entry at all; thus, if there is no room for an
	 * extra entry, then the cache cannot be empty.
	 */
	if (cc->store_ptr > (cc->store_len - LRU_ENTRY_LEN)) {
		/*
		 * Evict tail. If the buffer has room for a single entry,
		 * then this may also be the head.
		 */
		x = cc->tail;
		cc->tail = get_prev(cc, x);
		if (cc->tail == ADDR_NULL) {
			cc->head = ADDR_NULL;
		} else {
			set_next(cc, cc->tail, ADDR_NULL);
		}

		/*
		 * Remove the node from the tree.
		 */
		remove_node(cc, x);
	} else {
		/*
		 * Allocate room for new node.
		 */
		x = cc->store_ptr;
		cc->store_ptr += LRU_ENTRY_LEN;
	}

	/*
	 * Find the emplacement for the new node, and link it.
	 */
	find_node(cc, id, &alx);
	set_link(cc, alx, x);
	set_left(cc, x, ADDR_NULL);
	set_right(cc, x, ADDR_NULL);

	/*
	 * New entry becomes new list head. It may also become the list
	 * tail if the cache was empty at that point.
	 */
	if (cc->head == ADDR_NULL) {
		cc->tail = x;
	} else {
		set_prev(cc, cc->head, x);
	}
	set_prev(cc, x, ADDR_NULL);
	set_next(cc, x, cc->head);
	cc->head = x;

	/*
	 * Fill data in the entry.
	 */
	memcpy(cc->store + x + SESSION_ID_OFF, id, SESSION_ID_LEN);
	memcpy(cc->store + x + MASTER_SECRET_OFF,
		params->master_secret, MASTER_SECRET_LEN);
	br_enc16be(cc->store + x + VERSION_OFF, params->version);
	br_enc16be(cc->store + x + CIPHER_SUITE_OFF, params->cipher_suite);
}

static int
lru_load(const br_ssl_session_cache_class **ctx,
	br_ssl_server_context *server_ctx,
	br_ssl_session_parameters *params)
{
	br_ssl_session_cache_lru *cc;
	unsigned char id[SESSION_ID_LEN];
	uint32_t x;

	(void)server_ctx;
	cc = (br_ssl_session_cache_lru *)ctx;
	if (!cc->init_done) {
		return 0;
	}
	mask_id(cc, params->session_id, id);
	x = find_node(cc, id, NULL);
	if (x != ADDR_NULL) {
		params->version = br_dec16be(
			cc->store + x + VERSION_OFF);
		params->cipher_suite = br_dec16be(
			cc->store + x + CIPHER_SUITE_OFF);
		memcpy(params->master_secret,
			cc->store + x + MASTER_SECRET_OFF,
			MASTER_SECRET_LEN);
		if (x != cc->head) {
			/*
			 * Found node is not at list head, so move
			 * it to the head.
			 */
			uint32_t p, n;

			p = get_prev(cc, x);
			n = get_next(cc, x);
			set_next(cc, p, n);
			if (n == ADDR_NULL) {
				cc->tail = p;
			} else {
				set_prev(cc, n, p);
			}
			set_prev(cc, cc->head, x);
			set_next(cc, x, cc->head);
			set_prev(cc, x, ADDR_NULL);
			cc->head = x;
		}
		return 1;
	}
	return 0;
}

static const br_ssl_session_cache_class lru_class = {
	sizeof(br_ssl_session_cache_lru),
	&lru_save,
	&lru_load
};

/* see inner.h */
void
br_ssl_session_cache_lru_init(br_ssl_session_cache_lru *cc,
	unsigned char *store, size_t store_len)
{
	cc->vtable = &lru_class;
	cc->store = store;
	cc->store_len = store_len;
	cc->store_ptr = 0;
	cc->init_done = 0;
	cc->head = ADDR_NULL;
	cc->tail = ADDR_NULL;
	cc->root = ADDR_NULL;
}


/* ===== src/ssl/ssl_ccert_single_rsa.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

static void
cc_none0(const br_ssl_client_certificate_class **pctx)
{
	(void)pctx;
}

static void
cc_none1(const br_ssl_client_certificate_class **pctx, size_t len)
{
	(void)pctx;
	(void)len;
}

static void
cc_none2(const br_ssl_client_certificate_class **pctx,
	const unsigned char *data, size_t len)
{
	(void)pctx;
	(void)data;
	(void)len;
}

static void
cc_choose(const br_ssl_client_certificate_class **pctx,
	const br_ssl_client_context *cc, uint32_t auth_types,
	br_ssl_client_certificate *choices)
{
	br_ssl_client_certificate_rsa_context *zc;
	int x;

	(void)cc;
	zc = (br_ssl_client_certificate_rsa_context *)pctx;
	x = br_ssl_choose_hash((unsigned)auth_types);
	if (x == 0 && (auth_types & 1) == 0) {
		memset(choices, 0, sizeof *choices);
	}
	choices->auth_type = BR_AUTH_RSA;
	choices->hash_id = x;
	choices->chain = zc->chain;
	choices->chain_len = zc->chain_len;
}

/*
 * OID for hash functions in RSA signatures.
 */
static const unsigned char HASH_OID_SHA1[] = {
	0x05, 0x2B, 0x0E, 0x03, 0x02, 0x1A
};

static const unsigned char HASH_OID_SHA224[] = {
	0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x04
};

static const unsigned char HASH_OID_SHA256[] = {
	0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01
};

static const unsigned char HASH_OID_SHA384[] = {
	0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x02
};

static const unsigned char HASH_OID_SHA512[] = {
	0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x03
};

static const unsigned char *HASH_OID[] = {
	HASH_OID_SHA1,
	HASH_OID_SHA224,
	HASH_OID_SHA256,
	HASH_OID_SHA384,
	HASH_OID_SHA512
};

static size_t
cc_do_sign(const br_ssl_client_certificate_class **pctx,
	int hash_id, size_t hv_len, unsigned char *data, size_t len)
{
	br_ssl_client_certificate_rsa_context *zc;
	unsigned char hv[64];
	const unsigned char *hash_oid;
	size_t sig_len;

	zc = (br_ssl_client_certificate_rsa_context *)pctx;
	memcpy(hv, data, hv_len);
	if (hash_id == 0) {
		hash_oid = NULL;
	} else if (hash_id >= 2 && hash_id <= 6) {
		hash_oid = HASH_OID[hash_id - 2];
	} else {
		return 0;
	}
	sig_len = (zc->sk->n_bitlen + 7) >> 3;
	if (len < sig_len) {
		return 0;
	}
	return zc->irsasign(hash_oid, hv, hv_len, zc->sk, data) ? sig_len : 0;
}

static const br_ssl_client_certificate_class ccert_vtable = {
	sizeof(br_ssl_client_certificate_rsa_context),
	cc_none0, /* start_name_list */
	cc_none1, /* start_name */
	cc_none2, /* append_name */
	cc_none0, /* end_name */
	cc_none0, /* end_name_list */
	cc_choose,
	0,
	cc_do_sign
};

/* see bearssl_ssl.h */
void
br_ssl_client_set_single_rsa(br_ssl_client_context *cc,
	const br_x509_certificate *chain, size_t chain_len,
	const br_rsa_private_key *sk, br_rsa_pkcs1_sign irsasign)
{
	cc->client_auth.single_rsa.vtable = &ccert_vtable;
	cc->client_auth.single_rsa.chain = chain;
	cc->client_auth.single_rsa.chain_len = chain_len;
	cc->client_auth.single_rsa.sk = sk;
	cc->client_auth.single_rsa.irsasign = irsasign;
	cc->client_auth_vtable = &cc->client_auth.single_rsa.vtable;
}


/* ===== src/ssl/ssl_ccert_single_ec.c ===== */
/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* (already included) */

static void
cc_none0(const br_ssl_client_certificate_class **pctx)
{
	(void)pctx;
}

static void
cc_none1(const br_ssl_client_certificate_class **pctx, size_t len)
{
	(void)pctx;
	(void)len;
}

static void
cc_none2(const br_ssl_client_certificate_class **pctx,
	const unsigned char *data, size_t len)
{
	(void)pctx;
	(void)data;
	(void)len;
}

static void
cc_choose(const br_ssl_client_certificate_class **pctx,
	const br_ssl_client_context *cc, uint32_t auth_types,
	br_ssl_client_certificate *choices)
{
	br_ssl_client_certificate_ec_context *zc;
	int x;
	int scurve;

	zc = (br_ssl_client_certificate_ec_context *)pctx;
	scurve = br_ssl_client_get_server_curve(cc);

	if ((zc->allowed_usages & BR_KEYTYPE_KEYX) != 0
		&& scurve == zc->sk->curve)
	{
		int x;

		x = (zc->issuer_key_type == BR_KEYTYPE_RSA) ? 16 : 17;
		if (((auth_types >> x) & 1) != 0) {
			choices->auth_type = BR_AUTH_ECDH;
			choices->hash_id = -1;
			choices->chain = zc->chain;
			choices->chain_len = zc->chain_len;
		}
	}

	/*
	 * For ECDSA authentication, we must choose an appropriate
	 * hash function.
	 */
	x = br_ssl_choose_hash((unsigned)(auth_types >> 8));
	if (x == 0 || (zc->allowed_usages & BR_KEYTYPE_SIGN) == 0) {
		memset(choices, 0, sizeof *choices);
		return;
	}
	choices->auth_type = BR_AUTH_ECDSA;
	choices->hash_id = x;
	choices->chain = zc->chain;
	choices->chain_len = zc->chain_len;
}

static uint32_t
cc_do_keyx(const br_ssl_client_certificate_class **pctx,
	unsigned char *data, size_t len)
{
	br_ssl_client_certificate_ec_context *zc;

	zc = (br_ssl_client_certificate_ec_context *)pctx;
	return zc->iec->mul(data, len, zc->sk->x, zc->sk->xlen, zc->sk->curve);
}

static size_t
cc_do_sign(const br_ssl_client_certificate_class **pctx,
	int hash_id, size_t hv_len, unsigned char *data, size_t len)
{
	br_ssl_client_certificate_ec_context *zc;
	unsigned char hv[64];
	const br_hash_class *hc;

	zc = (br_ssl_client_certificate_ec_context *)pctx;
	memcpy(hv, data, hv_len);
	hc = br_multihash_getimpl(zc->mhash, hash_id);
	if (hc == NULL) {
		return 0;
	}
	if (len < 139) {
		return 0;
	}
	return zc->iecdsa(zc->iec, hc, hv, zc->sk, data);
}

static const br_ssl_client_certificate_class ccert_vtable = {
	sizeof(br_ssl_client_certificate_ec_context),
	cc_none0, /* start_name_list */
	cc_none1, /* start_name */
	cc_none2, /* append_name */
	cc_none0, /* end_name */
	cc_none0, /* end_name_list */
	cc_choose,
	cc_do_keyx,
	cc_do_sign
};

/* see bearssl_ssl.h */
void
br_ssl_client_set_single_ec(br_ssl_client_context *cc,
	const br_x509_certificate *chain, size_t chain_len,
	const br_ec_private_key *sk, unsigned allowed_usages,
	unsigned cert_issuer_key_type,
	const br_ec_impl *iec, br_ecdsa_sign iecdsa)
{
	cc->client_auth.single_ec.vtable = &ccert_vtable;
	cc->client_auth.single_ec.chain = chain;
	cc->client_auth.single_ec.chain_len = chain_len;
	cc->client_auth.single_ec.sk = sk;
	cc->client_auth.single_ec.allowed_usages = allowed_usages;
	cc->client_auth.single_ec.issuer_key_type = cert_issuer_key_type;
	cc->client_auth.single_ec.mhash = &cc->eng.mhash;
	cc->client_auth.single_ec.iec = iec;
	cc->client_auth.single_ec.iecdsa = iecdsa;
	cc->client_auth_vtable = &cc->client_auth.single_ec.vtable;
}


#ifdef __clang__
#pragma clang diagnostic pop
#endif
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#endif /* BEARSSL_AMALG_C */