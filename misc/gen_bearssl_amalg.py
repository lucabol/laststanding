#!/usr/bin/env python3
"""Generate BearSSL amalgamation for laststanding - minimal TLS 1.2 client."""
import os, sys, re

bearssl_root = sys.argv[1] if len(sys.argv) > 1 else r'C:\Users\lucabol\dev\bearssl'
out_path = sys.argv[2] if len(sys.argv) > 2 else r'C:\Users\lucabol\dev\laststanding\bearssl_amalg.c'

src = os.path.join(bearssl_root, 'src')
inc = os.path.join(bearssl_root, 'inc')

# Minimal TLS 1.2 client file list
files = [
    # codec
    'codec/ccopy.c', 'codec/dec16be.c', 'codec/dec32be.c',
    'codec/enc16be.c', 'codec/enc16le.c', 'codec/enc32be.c', 'codec/enc64be.c',
    'codec/pemdec.c',
    # hash
    'hash/dig_oid.c', 'hash/dig_size.c', 'hash/ghash_ctmul.c', 'hash/ghash_ctmul32.c',
    'hash/md5.c', 'hash/md5sha1.c', 'hash/multihash.c',
    'hash/sha1.c', 'hash/sha2big.c', 'hash/sha2small.c',
    # mac
    'mac/hmac.c', 'mac/hmac_ct.c',
    # rand
    'rand/hmac_drbg.c',
    # int (i31 engine)
    'int/i31_add.c', 'int/i31_bitlen.c', 'int/i31_decmod.c', 'int/i31_decode.c',
    'int/i31_decred.c', 'int/i31_encode.c', 'int/i31_fmont.c', 'int/i31_iszero.c',
    'int/i31_modpow.c', 'int/i31_montmul.c', 'int/i31_mulacc.c', 'int/i31_muladd.c',
    'int/i31_ninv31.c', 'int/i31_reduce.c', 'int/i31_rshift.c', 'int/i31_sub.c',
    'int/i31_tmont.c', 'int/i32_div32.c',
    # int (i15 for some EC paths)
    'int/i15_core.c', 'int/i15_ext1.c', 'int/i15_ext2.c',
    # rsa (client: pub + verify only)
    'rsa/rsa_i31_pkcs1_vrfy.c', 'rsa/rsa_i31_pub.c', 'rsa/rsa_pkcs1_sig_unpad.c',
    'rsa/rsa_ssl_decrypt.c',
    # ec (i31 only — i15 has static name collisions in amalgamation)
    'ec/ec_prime_i31.c', 'ec/ec_prime_i31_secp256r1.c',
    'ec/ec_prime_i31_secp384r1.c', 'ec/ec_prime_i31_secp521r1.c',
    'ec/ec_secp256r1.c', 'ec/ec_secp384r1.c', 'ec/ec_secp521r1.c',
    'ec/ecdsa_i31_vrfy_asn1.c', 'ec/ecdsa_i31_vrfy_raw.c', 'ec/ecdsa_i31_bits.c',
    'ec/ecdsa_atr.c', 'ec/ecdsa_rta.c',
    # symcipher (AES constant-time + ChaCha20/Poly1305)
    'symcipher/aes_common.c', 'symcipher/aes_ct.c',
    'symcipher/aes_ct_enc.c', 'symcipher/aes_ct_dec.c', 'symcipher/aes_ct_ctr.c',
    'symcipher/aes_ct_cbcdec.c', 'symcipher/aes_ct_cbcenc.c',
    'symcipher/chacha20_ct.c', 'symcipher/poly1305_ctmul.c',
    # x509
    'x509/x509_decoder.c', 'x509/x509_minimal.c', 'x509/x509_minimal_full.c',
    'x509/x509_knownkey.c', 'x509/skey_decoder.c',
    # ssl (client only)
    'ssl/prf.c', 'ssl/prf_md5sha1.c', 'ssl/prf_sha256.c', 'ssl/prf_sha384.c',
    'ssl/ssl_engine.c', 'ssl/ssl_client.c', 'ssl/ssl_client_full.c',
    'ssl/ssl_hs_client.c', 'ssl/ssl_hashes.c', 'ssl/ssl_io.c',
    'ssl/ssl_rec_gcm.c', 'ssl/ssl_rec_cbc.c', 'ssl/ssl_rec_chapol.c',
    'ssl/ssl_lru.c',
    'ssl/ssl_ccert_single_rsa.c',
    # ssl_ccert_single_ec.c removed: shares static names with ssl_ccert_single_rsa.c
]

pub_headers = [
    'bearssl_hash.h', 'bearssl_block.h', 'bearssl_hmac.h', 'bearssl_prf.h',
    'bearssl_rand.h', 'bearssl_ec.h', 'bearssl_rsa.h', 'bearssl_x509.h',
    'bearssl_ssl.h', 'bearssl_pem.h', 'bearssl.h',
]

out = []
out.append('/*')
out.append(' * BearSSL amalgamation for laststanding - minimal TLS 1.2 client')
out.append(' * Auto-generated from BearSSL v0.6 (Thomas Pornin)')
out.append(' * License: MIT')
out.append(' *')
out.append(' * Single-file amalgamation containing only TLS 1.2 client code.')
out.append(' * Uses i31 big-integer engine, constant-time AES, ECDHE+RSA.')
out.append(' * No server code. No dynamic allocation (zero malloc).')
out.append(' */')
out.append('')
out.append('#ifndef BEARSSL_AMALG_C')
out.append('#define BEARSSL_AMALG_C')
out.append('')
out.append('#ifdef __clang__')
out.append('#pragma clang diagnostic push')
out.append('#pragma clang diagnostic ignored "-Wunused-function"')
out.append('#pragma clang diagnostic ignored "-Wunused-parameter"')
out.append('#pragma clang diagnostic ignored "-Wsign-conversion"')
out.append('#pragma clang diagnostic ignored "-Wconversion"')
out.append('#pragma clang diagnostic ignored "-Wshorten-64-to-32"')
out.append('#pragma clang diagnostic ignored "-Wimplicit-fallthrough"')
out.append('#pragma clang diagnostic ignored "-Wcast-qual"')
out.append('#pragma clang diagnostic ignored "-Wmissing-prototypes"')
out.append('#pragma clang diagnostic ignored "-Wmissing-variable-declarations"')
out.append('#endif')
out.append('#ifdef __GNUC__')
out.append('#pragma GCC diagnostic push')
out.append('#pragma GCC diagnostic ignored "-Wunused-function"')
out.append('#pragma GCC diagnostic ignored "-Wunused-parameter"')
out.append('#pragma GCC diagnostic ignored "-Wsign-conversion"')
out.append('#pragma GCC diagnostic ignored "-Wconversion"')
out.append('#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"')
out.append('#pragma GCC diagnostic ignored "-Wcast-qual"')
out.append('#endif')
out.append('')

def strip_includes(content):
    """Remove includes of bearssl and inner headers (already inlined)."""
    content = re.sub(r'#include\s+"bearssl[^"]*\.h"', '/* (already included) */', content)
    content = re.sub(r'#include\s+"inner\.h"', '/* (already included) */', content)
    content = re.sub(r'#include\s+"config\.h"', '/* (already included) */', content)
    return content

# Static names that conflict across BearSSL source files.
# These get #define'd to unique names per file, then #undef'd after.
CONFLICT_NAMES = []  # Disabled — separate compilation handles this

def wrap_with_defines(content, prefix):
    """Add #define PREFIX_name name / #undef name around content to avoid collisions."""
    defs = []
    undefs = []
    for name in CONFLICT_NAMES:
        defs.append(f'#define {name} {prefix}{name}')
        undefs.append(f'#undef {name}')
    return '\n'.join(defs) + '\n' + content + '\n' + '\n'.join(undefs) + '\n'

# Public headers
for h in pub_headers:
    path = os.path.join(inc, h)
    if not os.path.exists(path):
        print(f'WARNING: missing {path}', file=sys.stderr)
        continue
    with open(path, 'r', encoding='utf-8', errors='replace') as f:
        content = strip_includes(f.read())
    out.append(f'/* ===== inc/{h} ===== */')
    out.append(content)
    out.append('')

# Config header
config_path = os.path.join(src, 'config.h')
with open(config_path, 'r', encoding='utf-8', errors='replace') as f:
    out.append('/* ===== src/config.h ===== */')
    out.append(f.read())
    out.append('')

# Internal header
inner_path = os.path.join(src, 'inner.h')
with open(inner_path, 'r', encoding='utf-8', errors='replace') as f:
    content = strip_includes(f.read())
out.append('/* ===== src/inner.h ===== */')
out.append(content)
out.append('')

# Source files — each wrapped with #define/#undef to avoid static name collisions
file_idx = 0
for fname in files:
    path = os.path.join(src, fname)
    if not os.path.exists(path):
        print(f'WARNING: missing {path}', file=sys.stderr)
        continue
    with open(path, 'r', encoding='utf-8', errors='replace') as f:
        content = strip_includes(f.read())
    # Special handling for SHA K arrays — rename to avoid collision
    if fname == 'hash/sha2big.c':
        content = content.replace('static const uint64_t K[', 'static const uint64_t K_sha512[')
        content = content.replace('K[j]', 'K_sha512[j]')
    elif fname == 'hash/sha2small.c':
        content = content.replace('static const uint32_t K[', 'static const uint32_t K_sha256[')
        content = content.replace('K[j]', 'K_sha256[j]')
    prefix = f'br_{file_idx}_'
    content = wrap_with_defines(content, prefix)
    out.append(f'/* ===== src/{fname} ===== */')
    out.append(content)
    out.append('')
    file_idx += 1

out.append('#ifdef __clang__')
out.append('#pragma clang diagnostic pop')
out.append('#endif')
out.append('#ifdef __GNUC__')
out.append('#pragma GCC diagnostic pop')
out.append('#endif')
out.append('')
out.append('#endif /* BEARSSL_AMALG_C */')

result = '\n'.join(out)
with open(out_path, 'w', encoding='utf-8') as f:
    f.write(result)

print(f'Written {len(result)} bytes ({len(result)//1024} KB)')
print(f'Source files: {len(files)}')
print(f'Headers: {len(pub_headers)} public + inner.h + config.h')
