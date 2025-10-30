#include "../FileMonitor.h"
#include "../FileUtils.h"
#include <ace/Log_Msg.h>
#include <ace/OS_NS_unistd.h>
#include <ace/OS_NS_sys_stat.h>
#include <fstream>
#include <vector>
#include <algorithm>

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
    ACE_ERROR((LM_ERROR, ACE_TEXT("    Assertion failed: %d != %d at line %d\n"), \
               (int)(a), (int)(b), __LINE__)); \
    throw "Assertion failed"; \
  }

#define ASSERT_STR_EQ(a, b) \
  if (std::string(a) != std::string(b)) { \
    ACE_ERROR((LM_ERROR, ACE_TEXT("    Assertion failed: '%C' != '%C' at line %d\n"), \
               std::string(a).c_str(), std::string(b).c_str(), __LINE__)); \
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

// Helper: Create test directory
void create_test_directory(const char* dir)
{
  ACE_OS::mkdir(dir);
}

// Helper: Cleanup test directory
void cleanup_test_directory(const char* dir)
{
  std::vector<std::string> files;
  if (DirShare::list_directory_files(dir, files)) {
    for (size_t i = 0; i < files.size(); ++i) {
      std::string path = std::string(dir) + "/" + files[i];
      ACE_OS::unlink(path.c_str());
    }
  }
  ACE_OS::rmdir(dir);
}

// Test: Monitor detects file creation
TEST(detect_file_creation)
{
  const char* test_dir = "test_monitor_create";
  create_test_directory(test_dir);

  DirShare::FileMonitor monitor(test_dir);

  // Initial scan (empty directory)
  std::vector<std::string> created, modified, deleted;
  monitor.scan_for_changes(created, modified, deleted);
  ASSERT_EQ(created.size(), 0u);

  // Create a file
  std::string test_file = std::string(test_dir) + "/newfile.txt";
  std::ofstream file(test_file.c_str());
  file << "test content";
  file.close();

  // Scan again
  monitor.scan_for_changes(created, modified, deleted);
  ASSERT_EQ(created.size(), 1u);
  ASSERT_STR_EQ(created[0], "newfile.txt");
  ASSERT_EQ(modified.size(), 0u);
  ASSERT_EQ(deleted.size(), 0u);

  cleanup_test_directory(test_dir);
}

// Test: Monitor detects file modification
TEST(detect_file_modification)
{
  const char* test_dir = "test_monitor_modify";
  create_test_directory(test_dir);

  // Create initial file
  std::string test_file = std::string(test_dir) + "/testfile.txt";
  std::ofstream file(test_file.c_str());
  file << "initial content";
  file.close();

  DirShare::FileMonitor monitor(test_dir);

  // Initial scan (establish baseline)
  std::vector<std::string> created, modified, deleted;
  monitor.scan_for_changes(created, modified, deleted);
  ASSERT_EQ(created.size(), 1u);

  // Wait a bit to ensure timestamp changes
  ACE_OS::sleep(1);

  // Modify file
  file.open(test_file.c_str(), std::ios::trunc);
  file << "modified content that is different";
  file.close();

  // Scan again
  monitor.scan_for_changes(created, modified, deleted);
  ASSERT_EQ(created.size(), 0u);
  ASSERT_EQ(modified.size(), 1u);
  ASSERT_STR_EQ(modified[0], "testfile.txt");
  ASSERT_EQ(deleted.size(), 0u);

  cleanup_test_directory(test_dir);
}

// Test: Monitor detects file deletion
TEST(detect_file_deletion)
{
  const char* test_dir = "test_monitor_delete";
  create_test_directory(test_dir);

  // Create initial file
  std::string test_file = std::string(test_dir) + "/deleteme.txt";
  std::ofstream file(test_file.c_str());
  file << "to be deleted";
  file.close();

  DirShare::FileMonitor monitor(test_dir);

  // Initial scan (establish baseline)
  std::vector<std::string> created, modified, deleted;
  monitor.scan_for_changes(created, modified, deleted);
  ASSERT_EQ(created.size(), 1u);

  // Delete file
  ACE_OS::unlink(test_file.c_str());

  // Scan again
  monitor.scan_for_changes(created, modified, deleted);
  ASSERT_EQ(created.size(), 0u);
  ASSERT_EQ(modified.size(), 0u);
  ASSERT_EQ(deleted.size(), 1u);
  ASSERT_STR_EQ(deleted[0], "deleteme.txt");

  cleanup_test_directory(test_dir);
}

// Test: Monitor detects multiple changes
TEST(detect_multiple_changes)
{
  const char* test_dir = "test_monitor_multiple";
  create_test_directory(test_dir);

  // Create initial files
  std::string file1 = std::string(test_dir) + "/file1.txt";
  std::string file2 = std::string(test_dir) + "/file2.txt";
  std::ofstream(file1.c_str()) << "file1 content";
  std::ofstream(file2.c_str()) << "file2 content";

  DirShare::FileMonitor monitor(test_dir);

  // Initial scan
  std::vector<std::string> created, modified, deleted;
  monitor.scan_for_changes(created, modified, deleted);
  ASSERT_EQ(created.size(), 2u);

  // Wait for timestamp change
  ACE_OS::sleep(1);

  // Make multiple changes:
  // - Create file3
  // - Modify file1
  // - Delete file2
  std::string file3 = std::string(test_dir) + "/file3.txt";
  std::ofstream(file3.c_str()) << "file3 content";
  std::ofstream(file1.c_str(), std::ios::trunc) << "file1 modified";
  ACE_OS::unlink(file2.c_str());

  // Scan again
  monitor.scan_for_changes(created, modified, deleted);
  ASSERT_EQ(created.size(), 1u);
  ASSERT_TRUE(std::find(created.begin(), created.end(), "file3.txt") != created.end());
  ASSERT_EQ(modified.size(), 1u);
  ASSERT_TRUE(std::find(modified.begin(), modified.end(), "file1.txt") != modified.end());
  ASSERT_EQ(deleted.size(), 1u);
  ASSERT_TRUE(std::find(deleted.begin(), deleted.end(), "file2.txt") != deleted.end());

  cleanup_test_directory(test_dir);
}

// Test: Get all files
TEST(get_all_files)
{
  const char* test_dir = "test_monitor_getall";
  create_test_directory(test_dir);

  // Create test files
  std::string file1 = std::string(test_dir) + "/file1.txt";
  std::string file2 = std::string(test_dir) + "/file2.txt";
  std::ofstream(file1.c_str()) << "test1";
  std::ofstream(file2.c_str()) << "test2";

  DirShare::FileMonitor monitor(test_dir);

  // Get all files
  std::vector<DirShare::FileMetadata> files = monitor.get_all_files();
  ASSERT_EQ(files.size(), 2u);

  // Verify metadata contains filenames
  bool found_file1 = false, found_file2 = false;
  for (size_t i = 0; i < files.size(); ++i) {
    std::string name = files[i].filename.in();
    if (name == "file1.txt") found_file1 = true;
    if (name == "file2.txt") found_file2 = true;
  }
  ASSERT_TRUE(found_file1);
  ASSERT_TRUE(found_file2);

  cleanup_test_directory(test_dir);
}

// Test: Get file metadata
TEST(get_file_metadata)
{
  const char* test_dir = "test_monitor_metadata";
  create_test_directory(test_dir);

  // Create test file with known content
  std::string test_file = std::string(test_dir) + "/metadata_test.txt";
  const char* content = "test content for metadata";
  std::ofstream file(test_file.c_str(), std::ios::binary);
  file.write(content, strlen(content));
  file.close();

  DirShare::FileMonitor monitor(test_dir);

  // Get file metadata
  DirShare::FileMetadata metadata;
  bool result = monitor.get_file_metadata("metadata_test.txt", metadata);
  ASSERT_TRUE(result);

  // Verify metadata
  ASSERT_STR_EQ(std::string(metadata.filename.in()), "metadata_test.txt");
  ASSERT_EQ(metadata.size, strlen(content));
  ASSERT_TRUE(metadata.checksum != 0); // Should have valid checksum

  cleanup_test_directory(test_dir);
}

// Test: Monitor with nonexistent directory
TEST(nonexistent_directory)
{
  const char* test_dir = "nonexistent_dir_12345";

  // Constructor should handle nonexistent directory gracefully
  DirShare::FileMonitor monitor(test_dir);

  // Scan should fail gracefully
  std::vector<std::string> created, modified, deleted;
  bool result = monitor.scan_for_changes(created, modified, deleted);
  ASSERT_FALSE(result);
}

int ACE_TMAIN(int, ACE_TCHAR*[])
{
  ACE_DEBUG((LM_INFO, ACE_TEXT("=== FileMonitor Unit Tests ===\n")));

  run_test_detect_file_creation();
  run_test_detect_file_modification();
  run_test_detect_file_deletion();
  run_test_detect_multiple_changes();
  run_test_get_all_files();
  run_test_get_file_metadata();
  run_test_nonexistent_directory();

  ACE_DEBUG((LM_INFO, ACE_TEXT("\n=== Test Results ===\n")));
  ACE_DEBUG((LM_INFO, ACE_TEXT("  Passed: %d\n"), g_tests_passed));
  ACE_DEBUG((LM_INFO, ACE_TEXT("  Failed: %d\n"), g_tests_failed));

  return g_tests_failed > 0 ? 1 : 0;
}
