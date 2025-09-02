#ifndef FILE_CHUNK_PUBLISHER_H
#define FILE_CHUNK_PUBLISHER_H

#include "FileSyncTypeSupportImpl.h"
#include <dds/DCPS/LocalObject.h>
#include <memory>
#include <vector>

namespace FileSync {

/**
 * Publisher for FileChunk messages.
 * Publishes file content in manageable chunks.
 */
class FileChunkPublisher {
public:
  /**
   * Constructor
   * @param participant DDS domain participant
   */
  explicit FileChunkPublisher(DDS::DomainParticipant_ptr participant);
  
  /**
   * Destructor
   */
  ~FileChunkPublisher();

  /**
   * Initialize the publisher, topic, and data writer
   * @return true if successful, false otherwise
   */
  bool initialize();

  /**
   * Publish file content as chunks
   * @param file_id Relative path of the file
   * @param file_content Complete file content
   * @param file_hash SHA-256 hash of file content
   * @param chunk_size Size of each chunk in bytes
   * @return true if successful, false otherwise
   */
  bool publish_file_chunks(
    const std::string& file_id,
    const std::vector<uint8_t>& file_content,
    const std::string& file_hash,
    size_t chunk_size = 65536);

  /**
   * Shutdown and cleanup
   */
  void shutdown();

private:
  // Non-copyable
  FileChunkPublisher(const FileChunkPublisher&) = delete;
  FileChunkPublisher& operator=(const FileChunkPublisher&) = delete;

  class Impl;
  std::unique_ptr<Impl> pimpl_;
};

} // namespace FileSync

#endif // FILE_CHUNK_PUBLISHER_H