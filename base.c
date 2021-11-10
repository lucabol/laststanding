#include "lasts.h"

int argc_;
char** argv_;

int args() {
    // Test that at least I got the program name.
    test_cond(argc_ > 0);
    test_cond(strlen(argv_[0]) > 0);

    // Print all parameters (i.e., test for unicode).
    for(int i = 0; i < argc_; i++) {
        write(STDOUT, argv_[i], strlen(argv_[i]));
        write(STDOUT, "\n", 1);
    }
    return TEST_OK;
}
int main(int argc, char* argv[]) {
    argc_ = argc;
    argv_ = argv;

    test_run(args);
    return 0;
}
