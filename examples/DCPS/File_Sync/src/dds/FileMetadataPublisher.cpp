#include "FileMetadataPublisher.h"
#include <iostream>

namespace FileSync {

class FileMetadataPublisher::Impl {
public:
  explicit Impl(DDS::DomainParticipant_ptr participant)
    : participant_(DDS::DomainParticipant::_duplicate(participant))
  {
  }

  ~Impl() {
    shutdown();
  }

  bool initialize() {
    // TODO: Create topic, publisher, and data writer
    std::cout << "INFO: FileMetadataPublisher initialized (stub)" << std::endl;
    return true;
  }

  bool publish_file_created_or_modified(
    const std::string& file_id,
    long long mod_time,
    const std::string& file_hash,
    const std::string& publisher_id)
  {
    // TODO: Create and publish FileMetadata sample
    std::cout << "INFO: Publishing file metadata for: " << file_id 
              << " (hash: " << file_hash.substr(0, 8) << "...)" << std::endl;
    return true;
  }

  bool publish_file_deleted(
    const std::string& file_id,
    const std::string& publisher_id)
  {
    // TODO: Create and publish FileMetadata deletion sample
    std::cout << "INFO: Publishing file deletion for: " << file_id << std::endl;
    return true;
  }

  void shutdown() {
    // TODO: Cleanup DDS resources
    std::cout << "INFO: FileMetadataPublisher shutdown" << std::endl;
  }

private:
  DDS::DomainParticipant_var participant_;
  // TODO: Add DDS objects (topic, publisher, data writer)
};

FileMetadataPublisher::FileMetadataPublisher(DDS::DomainParticipant_ptr participant)
  : pimpl_(std::make_unique<Impl>(participant))
{
}

FileMetadataPublisher::~FileMetadataPublisher() = default;

bool FileMetadataPublisher::initialize() {
  return pimpl_->initialize();
}

bool FileMetadataPublisher::publish_file_created_or_modified(
  const std::string& file_id,
  long long mod_time,
  const std::string& file_hash,
  const std::string& publisher_id)
{
  return pimpl_->publish_file_created_or_modified(file_id, mod_time, file_hash, publisher_id);
}

bool FileMetadataPublisher::publish_file_deleted(
  const std::string& file_id,
  const std::string& publisher_id)
{
  return pimpl_->publish_file_deleted(file_id, publisher_id);
}

void FileMetadataPublisher::shutdown() {
  pimpl_->shutdown();
}

} // namespace FileSync