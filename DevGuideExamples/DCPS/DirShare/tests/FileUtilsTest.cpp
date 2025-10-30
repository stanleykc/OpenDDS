#include "../FileUtils.h"
#include <ace/Log_Msg.h>
#include <ace/OS_NS_unistd.h>
#include <ace/OS_NS_sys_stat.h>
#include <fstream>
#include <vector>

// Simple test framework
int g_tests_passed = 0;
int g_tests_failed = 0;

#define TEST(name) \
  void test_##name(); \
  void run_test_##name() { \
    ACE_DEBUG((LM_INFO, ACE_TEXT("Running test: %C\n"), #name)); \
    try { \
      test_##name(); \
      ACE_DEBUG((LM_INFO, ACE_TEXT("  PASS: %C\n"), #name)); \
      g_tests_passed++; \
    } catch (const char* error) { \
      ACE_ERROR((LM_ERROR, ACE_TEXT("  FAIL: %C - %C\n"), #name, error)); \
      g_tests_failed++; \
    } \
  } \
  void test_##name()

#define ASSERT_EQ(a, b) \
  if ((a) != (b)) { \
    ACE_ERROR((LM_ERROR, ACE_TEXT("    Assertion failed: %llu != %llu at line %d\n"), \
               (unsigned long long)(a), (unsigned long long)(b), __LINE__)); \
    throw "Assertion failed"; \
  }

#define ASSERT_TRUE(expr) \
  if (!(expr)) { \
    ACE_ERROR((LM_ERROR, ACE_TEXT("    Assertion failed: %C at line %d\n"), \
               #expr, __LINE__)); \
    throw "Assertion failed"; \
  }

#define ASSERT_FALSE(expr) \
  if ((expr)) { \
    ACE_ERROR((LM_ERROR, ACE_TEXT("    Assertion failed: !(%C) at line %d\n"), \
               #expr, __LINE__)); \
    throw "Assertion failed"; \
  }

// Test: Write and read file
TEST(write_read_file)
{
  const char* test_file = "test_write_read.txt";
  const unsigned char test_data[] = "Hello, FileUtils!";
  size_t data_size = sizeof(test_data) - 1; // Exclude null terminator

  // Write file
  bool write_result = DirShare::write_file(test_file, test_data, data_size);
  ASSERT_TRUE(write_result);

  // Read file
  std::vector<unsigned char> read_data;
  bool read_result = DirShare::read_file(test_file, read_data);
  ASSERT_TRUE(read_result);
  ASSERT_EQ(read_data.size(), data_size);

  // Verify content
  for (size_t i = 0; i < data_size; ++i) {
    ASSERT_EQ(read_data[i], test_data[i]);
  }

  // Cleanup
  ACE_OS::unlink(test_file);
}

// Test: File exists
TEST(file_exists)
{
  const char* test_file = "test_exists.txt";

  // File should not exist initially
  ASSERT_FALSE(DirShare::file_exists(test_file));

  // Create file
  std::ofstream file(test_file);
  file << "test";
  file.close();

  // File should now exist
  ASSERT_TRUE(DirShare::file_exists(test_file));

  // Cleanup
  ACE_OS::unlink(test_file);
}

// Test: Get file size
TEST(get_file_size)
{
  const char* test_file = "test_size.txt";
  const char* test_data = "12345";
  size_t expected_size = 5;

  // Create file
  std::ofstream file(test_file, std::ios::binary);
  file.write(test_data, expected_size);
  file.close();

  // Get file size
  unsigned long long size;
  bool result = DirShare::get_file_size(test_file, size);
  ASSERT_TRUE(result);
  ASSERT_EQ(size, expected_size);

  // Cleanup
  ACE_OS::unlink(test_file);
}

// Test: Delete file
TEST(delete_file)
{
  const char* test_file = "test_delete.txt";

  // Create file
  std::ofstream file(test_file);
  file << "test";
  file.close();

  ASSERT_TRUE(DirShare::file_exists(test_file));

  // Delete file
  bool result = DirShare::delete_file(test_file);
  ASSERT_TRUE(result);
  ASSERT_FALSE(DirShare::file_exists(test_file));
}

// Test: Is directory
TEST(is_directory)
{
  const char* test_dir = "test_dir";
  const char* test_file = "test_file.txt";

  // Create directory
  ACE_OS::mkdir(test_dir);
  ASSERT_TRUE(DirShare::is_directory(test_dir));

  // Create file
  std::ofstream file(test_file);
  file << "test";
  file.close();

  // File should not be a directory
  ASSERT_FALSE(DirShare::is_directory(test_file));

  // Cleanup
  ACE_OS::rmdir(test_dir);
  ACE_OS::unlink(test_file);
}

// Test: List directory files
TEST(list_directory_files)
{
  const char* test_dir = "test_list_dir";
  ACE_OS::mkdir(test_dir);

  // Create test files
  std::string file1 = std::string(test_dir) + "/file1.txt";
  std::string file2 = std::string(test_dir) + "/file2.txt";
  std::ofstream(file1.c_str()) << "test1";
  std::ofstream(file2.c_str()) << "test2";

  // List files
  std::vector<std::string> files;
  bool result = DirShare::list_directory_files(test_dir, files);
  ASSERT_TRUE(result);
  ASSERT_EQ(files.size(), 2u);

  // Cleanup
  ACE_OS::unlink(file1.c_str());
  ACE_OS::unlink(file2.c_str());
  ACE_OS::rmdir(test_dir);
}

// Test: Validate filename - safe names
TEST(validate_filename_safe)
{
  ASSERT_TRUE(DirShare::is_valid_filename("file.txt"));
  ASSERT_TRUE(DirShare::is_valid_filename("document.pdf"));
  ASSERT_TRUE(DirShare::is_valid_filename("my-file_123.txt"));
}

// Test: Validate filename - reject path traversal
TEST(validate_filename_reject_traversal)
{
  ASSERT_FALSE(DirShare::is_valid_filename("../etc/passwd"));
  ASSERT_FALSE(DirShare::is_valid_filename("..\\windows\\system32"));
  ASSERT_FALSE(DirShare::is_valid_filename("file/../../etc"));
}

// Test: Validate filename - reject absolute paths
TEST(validate_filename_reject_absolute)
{
  ASSERT_FALSE(DirShare::is_valid_filename("/etc/passwd"));
  ASSERT_FALSE(DirShare::is_valid_filename("\\Windows\\System32"));
  ASSERT_FALSE(DirShare::is_valid_filename("C:\\Windows"));
}

// Test: Validate filename - reject subdirectories
TEST(validate_filename_reject_subdirs)
{
  ASSERT_FALSE(DirShare::is_valid_filename("subdir/file.txt"));
  ASSERT_FALSE(DirShare::is_valid_filename("subdir\\file.txt"));
}

// Test: Get and set file modification time
TEST(get_set_file_mtime)
{
  const char* test_file = "test_mtime.txt";

  // Create file
  std::ofstream file(test_file);
  file << "test";
  file.close();

  // Get original modification time
  unsigned long long orig_sec, new_sec;
  unsigned long orig_nsec, new_nsec;
  bool result = DirShare::get_file_mtime(test_file, orig_sec, orig_nsec);
  ASSERT_TRUE(result);

  // Set new modification time (1 hour earlier)
  unsigned long long test_sec = orig_sec - 3600;
  unsigned long test_nsec = 0;
  result = DirShare::set_file_mtime(test_file, test_sec, test_nsec);
  ASSERT_TRUE(result);

  // Verify modification time changed
  result = DirShare::get_file_mtime(test_file, new_sec, new_nsec);
  ASSERT_TRUE(result);
  ASSERT_EQ(new_sec, test_sec);

  // Cleanup
  ACE_OS::unlink(test_file);
}

int ACE_TMAIN(int, ACE_TCHAR*[])
{
  ACE_DEBUG((LM_INFO, ACE_TEXT("=== FileUtils Unit Tests ===\n")));

  run_test_write_read_file();
  run_test_file_exists();
  run_test_get_file_size();
  run_test_delete_file();
  run_test_is_directory();
  run_test_list_directory_files();
  run_test_validate_filename_safe();
  run_test_validate_filename_reject_traversal();
  run_test_validate_filename_reject_absolute();
  run_test_validate_filename_reject_subdirs();
  run_test_get_set_file_mtime();

  ACE_DEBUG((LM_INFO, ACE_TEXT("\n=== Test Results ===\n")));
  ACE_DEBUG((LM_INFO, ACE_TEXT("  Passed: %d\n"), g_tests_passed));
  ACE_DEBUG((LM_INFO, ACE_TEXT("  Failed: %d\n"), g_tests_failed));

  return g_tests_failed > 0 ? 1 : 0;
}
