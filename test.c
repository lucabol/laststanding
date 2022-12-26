#include "lasts.h"

#define TEST_OK 0
#define TEST_FAIL -1

#define test_start() int ___t = 0; int ___e = 0
#define test_end() char ___outb[40]; char ___b[10]; char* ___out = ___outb; \
  ___out = strpush(___out, "Tests: "); \
  ___out = strpush(___out, ltoa(___t - ___e, ___b, 10)); \
  ___out = strpush(___out, "/"); \
  ___out = strpush(___out, ltoa(___t, ___b, 10)); \
  ___out = strpush(___out, "\n"); \
  write(LASTS_STDOUT,___outb, strlen(___outb));
#define test_run(f) do { ___t++; if(f() == TEST_FAIL) { write(LASTS_STDOUT, #f " failed\n", sizeof(#f) + 7);___e++;}} while(0)
#define test_cond(c) do { if(!(c)) { write(LASTS_STDOUT, #c " failed\n", sizeof(#c) + 7); return TEST_FAIL;} } while(0)
#define test_cond0(c) do { if((c) != 0) { write(LASTS_STDOUT, #c " failed\n", sizeof(#c) + 7); return TEST_FAIL;} } while(0)

int argc_;
char** argv_;

// Copy source to dest and returns pointer to next byte to write to
char * strpush(char* dest, const char* source) {
  return strcpy(dest, source) + strlen(dest);
}
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
    for(int i = 0; i < argc_; i++) {
        write(STDOUT, argv_[i], strlen(argv_[i]));
        write(STDOUT, "\n", 1);
    }

    test_start();
    test_run(args);
    test_end();
    return 0;
}
