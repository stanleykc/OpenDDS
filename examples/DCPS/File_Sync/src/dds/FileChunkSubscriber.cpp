#include "FileChunkSubscriber.h"
#include <iostream>

namespace FileSync {

class FileChunkSubscriber::Impl {
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
    std::cout << "INFO: FileChunkSubscriber initialized (stub)" << std::endl;
    return true;
  }

  void shutdown() {
    // TODO: Cleanup DDS resources
    std::cout << "INFO: FileChunkSubscriber shutdown" << std::endl;
  }

private:
  DDS::DomainParticipant_var participant_;
  // TODO: Add DDS objects (topic, subscriber, data reader)
};

FileChunkSubscriber::FileChunkSubscriber(DDS::DomainParticipant_ptr participant)
  : pimpl_(std::make_unique<Impl>(participant))
{
}

FileChunkSubscriber::~FileChunkSubscriber() = default;

bool FileChunkSubscriber::initialize() {
  return pimpl_->initialize();
}

void FileChunkSubscriber::shutdown() {
  pimpl_->shutdown();
}

} // namespace FileSync