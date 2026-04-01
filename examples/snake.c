#define L_MAINFILE
#include "l_os.h"

// Snake game — runs in a terminal using only l_os.h (no libc)
// Usage: snake

#define W 40
#define H 20
#define MAX_LEN (W * H)

typedef struct { int x, y; } Pos;

static Pos snake[MAX_LEN];
static int snake_len;
static int dir;  // 0=right, 1=down, 2=left, 3=up
static Pos apple;
static int score;
static int game_over;

static void write_str(const char *s) {
    write(STDOUT, s, strlen(s));
}

static void write_num(int n) {
    char buf[12];
    itoa(n, buf, 10);
    write_str(buf);
}

static void move_cursor(int row, int col) {
    write_str("\033[");
    write_num(row);
    write_str(";");
    write_num(col);
    write_str("H");
}

static void place_apple(void) {
    int tries = 0;
    do {
        apple.x = (l_rand() % (W - 2)) + 1;
        apple.y = (l_rand() % (H - 2)) + 1;
        int on_snake = 0;
        for (int i = 0; i < snake_len; i++) {
            if (snake[i].x == apple.x && snake[i].y == apple.y) {
                on_snake = 1;
                break;
            }
        }
        if (!on_snake) break;
    } while (++tries < 1000);
}

static void init_game(void) {
    snake_len = 3;
    dir = 0;
    score = 0;
    game_over = 0;
    for (int i = 0; i < snake_len; i++) {
        snake[i].x = 5 - i;
        snake[i].y = H / 2;
    }
    l_srand(12345);
    place_apple();
}

static void draw(void) {
    move_cursor(1, 1);

    char line[W + 3];

    for (int y = 0; y < H; y++) {
        int pos = 0;
        for (int x = 0; x < W; x++) {
            char c = ' ';
            if (y == 0 || y == H - 1) c = '-';
            else if (x == 0 || x == W - 1) c = '|';

            if (x == apple.x && y == apple.y) c = '@';

            for (int s = 0; s < snake_len; s++) {
                if (snake[s].x == x && snake[s].y == y) {
                    c = (s == 0) ? 'O' : '#';
                    break;
                }
            }
            line[pos++] = c;
        }
        line[pos++] = '\n';
        write(STDOUT, line, pos);
    }

    write_str("Score: ");
    write_num(score);
    write_str("  WASD=move Q=quit     \n");
}

static void update(void) {
    Pos head = snake[0];

    switch (dir) {
        case 0: head.x++; break;
        case 1: head.y++; break;
        case 2: head.x--; break;
        case 3: head.y--; break;
    }

    // Wall collision
    if (head.x <= 0 || head.x >= W - 1 || head.y <= 0 || head.y >= H - 1) {
        game_over = 1;
        return;
    }

    // Self collision
    for (int i = 0; i < snake_len; i++) {
        if (snake[i].x == head.x && snake[i].y == head.y) {
            game_over = 1;
            return;
        }
    }

    int ate = (head.x == apple.x && head.y == apple.y);

    // Shift body
    if (!ate) {
        for (int i = snake_len - 1; i > 0; i--)
            snake[i] = snake[i - 1];
    } else {
        // Grow: shift everything, keep tail
        if (snake_len < MAX_LEN) snake_len++;
        for (int i = snake_len - 1; i > 0; i--)
            snake[i] = snake[i - 1];
        score += 10;
        l_srand(l_rand_state + (unsigned int)score);
        place_apple();
    }

    snake[0] = head;
}

int main(int argc, char *argv[]) {
    (void)argc; (void)argv;

    unsigned long old_mode = l_term_raw();
    write_str(L_ANSI_CLEAR);  // Clear screen
    write_str(L_ANSI_HIDE_CUR); // Hide cursor

    init_game();

    while (!game_over) {
        char c;
        ssize_t n = l_read_nonblock(L_STDIN, &c, 1);
        if (n > 0) {
            switch (c) {
                case 'w': case 'W': if (dir != 1) dir = 3; break;
                case 's': case 'S': if (dir != 3) dir = 1; break;
                case 'a': case 'A': if (dir != 0) dir = 2; break;
                case 'd': case 'D': if (dir != 2) dir = 0; break;
                case 'q': case 'Q': game_over = 1; break;
            }
        }

        update();
        if (!game_over) draw();
        l_sleep_ms(120);
    }

    // Game over screen
    write_str(L_ANSI_CLEAR);
    move_cursor(H / 2, W / 2 - 6);
    write_str("GAME OVER!\n");
    move_cursor(H / 2 + 1, W / 2 - 7);
    write_str("Final score: ");
    write_num(score);
    write_str("\n");

    write_str(L_ANSI_SHOW_CUR); // Show cursor
    l_term_restore(old_mode);

    return 0;
}
