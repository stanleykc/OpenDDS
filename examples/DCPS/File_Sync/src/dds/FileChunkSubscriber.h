#ifndef FILE_CHUNK_SUBSCRIBER_H
#define FILE_CHUNK_SUBSCRIBER_H

#include "FileSyncTypeSupportImpl.h"
#include <dds/DCPS/LocalObject.h>
#include <memory>

namespace FileSync {

/**
 * Subscriber for FileChunk messages.
 * Receives and reassembles file chunks from peer nodes.
 */
class FileChunkSubscriber {
public:
  /**
   * Constructor
   * @param participant DDS domain participant
   */
  explicit FileChunkSubscriber(DDS::DomainParticipant_ptr participant);
  
  /**
   * Destructor
   */
  ~FileChunkSubscriber();

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
  FileChunkSubscriber(const FileChunkSubscriber&) = delete;
  FileChunkSubscriber& operator=(const FileChunkSubscriber&) = delete;

  class Impl;
  std::unique_ptr<Impl> pimpl_;
};

} // namespace FileSync

#endif // FILE_CHUNK_SUBSCRIBER_H