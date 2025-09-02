#ifndef FILE_METADATA_SUBSCRIBER_H
#define FILE_METADATA_SUBSCRIBER_H

#include "FileSyncTypeSupportImpl.h"
#include <dds/DCPS/LocalObject.h>
#include <memory>

namespace FileSync {

/**
 * Subscriber for FileMetadata messages.
 * Receives file metadata changes from peer nodes.
 */
class FileMetadataSubscriber {
public:
  /**
   * Constructor
   * @param participant DDS domain participant
   */
  explicit FileMetadataSubscriber(DDS::DomainParticipant_ptr participant);
  
  /**
   * Destructor
   */
  ~FileMetadataSubscriber();

  /**
   * Initialize the subscriber, topic, and data reader
   * @return true if successful, false otherwise
   */
  bool initialize();

  /**
   * Shutdown and cleanup
   */
  void shutdown();

private:
  // Non-copyable
  FileMetadataSubscriber(const FileMetadataSubscriber&) = delete;
  FileMetadataSubscriber& operator=(const FileMetadataSubscriber&) = delete;

  class Impl;
  std::unique_ptr<Impl> pimpl_;
};

} // namespace FileSync

#endif // FILE_METADATA_SUBSCRIBER_H