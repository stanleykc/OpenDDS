#include "../Checksum.h"
#include <ace/Log_Msg.h>
#include <ace/OS_NS_unistd.h>
#include <ace/OS_NS_fcntl.h>
#include <fstream>
#include <vector>
#include <cstring>

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

#define ASSERT_TRUE(expr) \
  if (!(expr)) { \
    ACE_ERROR((LM_ERROR, ACE_TEXT("    Assertion failed: %C at line %d\n"), \
               #expr, __LINE__)); \
    throw "Assertion failed"; \
  }

// Test: CRC32 of empty data
TEST(crc32_empty_data)
{
  const unsigned char data[] = "";
  unsigned long crc = DirShare::calculate_crc32(data, 0);
  // CRC32 of empty data should be 0
  ASSERT_EQ(crc, 0);
}

// Test: CRC32 of known data
TEST(crc32_known_value)
{
  const unsigned char data[] = "123456789";
  unsigned long crc = DirShare::calculate_crc32(data, strlen((const char*)data));
  // Known CRC32 value for "123456789" is 0xCBF43926
  ASSERT_EQ(crc, 0xCBF43926);
}

// Test: CRC32 incremental calculation
TEST(crc32_incremental)
{
  const unsigned char data[] = "123456789";

  // Calculate in one pass
  unsigned long crc_full = DirShare::calculate_crc32(data, strlen((const char*)data));

  // Calculate incrementally
  unsigned long crc_inc = 0xFFFFFFFF;
  crc_inc = DirShare::calculate_crc32_incremental(data, 4, crc_inc);
  crc_inc = DirShare::calculate_crc32_incremental(data + 4, 5, crc_inc);
  crc_inc = DirShare::finalize_crc32(crc_inc);

  ASSERT_EQ(crc_full, crc_inc);
}

// Test: CRC32 file calculation
TEST(crc32_file)
{
  const char* test_file = "test_crc32_file.txt";
  const char* test_data = "Hello, World!";

  // Create test file
  std::ofstream file(test_file, std::ios::binary);
  file.write(test_data, strlen(test_data));
  file.close();

  // Calculate CRC32 of file
  unsigned long checksum;
  bool result = DirShare::calculate_file_crc32(test_file, checksum);

  ASSERT_TRUE(result);

  // Verify against direct calculation
  unsigned long expected = DirShare::calculate_crc32(
    (const unsigned char*)test_data, strlen(test_data));
  ASSERT_EQ(checksum, expected);

  // Cleanup
  ACE_OS::unlink(test_file);
}

// Test: CRC32 file not found
TEST(crc32_file_not_found)
{
  unsigned long checksum;
  bool result = DirShare::calculate_file_crc32("nonexistent_file.txt", checksum);
  ASSERT_TRUE(!result); // Should fail
}

// Test: CRC32 consistency
TEST(crc32_consistency)
{
  const unsigned char data[] = "Test data for consistency check";
  unsigned long crc1 = DirShare::calculate_crc32(data, strlen((const char*)data));
  unsigned long crc2 = DirShare::calculate_crc32(data, strlen((const char*)data));
  ASSERT_EQ(crc1, crc2); // Should be deterministic
}

int ACE_TMAIN(int, ACE_TCHAR*[])
{
  ACE_DEBUG((LM_INFO, ACE_TEXT("=== Checksum Unit Tests ===\n")));

  run_test_crc32_empty_data();
  run_test_crc32_known_value();
  run_test_crc32_incremental();
  run_test_crc32_file();
  run_test_crc32_file_not_found();
  run_test_crc32_consistency();

  ACE_DEBUG((LM_INFO, ACE_TEXT("\n=== Test Results ===\n")));
  ACE_DEBUG((LM_INFO, ACE_TEXT("  Passed: %d\n"), g_tests_passed));
  ACE_DEBUG((LM_INFO, ACE_TEXT("  Failed: %d\n"), g_tests_failed));

  return g_tests_failed > 0 ? 1 : 0;
}
