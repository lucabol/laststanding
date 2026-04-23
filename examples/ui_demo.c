#define L_WITHSNPRINTF
#define L_MAINFILE
#include "l_ui.h"

// ui_demo — Full widget showcase for l_ui.h
// Usage: ui_demo
// Exit: press Escape

#define WIN_W 640
#define WIN_H 480

int main(int argc, char *argv[]) {
    l_getenv_init(argc, argv);

    L_Canvas canvas;
    if (l_canvas_open(&canvas, WIN_W, WIN_H, "UI Demo") != 0) {
        puts("Could not open canvas\n");
        return 1;
    }
    int s = canvas.scale;  // HiDPI multiplier for hardcoded coords.
    int cw = canvas.width, ch = canvas.height;

    L_UI ui;
    l_ui_init(&ui);  // font_scale auto-picked from canvas.scale in l_ui_begin.

    int click_count = 0;
    int checked = 0;
    int volume = 50;
    char name[L_UI_MAX_TEXT];
    name[0] = '\0';

    while (l_canvas_alive(&canvas)) {
        l_ui_begin(&ui, &canvas);
        l_canvas_clear(&canvas, L_RGB(30, 30, 34));

        // Escape to exit
        if (ui.key == 27) break;

        // Background panel
        l_ui_panel(&ui, 10 * s, 10 * s, cw - 20 * s, ch - 20 * s);

        // Title (scale 2x font relative to UI base scale)
        {
            int saved = ui.font_scale;
            ui.font_scale = 2 * s;
            l_ui__draw_text(&canvas, 24 * s, 20 * s, "UI Widget Demo",
                            ui.theme.accent, 2 * s);
            ui.font_scale = saved;
        }

        // Column layout for widgets
        l_ui_column_begin(&ui, 30 * s, 56 * s, 6 * s);

        // Label
        {
            int y = l_ui_next(&ui, 10 * s);
            l_ui_label(&ui, ui.layout_x, y, "A simple label");
        }

        // Buttons row
        {
            int y = l_ui_next(&ui, 24 * s);
            if (l_ui_button(&ui, 30 * s, y, 100 * s, 24 * s, "Click Me"))
                click_count++;
            if (l_ui_button(&ui, 140 * s, y, 100 * s, 24 * s, "Reset")) {
                click_count = 0;
                checked = 0;
                volume = 50;
                name[0] = '\0';
            }
        }

        // Checkbox
        {
            int y = l_ui_next(&ui, 16 * s);
            l_ui_checkbox(&ui, 30 * s, y, "Enable feature", &checked);
        }

        // Slider with label
        {
            int y = l_ui_next(&ui, 8 * s);
            l_ui_label(&ui, 30 * s, y, "Volume:");
            y = l_ui_next(&ui, 16 * s);
            l_ui_slider(&ui, 30 * s, y, 200 * s, &volume, 0, 100);
        }

        // Textbox with label
        {
            int y = l_ui_next(&ui, 8 * s);
            l_ui_label(&ui, 30 * s, y, "Name:");
            y = l_ui_next(&ui, 20 * s);
            l_ui_textbox(&ui, 30 * s, y, 200 * s, name, L_UI_MAX_TEXT);
        }

        // Separator
        {
            int y = l_ui_next(&ui, 6 * s);
            l_ui_separator(&ui, 30 * s, y + 2 * s, cw - 80 * s);
        }

        // Status display
        {
            int y = l_ui_next(&ui, 10 * s);
            char status[128];

            l_snprintf(status, sizeof(status), "Clicks: %d", click_count);
            l_ui_label(&ui, 30 * s, y, status);

            y = l_ui_next(&ui, 10 * s);
            l_snprintf(status, sizeof(status), "Checkbox: %s", checked ? "ON" : "OFF");
            l_ui_label(&ui, 30 * s, y, status);

            y = l_ui_next(&ui, 10 * s);
            l_snprintf(status, sizeof(status), "Volume: %d", volume);
            l_ui_label(&ui, 30 * s, y, status);

            y = l_ui_next(&ui, 10 * s);
            l_snprintf(status, sizeof(status), "Name: %s", name);
            l_ui_label(&ui, 30 * s, y, status);
        }

        l_ui_layout_end(&ui);

        l_ui_end(&ui);
        l_canvas_flush(&canvas);
    }

    l_canvas_close(&canvas);
    return 0;
}
