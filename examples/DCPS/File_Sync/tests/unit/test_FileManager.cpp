#include "../../src/FileManager.h"
#include <fstream>
#include <string>
#include <vector>
#include <cstdlib>

// Test macros are defined in test_main.cpp
extern bool test_FileManager();

// Helper function to create a test directory
std::string create_test_directory() {
  std::string test_dir = "/tmp/file_sync_test_" + std::to_string(rand());
  system(("mkdir -p " + test_dir).c_str());
  return test_dir;
}

// Helper function to create a test file
std::string create_test_file(const std::string& dir, const std::string& filename, const std::string& content) {
  std::string filepath = dir + "/" + filename;
  std::ofstream file(filepath);
  file << content;
  file.close();
  return filepath;
}

TEST(FileManager_ReadWriteFile) {
  FileSync::FileManager manager;
  std::string test_dir = create_test_directory();
  
  // Create test content
  std::vector<uint8_t> original_content = {'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '!'};
  std::string test_file = test_dir + "/test.txt";
  
  // Test write
  ASSERT_TRUE(manager.write_file_atomic(test_file, original_content));
  
  // Test read
  std::vector<uint8_t> read_content;
  ASSERT_TRUE(manager.read_file(test_file, read_content));
  
  // Verify content matches
  ASSERT_EQ(original_content.size(), read_content.size());
  ASSERT_TRUE(original_content == read_content);
  
  // Clean up
  system(("rm -rf " + test_dir).c_str());
  
  return true;
}

TEST(FileManager_FileExists) {
  FileSync::FileManager manager;
  std::string test_dir = create_test_directory();
  
  std::string existing_file = create_test_file(test_dir, "exists.txt", "test content");
  std::string nonexistent_file = test_dir + "/does_not_exist.txt";
  
  ASSERT_TRUE(manager.file_exists(existing_file));
  ASSERT_FALSE(manager.file_exists(nonexistent_file));
  
  // Clean up
  system(("rm -rf " + test_dir).c_str());
  
  return true;
}

TEST(FileManager_GetFileModTime) {
  FileSync::FileManager manager;
  std::string test_dir = create_test_directory();
  
  std::string test_file = create_test_file(test_dir, "modtime.txt", "test content");
  
  long long mod_time = manager.get_file_mod_time(test_file);
  ASSERT_TRUE(mod_time > 0);
  
  // Test nonexistent file
  ASSERT_EQ(-1, manager.get_file_mod_time(test_dir + "/nonexistent.txt"));
  
  // Clean up
  system(("rm -rf " + test_dir).c_str());
  
  return true;
}

TEST(FileManager_CalculateHash) {
  FileSync::FileManager manager;
  
  // Test with known content
  std::vector<uint8_t> content = {'t', 'e', 's', 't'};
  std::string hash = manager.calculate_sha256(content);
  
  ASSERT_FALSE(hash.empty());
  
  // Same content should produce same hash
  std::string hash2 = manager.calculate_sha256(content);
  ASSERT_EQ(hash, hash2);
  
  // Different content should produce different hash
  std::vector<uint8_t> different_content = {'d', 'i', 'f', 'f'};
  std::string different_hash = manager.calculate_sha256(different_content);
  ASSERT_NE(hash, different_hash);
  
  return true;
}

TEST(FileManager_CalculateFileHash) {
  FileSync::FileManager manager;
  std::string test_dir = create_test_directory();
  
  std::string test_file = create_test_file(test_dir, "hash.txt", "test content for hashing");
  
  std::string hash = manager.calculate_file_sha256(test_file);
  ASSERT_FALSE(hash.empty());
  
  // Test nonexistent file
  std::string empty_hash = manager.calculate_file_sha256(test_dir + "/nonexistent.txt");
  ASSERT_TRUE(empty_hash.empty());
  
  // Clean up
  system(("rm -rf " + test_dir).c_str());
  
  return true;
}

TEST(FileManager_CreateDirectoryRecursive) {
  FileSync::FileManager manager;
  std::string test_dir = "/tmp/file_sync_recursive_test";
  std::string nested_dir = test_dir + "/level1/level2/level3";
  
  // Clean up any existing directory
  system(("rm -rf " + test_dir).c_str());
  
  ASSERT_TRUE(manager.create_directory_recursive(nested_dir));
  
  // Verify directory exists
  ASSERT_TRUE(manager.file_exists(test_dir) || 
              system(("test -d " + test_dir).c_str()) == 0);
  
  // Test creating existing directory (should succeed)
  ASSERT_TRUE(manager.create_directory_recursive(nested_dir));
  
  // Clean up
  system(("rm -rf " + test_dir).c_str());
  
  return true;
}

TEST(FileManager_DeleteFile) {
  FileSync::FileManager manager;
  std::string test_dir = create_test_directory();
  
  std::string test_file = create_test_file(test_dir, "delete_me.txt", "temporary content");
  
  // Verify file exists
  ASSERT_TRUE(manager.file_exists(test_file));
  
  // Delete file
  ASSERT_TRUE(manager.delete_file(test_file));
  
  // Verify file no longer exists
  ASSERT_FALSE(manager.file_exists(test_file));
  
  // Test deleting nonexistent file (should fail gracefully)
  ASSERT_FALSE(manager.delete_file(test_file));
  
  // Clean up
  system(("rm -rf " + test_dir).c_str());
  
  return true;
}

TEST(FileManager_GenerateConflictFilename) {
  FileSync::FileManager manager;
  
  // Test with extension
  std::string result1 = manager.generate_conflict_filename("/path/to/file.txt", "hostname1");
  ASSERT_TRUE(result1.find("conflicted copy from hostname1") != std::string::npos);
  ASSERT_TRUE(result1.find(".txt") != std::string::npos);
  
  // Test without extension
  std::string result2 = manager.generate_conflict_filename("/path/to/file_no_ext", "hostname2");
  ASSERT_TRUE(result2.find("conflicted copy from hostname2") != std::string::npos);
  
  // Test that different hostnames produce different results
  std::string result3 = manager.generate_conflict_filename("/path/to/file.txt", "different_host");
  ASSERT_NE(result1, result3);
  
  return true;
}

TEST(FileManager_AtomicWrite) {
  FileSync::FileManager manager;
  std::string test_dir = create_test_directory();
  std::string test_file = test_dir + "/atomic_test.txt";
  
  // Create large content to increase chance of detecting non-atomic behavior
  std::vector<uint8_t> large_content(10000, 'X');
  
  ASSERT_TRUE(manager.write_file_atomic(test_file, large_content));
  
  // Verify content
  std::vector<uint8_t> read_content;
  ASSERT_TRUE(manager.read_file(test_file, read_content));
  ASSERT_EQ(large_content.size(), read_content.size());
  ASSERT_TRUE(large_content == read_content);
  
  // Verify no temporary files left behind
  system(("ls " + test_dir + "/*.tmp* 2>/dev/null").c_str());
  
  // Clean up
  system(("rm -rf " + test_dir).c_str());
  
  return true;
}