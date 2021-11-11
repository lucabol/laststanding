#include "lasts.h"

#define TEST_OK 0
#define TEST_FAIL -1
#define test_run(f) do { if(f() == -1) write(LASTS_STDOUT, #f " failed\n", strlen(#f) + 8);} while(0)
#define test_cond(c) do { if(!(c)) { write(LASTS_STDOUT, #c " failed\n", strlen(#c) + 8); return TEST_FAIL;} } while(0)
#define test_cond0(c) do { if((c) != 0) { write(LASTS_STDOUT, #c " failed\n", strlen(#c) + 8); return TEST_FAIL;} } while(0)

int argc_;
char** argv_;

int args() {
    // Test that at least I got the program name.
    test_cond(argc_ > 0);
    test_cond(strlen(argv_[0]) > 0);
    test_cond(strstr(argv_[0], "test") != NULL);

    return TEST_OK;
}
int main(int argc, char* argv[]) {
    argc_ = argc;
    argv_ = argv;

    // Print all parameters (i.e., test for unicode).
    for(int i = 0; i < argc_; i++) {
        write(STDOUT, argv_[i], strlen(argv_[i]));
        write(STDOUT, "\n", 1);
    }

    test_run(args);
    return 0;
}
