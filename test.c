#include "l_os.h"
#define PRINTF_ALIAS_STANDARD_FUNCTION_NAMES
#define PRINTF_VISIBILITY
#include "l_printf.h" // https://github.com/eyalroz/printf#cmake-options-and-preprocessor-definitions cat printf.h printf.c > lprintf.h

#define TEST_OK 0
#define TEST_FAIL -1

#define test_start() int ___t = 0; int ___e = 0
#define test_end() printf("Tests: %i / %i \n", ___t - ___e, ___t);
#define test_run(f) do { ___t++; if(f() == TEST_FAIL) { printf("FAILURE TEST: " #f "\n");___e++;}} while(0)
#define test_cond(c) do { if(!(c)) { printf("FAILURE CONDITION: " #c "\n"); return TEST_FAIL;} } while(0)
#define test_cond0(c) do { if((c) != 0) { printf("FAILURE CONDITION: " #c "\n"); return TEST_FAIL;} } while(0)
#define test_runall(ts) do { for(unsigned i = 0; i < sizeof(ts)/sizeof(ts[0]);i++) test_run(ts[i]);} while(0)

int argc_;
char** argv_;

typedef int (*test_func)();


int can_read_and_print_args() {
    // Print all parameters (i.e., test visually for correct unicode).
    // But the standard printf (without the lstandalone wrapper) with mingw doesn't put the console in unicode mode, hence prints garbage
    // AKA it doesn't call SetConsoleOutputCP(CP_UTF8);
    for(int i = 1; i < argc_; i++) {
        printf("%s\n", argv_[i]);
    }
    return TEST_OK;
} 
int can_read_prog_name() {
    test_cond(argc_ >= 0);
    test_cond(strlen(argv_[0]) > 0);
    test_cond(strstr(argv_[0], "test") != NULL);

    return TEST_OK;
}

int can_open_and_close_files() {

  LASTS_FD w = open_write("test_file");
  close(w);
  return TEST_OK;
}

test_func tests[] = {can_read_and_print_args, can_read_prog_name, can_open_and_close_files};

int main(int argc, char* argv[]) {
    argc_ = argc;
    argv_ = argv;

    test_start();
    test_runall(tests);
    test_end();
    return 0;
}
