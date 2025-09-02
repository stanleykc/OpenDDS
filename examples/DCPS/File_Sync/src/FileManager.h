#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include <string>
#include <vector>
#include <memory>

namespace FileSync {

/**
 * Manages file I/O operations for the File_Sync application.
 * 
 * This class provides:
 * - Safe file reading and writing operations
 * - Atomic file writes using temporary files
 * - SHA-256 hash calculation and verification
 * - Directory management and creation
 */
class FileManager {
public:
  /**
   * Constructor
   */
  FileManager();
  
  /**
   * Destructor
   */
  ~FileManager();

  /**
   * Read entire file content into memory
   * @param file_path Path to the file to read
   * @param content Output vector to store file content
   * @return true if successful, false otherwise
   */
  bool read_file(const std::string& file_path, std::vector<uint8_t>& content);

  /**
   * Write content to file atomically (using temporary file + rename)
   * @param file_path Path where to write the file
   * @param content File content to write
   * @return true if successful, false otherwise
   */
  bool write_file_atomic(const std::string& file_path, const std::vector<uint8_t>& content);

  /**
   * Calculate SHA-256 hash of file content
   * @param content File content
   * @return SHA-256 hash as hex string, empty string on error
   */
  std::string calculate_sha256(const std::vector<uint8_t>& content);

  /**
   * Calculate SHA-256 hash of a file
   * @param file_path Path to the file
   * @return SHA-256 hash as hex string, empty string on error
   */
  std::string calculate_file_sha256(const std::string& file_path);

  /**
   * Check if file exists
   * @param file_path Path to check
   * @return true if file exists and is readable, false otherwise
   */
  bool file_exists(const std::string& file_path);

  /**
   * Get file modification time
   * @param file_path Path to the file
   * @return Modification time in seconds since epoch, -1 on error
   */
  long long get_file_mod_time(const std::string& file_path);

  /**
   * Create directory recursively if it doesn't exist
   * @param dir_path Directory path to create
   * @return true if successful or already exists, false otherwise
   */
  bool create_directory_recursive(const std::string& dir_path);

  /**
   * Delete file
   * @param file_path Path to the file to delete
   * @return true if successful, false otherwise
   */
  bool delete_file(const std::string& file_path);

  /**
   * Generate conflict filename for a file that has local modifications
   * @param original_path Original file path
   * @param peer_hostname Hostname of the peer that sent the conflicting version
   * @return New filename for the conflicted copy
   */
  std::string generate_conflict_filename(
    const std::string& original_path, 
    const std::string& peer_hostname);

private:
  // Non-copyable
  FileManager(const FileManager&) = delete;
  FileManager& operator=(const FileManager&) = delete;

  class Impl;
  std::unique_ptr<Impl> pimpl_;
};

} // namespace FileSync

#endif // FILE_MANAGER_H