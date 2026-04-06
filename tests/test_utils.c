// Utility, data-structure, time, and math coverage split from the old monolith.

#define L_WITHSNPRINTF
#define L_MAINFILE
#include "l_os.h"
#include "test_support.h"

TEST_DECLARE_COUNTERS();

void test_time(void) {
    TEST_FUNCTION("l_time");
    {
        long long t1 = l_time((long long *)0);
        TEST_ASSERT(t1 > 1700000000LL, "time is after 2023");
        long long t2 = 0;
        long long t3 = l_time(&t2);
        TEST_ASSERT(t2 == t3, "pointer and return match");
        TEST_ASSERT(t2 >= t1, "time doesn't go backward");
    }
    TEST_SECTION_PASS("l_time");
}

void test_arena(void) {
    TEST_FUNCTION("L_Arena");

    L_Arena a = l_arena_init(4096);
    TEST_ASSERT(a.base != (unsigned char *)0, "arena init returns non-NULL base");
    TEST_ASSERT(a.cap == 4096, "arena init sets cap");
    TEST_ASSERT(a.used == 0, "arena init sets used=0");

    void *p1 = l_arena_alloc(&a, 16);
    TEST_ASSERT(p1 != (void *)0, "arena alloc returns non-NULL");
    TEST_ASSERT(((size_t)p1 % 8) == 0, "arena alloc aligned to 8");

    void *p2 = l_arena_alloc(&a, 32);
    TEST_ASSERT(p2 != (void *)0, "arena second alloc non-NULL");
    TEST_ASSERT(((size_t)p2 % 8) == 0, "arena second alloc aligned");
    TEST_ASSERT((unsigned char *)p2 >= (unsigned char *)p1 + 16, "allocs do not overlap");

    // Write and read back through arena
    l_memset(p1, 0xAB, 16);
    l_memset(p2, 0xCD, 32);
    TEST_ASSERT(((unsigned char *)p1)[0] == 0xAB, "arena write/read p1");
    TEST_ASSERT(((unsigned char *)p2)[0] == 0xCD, "arena write/read p2");

    // Reset and reuse
    l_arena_reset(&a);
    TEST_ASSERT(a.used == 0, "arena reset sets used=0");
    void *p3 = l_arena_alloc(&a, 16);
    TEST_ASSERT(p3 == a.base, "alloc after reset reuses base region");

    // Arena full
    L_Arena tiny = l_arena_init(64);
    TEST_ASSERT(tiny.base != (unsigned char *)0, "small arena init");
    void *fill = l_arena_alloc(&tiny, 64);
    TEST_ASSERT(fill != (void *)0, "fill small arena");
    void *over = l_arena_alloc(&tiny, 1);
    TEST_ASSERT(over == (void *)0, "arena full returns NULL");
    l_arena_free(&tiny);
    TEST_ASSERT(tiny.base == (unsigned char *)0, "arena free sets base NULL");

    l_arena_free(&a);
    TEST_ASSERT(a.base == (unsigned char *)0, "arena free main");

    TEST_SECTION_PASS("L_Arena");
}

void test_buf(void) {
    TEST_FUNCTION("L_Buf");

    L_Buf b;
    l_buf_init(&b);
    TEST_ASSERT(b.data == (unsigned char *)0, "buf init zeroes data");
    TEST_ASSERT(b.len == 0, "buf init zeroes len");
    TEST_ASSERT(b.cap == 0, "buf init zeroes cap");

    // Push and verify
    const char *hello = "hello";
    TEST_ASSERT(l_buf_push(&b, hello, 5) == 0, "buf push succeeds");
    TEST_ASSERT(b.len == 5, "buf len after push");
    TEST_ASSERT(l_memcmp(b.data, "hello", 5) == 0, "buf data matches");

    // Multiple pushes accumulate
    TEST_ASSERT(l_buf_push(&b, " world", 6) == 0, "buf second push");
    TEST_ASSERT(b.len == 11, "buf len accumulates");
    TEST_ASSERT(l_memcmp(b.data, "hello world", 11) == 0, "buf accumulated data");

    // Printf
    int wrote = l_buf_printf(&b, " %d", 42);
    TEST_ASSERT(wrote == 3, "buf printf returns bytes written");
    TEST_ASSERT(b.len == 14, "buf len after printf");
    TEST_ASSERT(l_memcmp(b.data, "hello world 42", 14) == 0, "buf printf data");

    // Clear
    l_buf_clear(&b);
    TEST_ASSERT(b.len == 0, "buf clear resets len");
    TEST_ASSERT(b.data != (unsigned char *)0, "buf clear keeps data");
    TEST_ASSERT(b.cap > 0, "buf clear keeps cap");

    // Push beyond initial capacity triggers growth
    size_t old_cap = b.cap;
    size_t big = old_cap + 1;
    unsigned char *tmp = (unsigned char *)l_mmap((void *)0, big, L_PROT_READ | L_PROT_WRITE,
                                                 L_MAP_PRIVATE | L_MAP_ANONYMOUS, (L_FD)-1, 0);
    l_memset(tmp, 0x77, big);
    TEST_ASSERT(l_buf_push(&b, tmp, big) == 0, "buf push triggers growth");
    TEST_ASSERT(b.cap > old_cap, "buf cap grew");
    TEST_ASSERT(b.len == big, "buf len after large push");
    TEST_ASSERT(b.data[0] == 0x77 && b.data[big - 1] == 0x77, "buf large push data correct");
    l_munmap(tmp, big);

    // Large push > 4096
    l_buf_clear(&b);
    size_t huge = 8192;
    unsigned char *htmp = (unsigned char *)l_mmap((void *)0, huge, L_PROT_READ | L_PROT_WRITE,
                                                  L_MAP_PRIVATE | L_MAP_ANONYMOUS, (L_FD)-1, 0);
    l_memset(htmp, 0xAA, huge);
    TEST_ASSERT(l_buf_push(&b, htmp, huge) == 0, "buf large push >4096");
    TEST_ASSERT(b.len == huge, "buf len after huge push");
    TEST_ASSERT(b.data[0] == 0xAA && b.data[huge - 1] == 0xAA, "buf huge data correct");
    l_munmap(htmp, huge);

    // Free
    l_buf_free(&b);
    TEST_ASSERT(b.data == (unsigned char *)0, "buf free zeroes data");
    TEST_ASSERT(b.len == 0, "buf free zeroes len");
    TEST_ASSERT(b.cap == 0, "buf free zeroes cap");

    TEST_SECTION_PASS("L_Buf");
}

void test_str_constructors(void) {
    TEST_FUNCTION("L_Str constructors");

    L_Str s1 = l_str("hello");
    TEST_ASSERT(s1.len == 5, "l_str hello len=5");
    TEST_ASSERT(l_memcmp(s1.data, "hello", 5) == 0, "l_str hello data");

    L_Str s2 = l_str("");
    TEST_ASSERT(s2.len == 0, "l_str empty len=0");

    L_Str s3 = l_str((const char *)0);
    TEST_ASSERT(s3.len == 0, "l_str NULL len=0");

    L_Str s4 = l_str_from("hello", 3);
    TEST_ASSERT(s4.len == 3, "l_str_from len=3");
    TEST_ASSERT(l_memcmp(s4.data, "hel", 3) == 0, "l_str_from data");

    L_Str s5 = l_str_null();
    TEST_ASSERT(s5.data == (const char *)0, "l_str_null data=NULL");
    TEST_ASSERT(s5.len == 0, "l_str_null len=0");

    TEST_SECTION_PASS("L_Str constructors");
}

void test_str_comparison(void) {
    TEST_FUNCTION("L_Str comparison");

    // l_str_eq
    TEST_ASSERT(l_str_eq(l_str("abc"), l_str("abc")) == 1, "str_eq equal");
    TEST_ASSERT(l_str_eq(l_str("abc"), l_str("xyz")) == 0, "str_eq different");
    TEST_ASSERT(l_str_eq(l_str("ab"), l_str("abc")) == 0, "str_eq diff length");
    TEST_ASSERT(l_str_eq(l_str(""), l_str("")) == 1, "str_eq both empty");

    // l_str_cmp
    TEST_ASSERT(l_str_cmp(l_str("abc"), l_str("abd")) < 0, "str_cmp less");
    TEST_ASSERT(l_str_cmp(l_str("abd"), l_str("abc")) > 0, "str_cmp greater");
    TEST_ASSERT(l_str_cmp(l_str("abc"), l_str("abc")) == 0, "str_cmp equal");
    TEST_ASSERT(l_str_cmp(l_str("ab"), l_str("abc")) < 0, "str_cmp prefix shorter");

    // l_str_startswith
    TEST_ASSERT(l_str_startswith(l_str("hello world"), l_str("hello")) == 1, "startswith true");
    TEST_ASSERT(l_str_startswith(l_str("hello"), l_str("world")) == 0, "startswith false");
    TEST_ASSERT(l_str_startswith(l_str("hi"), l_str("hello")) == 0, "startswith prefix longer");
    TEST_ASSERT(l_str_startswith(l_str("hello"), l_str("")) == 1, "startswith empty prefix");

    // l_str_endswith
    TEST_ASSERT(l_str_endswith(l_str("hello world"), l_str("world")) == 1, "endswith true");
    TEST_ASSERT(l_str_endswith(l_str("hello"), l_str("world")) == 0, "endswith false");
    TEST_ASSERT(l_str_endswith(l_str("hi"), l_str("hello")) == 0, "endswith suffix longer");
    TEST_ASSERT(l_str_endswith(l_str("hello"), l_str("")) == 1, "endswith empty suffix");

    // l_str_contains
    TEST_ASSERT(l_str_contains(l_str("hello world"), l_str("lo w")) == 1, "contains found");
    TEST_ASSERT(l_str_contains(l_str("hello"), l_str("xyz")) == 0, "contains not found");
    TEST_ASSERT(l_str_contains(l_str("hello"), l_str("")) == 1, "contains empty needle");
    TEST_ASSERT(l_str_contains(l_str("hi"), l_str("hello")) == 0, "contains needle longer");

    TEST_SECTION_PASS("L_Str comparison");
}

void test_str_slicing(void) {
    TEST_FUNCTION("L_Str slicing");

    // l_str_sub
    L_Str hw = l_str("hello world");
    L_Str sub1 = l_str_sub(hw, 6, 5);
    TEST_ASSERT(sub1.len == 5, "str_sub normal len");
    TEST_ASSERT(l_memcmp(sub1.data, "world", 5) == 0, "str_sub normal data");

    L_Str sub2 = l_str_sub(hw, 50, 5);
    TEST_ASSERT(sub2.len == 0, "str_sub start beyond end");

    L_Str sub3 = l_str_sub(hw, 8, 100);
    TEST_ASSERT(sub3.len == 3, "str_sub len clamped");
    TEST_ASSERT(l_memcmp(sub3.data, "rld", 3) == 0, "str_sub clamped data");

    // l_str_trim
    L_Str trimmed = l_str_trim(l_str("  hello  "));
    TEST_ASSERT(l_str_eq(trimmed, l_str("hello")), "trim leading+trailing");
    TEST_ASSERT(l_str_trim(l_str("\t\n hi \t")).len == 2, "trim tabs/newlines");
    TEST_ASSERT(l_str_trim(l_str("hello")).len == 5, "trim no whitespace");
    TEST_ASSERT(l_str_trim(l_str("   ")).len == 0, "trim all whitespace");
    TEST_ASSERT(l_str_trim(l_str("")).len == 0, "trim empty");

    // l_str_ltrim / l_str_rtrim
    TEST_ASSERT(l_str_eq(l_str_ltrim(l_str("  hi")), l_str("hi")), "ltrim leading");
    TEST_ASSERT(l_str_eq(l_str_rtrim(l_str("hi  ")), l_str("hi")), "rtrim trailing");

    // l_str_chr
    TEST_ASSERT(l_str_chr(l_str("abcabc"), 'b') == 1, "str_chr found first");
    TEST_ASSERT(l_str_chr(l_str("abc"), 'z') == -1, "str_chr not found");

    // l_str_rchr
    TEST_ASSERT(l_str_rchr(l_str("abcabc"), 'b') == 4, "str_rchr last");
    TEST_ASSERT(l_str_rchr(l_str("abc"), 'z') == -1, "str_rchr not found");

    // l_str_find
    TEST_ASSERT(l_str_find(l_str("hello world"), l_str("world")) == 6, "str_find found");
    TEST_ASSERT(l_str_find(l_str("hello"), l_str("xyz")) == -1, "str_find not found");
    TEST_ASSERT(l_str_find(l_str("hello"), l_str("")) == 0, "str_find empty needle");

    TEST_SECTION_PASS("L_Str slicing");
}

void test_str_arena(void) {
    TEST_FUNCTION("L_Str arena ops");

    L_Arena a = l_arena_init(4096);

    // l_str_dup
    L_Str orig = l_str("hello");
    L_Str dup = l_str_dup(&a, orig);
    TEST_ASSERT(l_str_eq(dup, orig), "str_dup content matches");
    TEST_ASSERT(dup.data != orig.data, "str_dup pointers differ");

    // l_str_cat
    L_Str cat = l_str_cat(&a, l_str("foo"), l_str("bar"));
    TEST_ASSERT(cat.len == 6, "str_cat len");
    TEST_ASSERT(l_str_eq(cat, l_str("foobar")), "str_cat content");

    // l_str_cstr
    L_Str src = l_str("test");
    char *cs = l_str_cstr(&a, src);
    TEST_ASSERT(l_strcmp(cs, "test") == 0, "str_cstr null-terminated");
    TEST_ASSERT(cs[4] == '\0', "str_cstr explicit NUL");

    // l_str_from_cstr
    const char *orig_cstr = "arena copy";
    L_Str fc = l_str_from_cstr(&a, orig_cstr);
    TEST_ASSERT(l_str_eq(fc, l_str("arena copy")), "str_from_cstr content");
    TEST_ASSERT(fc.data != orig_cstr, "str_from_cstr is a copy");

    l_arena_free(&a);
    TEST_SECTION_PASS("L_Str arena ops");
}

void test_str_split_join(void) {
    TEST_FUNCTION("L_Str split/join");

    L_Arena a = l_arena_init(4096);

    // l_str_split normal
    L_Str *parts;
    int n = l_str_split(&a, l_str("a,b,c"), l_str(","), &parts);
    TEST_ASSERT(n == 3, "split a,b,c count=3");
    TEST_ASSERT(l_str_eq(parts[0], l_str("a")), "split part[0]=a");
    TEST_ASSERT(l_str_eq(parts[1], l_str("b")), "split part[1]=b");
    TEST_ASSERT(l_str_eq(parts[2], l_str("c")), "split part[2]=c");

    // split no delimiter found
    L_Str *parts2;
    int n2 = l_str_split(&a, l_str("hello"), l_str(","), &parts2);
    TEST_ASSERT(n2 == 1, "split no delim count=1");
    TEST_ASSERT(l_str_eq(parts2[0], l_str("hello")), "split no delim part");

    // split empty string
    L_Str *parts3;
    int n3 = l_str_split(&a, l_str(""), l_str(","), &parts3);
    TEST_ASSERT(n3 == 1, "split empty str count=1");
    TEST_ASSERT(parts3[0].len == 0, "split empty str part len=0");

    // l_str_join
    L_Str jp[3] = { l_str("a"), l_str("b"), l_str("c") };
    L_Str joined = l_str_join(&a, jp, 3, l_str(","));
    TEST_ASSERT(l_str_eq(joined, l_str("a,b,c")), "join a,b,c");

    // join with empty separator
    L_Str joined2 = l_str_join(&a, jp, 3, l_str(""));
    TEST_ASSERT(l_str_eq(joined2, l_str("abc")), "join empty sep");

    l_arena_free(&a);
    TEST_SECTION_PASS("L_Str split/join");
}

void test_str_case(void) {
    TEST_FUNCTION("L_Str case conversion");

    L_Arena a = l_arena_init(4096);

    TEST_ASSERT(l_str_eq(l_str_upper(&a, l_str("hello")), l_str("HELLO")), "upper hello");
    TEST_ASSERT(l_str_eq(l_str_lower(&a, l_str("HELLO")), l_str("hello")), "lower HELLO");
    TEST_ASSERT(l_str_eq(l_str_upper(&a, l_str("hElLo")), l_str("HELLO")), "upper mixed");
    TEST_ASSERT(l_str_eq(l_str_lower(&a, l_str("ABC123")), l_str("abc123")), "lower with digits");

    l_arena_free(&a);
    TEST_SECTION_PASS("L_Str case conversion");
}

void test_str_buf(void) {
    TEST_FUNCTION("L_Str buf helpers");

    L_Buf b;
    l_buf_init(&b);

    // l_buf_push_str
    TEST_ASSERT(l_buf_push_str(&b, l_str("hello")) == 0, "buf_push_str ok");
    TEST_ASSERT(b.len == 5, "buf_push_str len");
    TEST_ASSERT(l_memcmp(b.data, "hello", 5) == 0, "buf_push_str data");

    // l_buf_push_cstr
    TEST_ASSERT(l_buf_push_cstr(&b, " world") == 0, "buf_push_cstr ok");
    TEST_ASSERT(b.len == 11, "buf_push_cstr len");

    // l_buf_push_int
    l_buf_clear(&b);
    TEST_ASSERT(l_buf_push_int(&b, 42) == 0, "buf_push_int 42 ok");
    TEST_ASSERT(l_str_eq(l_buf_as_str(&b), l_str("42")), "buf_push_int 42 content");

    l_buf_clear(&b);
    TEST_ASSERT(l_buf_push_int(&b, -7) == 0, "buf_push_int negative ok");
    TEST_ASSERT(l_str_eq(l_buf_as_str(&b), l_str("-7")), "buf_push_int negative content");

    // l_buf_as_str
    l_buf_clear(&b);
    l_buf_push_cstr(&b, "view");
    L_Str view = l_buf_as_str(&b);
    TEST_ASSERT(view.len == 4, "buf_as_str len");
    TEST_ASSERT(l_memcmp(view.data, "view", 4) == 0, "buf_as_str data");

    l_buf_free(&b);
    TEST_SECTION_PASS("L_Str buf helpers");
}

void test_ansi_helpers(void) {
    TEST_FUNCTION("l_ansi_move / l_ansi_color / ANSI macros");

    char buf[64];

    // l_ansi_move
    l_ansi_move(buf, sizeof(buf), 5, 10);
    TEST_ASSERT(l_strcmp(buf, "\033[5;10H") == 0, "ansi_move 5,10");

    // l_ansi_color fg-only
    l_ansi_color(buf, sizeof(buf), 1, -1);
    TEST_ASSERT(l_strcmp(buf, "\033[31m") == 0, "ansi_color fg-only red");

    // l_ansi_color fg+bg
    l_ansi_color(buf, sizeof(buf), 7, 0);
    TEST_ASSERT(l_strcmp(buf, "\033[37;40m") == 0, "ansi_color fg+bg white on black");

    // ANSI macros
    TEST_ASSERT(l_strcmp(L_ANSI_CLEAR, "\033[2J") == 0, "L_ANSI_CLEAR value");
    TEST_ASSERT(l_strcmp(L_ANSI_RESET, "\033[0m") == 0, "L_ANSI_RESET value");

    TEST_SECTION_PASS("ansi_helpers");
}

void test_map(void) {
    TEST_FUNCTION("L_Map");

    L_Arena a = l_arena_init(4096);
    L_Map m = l_map_init(&a, 16);
    TEST_ASSERT(m.slots != (void *)0, "map init allocates slots");
    TEST_ASSERT(m.len == 0, "map starts empty");

    // Put several key/value pairs (use string literals as values via cast)
    int r;
    r = l_map_put(&m, "apple", 5, (void *)"red");
    TEST_ASSERT(r == 0, "put apple succeeds");
    r = l_map_put(&m, "banana", 6, (void *)"yellow");
    TEST_ASSERT(r == 0, "put banana succeeds");
    r = l_map_put(&m, "grape", 5, (void *)"purple");
    TEST_ASSERT(r == 0, "put grape succeeds");
    TEST_ASSERT(m.len == 3, "map has 3 entries");

    // Get each key and verify values
    char *v1 = (char *)l_map_get(&m, "apple", 5);
    TEST_ASSERT(v1 != (void *)0 && l_strcmp(v1, "red") == 0, "get apple returns red");
    char *v2 = (char *)l_map_get(&m, "banana", 6);
    TEST_ASSERT(v2 != (void *)0 && l_strcmp(v2, "yellow") == 0, "get banana returns yellow");
    char *v3 = (char *)l_map_get(&m, "grape", 5);
    TEST_ASSERT(v3 != (void *)0 && l_strcmp(v3, "purple") == 0, "get grape returns purple");

    // Get non-existent key
    void *v4 = l_map_get(&m, "melon", 5);
    TEST_ASSERT(v4 == (void *)0, "get non-existent key returns NULL");

    // Delete a key
    int dr = l_map_del(&m, "banana", 6);
    TEST_ASSERT(dr == 0, "del banana succeeds");
    TEST_ASSERT(l_map_get(&m, "banana", 6) == (void *)0, "get deleted key returns NULL");
    TEST_ASSERT(m.len == 2, "map has 2 entries after delete");

    // Put duplicate key (overwrite)
    r = l_map_put(&m, "apple", 5, (void *)"green");
    TEST_ASSERT(r == 0, "put duplicate key succeeds");
    char *v5 = (char *)l_map_get(&m, "apple", 5);
    TEST_ASSERT(v5 != (void *)0 && l_strcmp(v5, "green") == 0, "overwritten value is green");
    TEST_ASSERT(m.len == 2, "map length unchanged after overwrite");

    // Verify grape still accessible after banana deletion
    char *v6 = (char *)l_map_get(&m, "grape", 5);
    TEST_ASSERT(v6 != (void *)0 && l_strcmp(v6, "purple") == 0, "grape still accessible after del");

    l_arena_free(&a);
    TEST_SECTION_PASS("L_Map");
}

void test_map_high_load(void) {
    TEST_FUNCTION("L_Map high-load update");

    /*
     * Regression: l_map_put previously checked the 75% load-factor guard
     * before scanning for an existing key, so updating a key in a map at or
     * above 75% capacity returned -1 instead of succeeding.
     *
     * With capacity=16 the check triggers when m->len*4 >= 16*3 = 48,
     * i.e. when m->len >= 12.  Insert 11 items (safely under the limit),
     * then insert the 12th (allowed because we check before this insert),
     * yielding m->len==12.  Now update one of those 12 keys — the old code
     * would reject it; the fixed code must return 0.
     */
    L_Arena a = l_arena_init(8192);
    L_Map m = l_map_init(&a, 16);
    TEST_ASSERT(m.slots != (void *)0, "high-load map init");

    /* keys[] are string literals; their addresses remain stable. */
    const char *keys[] = {
        "k00","k01","k02","k03","k04","k05",
        "k06","k07","k08","k09","k10","k11"
    };
    int n = 12;
    int i;
    for (i = 0; i < n; i++) {
        int r = l_map_put(&m, keys[i], 3, (void *)(uintptr_t)(i + 1));
        TEST_ASSERT(r == 0, "high-load insert succeeds");
    }
    TEST_ASSERT(m.len == 12, "map has 12 entries (75% of 16)");

    /* Update every existing key at full 75% load — must all succeed. */
    for (i = 0; i < n; i++) {
        int r = l_map_put(&m, keys[i], 3, (void *)(uintptr_t)(i + 100));
        TEST_ASSERT(r == 0, "high-load update succeeds (not rejected by capacity check)");
    }

    /* Verify updated values. */
    for (i = 0; i < n; i++) {
        void *v = l_map_get(&m, keys[i], 3);
        TEST_ASSERT(v == (void *)(uintptr_t)(i + 100), "high-load updated value correct");
    }
    TEST_ASSERT(m.len == 12, "map len unchanged after updates");

    l_arena_free(&a);
    TEST_SECTION_PASS("L_Map high-load update");
}

void test_str_split_edge_cases(void) {
    TEST_FUNCTION("l_str_split edge cases");

    L_Arena a = l_arena_init(4096);

    /* Consecutive delimiters produce empty tokens */
    L_Str *parts;
    int n = l_str_split(&a, l_str("a,,b"), l_str(","), &parts);
    TEST_ASSERT(n == 3, "consecutive delims: count=3");
    TEST_ASSERT(l_str_eq(parts[0], l_str("a")),  "consecutive delims: part[0]=a");
    TEST_ASSERT(parts[1].len == 0,                "consecutive delims: part[1] empty");
    TEST_ASSERT(l_str_eq(parts[2], l_str("b")),  "consecutive delims: part[2]=b");

    /* Leading delimiter produces empty first token */
    L_Str *parts2;
    int n2 = l_str_split(&a, l_str(",x"), l_str(","), &parts2);
    TEST_ASSERT(n2 == 2, "leading delim: count=2");
    TEST_ASSERT(parts2[0].len == 0,               "leading delim: part[0] empty");
    TEST_ASSERT(l_str_eq(parts2[1], l_str("x")), "leading delim: part[1]=x");

    /* Trailing delimiter produces empty last token */
    L_Str *parts3;
    int n3 = l_str_split(&a, l_str("y,"), l_str(","), &parts3);
    TEST_ASSERT(n3 == 2, "trailing delim: count=2");
    TEST_ASSERT(l_str_eq(parts3[0], l_str("y")), "trailing delim: part[0]=y");
    TEST_ASSERT(parts3[1].len == 0,               "trailing delim: part[1] empty");

    /* Multi-character delimiter */
    L_Str *parts4;
    int n4 = l_str_split(&a, l_str("a::b::c"), l_str("::"), &parts4);
    TEST_ASSERT(n4 == 3, "multi-char delim: count=3");
    TEST_ASSERT(l_str_eq(parts4[0], l_str("a")), "multi-char delim: part[0]=a");
    TEST_ASSERT(l_str_eq(parts4[1], l_str("b")), "multi-char delim: part[1]=b");
    TEST_ASSERT(l_str_eq(parts4[2], l_str("c")), "multi-char delim: part[2]=c");

    /* Delimiter longer than string */
    L_Str *parts5;
    int n5 = l_str_split(&a, l_str("ab"), l_str("abc"), &parts5);
    TEST_ASSERT(n5 == 1, "delim longer than string: count=1");
    TEST_ASSERT(l_str_eq(parts5[0], l_str("ab")), "delim longer than string: whole string returned");

    /* l_str_join with single element */
    L_Str single[1] = { l_str("hello") };
    L_Str joined_single = l_str_join(&a, single, 1, l_str(","));
    TEST_ASSERT(l_str_eq(joined_single, l_str("hello")), "join single element");

    /* l_str_join with zero elements */
    L_Str joined_zero = l_str_join(&a, single, 0, l_str(","));
    TEST_ASSERT(joined_zero.data == (const char *)0, "join zero elements returns null");

    l_arena_free(&a);
    TEST_SECTION_PASS("l_str_split edge cases");
}

void test_gmtime_strftime(void) {
    TEST_FUNCTION("l_gmtime / l_strftime");

    // Epoch: 1970-01-01 00:00:00 Thursday
    L_Tm t0 = l_gmtime(0);
    TEST_ASSERT(t0.year == 70, "epoch year is 70 (1970)");
    TEST_ASSERT(t0.mon == 0, "epoch month is 0 (January)");
    TEST_ASSERT(t0.mday == 1, "epoch day is 1");
    TEST_ASSERT(t0.hour == 0, "epoch hour is 0");
    TEST_ASSERT(t0.min == 0, "epoch min is 0");
    TEST_ASSERT(t0.sec == 0, "epoch sec is 0");
    TEST_ASSERT(t0.wday == 4, "epoch is Thursday (wday=4)");

    // 1000000000: 2001-09-09 01:46:40 Sunday
    L_Tm t1 = l_gmtime(1000000000);
    TEST_ASSERT(t1.year == 101, "1e9 year is 101 (2001)");
    TEST_ASSERT(t1.mon == 8, "1e9 month is 8 (September)");
    TEST_ASSERT(t1.mday == 9, "1e9 day is 9");
    TEST_ASSERT(t1.hour == 1, "1e9 hour is 1");
    TEST_ASSERT(t1.min == 46, "1e9 min is 46");
    TEST_ASSERT(t1.sec == 40, "1e9 sec is 40");
    TEST_ASSERT(t1.wday == 0, "1e9 is Sunday (wday=0)");

    // strftime with "%Y-%m-%d" on epoch
    char buf[64];
    int len1 = l_strftime(buf, sizeof(buf), "%Y-%m-%d", &t0);
    TEST_ASSERT(len1 > 0, "strftime Y-m-d returns positive length");
    TEST_ASSERT(l_strcmp(buf, "1970-01-01") == 0, "strftime epoch date is 1970-01-01");

    // strftime with "%H:%M:%S" on epoch
    int len2 = l_strftime(buf, sizeof(buf), "%H:%M:%S", &t0);
    TEST_ASSERT(len2 > 0, "strftime H:M:S returns positive length");
    TEST_ASSERT(l_strcmp(buf, "00:00:00") == 0, "strftime epoch time is 00:00:00");

    // strftime with day-of-week
    int len3 = l_strftime(buf, sizeof(buf), "%a", &t0);
    TEST_ASSERT(len3 == 3, "strftime %a returns 3 chars");
    TEST_ASSERT(l_strcmp(buf, "Thu") == 0, "strftime epoch weekday is Thu");

    // strftime on 1e9 date
    int len4 = l_strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &t1);
    TEST_ASSERT(len4 > 0, "strftime 1e9 returns positive length");
    TEST_ASSERT(l_strcmp(buf, "2001-09-09 01:46:40") == 0, "strftime 1e9 full datetime");

    TEST_SECTION_PASS("l_gmtime / l_strftime");
}

static int l_test_tm_equal(L_Tm a, L_Tm b) {
    return a.sec == b.sec &&
           a.min == b.min &&
           a.hour == b.hour &&
           a.mday == b.mday &&
           a.mon == b.mon &&
           a.year == b.year &&
           a.wday == b.wday &&
           a.yday == b.yday;
}

static int l_test_localtime_matches_plausible_offset(long long timestamp) {
    L_Tm local = l_localtime(timestamp);
    for (int minutes = -24 * 60; minutes <= 24 * 60; minutes++) {
        L_Tm shifted = l_gmtime(timestamp + (long long)minutes * 60);
        if (l_test_tm_equal(local, shifted))
            return 1;
    }
    return 0;
}

void test_localtime(void) {
    TEST_FUNCTION("l_localtime");

    TEST_ASSERT(l_test_localtime_matches_plausible_offset(0),
                "localtime(epoch) matches a plausible timezone offset");
    TEST_ASSERT(l_test_localtime_matches_plausible_offset(1000000000LL),
                "localtime(1e9) matches a plausible timezone offset");

#ifndef _WIN32
    {
        const char *old_tz = l_getenv("TZ");
        char old_tz_buf[256];
        int had_old_tz = 0;
        long long ts = 1000000000LL;

        if (old_tz) {
            size_t old_len = l_strlen(old_tz);
            if (old_len >= sizeof(old_tz_buf))
                old_len = sizeof(old_tz_buf) - 1;
            l_memcpy(old_tz_buf, old_tz, old_len);
            old_tz_buf[old_len] = '\0';
            had_old_tz = 1;
        }

        TEST_ASSERT(l_setenv("TZ", "UTC-3") == 0, "setenv updates TZ for localtime test");
        TEST_ASSERT(l_test_tm_equal(l_localtime(ts), l_gmtime(ts + 3 * 3600)),
                    "localtime honors simple TZ offset parsing");

        if (had_old_tz)
            TEST_ASSERT(l_setenv("TZ", old_tz_buf) == 0, "restore original TZ");
        else
            TEST_ASSERT(l_unsetenv("TZ") == 0, "unset temporary TZ");
    }
#endif

    TEST_SECTION_PASS("l_localtime");
}

void test_fnmatch(void) {
    TEST_FUNCTION("l_fnmatch");

    // Wildcard extension matching
    TEST_ASSERT(l_fnmatch("*.c", "hello.c") == 0, "*.c matches hello.c");
    TEST_ASSERT(l_fnmatch("*.c", "hello.h") != 0, "*.c does not match hello.h");

    // Single character wildcard
    TEST_ASSERT(l_fnmatch("test?", "test1") == 0, "test? matches test1");
    TEST_ASSERT(l_fnmatch("test?", "test12") != 0, "test? does not match test12");

    // Character classes
    TEST_ASSERT(l_fnmatch("[abc]", "b") == 0, "[abc] matches b");
    TEST_ASSERT(l_fnmatch("[abc]", "d") != 0, "[abc] does not match d");

    // Universal wildcard
    TEST_ASSERT(l_fnmatch("*", "anything") == 0, "* matches anything");
    TEST_ASSERT(l_fnmatch("*", "") == 0, "* matches empty string");

    // Exact match
    TEST_ASSERT(l_fnmatch("hello", "hello") == 0, "exact match hello");
    TEST_ASSERT(l_fnmatch("hello", "world") != 0, "hello does not match world");

    // Empty pattern vs empty string
    TEST_ASSERT(l_fnmatch("", "") == 0, "empty pattern matches empty string");

    // Complex pattern
    TEST_ASSERT(l_fnmatch("*.tar.gz", "archive.tar.gz") == 0, "*.tar.gz matches");
    TEST_ASSERT(l_fnmatch("*.tar.gz", "archive.tar.bz2") != 0, "*.tar.gz no match .bz2");

    TEST_SECTION_PASS("l_fnmatch");
}

void test_sha256(void) {
    TEST_FUNCTION("l_sha256");

    unsigned char hash[32];

    // SHA-256 of empty string
    // Expected: e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855
    l_sha256("", 0, hash);
    TEST_ASSERT(hash[0] == 0xe3, "sha256 empty byte 0");
    TEST_ASSERT(hash[1] == 0xb0, "sha256 empty byte 1");
    TEST_ASSERT(hash[2] == 0xc4, "sha256 empty byte 2");
    TEST_ASSERT(hash[3] == 0x42, "sha256 empty byte 3");
    // Check last 4 bytes too
    TEST_ASSERT(hash[28] == 0x78, "sha256 empty byte 28");
    TEST_ASSERT(hash[29] == 0x52, "sha256 empty byte 29");
    TEST_ASSERT(hash[30] == 0xb8, "sha256 empty byte 30");
    TEST_ASSERT(hash[31] == 0x55, "sha256 empty byte 31");

    // SHA-256 of "abc"
    // Expected: ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad
    l_sha256("abc", 3, hash);
    TEST_ASSERT(hash[0] == 0xba, "sha256 abc byte 0");
    TEST_ASSERT(hash[1] == 0x78, "sha256 abc byte 1");
    TEST_ASSERT(hash[2] == 0x16, "sha256 abc byte 2");
    TEST_ASSERT(hash[3] == 0xbf, "sha256 abc byte 3");

    // Test incremental API: hash "abc" in two parts "a" + "bc"
    L_Sha256 ctx;
    unsigned char hash2[32];
    l_sha256_init(&ctx);
    l_sha256_update(&ctx, "a", 1);
    l_sha256_update(&ctx, "bc", 2);
    l_sha256_final(&ctx, hash2);
    TEST_ASSERT(l_memcmp(hash, hash2, 32) == 0, "incremental sha256 matches one-shot");

    TEST_SECTION_PASS("L_Sha256");
}

void test_math(void) {
    TEST_FUNCTION("l_fabs");
    TEST_ASSERT(l_fabs(-3.14) == 3.14, "fabs(-3.14) == 3.14");
    TEST_ASSERT(l_fabs(3.14) == 3.14, "fabs(3.14) == 3.14");
    TEST_ASSERT(l_fabs(0.0) == 0.0, "fabs(0.0) == 0.0");

    TEST_FUNCTION("l_floor");
    TEST_ASSERT(l_floor(2.7) == 2.0, "floor(2.7) == 2.0");
    TEST_ASSERT(l_floor(-2.3) == -3.0, "floor(-2.3) == -3.0");
    TEST_ASSERT(l_floor(5.0) == 5.0, "floor(5.0) == 5.0");

    TEST_FUNCTION("l_ceil");
    TEST_ASSERT(l_ceil(2.3) == 3.0, "ceil(2.3) == 3.0");
    TEST_ASSERT(l_ceil(-2.7) == -2.0, "ceil(-2.7) == -2.0");
    TEST_ASSERT(l_ceil(5.0) == 5.0, "ceil(5.0) == 5.0");

    TEST_FUNCTION("l_fmod");
    TEST_ASSERT(l_fabs(l_fmod(5.5, 2.0) - 1.5) < 1e-10, "fmod(5.5, 2.0) ~= 1.5");
    TEST_ASSERT(l_fabs(l_fmod(10.0, 3.0) - 1.0) < 1e-10, "fmod(10.0, 3.0) ~= 1.0");

    TEST_FUNCTION("l_sqrt");
    TEST_ASSERT(l_fabs(l_sqrt(4.0) - 2.0) < 1e-10, "sqrt(4.0) == 2.0");
    TEST_ASSERT(l_fabs(l_sqrt(2.0) - 1.4142135623730951) < 1e-10, "sqrt(2.0) ~= 1.41421356...");
    TEST_ASSERT(l_sqrt(0.0) == 0.0, "sqrt(0.0) == 0.0");

    TEST_FUNCTION("l_sin");
    TEST_ASSERT(l_fabs(l_sin(0.0)) < 1e-10, "sin(0) == 0");
    TEST_ASSERT(l_fabs(l_sin(L_PI_2) - 1.0) < 1e-10, "sin(pi/2) ~= 1.0");
    TEST_ASSERT(l_fabs(l_sin(L_PI)) < 1e-10, "sin(pi) ~= 0");

    TEST_FUNCTION("l_cos");
    TEST_ASSERT(l_fabs(l_cos(0.0) - 1.0) < 1e-10, "cos(0) == 1.0");
    TEST_ASSERT(l_fabs(l_cos(L_PI) + 1.0) < 1e-10, "cos(pi) ~= -1.0");

    TEST_FUNCTION("l_exp");
    TEST_ASSERT(l_exp(0.0) == 1.0, "exp(0) == 1.0");
    TEST_ASSERT(l_fabs(l_exp(1.0) - L_E) < 1e-10, "exp(1) ~= e");

    TEST_FUNCTION("l_log");
    TEST_ASSERT(l_fabs(l_log(L_E) - 1.0) < 1e-10, "log(e) ~= 1.0");
    TEST_ASSERT(l_fabs(l_log(1.0)) < 1e-10, "log(1) == 0");

    TEST_FUNCTION("l_pow");
    TEST_ASSERT(l_fabs(l_pow(2.0, 10.0) - 1024.0) < 1e-10, "pow(2, 10) == 1024");
    TEST_ASSERT(l_pow(5.0, 0.0) == 1.0, "pow(5, 0) == 1.0");

    TEST_FUNCTION("l_atan2");
    TEST_ASSERT(l_fabs(l_atan2(1.0, 1.0) - L_PI / 4.0) < 1e-10, "atan2(1,1) ~= pi/4");
    TEST_ASSERT(l_fabs(l_atan2(0.0, 1.0)) < 1e-10, "atan2(0,1) ~= 0");

    TEST_SECTION_PASS("math");
}

void test_math_extended(void) {
    TEST_FUNCTION("l_tan");
    TEST_ASSERT(l_fabs(l_tan(0.0)) < 1e-10, "tan(0) == 0");
    TEST_ASSERT(l_fabs(l_tan(L_PI / 4.0) - 1.0) < 1e-10, "tan(pi/4) ~= 1.0");
    TEST_ASSERT(l_fabs(l_tan(L_PI)) < 1e-8, "tan(pi) ~= 0");

    TEST_FUNCTION("l_asin");
    TEST_ASSERT(l_fabs(l_asin(0.0)) < 1e-10, "asin(0) == 0");
    TEST_ASSERT(l_fabs(l_asin(1.0) - L_PI_2) < 1e-10, "asin(1) ~= pi/2");
    TEST_ASSERT(l_fabs(l_asin(-1.0) + L_PI_2) < 1e-10, "asin(-1) ~= -pi/2");
    TEST_ASSERT(l_fabs(l_asin(0.5) - 0.5235987755982988) < 1e-10, "asin(0.5) ~= pi/6");

    TEST_FUNCTION("l_acos");
    TEST_ASSERT(l_fabs(l_acos(1.0)) < 1e-10, "acos(1) == 0");
    TEST_ASSERT(l_fabs(l_acos(0.0) - L_PI_2) < 1e-10, "acos(0) ~= pi/2");
    TEST_ASSERT(l_fabs(l_acos(-1.0) - L_PI) < 1e-10, "acos(-1) ~= pi");

    TEST_FUNCTION("l_atan");
    TEST_ASSERT(l_fabs(l_atan(0.0)) < 1e-10, "atan(0) == 0");
    TEST_ASSERT(l_fabs(l_atan(1.0) - L_PI / 4.0) < 1e-10, "atan(1) ~= pi/4");
    TEST_ASSERT(l_fabs(l_atan(-1.0) + L_PI / 4.0) < 1e-10, "atan(-1) ~= -pi/4");

    TEST_FUNCTION("l_log10");
    TEST_ASSERT(l_fabs(l_log10(1.0)) < 1e-10, "log10(1) == 0");
    TEST_ASSERT(l_fabs(l_log10(10.0) - 1.0) < 1e-10, "log10(10) ~= 1.0");
    TEST_ASSERT(l_fabs(l_log10(100.0) - 2.0) < 1e-10, "log10(100) ~= 2.0");
    TEST_ASSERT(l_fabs(l_log10(1000.0) - 3.0) < 1e-10, "log10(1000) ~= 3.0");

    TEST_FUNCTION("l_log2");
    TEST_ASSERT(l_fabs(l_log2(1.0)) < 1e-10, "log2(1) == 0");
    TEST_ASSERT(l_fabs(l_log2(2.0) - 1.0) < 1e-10, "log2(2) ~= 1.0");
    TEST_ASSERT(l_fabs(l_log2(8.0) - 3.0) < 1e-10, "log2(8) ~= 3.0");
    TEST_ASSERT(l_fabs(l_log2(1024.0) - 10.0) < 1e-10, "log2(1024) ~= 10.0");

    TEST_FUNCTION("l_round");
    TEST_ASSERT(l_round(2.3) == 2.0, "round(2.3) == 2.0");
    TEST_ASSERT(l_round(2.7) == 3.0, "round(2.7) == 3.0");
    TEST_ASSERT(l_round(2.5) == 3.0, "round(2.5) == 3.0");
    TEST_ASSERT(l_round(-2.3) == -2.0, "round(-2.3) == -2.0");
    TEST_ASSERT(l_round(-2.7) == -3.0, "round(-2.7) == -3.0");
    TEST_ASSERT(l_round(-2.5) == -3.0, "round(-2.5) == -3.0");
    TEST_ASSERT(l_round(0.0) == 0.0, "round(0) == 0");

    TEST_FUNCTION("l_trunc");
    TEST_ASSERT(l_trunc(2.9) == 2.0, "trunc(2.9) == 2.0");
    TEST_ASSERT(l_trunc(-2.9) == -2.0, "trunc(-2.9) == -2.0");
    TEST_ASSERT(l_trunc(0.0) == 0.0, "trunc(0) == 0");
    TEST_ASSERT(l_trunc(5.0) == 5.0, "trunc(5.0) == 5.0");

    TEST_FUNCTION("l_hypot");
    TEST_ASSERT(l_fabs(l_hypot(3.0, 4.0) - 5.0) < 1e-10, "hypot(3,4) ~= 5.0");
    TEST_ASSERT(l_fabs(l_hypot(0.0, 5.0) - 5.0) < 1e-10, "hypot(0,5) ~= 5.0");
    TEST_ASSERT(l_fabs(l_hypot(5.0, 0.0) - 5.0) < 1e-10, "hypot(5,0) ~= 5.0");
    TEST_ASSERT(l_fabs(l_hypot(1.0, 1.0) - l_sqrt(2.0)) < 1e-10, "hypot(1,1) ~= sqrt(2)");

    TEST_SECTION_PASS("math extended");
}

void test_mktime(void) {
    TEST_FUNCTION("l_mktime");

    /* Unix epoch: 1970-01-01 00:00:00 UTC */
    {
        L_Tm tm = {0};
        tm.year = 70; tm.mon = 0; tm.mday = 1;
        tm.hour = 0; tm.min = 0; tm.sec = 0;
        long long ts = l_mktime(&tm);
        TEST_ASSERT(ts == 0, "mktime epoch == 0");
    }
    /* 2000-01-01 00:00:00 UTC = 946684800 */
    {
        L_Tm tm = {0};
        tm.year = 100; tm.mon = 0; tm.mday = 1;
        long long ts = l_mktime(&tm);
        TEST_ASSERT(ts == 946684800LL, "mktime 2000-01-01");
    }
    /* 2024-02-29 12:30:45 UTC (leap year) */
    {
        L_Tm tm = {0};
        tm.year = 124; tm.mon = 1; tm.mday = 29;
        tm.hour = 12; tm.min = 30; tm.sec = 45;
        long long ts = l_mktime(&tm);
        TEST_ASSERT(ts == 1709209845LL, "mktime 2024-02-29 12:30:45");
    }
    /* Roundtrip: mktime(gmtime(ts)) == ts */
    {
        long long test_times[] = {0, 86400, 946684800LL, 1609459200LL, 1700000000LL, -86400};
        for (int i = 0; i < 6; i++) {
            L_Tm tm = l_gmtime(test_times[i]);
            long long back = l_mktime(&tm);
            TEST_ASSERT(back == test_times[i], "mktime/gmtime roundtrip");
        }
    }
    /* 1969-12-31 23:59:59 UTC = -1 */
    {
        L_Tm tm = {0};
        tm.year = 69; tm.mon = 11; tm.mday = 31;
        tm.hour = 23; tm.min = 59; tm.sec = 59;
        long long ts = l_mktime(&tm);
        TEST_ASSERT(ts == -1, "mktime pre-epoch -1");
    }

    TEST_SECTION_PASS("l_mktime");
}

void test_strtof(void) {
    TEST_FUNCTION("l_strtof");
    char *ep;

    /* Basic positive */
    TEST_ASSERT(l_strtof("0", (char **)0) == 0.0f, "strtof '0'");
    TEST_ASSERT(l_strtof("1", (char **)0) == 1.0f, "strtof '1'");
    {
        float v = l_strtof("3.14", (char **)0);
        TEST_ASSERT(v > 3.13f && v < 3.15f, "strtof '3.14'");
    }
    TEST_ASSERT(l_strtof("100.0", (char **)0) == 100.0f, "strtof '100.0'");

    /* Negative */
    TEST_ASSERT(l_strtof("-1", (char **)0) == -1.0f, "strtof '-1'");
    {
        float v = l_strtof("-3.14", (char **)0);
        TEST_ASSERT(v < -3.13f && v > -3.15f, "strtof '-3.14'");
    }

    /* Leading sign */
    TEST_ASSERT(l_strtof("+2.5", (char **)0) == 2.5f, "strtof '+2.5'");

    /* Leading whitespace */
    TEST_ASSERT(l_strtof("  42.0", (char **)0) == 42.0f, "strtof leading spaces");

    /* Exponent */
    TEST_ASSERT(l_strtof("1e3", (char **)0) == 1000.0f, "strtof '1e3'");
    TEST_ASSERT(l_strtof("1.5e2", (char **)0) == 150.0f, "strtof '1.5e2'");
    {
        float v = l_strtof("1E-1", (char **)0);
        TEST_ASSERT(v > 0.09f && v < 0.11f, "strtof '1E-1'");
    }
    TEST_ASSERT(l_strtof("2.5e+1", (char **)0) == 25.0f, "strtof '2.5e+1'");
    TEST_ASSERT(l_strtof("-1e2", (char **)0) == -100.0f, "strtof '-1e2'");

    /* Fractional without leading digit */
    TEST_ASSERT(l_strtof(".5", (char **)0) == 0.5f, "strtof '.5'");

    /* endptr */
    ep = (char *)0;
    l_strtof("3.14xyz", &ep);
    TEST_ASSERT(ep != (char *)0 && ep[0] == 'x', "strtof endptr stops at non-numeric");

    ep = (char *)0;
    l_strtof("nope", &ep);
    TEST_ASSERT(ep != (char *)0 && ep[0] == 'n', "strtof no digits -> endptr at start");

    /* Special: inf */
    {
        float inf_val = l_strtof("inf", (char **)0);
        TEST_ASSERT(inf_val > 3.4e38f, "strtof 'inf' is infinity");
        float ninf_val = l_strtof("-inf", (char **)0);
        TEST_ASSERT(ninf_val < -3.4e38f, "strtof '-inf' is negative infinity");
    }

    /* Special: nan */
    {
        float nan_val = l_strtof("nan", (char **)0);
        TEST_ASSERT(nan_val != nan_val, "strtof 'nan' is NaN");
    }

    TEST_SECTION_PASS("l_strtof");
}

void test_str_replace_helper(void) {
    TEST_FUNCTION("l_str_replace");

    L_Arena a = l_arena_init(4096);

    // Basic replace
    L_Str result = l_str_replace(&a, l_str("hello there hello"),
                                  l_str("hello"), l_str("world"));
    TEST_ASSERT(l_str_eq(result, l_str("world there world")), "replace hello->world");

    // Replace with empty string (deletion)
    L_Str result2 = l_str_replace(&a, l_str("aXbXcX"), l_str("X"), l_str(""));
    TEST_ASSERT(l_str_eq(result2, l_str("abc")), "replace with empty (delete)");

    // No match returns original content
    L_Str result3 = l_str_replace(&a, l_str("no match here"), l_str("xyz"), l_str("abc"));
    TEST_ASSERT(l_str_eq(result3, l_str("no match here")), "replace no match");

    // Replace at start
    L_Str result4 = l_str_replace(&a, l_str("AAtest"), l_str("AA"), l_str("BB"));
    TEST_ASSERT(l_str_eq(result4, l_str("BBtest")), "replace at start");

    // Replace at end
    L_Str result5 = l_str_replace(&a, l_str("testAA"), l_str("AA"), l_str("BB"));
    TEST_ASSERT(l_str_eq(result5, l_str("testBB")), "replace at end");

    // Replace with longer string
    L_Str result6 = l_str_replace(&a, l_str("ab"), l_str("a"), l_str("xyz"));
    TEST_ASSERT(l_str_eq(result6, l_str("xyzb")), "replace with longer");

    l_arena_free(&a);
    TEST_SECTION_PASS("l_str_replace");
}

// Comparator for large structs — compares by the first int field
typedef struct { int key; char pad[512]; } BigElem;
static int big_elem_cmp(const void *a, const void *b) {
    int ka = ((const BigElem *)a)->key;
    int kb = ((const BigElem *)b)->key;
    return (ka > kb) - (ka < kb);
}

void test_qsort_large_element(void) {
    TEST_FUNCTION("l_qsort large elements (>256 bytes)");

    BigElem arr[5];
    int keys[] = {5, 3, 1, 4, 2};
    for (int i = 0; i < 5; i++) {
        l_memset(&arr[i], 0, sizeof(BigElem));
        arr[i].key = keys[i];
        l_memset(arr[i].pad, (char)('A' + i), sizeof(arr[i].pad));
    }

    l_qsort(arr, 5, sizeof(BigElem), big_elem_cmp);

    TEST_ASSERT(arr[0].key == 1, "large elem sort [0]==1");
    TEST_ASSERT(arr[1].key == 2, "large elem sort [1]==2");
    TEST_ASSERT(arr[2].key == 3, "large elem sort [2]==3");
    TEST_ASSERT(arr[3].key == 4, "large elem sort [3]==4");
    TEST_ASSERT(arr[4].key == 5, "large elem sort [4]==5");

    // Verify padding survived intact (originally key=1 was arr[2], pad='C')
    TEST_ASSERT(arr[0].pad[0] == 'C', "large elem pad preserved for key 1");
    TEST_ASSERT(arr[4].pad[0] == 'A', "large elem pad preserved for key 5");

    TEST_SECTION_PASS("l_qsort large elements");
}

void test_rand_ctx(void) {
    TEST_FUNCTION("l_rand_ctx (independent streams)");

    // Two contexts with same seed produce same sequence
    L_RandCtx a, b;
    l_rand_ctx_init(&a, 42);
    l_rand_ctx_init(&b, 42);
    TEST_ASSERT(l_rand_ctx(&a) == l_rand_ctx(&b), "same seed same first value");
    TEST_ASSERT(l_rand_ctx(&a) == l_rand_ctx(&b), "same seed same second value");

    // Different seeds produce different sequences
    L_RandCtx c;
    l_rand_ctx_init(&c, 99);
    l_rand_ctx_init(&a, 42);
    TEST_ASSERT(l_rand_ctx(&a) != l_rand_ctx(&c), "different seeds differ");

    // Context is independent from global state
    l_srand(42);
    unsigned int global_val = l_rand();
    l_rand_ctx_init(&a, 42);
    unsigned int ctx_val = l_rand_ctx(&a);
    TEST_ASSERT(global_val == ctx_val, "ctx matches global with same seed");

    // Advancing one doesn't affect the other
    l_srand(42);
    l_rand_ctx_init(&a, 42);
    l_rand();  // advance global
    l_rand();  // advance global again
    unsigned int ctx_first = l_rand_ctx(&a);  // ctx still at position 1
    l_srand(42);
    unsigned int global_first = l_rand();
    TEST_ASSERT(ctx_first == global_first, "ctx unaffected by global advances");

    // l_srand_ctx reseeds
    l_rand_ctx_init(&a, 1);
    l_rand_ctx(&a);
    l_srand_ctx(&a, 42);
    l_rand_ctx_init(&b, 42);
    TEST_ASSERT(l_rand_ctx(&a) == l_rand_ctx(&b), "reseed produces same sequence");

    TEST_SECTION_PASS("l_rand_ctx");
}

void test_getopt_ctx(void) {
    TEST_FUNCTION("l_getopt_ctx (nested parsing)");

    // Basic option parsing
    {
        L_GetoptCtx ctx;
        l_getopt_ctx_init(&ctx);
        char *args[] = { "prog", "-v", "-n", "42", NULL };
        int c1 = l_getopt_ctx(&ctx, 4, args, "vn:");
        TEST_ASSERT(c1 == 'v', "first option is v");
        int c2 = l_getopt_ctx(&ctx, 4, args, "vn:");
        TEST_ASSERT(c2 == 'n', "second option is n");
        TEST_ASSERT(ctx.optarg != NULL && l_strcmp(ctx.optarg, "42") == 0, "optarg is 42");
        int c3 = l_getopt_ctx(&ctx, 4, args, "vn:");
        TEST_ASSERT(c3 == -1, "done after all options");
    }

    // Unknown option
    {
        L_GetoptCtx ctx;
        l_getopt_ctx_init(&ctx);
        char *args[] = { "prog", "-x", NULL };
        int c = l_getopt_ctx(&ctx, 2, args, "vn:");
        TEST_ASSERT(c == '?', "unknown option returns ?");
        TEST_ASSERT(ctx.optopt == 'x', "optopt is the unknown char");
    }

    // Nested parsing — two independent contexts
    {
        L_GetoptCtx outer, inner;
        l_getopt_ctx_init(&outer);
        l_getopt_ctx_init(&inner);

        char *outer_args[] = { "shell", "-v", "subcmd", "-n", "5", NULL };
        int c = l_getopt_ctx(&outer, 5, outer_args, "v");
        TEST_ASSERT(c == 'v', "outer parses -v");
        c = l_getopt_ctx(&outer, 5, outer_args, "v");
        TEST_ASSERT(c == -1, "outer done at non-option");

        // Inner parses remaining args
        int sub_argc = 5 - outer.optind;
        char **sub_argv = outer_args + outer.optind;
        c = l_getopt_ctx(&inner, sub_argc, sub_argv, "n:");
        TEST_ASSERT(c == 'n', "inner parses -n from subcommand");
        TEST_ASSERT(inner.optarg != NULL && l_strcmp(inner.optarg, "5") == 0,
                    "inner optarg is 5");

        // Outer state was not clobbered
        TEST_ASSERT(outer.optind == 2, "outer optind unchanged after inner parse");
    }

    // Grouped short options
    {
        L_GetoptCtx ctx;
        l_getopt_ctx_init(&ctx);
        char *args[] = { "prog", "-vf", NULL };
        int c1 = l_getopt_ctx(&ctx, 2, args, "vf");
        TEST_ASSERT(c1 == 'v', "grouped first is v");
        int c2 = l_getopt_ctx(&ctx, 2, args, "vf");
        TEST_ASSERT(c2 == 'f', "grouped second is f");
    }

    // -- stops parsing
    {
        L_GetoptCtx ctx;
        l_getopt_ctx_init(&ctx);
        char *args[] = { "prog", "--", "-v", NULL };
        int c = l_getopt_ctx(&ctx, 3, args, "v");
        TEST_ASSERT(c == -1, "-- stops option parsing");
        TEST_ASSERT(ctx.optind == 2, "optind past --");
    }

    TEST_SECTION_PASS("l_getopt_ctx");
}

int main(int argc, char *argv[]) {
    l_getenv_init(argc, argv);
    test_qsort_large_element();
    test_time();
    test_arena();
    test_buf();
    test_str_constructors();
    test_str_comparison();
    test_str_slicing();
    test_str_arena();
    test_str_split_join();
    test_str_case();
    test_str_buf();
    test_ansi_helpers();
    test_map();
    test_map_high_load();
    test_str_split_edge_cases();
    test_gmtime_strftime();
    test_localtime();
    test_fnmatch();
    test_sha256();
    test_math();
    test_math_extended();
    test_mktime();
    test_strtof();
    test_str_replace_helper();

    // Context variant tests
    test_rand_ctx();
    test_getopt_ctx();

    l_test_print_summary(passed_count, test_count);
    puts("PASS\n");
    return 0;
}
