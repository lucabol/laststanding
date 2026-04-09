#define L_MAINFILE
#include "l_gfx.h"

// clock.c — Analog clock demo using fixed-point sin/cos
// 320x240 canvas, animated clock face, Q/ESC=quit

#define W 320
#define H 240
#define CX 160
#define CY 120
#define RADIUS 100

// Signed sine table: 360 entries, values -1024..+1024 (10-bit fixed point)
// sin_tbl[i] = sin(i * pi / 180) * 1024
static const int16_t sin_tbl[360] = {
       0,  18,  36,  54,  71,  89, 107, 125, 143, 160,
     178, 195, 213, 230, 248, 265, 282, 299, 316, 333,
     350, 367, 383, 400, 416, 432, 448, 464, 480, 496,
     511, 526, 541, 556, 571, 585, 600, 614, 628, 642,
     655, 669, 682, 695, 707, 720, 732, 744, 756, 767,
     778, 789, 800, 810, 821, 831, 841, 850, 860, 869,
     878, 886, 895, 903, 911, 918, 926, 933, 940, 946,
     953, 959, 965, 970, 976, 981, 985, 990, 994, 998,
    1002,1005,1008,1011,1014,1016,1018,1020,1022,1023,
    1024,1023,1022,1020,1018,1016,1014,1011,1008,1005,
    1002, 998, 994, 990, 985, 981, 976, 970, 965, 959,
     953, 946, 940, 933, 926, 918, 911, 903, 895, 886,
     878, 869, 860, 850, 841, 831, 821, 810, 800, 789,
     778, 767, 756, 744, 732, 720, 707, 695, 682, 669,
     655, 642, 628, 614, 600, 585, 571, 556, 541, 526,
     511, 496, 480, 464, 448, 432, 416, 400, 383, 367,
     350, 333, 316, 299, 282, 265, 248, 230, 213, 195,
     178, 160, 143, 125, 107,  89,  71,  54,  36,  18,
       0, -18, -36, -54, -71, -89,-107,-125,-143,-160,
    -178,-195,-213,-230,-248,-265,-282,-299,-316,-333,
    -350,-367,-383,-400,-416,-432,-448,-464,-480,-496,
    -511,-526,-541,-556,-571,-585,-600,-614,-628,-642,
    -655,-669,-682,-695,-707,-720,-732,-744,-756,-767,
    -778,-789,-800,-810,-821,-831,-841,-850,-860,-869,
    -878,-886,-895,-903,-911,-918,-926,-933,-940,-946,
    -953,-959,-965,-970,-976,-981,-985,-990,-994,-998,
   -1002,-1005,-1008,-1011,-1014,-1016,-1018,-1020,-1022,-1023,
   -1024,-1023,-1022,-1020,-1018,-1016,-1014,-1011,-1008,-1005,
   -1002,-998,-994,-990,-985,-981,-976,-970,-965,-959,
    -953,-946,-940,-933,-926,-918,-911,-903,-895,-886,
    -878,-869,-860,-850,-841,-831,-821,-810,-800,-789,
    -778,-767,-756,-744,-732,-720,-707,-695,-682,-669,
    -655,-642,-628,-614,-600,-585,-571,-556,-541,-526,
    -511,-496,-480,-464,-448,-432,-416,-400,-383,-367,
    -350,-333,-316,-299,-282,-265,-248,-230,-213,-195,
    -178,-160,-143,-125,-107, -89, -71, -54, -36, -18
};

static int fp_sin(int deg) {
    deg = ((deg % 360) + 360) % 360;
    return sin_tbl[deg];
}

static int fp_cos(int deg) {
    return fp_sin(deg + 90);
}

// Draw a clock hand from center at angle (in degrees, 0=12 o'clock, clockwise)
static void draw_hand(L_Canvas *c, int angle, int length, uint32_t color) {
    // Convert clock angle (0=up) to math angle (0=right)
    // clock 0 = math 270, so math_angle = angle - 90
    int deg = angle - 90;
    int ex = CX + (fp_cos(deg) * length) / 1024;
    int ey = CY + (fp_sin(deg) * length) / 1024;
    l_line(c, CX, CY, ex, ey, color);
}

static void draw_face(L_Canvas *c) {
    // Outer circle
    l_circle(c, CX, CY, RADIUS, L_WHITE);
    l_circle(c, CX, CY, RADIUS - 1, L_WHITE);

    // Hour markers and numbers
    static const char *nums[12] = {
        "12","1","2","3","4","5","6","7","8","9","10","11"
    };
    for (int i = 0; i < 12; i++) {
        int deg = i * 30 - 90;
        // Tick mark
        int ix = CX + (fp_cos(deg) * (RADIUS - 8)) / 1024;
        int iy = CY + (fp_sin(deg) * (RADIUS - 8)) / 1024;
        int ox = CX + (fp_cos(deg) * (RADIUS - 2)) / 1024;
        int oy = CY + (fp_sin(deg) * (RADIUS - 2)) / 1024;
        l_line(c, ix, iy, ox, oy, L_WHITE);

        // Number
        int nx = CX + (fp_cos(deg) * (RADIUS - 20)) / 1024;
        int ny = CY + (fp_sin(deg) * (RADIUS - 20)) / 1024;
        // Center the text roughly
        int tw = (int)l_strlen(nums[i]) * 8;
        l_draw_text(c, nx - tw / 2, ny - 4, nums[i], L_RGB(200, 200, 200));
    }

    // Minute ticks
    for (int i = 0; i < 60; i++) {
        if (i % 5 == 0) continue;
        int deg = i * 6 - 90;
        int ix = CX + (fp_cos(deg) * (RADIUS - 4)) / 1024;
        int iy = CY + (fp_sin(deg) * (RADIUS - 4)) / 1024;
        int ox = CX + (fp_cos(deg) * (RADIUS - 2)) / 1024;
        int oy = CY + (fp_sin(deg) * (RADIUS - 2)) / 1024;
        l_line(c, ix, iy, ox, oy, L_RGB(100, 100, 100));
    }
}

int main(int argc, char *argv[]) {
    l_getenv_init(argc, argv);

    L_Canvas canvas;
    if (l_canvas_open(&canvas, W, H, "Analog Clock") != 0) {
        puts("No display available\n");
        return 0;
    }

    while (l_canvas_alive(&canvas)) {
        int key = l_canvas_key(&canvas);
        if (key == 'q' || key == 'Q' || key == 27) break;

        // Get real system time
        long long now = l_time((long long *)0);
        int total_secs = (int)(now % 86400);  // seconds since midnight UTC
        int hours   = total_secs / 3600;
        int minutes = (total_secs % 3600) / 60;
        int seconds = total_secs % 60;

        l_canvas_clear(&canvas, L_BLACK);
        draw_face(&canvas);

        // Draw hands
        int sec_angle = seconds * 6;          // 360/60 = 6 deg per sec
        int min_angle = minutes * 6 + seconds / 10;
        int hr_angle  = (hours % 12) * 30 + minutes / 2;

        draw_hand(&canvas, hr_angle,  55, L_WHITE);
        draw_hand(&canvas, min_angle, 75, L_RGB(180, 180, 255));
        draw_hand(&canvas, sec_angle, 85, L_RED);

        // Center dot
        l_fill_circle(&canvas, CX, CY, 3, L_RED);

        // HUD: show time as text
        char tbuf[16];
        tbuf[0] = '0' + (char)(hours / 10);
        tbuf[1] = '0' + (char)(hours % 10);
        tbuf[2] = ':';
        tbuf[3] = '0' + (char)(minutes / 10);
        tbuf[4] = '0' + (char)(minutes % 10);
        tbuf[5] = ':';
        tbuf[6] = '0' + (char)(seconds / 10);
        tbuf[7] = '0' + (char)(seconds % 10);
        tbuf[8] = '\0';
        l_draw_text(&canvas, CX - 32, CY + RADIUS + 8, tbuf, L_RGB(150, 150, 150));
        l_draw_text(&canvas, 2, H - 10, "Q:quit", L_RGB(80, 80, 80));

        l_canvas_flush(&canvas);
        l_sleep_ms(16);
    }

    l_canvas_close(&canvas);
    return 0;
}
