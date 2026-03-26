#define L_MAINFILE
#include "l_os.h"

int main(int argc, char *argv[]) {
    (void)argc; (void)argv;

    L_FD fd = l_open_write("_file_io_test.txt");
    l_write(fd, "Hello!\n", 7);
    l_close(fd);

    char buf[64];
    fd = l_open_read("_file_io_test.txt");
    ptrdiff_t n = l_read(fd, buf, sizeof(buf));
    l_close(fd);

    l_write(L_STDOUT, buf, n);
    l_unlink("_file_io_test.txt");
    return 0;
}
