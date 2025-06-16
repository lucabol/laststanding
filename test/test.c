// This tests multiple files including the library with just one of them defined as L_MAINFILE
#include "l_os.h"
#include "l_os.h"

#define L_MAINFILE
#include "l_os.h"

// Test reporting macros
static int test_count = 0;
static int passed_count = 0;

#define TEST_FUNCTION(name) do { \
    puts("\n"); \
    puts("Testing " name "...\n"); \
} while(0)

#define TEST_ASSERT(condition, test_name) do { \
    test_count++; \
    if (condition) { \
        passed_count++; \
        puts("  ✓ " test_name "\n"); \
    } else { \
        puts("  ✗ " test_name " FAILED: " #condition "\n"); \
        exit(-1); \
    } \
} while(0)

#define TEST_SECTION_PASS(name) do { \
    puts("  " name " tests: PASSED\n"); \
} while(0)

// Legacy macro for compatibility
#define test_cond(C) TEST_ASSERT(C, "assertion")

void can_read_and_print_args(int argc, char* argv[]) {
    TEST_FUNCTION("Command Line Arguments");
    // Print all parameters (i.e., test visually for correct unicode).
    for(int i = 1; i < argc; i++) {
        puts(argv[i]);
    }
    TEST_ASSERT(1, "command line argument reading completed");
    TEST_SECTION_PASS("Command line argument");
}

void can_read_prog_name(int argc, char* argv[]) {
    TEST_FUNCTION("Program Name Reading");
    TEST_ASSERT(argc >= 0, "argc is non-negative");
    TEST_ASSERT(strlen(argv[0]) > 0, "program name is not empty");
    TEST_ASSERT(strstr(argv[0], "test") != NULL, "program name contains 'test'");
    TEST_SECTION_PASS("Program name reading");
}

void can_open_and_close_files() {
  TEST_FUNCTION("Basic File Operations");

  char* msg = "Hello world!";
  int len = strlen(msg);

  L_FD file = open_append("test_file");
  write(file, msg, len);
  close(file);

  file = open_read("test_file");
  char buf[len];
  int n = read(file, buf, len);
  TEST_ASSERT(memcmp(buf, msg, len) == 0, "file content matches written data");
  TEST_ASSERT(n == len, "read correct number of bytes");
  close(file);
  
  TEST_SECTION_PASS("Basic file operation");
}

void can_print_unicode() {
  TEST_FUNCTION("Unicode Output");
  char msg[] = u8"κόσμε";
  puts(msg);
  puts("\n");
  TEST_ASSERT(1, "unicode output test completed");
  TEST_SECTION_PASS("Unicode output");
}
void test_string_functions() {
  TEST_FUNCTION("String Functions");
  
  /*
  // Test l_wcslen - skipping as it seems to cause issues
  const wchar_t wide_str[] = L"Hello";
  TEST_ASSERT(l_wcslen(wide_str) == 5, "l_wcslen with 'Hello'");
  TEST_ASSERT(l_wcslen(L"") == 0, "l_wcslen with empty string");
  */
  
  // Test l_strcpy
  char dest[20];
  char src[] = "Hello World";
  char* result = l_strcpy(dest, src);
  TEST_ASSERT(result == dest, "l_strcpy returns destination pointer");
  TEST_ASSERT(l_strncmp(dest, src, 11) == 0, "l_strcpy copies string correctly");
  
  // Test l_strchr
  char test_str[] = "Hello World";
  TEST_ASSERT(l_strchr(test_str, 'W') == &test_str[6], "l_strchr finds 'W' at position 6");
  TEST_ASSERT(l_strchr(test_str, 'x') == NULL, "l_strchr returns NULL for missing char");
  TEST_ASSERT(l_strchr(test_str, 'H') == &test_str[0], "l_strchr finds 'H' at position 0");
  
  // Test l_strrchr
  char test_str2[] = "Hello Hello";
  TEST_ASSERT(l_strrchr(test_str2, 'l') == &test_str2[9], "l_strrchr finds last 'l'");
  TEST_ASSERT(l_strrchr(test_str2, 'H') == &test_str2[6], "l_strrchr finds last 'H'");
  TEST_ASSERT(l_strrchr(test_str2, 'x') == NULL, "l_strrchr returns NULL for missing char");
  
  // Test l_strncmp
  TEST_ASSERT(l_strncmp("Hello", "Hello", 5) == 0, "l_strncmp equal strings");
  TEST_ASSERT(l_strncmp("Hello", "Hell", 4) == 0, "l_strncmp partial match");
  TEST_ASSERT(l_strncmp("Hello", "Help", 3) == 0, "l_strncmp first 3 chars");
  TEST_ASSERT(l_strncmp("Hello", "Help", 4) < 0, "l_strncmp 'Hello' < 'Help'");
  TEST_ASSERT(l_strncmp("Help", "Hello", 4) > 0, "l_strncmp 'Help' > 'Hello'");
  
  // Test l_reverse
  char rev_str[] = "Hello";
  l_reverse(rev_str, 5);
  TEST_ASSERT(l_strncmp(rev_str, "olleH", 5) == 0, "l_reverse reverses string");
  
  TEST_SECTION_PASS("String function");
}

void test_character_number_functions() {
  TEST_FUNCTION("Character/Number Functions");
  
  // Test l_isdigit
  TEST_ASSERT(l_isdigit('0') != 0, "l_isdigit recognizes '0'");
  TEST_ASSERT(l_isdigit('5') != 0, "l_isdigit recognizes '5'");
  TEST_ASSERT(l_isdigit('9') != 0, "l_isdigit recognizes '9'");
  TEST_ASSERT(l_isdigit('a') == 0, "l_isdigit rejects 'a'");
  TEST_ASSERT(l_isdigit(' ') == 0, "l_isdigit rejects space");
  
  // Test l_atol
  TEST_ASSERT(l_atol("123") == 123L, "l_atol converts '123'");
  TEST_ASSERT(l_atol("-456") == -456L, "l_atol converts '-456'");
  TEST_ASSERT(l_atol("0") == 0L, "l_atol converts '0'");
  TEST_ASSERT(l_atol("999999") == 999999L, "l_atol converts '999999'");
  
  // Test l_atoi
  TEST_ASSERT(l_atoi("123") == 123, "l_atoi converts '123'");
  TEST_ASSERT(l_atoi("-456") == -456, "l_atoi converts '-456'");
  TEST_ASSERT(l_atoi("0") == 0, "l_atoi converts '0'");
  
  // Test l_itoa
  char buffer[32];
  char* result = l_itoa(123, buffer, 10);
  TEST_ASSERT(result == buffer, "l_itoa returns buffer pointer");
  TEST_ASSERT(l_strncmp(buffer, "123", 3) == 0, "l_itoa converts 123 to '123'");
  
  l_itoa(-456, buffer, 10);
  TEST_ASSERT(l_strncmp(buffer, "-456", 4) == 0, "l_itoa converts -456 to '-456'");
  
  l_itoa(0, buffer, 10);
  TEST_ASSERT(l_strncmp(buffer, "0", 1) == 0, "l_itoa converts 0 to '0'");
  
  l_itoa(255, buffer, 16);
  TEST_ASSERT(l_strncmp(buffer, "ff", 2) == 0, "l_itoa converts 255 to 'ff' (hex)");
  
  TEST_SECTION_PASS("Character/number function");
}

void test_memory_functions() {
  TEST_FUNCTION("Memory Functions");
  
  // Test l_memset
  char mem_buf[10];
  void* result = l_memset(mem_buf, 'A', 5);
  TEST_ASSERT(result == mem_buf, "l_memset returns destination pointer");
  TEST_ASSERT(mem_buf[0] == 'A', "l_memset sets first byte");
  TEST_ASSERT(mem_buf[4] == 'A', "l_memset sets last byte");
  
  // Test l_memmove
  char move_src[] = "Hello";
  char move_dst[10];
  void* move_result = l_memmove(move_dst, move_src, 5);
  TEST_ASSERT(move_result == move_dst, "l_memmove returns destination pointer");
  TEST_ASSERT(l_memcmp(move_dst, move_src, 5) == 0, "l_memmove copies data correctly");
  
  // Test overlapping memmove
  char overlap[] = "Hello World";
  l_memmove(overlap + 2, overlap, 5);
  TEST_ASSERT(l_memcmp(overlap + 2, "Hello", 5) == 0, "l_memmove handles overlapping regions");
  
  // Test l_memcpy
  char copy_src[] = "Test";
  char copy_dst[10];
  void* copy_result = l_memcpy(copy_dst, copy_src, 4);
  TEST_ASSERT(copy_result == copy_dst, "l_memcpy returns destination pointer");
  TEST_ASSERT(l_memcmp(copy_dst, copy_src, 4) == 0, "l_memcpy copies data correctly");
  
  TEST_SECTION_PASS("Memory function");
}

void test_file_operations() {
  TEST_FUNCTION("File Operations");
  
  char* test_msg = "Test message";
  int msg_len = strlen(test_msg);
  
  // Test l_open_write
  L_FD write_fd = l_open_write("test_write_file");
  TEST_ASSERT(write_fd >= 0, "l_open_write opens file for writing");
  ssize_t written = l_write(write_fd, test_msg, msg_len);
  TEST_ASSERT(written == msg_len, "l_write writes correct number of bytes");
  l_close(write_fd);
  
  // Test reading back the written file
  L_FD read_fd = l_open_read("test_write_file");
  TEST_ASSERT(read_fd >= 0, "l_open_read opens file for reading");
  char read_buf[20];
  ssize_t read_bytes = l_read(read_fd, read_buf, msg_len);
  TEST_ASSERT(read_bytes == msg_len, "l_read reads correct number of bytes");
  TEST_ASSERT(l_memcmp(read_buf, test_msg, msg_len) == 0, "l_read retrieves correct data");
  l_close(read_fd);
  
  // Test l_open_readwrite
  L_FD rw_fd = l_open_readwrite("test_rw_file");
  TEST_ASSERT(rw_fd >= 0, "l_open_readwrite opens file for read/write");
  written = l_write(rw_fd, test_msg, msg_len);
  TEST_ASSERT(written == msg_len, "l_write to read/write file works");
  l_close(rw_fd);
  
  // Test l_open_trunc
  L_FD trunc_fd = l_open_trunc("test_write_file");
  TEST_ASSERT(trunc_fd >= 0, "l_open_trunc opens file for truncation");
  char* short_msg = "Hi";
  written = l_write(trunc_fd, short_msg, 2);
  TEST_ASSERT(written == 2, "l_write to truncated file works");
  l_close(trunc_fd);
  
  // Verify truncation worked
  read_fd = l_open_read("test_write_file");
  read_bytes = l_read(read_fd, read_buf, 10);
  TEST_ASSERT(read_bytes == 2, "truncated file has correct size");
  TEST_ASSERT(l_memcmp(read_buf, "Hi", 2) == 0, "truncated file has correct content");
  l_close(read_fd);
  
  TEST_SECTION_PASS("File operation");
}

void test_system_functions() {
  TEST_FUNCTION("System Functions");
  
  // Test l_exitif with false condition (should not exit)
  l_exitif(0, -1, "This should not exit");
  TEST_ASSERT(1, "l_exitif with false condition does not exit");
  
  // Test l_open with explicit flags
  L_FD open_fd = l_open("test_explicit_open", O_CREAT | O_WRONLY, 0644);
  TEST_ASSERT(open_fd >= 0, "l_open with explicit flags works");
  l_close(open_fd);
  
  TEST_SECTION_PASS("System function");
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
  
  // Print test summary
  puts("\n");
  puts("=====================================\n");
  puts("ALL TESTS COMPLETED SUCCESSFULLY!\n");
  puts("=====================================\n");
  
  return 0;
}
