#define L_MAINFILE
#include "l_os.h"

int main(int argc, char* argv[]) {
  if(argc != 2) {
    puts("countlines filename");
    exit(-1);
  }

  L_FD f = open_read(argv[1]);
  if(!f) {
    puts(argv[1]);
    exit(-1);
  }
  
  unsigned count = 0;
  char c;

  while(read(f, &c, 1))
    if(c == '\n')
      count +=1;

  char buffer[20];
  puts(itoa(count, buffer, 10));
  return count;
}
