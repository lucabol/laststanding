#include "lasts.h"

int main(int argc, char* argv[]) {
  for(int i = 0; i < argc; i++) {
          write(1, argv[i], strlen(argv[i]));
          write(1, "\n", 1);
  }
  return 0;
}
