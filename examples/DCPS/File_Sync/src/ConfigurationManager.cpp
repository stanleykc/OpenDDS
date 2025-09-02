#include "ConfigurationManager.h"

#include <ace/OS_NS_sys_stat.h>
#include <ace/OS_NS_unistd.h>
#include <ace/streams.h>

#include <fstream>
#include <iostream>
#include <sstream>

namespace FileSync {

ConfigurationManager::ConfigurationManager()
  : domain_id_(42)
  , chunk_size_(65536)  // 64KB default
  , max_file_size_(104857600)  // 100MB default
  , verbose_logging_(false)
  , daemon_mode_(false)
  , log_level_("info")
{
}

ConfigurationManager::~ConfigurationManager() = default;

bool ConfigurationManager::load_configuration(const std::string& config_file) {
  if (config_file.empty()) {
    // No config file specified, use defaults
    return true;
  }

  return parse_ini_file(config_file);
}

bool ConfigurationManager::parse_ini_file(const std::string& filename) {
  std::ifstream file(filename);
  if (!file.is_open()) {
    if (filename != "file_sync.conf") {
      // Only report error if user explicitly specified a config file
      std::cerr << "WARNING: Could not open config file: " << filename << std::endl;
      std::cerr << "Using default configuration" << std::endl;
    }
    return true; // Not finding default config is OK
  }

  std::string line;
  std::string current_section;
  size_t line_number = 0;

  while (std::getline(file, line)) {
    ++line_number;
    
    // Remove comments and trim whitespace
    size_t comment_pos = line.find('#');
    if (comment_pos != std::string::npos) {
      line = line.substr(0, comment_pos);
    }
    
    // Trim whitespace
    line.erase(0, line.find_first_not_of(" \t"));
    line.erase(line.find_last_not_of(" \t") + 1);
    
    if (line.empty()) {
      continue;
    }

    // Check for section header
    if (line.front() == '[' && line.back() == ']') {
      current_section = line.substr(1, line.length() - 2);
      continue;
    }

    // Parse key=value pairs
    size_t equals_pos = line.find('=');
    if (equals_pos == std::string::npos) {
      std::cerr << "WARNING: Invalid config line " << line_number 
                << " in " << filename << ": " << line << std::endl;
      continue;
    }

    std::string key = line.substr(0, equals_pos);
    std::string value = line.substr(equals_pos + 1);
    
    // Trim key and value
    key.erase(0, key.find_first_not_of(" \t"));
    key.erase(key.find_last_not_of(" \t") + 1);
    value.erase(0, value.find_first_not_of(" \t"));
    value.erase(value.find_last_not_of(" \t") + 1);

    // Process configuration values based on section
    if (current_section == "directories") {
      if (key == "source_dir") {
        source_dir_ = value;
      } else if (key == "dest_dir") {
        dest_dir_ = value;
      }
    } else if (current_section == "dds") {
      if (key == "domain_id") {
        domain_id_ = std::stoi(value);
      } else if (key == "dcps_config_file") {
        dcps_config_file_ = value;
      }
    } else if (current_section == "security") {
      if (key == "identity_ca") {
        identity_ca_ = value;
      } else if (key == "permissions_ca") {
        permissions_ca_ = value;
      } else if (key == "identity_certificate") {
        identity_certificate_ = value;
      } else if (key == "identity_private_key") {
        identity_private_key_ = value;
      } else if (key == "permissions_file") {
        permissions_file_ = value;
      } else if (key == "governance_file") {
        governance_file_ = value;
      }
    } else if (current_section == "sync") {
      if (key == "chunk_size") {
        chunk_size_ = std::stoull(value);
      } else if (key == "max_file_size") {
        max_file_size_ = std::stoull(value);
      } else if (key == "excluded_patterns") {
        // Parse comma-separated list
        std::stringstream ss(value);
        std::string pattern;
        excluded_patterns_.clear();
        while (std::getline(ss, pattern, ',')) {
          // Trim pattern
          pattern.erase(0, pattern.find_first_not_of(" \t"));
          pattern.erase(pattern.find_last_not_of(" \t") + 1);
          if (!pattern.empty()) {
            excluded_patterns_.push_back(pattern);
          }
        }
      }
    } else if (current_section == "logging") {
      if (key == "level") {
        log_level_ = value;
      } else if (key == "file") {
        log_file_ = value;
      }
    }
  }

  return true;
}

bool ConfigurationManager::validate_configuration() const {
  bool valid = true;

  // Validate required directories
  if (source_dir_.empty()) {
    std::cerr << "ERROR: Source directory not specified" << std::endl;
    valid = false;
  } else if (!validate_directory_access(source_dir_, true)) {
    std::cerr << "ERROR: Source directory not accessible: " << source_dir_ << std::endl;
    valid = false;
  }

  if (dest_dir_.empty()) {
    std::cerr << "ERROR: Destination directory not specified" << std::endl;
    valid = false;
  } else if (!validate_directory_access(dest_dir_, false)) {
    std::cerr << "ERROR: Cannot create/access destination directory: " << dest_dir_ << std::endl;
    valid = false;
  }

  // Validate domain ID
  if (domain_id_ < 0 || domain_id_ > 232) {
    std::cerr << "ERROR: Invalid DDS domain ID: " << domain_id_ 
              << " (must be 0-232)" << std::endl;
    valid = false;
  }

  // Validate chunk size
  if (chunk_size_ == 0 || chunk_size_ > 1048576) { // Max 1MB chunks
    std::cerr << "ERROR: Invalid chunk size: " << chunk_size_ 
              << " (must be 1-1048576 bytes)" << std::endl;
    valid = false;
  }

  return valid;
}

bool ConfigurationManager::validate_directory_access(const std::string& path, bool must_exist) const {
  ACE_stat st;
  int result = ACE_OS::stat(path.c_str(), &st);
  
  if (result == 0) {
    // Path exists, check if it's a directory
    return S_ISDIR(st.st_mode);
  } else if (!must_exist) {
    // Try to create the directory
    return ACE_OS::mkdir(path.c_str(), 0755) == 0;
  }
  
  return false;
}

bool ConfigurationManager::validate_file_access(const std::string& path) const {
  ACE_stat st;
  return ACE_OS::stat(path.c_str(), &st) == 0 && S_ISREG(st.st_mode);
}

} // namespace FileSync