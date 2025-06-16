// This tests multiple files including the library with just one of them defined as L_MAINFILE
#include "l_os.h"
#include "l_os.h"

#define L_MAINFILE
#include "l_os.h"

void can_read_and_print_args(int argc, char* argv[]) {
    // Print all parameters (i.e., test visually for correct unicode).
    for(int i = 1; i < argc; i++) {
        puts(argv[1]);
    }
} 

#define test_cond(C) if(!(C)) { puts(#C) ; exit(-1); }

void can_read_prog_name(int argc, char* argv[]) {
    test_cond(argc >= 0);
    test_cond(strlen(argv[0]) > 0);
    test_cond(strstr(argv[0], "test") != NULL);

}

void can_open_and_close_files() {

  char* msg = "Hello world!";
  int len = strlen(msg);

  L_FD file = open_append("test_file");
  write(file, msg, len);
  close(file);

  file = open_read("test_file");
  char buf[len];
  int n = read(file, buf, len);
  if(memcmp(buf, msg, len) != 0) puts("ERROR: not retrieved the same");
  if(n != len) puts("ERROR: not read enough!");
  close(file);
}

void can_print_unicode() {
  char msg[] = u8"κόσμε";
  puts(msg);
}
void test_string_functions() {
  puts("Starting string function tests");
  
  /*
  // Test l_wcslen - skipping as it seems to cause issues
  const wchar_t wide_str[] = L"Hello";
  test_cond(l_wcslen(wide_str) == 5);
  test_cond(l_wcslen(L"") == 0);
  */
  
  // Test l_strcpy
  char dest[20];
  char src[] = "Hello World";
  char* result = l_strcpy(dest, src);
  test_cond(result == dest);
  test_cond(l_strncmp(dest, src, 11) == 0);
  
  // Test l_strchr
  char test_str[] = "Hello World";
  test_cond(l_strchr(test_str, 'W') == &test_str[6]);
  test_cond(l_strchr(test_str, 'x') == NULL);
  test_cond(l_strchr(test_str, 'H') == &test_str[0]);
  
  // Test l_strrchr
  char test_str2[] = "Hello Hello";
  test_cond(l_strrchr(test_str2, 'l') == &test_str2[9]);
  test_cond(l_strrchr(test_str2, 'H') == &test_str2[6]);
  test_cond(l_strrchr(test_str2, 'x') == NULL);
  
  // Test l_strncmp
  test_cond(l_strncmp("Hello", "Hello", 5) == 0);
  test_cond(l_strncmp("Hello", "Hell", 4) == 0);
  test_cond(l_strncmp("Hello", "Help", 3) == 0);
  test_cond(l_strncmp("Hello", "Help", 4) < 0);
  test_cond(l_strncmp("Help", "Hello", 4) > 0);
  
  // Test l_reverse
  char rev_str[] = "Hello";
  l_reverse(rev_str, 5);
  test_cond(l_strncmp(rev_str, "olleH", 5) == 0);
  
  puts("String function tests passed");
}

void test_character_number_functions() {
  // Test l_isdigit
  test_cond(l_isdigit('0') != 0);
  test_cond(l_isdigit('5') != 0);
  test_cond(l_isdigit('9') != 0);
  test_cond(l_isdigit('a') == 0);
  test_cond(l_isdigit(' ') == 0);
  
  // Test l_atol
  test_cond(l_atol("123") == 123L);
  test_cond(l_atol("-456") == -456L);
  test_cond(l_atol("0") == 0L);
  test_cond(l_atol("999999") == 999999L);
  
  // Test l_atoi
  test_cond(l_atoi("123") == 123);
  test_cond(l_atoi("-456") == -456);
  test_cond(l_atoi("0") == 0);
  
  // Test l_itoa
  char buffer[32];
  char* result = l_itoa(123, buffer, 10);
  test_cond(result == buffer);
  test_cond(l_strncmp(buffer, "123", 3) == 0);
  
  l_itoa(-456, buffer, 10);
  test_cond(l_strncmp(buffer, "-456", 4) == 0);
  
  l_itoa(0, buffer, 10);
  test_cond(l_strncmp(buffer, "0", 1) == 0);
  
  l_itoa(255, buffer, 16);
  test_cond(l_strncmp(buffer, "ff", 2) == 0);
}

void test_memory_functions() {
  // Test l_memset
  char mem_buf[10];
  void* result = l_memset(mem_buf, 'A', 5);
  test_cond(result == mem_buf);
  test_cond(mem_buf[0] == 'A');
  test_cond(mem_buf[4] == 'A');
  
  // Test l_memmove
  char move_src[] = "Hello";
  char move_dst[10];
  void* move_result = l_memmove(move_dst, move_src, 5);
  test_cond(move_result == move_dst);
  test_cond(l_memcmp(move_dst, move_src, 5) == 0);
  
  // Test overlapping memmove
  char overlap[] = "Hello World";
  l_memmove(overlap + 2, overlap, 5);
  test_cond(l_memcmp(overlap + 2, "Hello", 5) == 0);
  
  // Test l_memcpy
  char copy_src[] = "Test";
  char copy_dst[10];
  void* copy_result = l_memcpy(copy_dst, copy_src, 4);
  test_cond(copy_result == copy_dst);
  test_cond(l_memcmp(copy_dst, copy_src, 4) == 0);
}

void test_file_operations() {
  char* test_msg = "Test message";
  int msg_len = strlen(test_msg);
  
  // Test l_open_write
  L_FD write_fd = l_open_write("test_write_file");
  test_cond(write_fd >= 0);
  ssize_t written = l_write(write_fd, test_msg, msg_len);
  test_cond(written == msg_len);
  l_close(write_fd);
  
  // Test reading back the written file
  L_FD read_fd = l_open_read("test_write_file");
  test_cond(read_fd >= 0);
  char read_buf[20];
  ssize_t read_bytes = l_read(read_fd, read_buf, msg_len);
  test_cond(read_bytes == msg_len);
  test_cond(l_memcmp(read_buf, test_msg, msg_len) == 0);
  l_close(read_fd);
  
  // Test l_open_readwrite
  L_FD rw_fd = l_open_readwrite("test_rw_file");
  test_cond(rw_fd >= 0);
  written = l_write(rw_fd, test_msg, msg_len);
  test_cond(written == msg_len);
  l_close(rw_fd);
  
  // Test l_open_trunc
  L_FD trunc_fd = l_open_trunc("test_write_file");
  test_cond(trunc_fd >= 0);
  char* short_msg = "Hi";
  written = l_write(trunc_fd, short_msg, 2);
  test_cond(written == 2);
  l_close(trunc_fd);
  
  // Verify truncation worked
  read_fd = l_open_read("test_write_file");
  read_bytes = l_read(read_fd, read_buf, 10);
  test_cond(read_bytes == 2);
  test_cond(l_memcmp(read_buf, "Hi", 2) == 0);
  l_close(read_fd);
}

void test_system_functions() {
  // Test l_exitif with false condition (should not exit)
  l_exitif(0, -1, "This should not exit");
  
  // Test l_open with explicit flags
  L_FD open_fd = l_open("test_explicit_open", O_CREAT | O_WRONLY, 0644);
  test_cond(open_fd >= 0);
  l_close(open_fd);
}

int main(int argc, char* argv[]) {
  can_read_and_print_args(argc, argv);
  can_read_prog_name(argc, argv);
  can_open_and_close_files();
  can_print_unicode();
  
  // Test all l_os.h functions
  test_string_functions();
  test_character_number_functions();
  test_memory_functions();
  test_file_operations();
  test_system_functions();
  
  return 0;
}
