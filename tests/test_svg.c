#define L_MAINFILE
#include "l_svg.h"
#include "test_support.h"

TEST_DECLARE_COUNTERS();

static const unsigned char svg_center_red_rect[] =
    "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"4\" height=\"4\">"
    "<rect x=\"1\" y=\"1\" width=\"2\" height=\"2\" fill=\"red\"/>"
    "</svg>";

static const unsigned char svg_viewbox_red_rect[] =
    "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 10 20\">"
    "<rect x=\"0\" y=\"0\" width=\"10\" height=\"20\" fill=\"#ff0000\"/>"
    "</svg>";

static const unsigned char svg_gradient_rect[] =
    "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"8\" height=\"1\" viewBox=\"0 0 8 1\">"
    "<defs>"
    "<linearGradient id=\"g\" x1=\"0%\" y1=\"0%\" x2=\"100%\" y2=\"0%\">"
    "<stop offset=\"0%\" stop-color=\"red\"/>"
    "<stop offset=\"100%\" stop-color=\"blue\"/>"
    "</linearGradient>"
    "</defs>"
    "<rect x=\"0\" y=\"0\" width=\"8\" height=\"1\" fill=\"url(#g)\"/>"
    "</svg>";

static inline unsigned int test_svg_red(uint32_t pixel) { return (pixel >> 16) & 0xFFu; }
static inline unsigned int test_svg_green(uint32_t pixel) { return (pixel >> 8) & 0xFFu; }
static inline unsigned int test_svg_blue(uint32_t pixel) { return pixel & 0xFFu; }

static void test_svg_invalid_data(void) {
    unsigned char garbage[] = { 0x00, 0x01, 0x02, 0x03 };
    int w = 0, h = 0;
    uint32_t *pixels;

    TEST_FUNCTION("svg invalid input");
    pixels = l_svg_load_mem(garbage, (int)sizeof(garbage), 0, &w, &h);
    TEST_CHECK(pixels == 0, "garbage data returns NULL");
}

static void test_svg_null_empty(void) {
    int w = 0, h = 0;
    uint32_t *pixels;

    TEST_FUNCTION("svg null and empty input");
    pixels = l_svg_load_mem(0, 0, 0, &w, &h);
    TEST_CHECK(pixels == 0, "NULL data returns NULL");
    pixels = l_svg_load_mem((const unsigned char *)"", 0, 0, &w, &h);
    TEST_CHECK(pixels == 0, "empty input returns NULL");
}

static void test_svg_explicit_size(void) {
    L_SvgOptions opts = { 16, 16, 96.0f };
    int w = 0, h = 0;
    uint32_t *pixels;

    TEST_FUNCTION("svg explicit sizing");
    pixels = l_svg_load_mem(svg_center_red_rect, (int)sizeof(svg_center_red_rect) - 1, &opts, &w, &h);
    TEST_CHECK(pixels != 0, "explicit size decode succeeds");
    TEST_CHECK(w == 16, "explicit width is honored");
    TEST_CHECK(h == 16, "explicit height is honored");
    if (pixels) {
        uint32_t center = pixels[(h / 2) * w + (w / 2)];
        TEST_CHECK(test_svg_red(center) == 255, "explicit size keeps red fill");
        TEST_CHECK(test_svg_green(center) == 0, "explicit size keeps green channel zero");
        TEST_CHECK(test_svg_blue(center) == 0, "explicit size keeps blue channel zero");
        l_svg_free_pixels(pixels, w, h);
    }
}

static void test_svg_viewbox_intrinsic(void) {
    int w = 0, h = 0;
    uint32_t *pixels;

    TEST_FUNCTION("svg viewBox intrinsic sizing");
    pixels = l_svg_load_mem(svg_viewbox_red_rect, (int)sizeof(svg_viewbox_red_rect) - 1, 0, &w, &h);
    TEST_CHECK(pixels != 0, "viewBox-only decode succeeds");
    TEST_CHECK(w == 10, "viewBox width becomes intrinsic width");
    TEST_CHECK(h == 20, "viewBox height becomes intrinsic height");
    if (pixels) {
        uint32_t center = pixels[(h / 2) * w + (w / 2)];
        TEST_CHECK(test_svg_red(center) == 255, "viewBox render keeps red fill");
        l_svg_free_pixels(pixels, w, h);
    }
}

static void test_svg_aspect_ratio(void) {
    L_SvgOptions opts = { 0, 40, 96.0f };
    int w = 0, h = 0;
    uint32_t *pixels;

    TEST_FUNCTION("svg inferred aspect ratio");
    pixels = l_svg_load_mem(svg_viewbox_red_rect, (int)sizeof(svg_viewbox_red_rect) - 1, &opts, &w, &h);
    TEST_CHECK(pixels != 0, "aspect-ratio decode succeeds");
    TEST_CHECK(w == 20, "width is inferred from aspect ratio");
    TEST_CHECK(h == 40, "requested height is honored");
    if (pixels) l_svg_free_pixels(pixels, w, h);
}

static void test_svg_gradient(void) {
    int w = 0, h = 0;
    uint32_t *pixels;

    TEST_FUNCTION("svg linear gradient");
    pixels = l_svg_load_mem(svg_gradient_rect, (int)sizeof(svg_gradient_rect) - 1, 0, &w, &h);
    TEST_CHECK(pixels != 0, "gradient decode succeeds");
    TEST_CHECK(w == 8, "gradient width is 8");
    TEST_CHECK(h == 1, "gradient height is 1");
    if (pixels) {
        uint32_t left = pixels[0];
        uint32_t right = pixels[w - 1];
        TEST_CHECK(left != right, "gradient endpoints differ");
        TEST_CHECK(test_svg_red(left) > test_svg_red(right), "left side is redder than right");
        TEST_CHECK(test_svg_blue(right) > test_svg_blue(left), "right side is bluer than left");
        l_svg_free_pixels(pixels, w, h);
    }
}

int main(int argc, char *argv[]) {
    l_getenv_init(argc, argv);
    puts("Testing l_svg.h SVG decoding...\n");

    test_svg_invalid_data();
    test_svg_null_empty();
    test_svg_explicit_size();
    test_svg_viewbox_intrinsic();
    test_svg_aspect_ratio();
    test_svg_gradient();

    l_test_print_summary(passed_count, test_count);
    if (passed_count != test_count) {
        puts("FAIL\n");
        exit(1);
    }
    puts("PASS\n");
    return 0;
}
