#ifndef CONFIGURATION_MANAGER_H
#define CONFIGURATION_MANAGER_H

#include <string>
#include <vector>

namespace FileSync {

/**
 * Manages application configuration from files and command line arguments.
 * 
 * This class handles:
 * - Reading configuration from files (INI format)
 * - Command line parameter overrides
 * - Configuration validation
 * - Providing access to configuration values
 */
class ConfigurationManager {
public:
  /**
   * Default constructor
   */
  ConfigurationManager();

  /**
   * Destructor
   */
  ~ConfigurationManager();

  /**
   * Load configuration from file
   * @param config_file Path to configuration file
   * @return true if successful, false otherwise
   */
  bool load_configuration(const std::string& config_file);

  /**
   * Validate that all required configuration is present and valid
   * @return true if configuration is valid, false otherwise
   */
  bool validate_configuration() const;

  // Directory configuration
  const std::string& get_source_directory() const { return source_dir_; }
  void set_source_directory(const std::string& dir) { source_dir_ = dir; }
  
  const std::string& get_destination_directory() const { return dest_dir_; }
  void set_destination_directory(const std::string& dir) { dest_dir_ = dir; }

  // DDS configuration
  int get_domain_id() const { return domain_id_; }
  void set_domain_id(int domain_id) { domain_id_ = domain_id; }
  
  const std::string& get_dcps_config_file() const { return dcps_config_file_; }
  void set_dcps_config_file(const std::string& file) { dcps_config_file_ = file; }

  // Security configuration
  const std::string& get_identity_ca() const { return identity_ca_; }
  const std::string& get_permissions_ca() const { return permissions_ca_; }
  const std::string& get_identity_certificate() const { return identity_certificate_; }
  const std::string& get_identity_private_key() const { return identity_private_key_; }
  const std::string& get_permissions_file() const { return permissions_file_; }
  const std::string& get_governance_file() const { return governance_file_; }

  // Sync configuration
  size_t get_chunk_size() const { return chunk_size_; }
  void set_chunk_size(size_t size) { chunk_size_ = size; }
  
  size_t get_max_file_size() const { return max_file_size_; }
  void set_max_file_size(size_t size) { max_file_size_ = size; }
  
  const std::vector<std::string>& get_excluded_patterns() const { return excluded_patterns_; }

  // Runtime configuration
  bool get_verbose_logging() const { return verbose_logging_; }
  void set_verbose_logging(bool verbose) { verbose_logging_ = verbose; }
  
  bool get_daemon_mode() const { return daemon_mode_; }
  void set_daemon_mode(bool daemon) { daemon_mode_ = daemon; }
  
  const std::string& get_log_file() const { return log_file_; }
  const std::string& get_log_level() const { return log_level_; }

private:
  // Directory paths
  std::string source_dir_;
  std::string dest_dir_;

  // DDS configuration
  int domain_id_;
  std::string dcps_config_file_;

  // Security configuration
  std::string identity_ca_;
  std::string permissions_ca_;
  std::string identity_certificate_;
  std::string identity_private_key_;
  std::string permissions_file_;
  std::string governance_file_;

  // Sync configuration
  size_t chunk_size_;
  size_t max_file_size_;
  std::vector<std::string> excluded_patterns_;

  // Runtime configuration
  bool verbose_logging_;
  bool daemon_mode_;
  std::string log_file_;
  std::string log_level_;

  // Helper methods
  bool parse_ini_file(const std::string& filename);
  bool validate_directory_access(const std::string& path, bool must_exist) const;
  bool validate_file_access(const std::string& path) const;
};

} // namespace FileSync

#endif // CONFIGURATION_MANAGER_H