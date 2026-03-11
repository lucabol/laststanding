# Decision: l_strstr empty-string behavior

**From:** Lambert (Tester)
**Date:** 2026-03-11
**Issue:** #12

## Observation

`l_strstr("", "")` returns `NULL`, while standard libc `strstr("", "")` returns a pointer to the empty haystack. This is because the implementation's `while (*s1)` loop doesn't execute when the haystack is empty, so even a zero-length needle match is never checked.

## Impact

This is a minor deviation from POSIX/C standard behavior. It likely won't affect typical usage but could surprise callers who depend on the standard contract.

## Recommendation

Dallas should decide whether to fix the implementation (add a check for `len == 0` before the loop) or document this as intentional. I've documented the current behavior in the test suite so it won't silently regress either way.
