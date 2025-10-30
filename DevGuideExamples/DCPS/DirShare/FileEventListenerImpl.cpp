#include "FileEventListenerImpl.h"
#include "FileUtils.h"
#include <ace/Log_Msg.h>
#include <ace/OS_NS_string.h>

namespace DirShare {

FileEventListenerImpl::FileEventListenerImpl(
  const std::string& shared_directory,
  DDS::DataWriter_ptr content_writer,
  DDS::DataWriter_ptr chunk_writer)
  : shared_directory_(shared_directory)
  , content_writer_(DDS::DataWriter::_duplicate(content_writer))
  , chunk_writer_(DDS::DataWriter::_duplicate(chunk_writer))
{
}

FileEventListenerImpl::~FileEventListenerImpl()
{
}

void FileEventListenerImpl::on_data_available(DDS::DataReader_ptr reader)
{
  FileEventDataReader_var event_reader = FileEventDataReader::_narrow(reader);
  if (!event_reader) {
    ACE_ERROR((LM_ERROR,
               ACE_TEXT("ERROR: %N:%l: FileEventListenerImpl::on_data_available() - ")
               ACE_TEXT("failed to narrow reader\n")));
    return;
  }

  FileEvent event;
  DDS::SampleInfo info;

  DDS::ReturnCode_t ret = event_reader->take_next_sample(event, info);

  while (ret == DDS::RETCODE_OK) {
    if (info.valid_data) {
      std::string filename = event.filename.in();

      ACE_DEBUG((LM_INFO,
                 ACE_TEXT("(%P|%t) FileEvent received: %C (operation: %d)\n"),
                 filename.c_str(),
                 event.operation));

      // Validate filename for security
      if (!is_valid_filename(filename)) {
        ACE_ERROR((LM_ERROR,
                   ACE_TEXT("ERROR: %N:%l: Invalid filename detected: %C\n"),
                   filename.c_str()));
        ret = event_reader->take_next_sample(event, info);
        continue;
      }

      // Dispatch based on operation type
      switch (event.operation) {
      case DirShare::CREATE:
        handle_create_event(event);
        break;

      case DirShare::MODIFY:
        handle_modify_event(event);
        break;

      case DirShare::DELETE:
        handle_delete_event(event);
        break;

      default:
        ACE_ERROR((LM_ERROR,
                   ACE_TEXT("ERROR: %N:%l: Unknown operation type: %d\n"),
                   event.operation));
        break;
      }
    }

    ret = event_reader->take_next_sample(event, info);
  }

  if (ret != DDS::RETCODE_NO_DATA) {
    ACE_ERROR((LM_ERROR,
               ACE_TEXT("ERROR: %N:%l: FileEventListenerImpl::on_data_available() - ")
               ACE_TEXT("take_next_sample failed: %d\n"),
               ret));
  }
}

void FileEventListenerImpl::handle_create_event(const FileEvent& event)
{
  std::string filename = event.filename.in();
  std::string full_path = shared_directory_ + "/" + filename;

  ACE_DEBUG((LM_INFO,
             ACE_TEXT("(%P|%t) Handling CREATE event for: %C\n"),
             filename.c_str()));

  // Check if file already exists locally
  if (file_exists(full_path)) {
    ACE_DEBUG((LM_INFO,
               ACE_TEXT("(%P|%t) File already exists locally, skipping: %C\n"),
               filename.c_str()));
    return;
  }

  // File will be received via FileContent or FileChunk topic
  // The listener will handle writing the file when content arrives
  ACE_DEBUG((LM_INFO,
             ACE_TEXT("(%P|%t) Waiting for file content to arrive for: %C\n"),
             filename.c_str()));
}

void FileEventListenerImpl::handle_modify_event(const FileEvent& event)
{
  std::string filename = event.filename.in();

  ACE_DEBUG((LM_INFO,
             ACE_TEXT("(%P|%t) Handling MODIFY event for: %C (Phase 5 - not yet implemented)\n"),
             filename.c_str()));

  // Phase 5 implementation:
  // - Compare timestamps with local file
  // - If remote is newer, accept the update
  // - Otherwise, ignore
}

void FileEventListenerImpl::handle_delete_event(const FileEvent& event)
{
  std::string filename = event.filename.in();

  ACE_DEBUG((LM_INFO,
             ACE_TEXT("(%P|%t) Handling DELETE event for: %C (Phase 6 - not yet implemented)\n"),
             filename.c_str()));

  // Phase 6 implementation:
  // - Compare timestamps with local file
  // - If remote timestamp is newer, delete local file
  // - Otherwise, ignore
}

bool FileEventListenerImpl::is_valid_filename(const std::string& filename) const
{
  // Reject empty filenames
  if (filename.empty()) {
    return false;
  }

  // Reject absolute paths (starting with / or drive letter on Windows)
  if (filename[0] == '/' || filename[0] == '\\') {
    return false;
  }
  if (filename.length() >= 2 && filename[1] == ':') {
    // Windows drive letter
    return false;
  }

  // Reject path traversal attempts
  if (filename.find("..") != std::string::npos) {
    return false;
  }

  // Reject filenames with path separators (no subdirectories allowed per spec)
  if (filename.find('/') != std::string::npos ||
      filename.find('\\') != std::string::npos) {
    return false;
  }

  return true;
}

// DDS callback stubs
void FileEventListenerImpl::on_requested_deadline_missed(
  DDS::DataReader_ptr,
  const DDS::RequestedDeadlineMissedStatus&)
{
}

void FileEventListenerImpl::on_requested_incompatible_qos(
  DDS::DataReader_ptr,
  const DDS::RequestedIncompatibleQosStatus&)
{
}

void FileEventListenerImpl::on_sample_rejected(
  DDS::DataReader_ptr,
  const DDS::SampleRejectedStatus&)
{
}

void FileEventListenerImpl::on_liveliness_changed(
  DDS::DataReader_ptr,
  const DDS::LivelinessChangedStatus&)
{
}

void FileEventListenerImpl::on_subscription_matched(
  DDS::DataReader_ptr,
  const DDS::SubscriptionMatchedStatus&)
{
}

void FileEventListenerImpl::on_sample_lost(
  DDS::DataReader_ptr,
  const DDS::SampleLostStatus&)
{
}

} // namespace DirShare
