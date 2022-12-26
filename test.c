#ifdef LSTANDALONE
#include "lasts.h"
#else
#include <stdio.h>
#include <string.h>
#endif

#define TEST_OK 0
#define TEST_FAIL -1

#define test_start() int ___t = 0; int ___e = 0
#define test_end() printf("Tests: %i / %i \n", ___t - ___e, ___t);
#define test_run(f) do { ___t++; if(f() == TEST_FAIL) { printf("FAILURE TEST: " #f "\n");___e++;}} while(0)
#define test_cond(c) do { if(!(c)) { printf("FAILURE CONDITION: " #c "\n"); return TEST_FAIL;} } while(0)
#define test_cond0(c) do { if((c) != 0) { printf("FAILURE CONDITION: " #c "\n"); return TEST_FAIL;} } while(0)

int argc_;
char** argv_;

int args() {
    // Test that at least I got the program name.
    test_cond(argc_ >= 0);
    test_cond(strlen(argv_[0]) > 0);
    test_cond(strstr(argv_[0], "test") != NULL);

    return TEST_OK;
}
int main(int argc, char* argv[]) {
    argc_ = argc;
    argv_ = argv;

    // Print all parameters (i.e., test visually for correct unicode).
    // But the standard printf (without the lstandalone wrapper) with mingw doesn't put the console in unicode mode, hence prints garbage
    // AKA it doesn't call SetConsoleOutputCP(CP_UTF8);
    for(int i = 1; i < argc_; i++) {
        printf("%s\n", argv_[i]);
    }

    test_start();
    test_run(args);
    test_end();
    return 0;
}
