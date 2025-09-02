#include "FileSystemMonitor.h"
#include <iostream>

namespace FileSync {

class FileSystemMonitor::Impl {
public:
  explicit Impl(const std::string& directory_path)
    : directory_path_(directory_path)
    , monitoring_(false)
  {
  }

  ~Impl() {
    stop_monitoring();
  }

  void set_change_callback(const FileChangeCallback& callback) {
    callback_ = callback;
  }

  bool start_monitoring() {
    if (monitoring_) {
      return true; // Already monitoring
    }

    // TODO: Implement platform-specific file system monitoring
    // For now, just use a simple polling approach
    monitoring_ = true;
    std::cout << "INFO: Started monitoring directory: " << directory_path_ << std::endl;
    return true;
  }

  void stop_monitoring() {
    if (!monitoring_) {
      return; // Not monitoring
    }

    monitoring_ = false;
    std::cout << "INFO: Stopped monitoring directory: " << directory_path_ << std::endl;
  }

  bool is_monitoring() const {
    return monitoring_;
  }

  void add_excluded_pattern(const std::string& pattern) {
    excluded_patterns_.push_back(pattern);
    std::cout << "INFO: Added excluded pattern: " << pattern << std::endl;
  }

  void process_events() {
    if (!monitoring_) {
      return;
    }

    // TODO: Implement actual event processing
    // This would scan the directory and compare with previous state
    // For now, this is just a stub
  }

private:
  std::string directory_path_;
  bool monitoring_;
  FileChangeCallback callback_;
  std::vector<std::string> excluded_patterns_;
  // TODO: Add platform-specific monitoring data structures
};

FileSystemMonitor::FileSystemMonitor(const std::string& directory_path)
  : pimpl_(std::make_unique<Impl>(directory_path))
{
}

FileSystemMonitor::~FileSystemMonitor() = default;

void FileSystemMonitor::set_change_callback(const FileChangeCallback& callback) {
  pimpl_->set_change_callback(callback);
}

bool FileSystemMonitor::start_monitoring() {
  return pimpl_->start_monitoring();
}

void FileSystemMonitor::stop_monitoring() {
  pimpl_->stop_monitoring();
}

bool FileSystemMonitor::is_monitoring() const {
  return pimpl_->is_monitoring();
}

void FileSystemMonitor::add_excluded_pattern(const std::string& pattern) {
  pimpl_->add_excluded_pattern(pattern);
}

void FileSystemMonitor::process_events() {
  pimpl_->process_events();
}

} // namespace FileSync