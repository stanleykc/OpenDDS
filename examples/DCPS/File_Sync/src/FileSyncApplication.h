#ifndef FILESYNC_APPLICATION_H
#define FILESYNC_APPLICATION_H

#include <string>

namespace FileSync {

class ConfigurationManager;

/**
 * Main application class that orchestrates all File_Sync components.
 * 
 * This class is responsible for:
 * - Initializing OpenDDS participant and topics
 * - Managing the lifecycle of all components
 * - Coordinating between file monitor and DDS publishers/subscribers
 * - Handling graceful shutdown
 */
class FileSyncApplication {
public:
  /**
   * Constructor
   * @param config_manager Reference to configuration manager
   */
  explicit FileSyncApplication(const ConfigurationManager& config_manager);
  
  /**
   * Destructor
   */
  ~FileSyncApplication();

  /**
   * Initialize the application and all its components
   * @param argc Command line argument count (for DDS initialization)
   * @param argv Command line arguments (for DDS initialization)
   * @return true if initialization successful, false otherwise
   */
  bool initialize(int argc, char* argv[]);

  /**
   * Run the application (blocks until shutdown requested)
   * @return Exit status code
   */
  int run();

  /**
   * Shutdown the application and cleanup all resources
   */
  void shutdown();

  /**
   * Check if application is running
   * @return true if running, false otherwise
   */
  bool is_running() const;

  /**
   * Request application shutdown (can be called from signal handlers)
   */
  void request_shutdown();

private:
  // Non-copyable
  FileSyncApplication(const FileSyncApplication&) = delete;
  FileSyncApplication& operator=(const FileSyncApplication&) = delete;

  // Forward declarations for implementation details
  class Impl;
  std::unique_ptr<Impl> pimpl_;
};

} // namespace FileSync

#endif // FILESYNC_APPLICATION_H