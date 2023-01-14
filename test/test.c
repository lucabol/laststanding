// This tests multiple files including the library with just one of them defined as L_MAINFILE
#include "l_os.h"
#include "l_os.h"

#define L_MAINFILE
#include "l_os.h"

void can_read_and_print_args(int argc, char* argv[]) {
    // Print all parameters (i.e., test visually for correct unicode).
    for(int i = 1; i < argc; i++) {
        puts(argv[1]);
    }
} 

#define test_cond(C) if(!(C)) { puts(#C) ; exit(-1); }

void can_read_prog_name(int argc, char* argv[]) {
    test_cond(argc >= 0);
    test_cond(strlen(argv[0]) > 0);
    test_cond(strstr(argv[0], "test") != NULL);

}

void can_open_and_close_files() {

  char* msg = "Hello world!";
  int len = strlen(msg);

  L_FD file = open_append("test_file");
  write(file, msg, len);
  close(file);

  file = open_read("test_file");
  char buf[len];
  int n = read(file, buf, len);
  if(memcmp(buf, msg, len) != 0) puts("ERROR: not retrieved the same");
  if(n != len) puts("ERROR: not read enough!");
  close(file);
}

void can_print_unicode() {
  char msg[] = u8"κόσμε";
  puts(msg);
}

int main(int argc, char* argv[]) {
  can_read_and_print_args(argc, argv);
  can_read_prog_name(argc, argv);
  can_open_and_close_files();
  can_print_unicode();
  return 0;
}
