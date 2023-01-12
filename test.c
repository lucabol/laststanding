#include "l_os.h"

void can_read_and_print_args(int argc, char* argv[]) {
    // Print all parameters (i.e., test visually for correct unicode).
    for(int i = 1; i < argc; i++) {
        write(LASTS_STDOUT, argv[i], strlen(argv[i]));
    }
} 

#define test_cond(C) if(!(C)) exit(-1)

void can_read_prog_name(int argc, char* argv[]) {
    test_cond(argc >= 0);
    test_cond(strlen(argv[0]) > 0);
    test_cond(strstr(argv[0], "test") != NULL);

}

void can_open_and_close_files() {

  LASTS_FD w = open_write("test_file");
  close(w);
}

void can_print_unicode() {
  char msg[] = "Բարեւ աշխարհ!\n";
  write(LASTS_STDOUT, msg, strlen(msg));
}

int main(int argc, char* argv[]) {
  can_read_and_print_args(argc, argv);
  can_read_prog_name(argc, argv);
  can_open_and_close_files();
  can_print_unicode();
  return 0;
}
