#ifndef FILE_METADATA_PUBLISHER_H
#define FILE_METADATA_PUBLISHER_H

#include "FileSyncTypeSupportImpl.h"
#include <dds/DCPS/LocalObject.h>
#include <memory>

namespace FileSync {

/**
 * Publisher for FileMetadata messages.
 * Publishes file metadata changes (create, modify, delete events).
 */
class FileMetadataPublisher {
public:
  /**
   * Constructor
   * @param participant DDS domain participant
   */
  explicit FileMetadataPublisher(DDS::DomainParticipant_ptr participant);
  
  /**
   * Destructor
   */
  ~FileMetadataPublisher();

  /**
   * Initialize the publisher, topic, and data writer
   * @return true if successful, false otherwise
   */
  bool initialize();

  /**
   * Publish metadata for a new or modified file
   * @param file_id Relative path of the file
   * @param mod_time Modification timestamp
   * @param file_hash SHA-256 hash of file content
   * @param publisher_id Unique identifier for this peer
   * @return true if successful, false otherwise
   */
  bool publish_file_created_or_modified(
    const std::string& file_id,
    long long mod_time,
    const std::string& file_hash,
    const std::string& publisher_id);

  /**
   * Publish metadata for a deleted file
   * @param file_id Relative path of the file
   * @param publisher_id Unique identifier for this peer
   * @return true if successful, false otherwise
   */
  bool publish_file_deleted(
    const std::string& file_id,
    const std::string& publisher_id);

  /**
   * Shutdown and cleanup
   */
  void shutdown();

private:
  // Non-copyable
  FileMetadataPublisher(const FileMetadataPublisher&) = delete;
  FileMetadataPublisher& operator=(const FileMetadataPublisher&) = delete;

  class Impl;
  std::unique_ptr<Impl> pimpl_;
};

} // namespace FileSync

#endif // FILE_METADATA_PUBLISHER_H