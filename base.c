#include "lasts.h"

int main(int argc, char* argv[]) {
  for(int i = 0; i < argc; i++) {
          write(1, argv[i], strlen(argv[i]));
          write(1, "\n", 1);
  }
  if(1 == 1 and 2 == 2) return 3;
  return 4;
}
