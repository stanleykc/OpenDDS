#include "FileManager.h"

#include <ace/OS_NS_sys_stat.h>
#include <ace/OS_NS_unistd.h>
#include <ace/OS_NS_time.h>

#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <ctime>

// TODO: Replace with proper SHA-256 implementation (OpenSSL)
#include <functional> // For std::hash as placeholder

namespace FileSync {

class FileManager::Impl {
public:
  Impl() = default;
  ~Impl() = default;

  bool read_file(const std::string& file_path, std::vector<uint8_t>& content) {
    std::ifstream file(file_path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
      std::cerr << "ERROR: Could not open file for reading: " << file_path << std::endl;
      return false;
    }

    auto file_size = file.tellg();
    if (file_size < 0) {
      std::cerr << "ERROR: Could not determine file size: " << file_path << std::endl;
      return false;
    }

    content.resize(static_cast<size_t>(file_size));
    file.seekg(0, std::ios::beg);
    file.read(reinterpret_cast<char*>(content.data()), file_size);

    if (!file.good() && !file.eof()) {
      std::cerr << "ERROR: Error reading file: " << file_path << std::endl;
      content.clear();
      return false;
    }

    return true;
  }

  bool write_file_atomic(const std::string& file_path, const std::vector<uint8_t>& content) {
    // Create temporary file
    std::string temp_path = file_path + ".tmp_filesync";
    
    // Ensure parent directory exists
    size_t last_slash = file_path.find_last_of("/\\");
    if (last_slash != std::string::npos) {
      std::string dir_path = file_path.substr(0, last_slash);
      if (!create_directory_recursive(dir_path)) {
        return false;
      }
    }

    // Write to temporary file
    std::ofstream temp_file(temp_path, std::ios::binary);
    if (!temp_file.is_open()) {
      std::cerr << "ERROR: Could not create temporary file: " << temp_path << std::endl;
      return false;
    }

    temp_file.write(reinterpret_cast<const char*>(content.data()), content.size());
    temp_file.close();

    if (!temp_file.good()) {
      std::cerr << "ERROR: Error writing temporary file: " << temp_path << std::endl;
      ACE_OS::unlink(temp_path.c_str()); // Clean up
      return false;
    }

    // Atomic rename
    if (ACE_OS::rename(temp_path.c_str(), file_path.c_str()) != 0) {
      std::cerr << "ERROR: Could not rename temporary file to final path: " 
                << temp_path << " -> " << file_path << std::endl;
      ACE_OS::unlink(temp_path.c_str()); // Clean up
      return false;
    }

    return true;
  }

  std::string calculate_sha256(const std::vector<uint8_t>& content) {
    // TODO: Implement proper SHA-256 using OpenSSL
    // For now, use a simple hash as placeholder
    std::hash<std::string> hasher;
    std::string content_str(content.begin(), content.end());
    size_t hash = hasher(content_str);
    
    std::stringstream ss;
    ss << std::hex << hash;
    std::string result = ss.str();
    
    // Pad to look more like a real SHA-256
    while (result.length() < 16) {
      result = "0" + result;
    }
    result += "_placeholder_hash";
    
    return result;
  }

  std::string calculate_file_sha256(const std::string& file_path) {
    std::vector<uint8_t> content;
    if (!read_file(file_path, content)) {
      return "";
    }
    return calculate_sha256(content);
  }

  bool file_exists(const std::string& file_path) {
    ACE_stat st;
    return ACE_OS::stat(file_path.c_str(), &st) == 0 && S_ISREG(st.st_mode);
  }

  long long get_file_mod_time(const std::string& file_path) {
    ACE_stat st;
    if (ACE_OS::stat(file_path.c_str(), &st) != 0) {
      return -1;
    }
    return static_cast<long long>(st.st_mtime);
  }

  bool create_directory_recursive(const std::string& dir_path) {
    if (dir_path.empty()) {
      return true;
    }

    // Check if directory already exists
    ACE_stat st;
    if (ACE_OS::stat(dir_path.c_str(), &st) == 0) {
      return S_ISDIR(st.st_mode);
    }

    // Create parent directory first
    size_t last_slash = dir_path.find_last_of("/\\");
    if (last_slash != std::string::npos && last_slash > 0) {
      std::string parent = dir_path.substr(0, last_slash);
      if (!create_directory_recursive(parent)) {
        return false;
      }
    }

    // Create this directory
    if (ACE_OS::mkdir(dir_path.c_str(), 0755) != 0) {
      std::cerr << "ERROR: Could not create directory: " << dir_path << std::endl;
      return false;
    }

    return true;
  }

  bool delete_file(const std::string& file_path) {
    if (ACE_OS::unlink(file_path.c_str()) != 0) {
      std::cerr << "ERROR: Could not delete file: " << file_path << std::endl;
      return false;
    }
    return true;
  }

  std::string generate_conflict_filename(
    const std::string& original_path,
    const std::string& peer_hostname)
  {
    // Find the file extension
    size_t last_dot = original_path.find_last_of('.');
    size_t last_slash = original_path.find_last_of("/\\");
    
    // Make sure the dot is in the filename, not in a directory name
    bool has_extension = (last_dot != std::string::npos && 
                         (last_slash == std::string::npos || last_dot > last_slash));

    // Generate timestamp
    time_t now = ACE_OS::time(nullptr);
    struct tm* tm_info = ACE_OS::localtime(&now);
    
    std::stringstream timestamp;
    timestamp << std::put_time(tm_info, "%Y-%m-%d %H-%M-%S");

    std::string result;
    if (has_extension) {
      std::string base = original_path.substr(0, last_dot);
      std::string ext = original_path.substr(last_dot);
      result = base + " (conflicted copy from " + peer_hostname + " " + 
               timestamp.str() + ")" + ext;
    } else {
      result = original_path + " (conflicted copy from " + peer_hostname + " " + 
               timestamp.str() + ")";
    }

    return result;
  }
};

FileManager::FileManager()
  : pimpl_(std::make_unique<Impl>())
{
}

FileManager::~FileManager() = default;

bool FileManager::read_file(const std::string& file_path, std::vector<uint8_t>& content) {
  return pimpl_->read_file(file_path, content);
}

bool FileManager::write_file_atomic(const std::string& file_path, const std::vector<uint8_t>& content) {
  return pimpl_->write_file_atomic(file_path, content);
}

std::string FileManager::calculate_sha256(const std::vector<uint8_t>& content) {
  return pimpl_->calculate_sha256(content);
}

std::string FileManager::calculate_file_sha256(const std::string& file_path) {
  return pimpl_->calculate_file_sha256(file_path);
}

bool FileManager::file_exists(const std::string& file_path) {
  return pimpl_->file_exists(file_path);
}

long long FileManager::get_file_mod_time(const std::string& file_path) {
  return pimpl_->get_file_mod_time(file_path);
}

bool FileManager::create_directory_recursive(const std::string& dir_path) {
  return pimpl_->create_directory_recursive(dir_path);
}

bool FileManager::delete_file(const std::string& file_path) {
  return pimpl_->delete_file(file_path);
}

std::string FileManager::generate_conflict_filename(
  const std::string& original_path,
  const std::string& peer_hostname)
{
  return pimpl_->generate_conflict_filename(original_path, peer_hostname);
}

} // namespace FileSync