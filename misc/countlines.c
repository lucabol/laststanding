#include <stdio.h>
#include <stdlib.h>
#include <string.h>

 /* reverse:  reverse string s in place */
 void reverse(char s[])
 {
     int i, j;
     char c;

     for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
         c = s[i];
         s[i] = s[j];
         s[j] = c;
     }
}

/* itoa:  convert n to characters in s */
 char* itoa(int n, char s[])
 {
     int i, sign;

     if ((sign = n) < 0)  /* record sign */
         n = -n;          /* make n positive */
     i = 0;
     do {       /* generate digits in reverse order */
         s[i++] = n % 10 + '0';   /* get next digit */
     } while ((n /= 10) > 0);     /* delete it */
     if (sign < 0)
         s[i++] = '-';
     s[i] = '\0';
     reverse(s);
     return s;
}    
int main(int argc, char* argv[]) {
  if(argc != 2) {
    puts("countlines filename");
    exit(-1);
  }

  FILE* f = fopen(argv[1], "r");
  if(!f) {
    puts(argv[1]);
    exit(-1);
  }
  
  unsigned count = 0;
  char buffer[2048];

  while(fgets(buffer, 2048, f))
      count +=1;

  fclose(f);

  puts(itoa(count, buffer));
  return count;
}
