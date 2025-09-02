#ifndef FILE_SYSTEM_MONITOR_H
#define FILE_SYSTEM_MONITOR_H

#include <string>
#include <functional>
#include <memory>

namespace FileSync {

/**
 * File system change event types
 */
enum class FileChangeType {
  Created,
  Modified,
  Deleted
};

/**
 * Callback function type for file system changes
 * Parameters: file_path, change_type
 */
using FileChangeCallback = std::function<void(const std::string&, FileChangeType)>;

/**
 * Monitors file system changes in a directory tree.
 * 
 * This class provides:
 * - Recursive directory monitoring
 * - Platform-specific optimizations (inotify, ReadDirectoryChangesW, etc.)
 * - Callback-based change notifications
 * - Filtering support for excluded patterns
 */
class FileSystemMonitor {
public:
  /**
   * Constructor
   * @param directory_path Path to monitor (will be monitored recursively)
   */
  explicit FileSystemMonitor(const std::string& directory_path);
  
  /**
   * Destructor
   */
  ~FileSystemMonitor();

  /**
   * Set callback function for file change notifications
   * @param callback Function to call when files change
   */
  void set_change_callback(const FileChangeCallback& callback);

  /**
   * Start monitoring the directory
   * @return true if successful, false otherwise
   */
  bool start_monitoring();

  /**
   * Stop monitoring the directory
   */
  void stop_monitoring();

  /**
   * Check if monitoring is active
   * @return true if monitoring, false otherwise
   */
  bool is_monitoring() const;

  /**
   * Add a pattern to exclude from monitoring (e.g., "*.tmp", "*~")
   * @param pattern Glob pattern to exclude
   */
  void add_excluded_pattern(const std::string& pattern);

  /**
   * Process pending file system events (for polling-based implementations)
   * This method should be called periodically in the main loop
   */
  void process_events();

private:
  // Non-copyable
  FileSystemMonitor(const FileSystemMonitor&) = delete;
  FileSystemMonitor& operator=(const FileSystemMonitor&) = delete;

  class Impl;
  std::unique_ptr<Impl> pimpl_;
};

} // namespace FileSync

#endif // FILE_SYSTEM_MONITOR_H