// -*- C++ -*-
// Configuration System - YAML-based configuration for CommunityPublisher
// Based on the CommUnity Application Design Document

#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <map>
#include <vector>

class Config {
public:
    Config();
    ~Config();

    // Configuration loading
    bool load(const std::string& config_file);
    bool loadFromString(const std::string& yaml_content);
    
    // Core configuration accessors
    const std::string& getDmpId() const { return dmp_id_; }
    void setDmpId(const std::string& dmp_id) { dmp_id_ = dmp_id; }
    
    // API configuration
    const std::string& getApiHost() const { return api_host_; }
    int getApiPort() const { return api_port_; }
    const std::string& getAuthToken() const { return auth_token_; }
    
    // DDS configuration
    int getDomainId() const { return domain_id_; }
    const std::string& getRtpsRelayHost() const { return rtps_relay_host_; }
    int getRtpsRelayPort() const { return rtps_relay_port_; }
    
    // Security configuration
    bool isSecurityEnabled() const { return security_enabled_; }
    const std::string& getIdentityCa() const { return identity_ca_; }
    const std::string& getIdentityCert() const { return identity_cert_; }
    const std::string& getIdentityKey() const { return identity_key_; }
    const std::string& getPermissionsCa() const { return permissions_ca_; }
    const std::string& getPermissionsFile() const { return permissions_file_; }
    const std::string& getGovernanceFile() const { return governance_file_; }
    
    // Data management configuration
    int getPurgeTimeoutSeconds() const { return purge_timeout_seconds_; }
    int getHeartbeatIntervalSeconds() const { return heartbeat_interval_seconds_; }
    
    // Logging configuration
    const std::string& getLogLevel() const { return log_level_; }
    bool isLogToConsole() const { return log_to_console_; }
    bool isLogToFile() const { return log_to_file_; }
    const std::string& getLogFile() const { return log_file_; }
    bool isLogToSyslog() const { return log_to_syslog_; }
    const std::string& getSyslogHost() const { return syslog_host_; }
    int getSyslogPort() const { return syslog_port_; }
    
    // Validation and defaults
    bool validate() const;
    void setDefaults();
    
    // Debug and status
    std::string toString() const;
    const std::string& getLastError() const { return last_error_; }

private:
    // Configuration parsing helpers
    bool parseYamlFile(const std::string& config_file);
    bool parseYamlString(const std::string& yaml_content);
    void parseSection(const std::map<std::string, std::string>& section, const std::string& prefix);
    std::string getValue(const std::map<std::string, std::string>& values, const std::string& key, const std::string& default_val = "");
    int getIntValue(const std::map<std::string, std::string>& values, const std::string& key, int default_val = 0);
    bool getBoolValue(const std::map<std::string, std::string>& values, const std::string& key, bool default_val = false);
    
    // Simple YAML parser (basic implementation)
    std::map<std::string, std::string> parseBasicYaml(const std::string& yaml_content);
    std::string trim(const std::string& str);
    void setError(const std::string& error);
    
    // Core configuration
    std::string dmp_id_;
    
    // API configuration
    std::string api_host_;
    int api_port_;
    std::string auth_token_;
    
    // DDS configuration
    int domain_id_;
    std::string rtps_relay_host_;
    int rtps_relay_port_;
    
    // Security configuration
    bool security_enabled_;
    std::string identity_ca_;
    std::string identity_cert_;
    std::string identity_key_;
    std::string permissions_ca_;
    std::string permissions_file_;
    std::string governance_file_;
    
    // Data management
    int purge_timeout_seconds_;
    int heartbeat_interval_seconds_;
    
    // Logging configuration
    std::string log_level_;
    bool log_to_console_;
    bool log_to_file_;
    std::string log_file_;
    bool log_to_syslog_;
    std::string syslog_host_;
    int syslog_port_;
    
    // Error tracking
    mutable std::string last_error_;
};

#endif // CONFIG_H