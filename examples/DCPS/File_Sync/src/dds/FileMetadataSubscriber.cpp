#include "FileMetadataSubscriber.h"
#include <iostream>

namespace FileSync {

class FileMetadataSubscriber::Impl {
public:
  explicit Impl(DDS::DomainParticipant_ptr participant)
    : participant_(DDS::DomainParticipant::_duplicate(participant))
  {
  }

  ~Impl() {
    shutdown();
  }

  bool initialize() {
    // TODO: Create topic, subscriber, data reader, and listener
    std::cout << "INFO: FileMetadataSubscriber initialized (stub)" << std::endl;
    return true;
  }

  void shutdown() {
    // TODO: Cleanup DDS resources
    std::cout << "INFO: FileMetadataSubscriber shutdown" << std::endl;
  }

private:
  DDS::DomainParticipant_var participant_;
  // TODO: Add DDS objects (topic, subscriber, data reader)
};

FileMetadataSubscriber::FileMetadataSubscriber(DDS::DomainParticipant_ptr participant)
  : pimpl_(std::make_unique<Impl>(participant))
{
}

FileMetadataSubscriber::~FileMetadataSubscriber() = default;

bool FileMetadataSubscriber::initialize() {
  return pimpl_->initialize();
}

void FileMetadataSubscriber::shutdown() {
  pimpl_->shutdown();
}

} // namespace FileSync