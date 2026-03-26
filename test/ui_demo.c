#define L_MAINFILE
#include "l_ui.h"

// ui_demo — Full widget showcase for l_ui.h
// Usage: ui_demo
// Exit: press Escape

#define WIN_W 640
#define WIN_H 480

int main(int argc, char *argv[]) {
    (void)argc; (void)argv;

    L_Canvas canvas;
    if (l_canvas_open(&canvas, WIN_W, WIN_H, "UI Demo") != 0) {
        puts("Could not open canvas\n");
        return 1;
    }

    L_UI ui;
    l_ui_init(&ui);
    ui.font_scale = 1;

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
        l_ui_panel(&ui, 10, 10, WIN_W - 20, WIN_H - 20);

        // Title (scale 2)
        {
            int saved = ui.font_scale;
            ui.font_scale = 2;
            l_ui__draw_text(&canvas, 24, 20, "UI Widget Demo",
                            ui.theme.accent, 2);
            ui.font_scale = saved;
        }

        // Column layout for widgets
        l_ui_column_begin(&ui, 30, 56, 6);

        // Label
        {
            int y = l_ui_next(&ui, 10);
            l_ui_label(&ui, ui.layout_x, y, "A simple label");
        }

        // Buttons row
        {
            int y = l_ui_next(&ui, 24);
            if (l_ui_button(&ui, 30, y, 100, 24, "Click Me"))
                click_count++;
            if (l_ui_button(&ui, 140, y, 100, 24, "Reset")) {
                click_count = 0;
                checked = 0;
                volume = 50;
                name[0] = '\0';
            }
        }

        // Checkbox
        {
            int y = l_ui_next(&ui, 16);
            l_ui_checkbox(&ui, 30, y, "Enable feature", &checked);
        }

        // Slider with label
        {
            int y = l_ui_next(&ui, 8);
            l_ui_label(&ui, 30, y, "Volume:");
            y = l_ui_next(&ui, 16);
            l_ui_slider(&ui, 30, y, 200, &volume, 0, 100);
        }

        // Textbox with label
        {
            int y = l_ui_next(&ui, 8);
            l_ui_label(&ui, 30, y, "Name:");
            y = l_ui_next(&ui, 20);
            l_ui_textbox(&ui, 30, y, 200, name, L_UI_MAX_TEXT);
        }

        // Separator
        {
            int y = l_ui_next(&ui, 6);
            l_ui_separator(&ui, 30, y + 2, WIN_W - 80);
        }

        // Status display
        {
            int y = l_ui_next(&ui, 10);
            char status[128];

            // Clicks
            char num_buf[12];
            l_itoa(click_count, num_buf, 10);
            l_strcpy(status, "Clicks: ");
            l_strcat(status, num_buf);
            l_ui_label(&ui, 30, y, status);

            y = l_ui_next(&ui, 10);
            l_strcpy(status, "Checkbox: ");
            l_strcat(status, checked ? "ON" : "OFF");
            l_ui_label(&ui, 30, y, status);

            y = l_ui_next(&ui, 10);
            l_itoa(volume, num_buf, 10);
            l_strcpy(status, "Volume: ");
            l_strcat(status, num_buf);
            l_ui_label(&ui, 30, y, status);

            y = l_ui_next(&ui, 10);
            l_strcpy(status, "Name: ");
            l_strcat(status, name);
            l_ui_label(&ui, 30, y, status);
        }

        l_ui_layout_end(&ui);

        l_ui_end(&ui);
        l_canvas_flush(&canvas);
    }

    l_canvas_close(&canvas);
    return 0;
}
