#define L_MAINFILE
#include "l_ui.h"

static int test_count = 0;
#define TEST_FUNCTION(name) puts("  Testing " name "...\n")
#define TEST_ASSERT(cond, msg) do { test_count++; if (!(cond)) { puts("    [FAIL] " msg "\n"); exit(-1); } else { puts("    [OK] " msg "\n"); } } while(0)

int main(int argc, char *argv[]) {
    (void)argc; (void)argv;

    L_UI ui;
    l_ui_init(&ui);

    // Verify theme defaults
    TEST_FUNCTION("l_ui_init");
    TEST_ASSERT(ui.font_scale == 1, "default font scale is 1");
    TEST_ASSERT(ui.theme.fg != 0, "theme fg is set");
    TEST_ASSERT(ui.hot == 0, "hot starts at 0");
    TEST_ASSERT(ui.active == 0, "active starts at 0");
    TEST_ASSERT(ui.focused == 0, "focused starts at 0");

    // Verify hash
    TEST_FUNCTION("l_ui__hash");
    uint32_t h1 = l_ui__hash("button", 10, 20);
    uint32_t h2 = l_ui__hash("button", 10, 20);
    uint32_t h3 = l_ui__hash("button", 11, 20);
    uint32_t h4 = l_ui__hash("other", 10, 20);
    TEST_ASSERT(h1 == h2, "same inputs produce same hash");
    TEST_ASSERT(h1 != h3, "different x produces different hash");
    TEST_ASSERT(h1 != h4, "different label produces different hash");
    TEST_ASSERT(h1 != 0, "hash is never zero");

    // Verify in_rect
    TEST_FUNCTION("l_ui__in_rect");
    TEST_ASSERT(l_ui__in_rect(15, 25, 10, 20, 20, 20) == 1, "point inside rect");
    TEST_ASSERT(l_ui__in_rect(5, 25, 10, 20, 20, 20) == 0, "point left of rect");
    TEST_ASSERT(l_ui__in_rect(30, 25, 10, 20, 20, 20) == 0, "point at right edge");
    TEST_ASSERT(l_ui__in_rect(15, 40, 10, 20, 20, 20) == 0, "point at bottom edge");

    // Verify text metrics
    TEST_FUNCTION("l_ui__text_width");
    TEST_ASSERT(l_ui__text_width("Hello", 1) == 40, "5 chars at scale 1 = 40px");
    TEST_ASSERT(l_ui__text_width("Hello", 2) == 80, "5 chars at scale 2 = 80px");
    TEST_ASSERT(l_ui__text_height(1) == 8, "height at scale 1 = 8px");
    TEST_ASSERT(l_ui__text_height(2) == 16, "height at scale 2 = 16px");

    puts("All l_ui tests passed\n");
    return 0;
}
