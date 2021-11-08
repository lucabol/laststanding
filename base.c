#include "lasts.h"

int main(int argc, char* argv[]) {
  for(int i = 0; i < argc; i++) {
          write(STDOUT, argv[i], strlen(argv[i]));
          write(STDOUT, "\n", 1);
  }
  return 0;
}
