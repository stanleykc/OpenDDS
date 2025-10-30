#ifndef DIRSHARE_FILEMONITOR_H
#define DIRSHARE_FILEMONITOR_H

#include "DirShareTypeSupportImpl.h"
#include <ace/Thread_Mutex.h>
#include <map>
#include <string>

namespace DirShare {

/**
 * FileMonitor: Monitors a directory for file system changes
 * Uses polling-based approach (1-2 second intervals) for cross-platform simplicity
 */
class FileMonitor {
public:
  /**
   * Constructor
   * @param directory_path Path to the directory to monitor
   */
  explicit FileMonitor(const std::string& directory_path);

  /**
   * Destructor
   */
  ~FileMonitor();

  /**
   * Scan the directory and detect changes since last scan
   * Compares current directory state with previous state
   * @param created_files Output: list of newly created files
   * @param modified_files Output: list of modified files
   * @param deleted_files Output: list of deleted files
   * @return true if scan successful, false on error
   */
  bool scan_for_changes(
    std::vector<std::string>& created_files,
    std::vector<std::string>& modified_files,
    std::vector<std::string>& deleted_files);

  /**
   * Get list of all files currently in directory
   * Used for initial directory snapshot
   * @return vector of FileMetadata for all files
   */
  std::vector<FileMetadata> get_all_files();

  /**
   * Get FileMetadata for a specific file
   * @param filename Filename relative to monitored directory
   * @param metadata Output: FileMetadata structure
   * @return true if file exists and metadata retrieved, false otherwise
   */
  bool get_file_metadata(const std::string& filename, FileMetadata& metadata);

private:
  /**
   * Internal file state tracking structure
   */
  struct FileState {
    unsigned long long size;
    unsigned long long timestamp_sec;
    unsigned long timestamp_nsec;
    unsigned long checksum;
  };

  std::string directory_path_;
  std::map<std::string, FileState> previous_state_;
  ACE_Thread_Mutex mutex_;

  /**
   * Build full path from relative filename
   */
  std::string build_path(const std::string& filename) const;

  /**
   * Calculate checksum for a file
   */
  bool calculate_file_checksum(const std::string& full_path, unsigned long& checksum);

  /**
   * Get modification time for a file
   */
  bool get_modification_time(const std::string& full_path,
                             unsigned long long& sec,
                             unsigned long& nsec);
};

} // namespace DirShare

#endif // DIRSHARE_FILEMONITOR_H
