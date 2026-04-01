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
    L_Str fc = l_str_from_cstr(&a, "arena copy");
    TEST_ASSERT(l_str_eq(fc, l_str("arena copy")), "str_from_cstr content");
    TEST_ASSERT(fc.data != "arena copy", "str_from_cstr is a copy");

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

int main(int argc, char *argv[]) {
    l_getenv_init(argc, argv);
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
    test_gmtime_strftime();
    test_localtime();
    test_fnmatch();
    test_sha256();
    test_math();
    test_str_replace_helper();

    l_test_print_summary(passed_count, test_count);
    puts("PASS\n");
    return 0;
}
