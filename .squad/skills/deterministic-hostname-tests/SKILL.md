---
name: "deterministic-hostname-tests"
description: "Local-only regression patterns for hostname resolution in socket tests"
domain: "testing"
confidence: "high"
source: "manual"
---

## Context
Hostname resolution is easy to over-test with flaky internet dependencies. In this repo, the safer pattern is to separate the raw IPv4 parser from the public resolver API and keep all end-to-end coverage on loopback.

## Patterns

### Separate parser assertions from resolver assertions
Keep `l_inet_addr("localhost") == 0` as a regression test for the dotted-quad parser. Put hostname-success assertions on `l_resolve(...)`, then feed its output into socket helpers.

### Test the public resolver directly
For `int l_resolve(const char *hostname, char *ip_out)`, cover:
- IPv4 literal passthrough (`"127.0.0.1"` stays unchanged)
- loopback hostname (`"localhost"`)
- NULL hostname
- NULL output buffer
- empty string
- malformed hostname
- invalid dotted quad (`"256.0.0.1"`)
- guarded-buffer write boundaries around the 15-byte IPv4 string

### Use one self-hosted loopback listener for both code paths
Open a local listener inside the test, resolve `"localhost"` once, then exercise the integration twice:
- once with the IP returned by `l_resolve("localhost", ip)`
- once with direct `"127.0.0.1"`

That proves hostname resolution works without depending on `/etc/hosts` contents beyond standard loopback setup, public DNS, or outbound internet access.

### Don’t assume socket helpers accept hostnames directly
If the approved surface is `l_resolve(...)`, treat that as the contract. Unless the implementation explicitly promises hostname-aware `l_socket_connect`/`l_socket_sendto`, use resolved IPv4 strings for integration tests rather than asserting direct `"localhost"` success.

### Pick ports from a per-process high range
Derive the starting port from `l_getpid()` and scan a bounded high-port range until bind succeeds. This avoids hard-coded port collisions across parallel runs while staying deterministic enough for CI.

### Reuse the shared loopback-listener helper
If `test/test.c` already has `open_loopback_listener()`, keep extending that helper instead of adding another ad hoc port chooser. One helper keeps hostname tests consistent across Windows, Linux, ARM, and AArch64.

### Verify actual data flow, not just connect success
After the loopback connect/send succeeds, accept or receive the packet and exchange a tiny payload (`"ok"` is enough). That catches regressions where resolution works but the integration path is still broken.

## Anti-Patterns

- **Using public domains in always-on tests** — adds DNS, routing, firewall, and internet flake to a systems test.
- **Expecting `l_inet_addr` to resolve hostnames** — that conflates the string parser with `l_resolve`.
- **Hard-coding a single favorite port** — invites intermittent CI failures from collisions.
