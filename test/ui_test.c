#define L_MAINFILE
#include "l_ui.h"

static int test_count = 0;
#define TEST_FUNCTION(name) puts("  Testing " name "...\n")
#define TEST_ASSERT(cond, msg) do { test_count++; if (!(cond)) { puts("    [FAIL] " msg "\n"); exit(-1); } else { puts("    [OK] " msg "\n"); } } while(0)

// In-memory canvas for widget tests (no display needed)
static uint32_t test_pixels[64 * 64];
static L_Canvas test_canvas = { .width = 64, .height = 64, .stride = 64 * 4, .pixels = test_pixels };

// Helper: set up UI with simulated input (bypasses l_ui_begin which calls l_canvas_mouse/key)
static void sim_begin(L_UI *ui, int mx, int my, int mbtn, int key) {
    ui->canvas = &test_canvas;
    ui->mouse_btn_prev = ui->mouse_btn;
    ui->mouse_x = mx;
    ui->mouse_y = my;
    ui->mouse_btn = mbtn;
    ui->key = key;
    ui->hot = 0;
    ui->tab_pressed = (key == 9);
    ui->enter_pressed = (key == 13);
    ui->cursor_blink++;
}

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

    // ---- Widget tests (using simulated canvas) ----

    // Button: no click when mouse is outside
    TEST_FUNCTION("l_ui_button");
    l_ui_init(&ui);
    l_canvas_clear(&test_canvas, L_BLACK);
    sim_begin(&ui, 0, 0, 0, 0);  // mouse away, no button
    int r = l_ui_button(&ui, 10, 10, 30, 20, "OK");
    TEST_ASSERT(r == 0, "button not clicked when mouse outside");

    // Button: mouse hovers, sets hot
    l_ui_init(&ui);
    l_canvas_clear(&test_canvas, L_BLACK);
    sim_begin(&ui, 15, 15, 0, 0);  // hover over button
    l_ui_button(&ui, 10, 10, 30, 20, "OK");
    TEST_ASSERT(ui.hot == l_ui__hash("OK", 10, 10), "button is hot when hovered");

    // Button: click = press + release on button
    l_ui_init(&ui);
    l_canvas_clear(&test_canvas, L_BLACK);
    // Frame 1: press mouse on button
    sim_begin(&ui, 15, 15, 1, 0);
    l_ui_button(&ui, 10, 10, 30, 20, "OK");
    l_ui_end(&ui);
    TEST_ASSERT(ui.active == l_ui__hash("OK", 10, 10), "button active on press");
    // Frame 2: release mouse on button
    sim_begin(&ui, 15, 15, 0, 0);
    r = l_ui_button(&ui, 10, 10, 30, 20, "OK");
    l_ui_end(&ui);
    TEST_ASSERT(r == 1, "button clicked on release");

    // Button: press on button, release outside = no click
    l_ui_init(&ui);
    l_canvas_clear(&test_canvas, L_BLACK);
    sim_begin(&ui, 15, 15, 1, 0);
    l_ui_button(&ui, 10, 10, 30, 20, "OK");
    l_ui_end(&ui);
    sim_begin(&ui, 0, 0, 0, 0);  // release outside
    r = l_ui_button(&ui, 10, 10, 30, 20, "OK");
    l_ui_end(&ui);
    TEST_ASSERT(r == 0, "no click when released outside");

    // Checkbox: toggle on click
    TEST_FUNCTION("l_ui_checkbox");
    l_ui_init(&ui);
    int checked = 0;
    l_canvas_clear(&test_canvas, L_BLACK);
    // Press on checkbox (box is 16x16 at scale 1)
    sim_begin(&ui, 5, 5, 1, 0);
    l_ui_checkbox(&ui, 0, 0, "Check", &checked);
    l_ui_end(&ui);
    // Release on checkbox
    sim_begin(&ui, 5, 5, 0, 0);
    r = l_ui_checkbox(&ui, 0, 0, "Check", &checked);
    l_ui_end(&ui);
    TEST_ASSERT(r == 1, "checkbox toggled returns 1");
    TEST_ASSERT(checked == 1, "checkbox toggled to checked");

    // Toggle again
    sim_begin(&ui, 5, 5, 1, 0);
    l_ui_checkbox(&ui, 0, 0, "Check", &checked);
    l_ui_end(&ui);
    sim_begin(&ui, 5, 5, 0, 0);
    l_ui_checkbox(&ui, 0, 0, "Check", &checked);
    l_ui_end(&ui);
    TEST_ASSERT(checked == 0, "checkbox toggled back to unchecked");

    // Slider: value clamping
    TEST_FUNCTION("l_ui_slider");
    l_ui_init(&ui);
    l_canvas_clear(&test_canvas, L_BLACK);
    int val = -5;
    sim_begin(&ui, 0, 0, 0, 0);
    l_ui_slider(&ui, 0, 0, 60, &val, 0, 100);
    TEST_ASSERT(val == 0, "slider clamps below min");

    val = 200;
    l_ui_slider(&ui, 0, 0, 60, &val, 0, 100);
    TEST_ASSERT(val == 100, "slider clamps above max");

    // Slider: drag updates value
    l_ui_init(&ui);
    val = 0;
    l_canvas_clear(&test_canvas, L_BLACK);
    // Press on slider area
    sim_begin(&ui, 30, 5, 1, 0);
    l_ui_slider(&ui, 0, 0, 60, &val, 0, 100);
    l_ui_end(&ui);
    // Still holding: value should update based on mouse position
    sim_begin(&ui, 30, 5, 1, 0);
    l_ui_slider(&ui, 0, 0, 60, &val, 0, 100);
    TEST_ASSERT(val > 0, "slider value updated by drag");

    // Textbox: character insertion
    TEST_FUNCTION("l_ui_textbox");
    l_ui_init(&ui);
    char buf[32];
    buf[0] = '\0';
    l_canvas_clear(&test_canvas, L_BLACK);
    // Click to focus
    sim_begin(&ui, 5, 5, 1, 0);
    l_ui_textbox(&ui, 0, 0, 60, buf, 32);
    l_ui_end(&ui);
    sim_begin(&ui, 5, 5, 0, 0);
    l_ui_end(&ui);
    // Type 'A'
    sim_begin(&ui, 5, 5, 0, 'A');
    r = l_ui_textbox(&ui, 0, 0, 60, buf, 32);
    TEST_ASSERT(r == 1, "textbox reports change on char insert");
    TEST_ASSERT(buf[0] == 'A' && buf[1] == '\0', "textbox inserts char");

    // Type 'B'
    sim_begin(&ui, 5, 5, 0, 'B');
    l_ui_textbox(&ui, 0, 0, 60, buf, 32);
    TEST_ASSERT(buf[0] == 'A' && buf[1] == 'B' && buf[2] == '\0', "textbox appends char");

    // Backspace
    sim_begin(&ui, 5, 5, 0, 8);
    r = l_ui_textbox(&ui, 0, 0, 60, buf, 32);
    TEST_ASSERT(r == 1, "textbox reports change on backspace");
    TEST_ASSERT(buf[0] == 'A' && buf[1] == '\0', "textbox backspace deletes");

    // Left arrow + insert at middle
    buf[0] = 'A'; buf[1] = 'C'; buf[2] = '\0';
    ui.cursor_pos = 2;
    sim_begin(&ui, 5, 5, 0, 1001);  // left arrow
    l_ui_textbox(&ui, 0, 0, 60, buf, 32);
    TEST_ASSERT(ui.cursor_pos == 1, "textbox left arrow moves cursor");
    sim_begin(&ui, 5, 5, 0, 'B');
    l_ui_textbox(&ui, 0, 0, 60, buf, 32);
    TEST_ASSERT(buf[0] == 'A' && buf[1] == 'B' && buf[2] == 'C' && buf[3] == '\0',
                "textbox inserts at cursor position");

    // Panel and separator (just ensure they don't crash)
    TEST_FUNCTION("l_ui_panel");
    l_ui_init(&ui);
    l_canvas_clear(&test_canvas, L_BLACK);
    sim_begin(&ui, 0, 0, 0, 0);
    r = l_ui_panel(&ui, 0, 0, 50, 50);
    TEST_ASSERT(r == 0, "panel returns 0");
    r = l_ui_separator(&ui, 0, 30, 50);
    TEST_ASSERT(r == 0, "separator returns 0");

    // Layout: column
    TEST_FUNCTION("l_ui_column_begin / l_ui_next");
    l_ui_init(&ui);
    l_ui_column_begin(&ui, 10, 20, 5);
    TEST_ASSERT(ui.layout_active == 1, "layout is active");
    TEST_ASSERT(ui.layout_dir == 1, "layout dir is vertical");
    int y1_pos = l_ui_next(&ui, 24);
    TEST_ASSERT(y1_pos == 20, "first next returns start y");
    TEST_ASSERT(ui.layout_y == 49, "layout_y advanced by size+spacing");
    int y2_pos = l_ui_next(&ui, 16);
    TEST_ASSERT(y2_pos == 49, "second next returns advanced y");
    TEST_ASSERT(ui.layout_y == 70, "layout_y advanced again");
    l_ui_layout_end(&ui);
    TEST_ASSERT(ui.layout_active == 0, "layout ended");

    // Layout: row
    TEST_FUNCTION("l_ui_row_begin / l_ui_next");
    l_ui_init(&ui);
    l_ui_row_begin(&ui, 10, 20, 5);
    TEST_ASSERT(ui.layout_dir == 0, "layout dir is horizontal");
    int x1_pos = l_ui_next(&ui, 50);
    TEST_ASSERT(x1_pos == 10, "first next returns start x");
    TEST_ASSERT(ui.layout_x == 65, "layout_x advanced");
    int x2_pos = l_ui_next(&ui, 30);
    TEST_ASSERT(x2_pos == 65, "second next returns advanced x");
    l_ui_layout_end(&ui);

    puts("All l_ui tests passed\n");
    return 0;
}
