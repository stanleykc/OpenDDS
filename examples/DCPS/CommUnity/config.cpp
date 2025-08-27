// -*- C++ -*-
// Configuration System Implementation

#include "config.h"
#include <ace/Log_Msg.h>
#include <fstream>
#include <sstream>
#include <algorithm>

using namespace std;

Config::Config() {
    setDefaults();
}

Config::~Config() {
}

void Config::setDefaults() {
    // Core configuration
    dmp_id_ = "community-publisher-default";
    
    // API configuration
    api_host_ = "0.0.0.0";
    api_port_ = 8080;
    auth_token_ = "secure_token_change_me";
    
    // DDS configuration
    domain_id_ = 0;
    rtps_relay_host_ = "relay.community.org";
    rtps_relay_port_ = 4444;
    
    // Security configuration
    security_enabled_ = false;
    identity_ca_ = "/etc/community/certs/identity_ca.pem";
    identity_cert_ = "/etc/community/certs/identity_cert.pem";
    identity_key_ = "/etc/community/certs/identity_key.pem";
    permissions_ca_ = "/etc/community/certs/permissions_ca.pem";
    permissions_file_ = "/etc/community/certs/permissions.xml";
    governance_file_ = "/etc/community/certs/governance.xml";
    
    // Data management
    purge_timeout_seconds_ = 3600;
    heartbeat_interval_seconds_ = 30;
    
    // Logging configuration
    log_level_ = "INFO";
    log_to_console_ = true;
    log_to_file_ = false;
    log_file_ = "/var/log/community/publisher.log";
    log_to_syslog_ = false;
    syslog_host_ = "localhost";
    syslog_port_ = 514;
    
    last_error_.clear();
}

bool Config::load(const string& config_file) {
    last_error_.clear();
    
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%P|%t) Loading configuration from: %C\n"), config_file.c_str()));
    
    if (!parseYamlFile(config_file)) {
        setError("Failed to parse configuration file: " + config_file);
        return false;
    }
    
    if (!validate()) {
        return false;
    }
    
    ACE_DEBUG((LM_INFO, ACE_TEXT("(%P|%t) Configuration loaded successfully\n")));
    return true;
}

bool Config::loadFromString(const string& yaml_content) {
    last_error_.clear();
    
    if (!parseYamlString(yaml_content)) {
        setError("Failed to parse YAML configuration");
        return false;
    }
    
    if (!validate()) {
        return false;
    }
    
    return true;
}

bool Config::parseYamlFile(const string& config_file) {
    ifstream file(config_file);
    if (!file.is_open()) {
        setError("Cannot open configuration file: " + config_file);
        return false;
    }
    
    stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    
    return parseYamlString(buffer.str());
}

bool Config::parseYamlString(const string& yaml_content) {
    try {
        map<string, string> config_map = parseBasicYaml(yaml_content);
        
        // Parse writer section
        dmp_id_ = getValue(config_map, "writer.dmp_id", dmp_id_);
        
        // Parse API section
        api_host_ = getValue(config_map, "writer.api.host", api_host_);
        api_port_ = getIntValue(config_map, "writer.api.port", api_port_);
        auth_token_ = getValue(config_map, "writer.api.auth_token", auth_token_);
        
        // Parse DDS section
        domain_id_ = getIntValue(config_map, "writer.dds.domain_id", domain_id_);
        rtps_relay_host_ = getValue(config_map, "writer.dds.rtps_relay.host", rtps_relay_host_);
        rtps_relay_port_ = getIntValue(config_map, "writer.dds.rtps_relay.port", rtps_relay_port_);
        
        // Parse security section
        security_enabled_ = getBoolValue(config_map, "writer.security.enabled", security_enabled_);
        identity_ca_ = getValue(config_map, "writer.security.identity_ca", identity_ca_);
        identity_cert_ = getValue(config_map, "writer.security.identity_cert", identity_cert_);
        identity_key_ = getValue(config_map, "writer.security.identity_key", identity_key_);
        permissions_ca_ = getValue(config_map, "writer.security.permissions_ca", permissions_ca_);
        permissions_file_ = getValue(config_map, "writer.security.permissions_file", permissions_file_);
        governance_file_ = getValue(config_map, "writer.security.governance_file", governance_file_);
        
        // Parse data management section
        purge_timeout_seconds_ = getIntValue(config_map, "writer.data.purge_timeout_seconds", purge_timeout_seconds_);
        heartbeat_interval_seconds_ = getIntValue(config_map, "writer.data.heartbeat_interval_seconds", heartbeat_interval_seconds_);
        
        // Parse logging section
        log_level_ = getValue(config_map, "logging.level", log_level_);
        log_to_console_ = getBoolValue(config_map, "logging.console.enabled", log_to_console_);
        log_to_file_ = getBoolValue(config_map, "logging.file.enabled", log_to_file_);
        log_file_ = getValue(config_map, "logging.file.path", log_file_);
        log_to_syslog_ = getBoolValue(config_map, "logging.syslog.enabled", log_to_syslog_);
        syslog_host_ = getValue(config_map, "logging.syslog.host", syslog_host_);
        syslog_port_ = getIntValue(config_map, "logging.syslog.port", syslog_port_);
        
        return true;
        
    } catch (const exception& e) {
        setError("Error parsing YAML: " + string(e.what()));
        return false;
    }
}

// Simple YAML parser (basic key-value pairs with nested structure)
map<string, string> Config::parseBasicYaml(const string& yaml_content) {
    map<string, string> result;
    istringstream stream(yaml_content);
    string line;
    vector<string> context;
    
    while (getline(stream, line)) {
        line = trim(line);
        
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') {
            continue;
        }
        
        // Count leading spaces to determine nesting level
        size_t indent = 0;
        for (char c : line) {
            if (c == ' ') indent++;
            else break;
        }
        
        // Determine nesting level (assuming 2 spaces per level)
        size_t level = indent / 2;
        
        // Adjust context stack based on nesting level
        while (context.size() > level) {
            context.pop_back();
        }
        
        // Parse key-value pair
        size_t colon_pos = line.find(':');
        if (colon_pos != string::npos) {
            string key = trim(line.substr(0, colon_pos));
            string value = trim(line.substr(colon_pos + 1));
            
            // Build full key path
            string full_key = key;
            if (!context.empty()) {
                full_key = "";
                for (size_t i = 0; i < context.size(); ++i) {
                    if (i > 0) full_key += ".";
                    full_key += context[i];
                }
                full_key += "." + key;
            }
            
            if (!value.empty()) {
                // Remove quotes if present
                if ((value.front() == '"' && value.back() == '"') || 
                    (value.front() == '\'' && value.back() == '\'')) {
                    value = value.substr(1, value.length() - 2);
                }
                result[full_key] = value;
            } else {
                // This is a section header
                if (context.size() == level) {
                    context.push_back(key);
                } else {
                    context.resize(level);
                    context.push_back(key);
                }
            }
        }
    }
    
    return result;
}

string Config::getValue(const map<string, string>& values, const string& key, const string& default_val) {
    auto it = values.find(key);
    return (it != values.end()) ? it->second : default_val;
}

int Config::getIntValue(const map<string, string>& values, const string& key, int default_val) {
    auto it = values.find(key);
    if (it != values.end()) {
        try {
            return stoi(it->second);
        } catch (...) {
            ACE_DEBUG((LM_WARNING, ACE_TEXT("(%P|%t) Invalid integer value for %C: %C\n"), 
                      key.c_str(), it->second.c_str()));
        }
    }
    return default_val;
}

bool Config::getBoolValue(const map<string, string>& values, const string& key, bool default_val) {
    auto it = values.find(key);
    if (it != values.end()) {
        string val = it->second;
        transform(val.begin(), val.end(), val.begin(), ::tolower);
        return (val == "true" || val == "yes" || val == "1" || val == "on");
    }
    return default_val;
}

bool Config::validate() const {
    // Validate required fields
    if (dmp_id_.empty()) {
        setError("DMP ID cannot be empty");
        return false;
    }
    
    if (api_host_.empty()) {
        setError("API host cannot be empty");
        return false;
    }
    
    if (api_port_ <= 0 || api_port_ > 65535) {
        setError("API port must be between 1 and 65535");
        return false;
    }
    
    if (auth_token_.empty()) {
        setError("Auth token cannot be empty");
        return false;
    }
    
    if (domain_id_ < 0 || domain_id_ > 232) {
        setError("DDS domain ID must be between 0 and 232");
        return false;
    }
    
    // Validate security configuration if enabled
    if (security_enabled_) {
        if (identity_ca_.empty() || identity_cert_.empty() || identity_key_.empty()) {
            setError("Security is enabled but identity certificates are not configured");
            return false;
        }
        
        if (permissions_ca_.empty() || permissions_file_.empty() || governance_file_.empty()) {
            setError("Security is enabled but permissions/governance files are not configured");
            return false;
        }
    }
    
    // Validate timeout values
    if (purge_timeout_seconds_ < 0) {
        setError("Purge timeout must be non-negative");
        return false;
    }
    
    if (heartbeat_interval_seconds_ < 1) {
        setError("Heartbeat interval must be at least 1 second");
        return false;
    }
    
    // Validate log level
    if (log_level_ != "DEBUG" && log_level_ != "INFO" && log_level_ != "WARN" && 
        log_level_ != "ERROR" && log_level_ != "FATAL") {
        setError("Log level must be one of: DEBUG, INFO, WARN, ERROR, FATAL");
        return false;
    }
    
    return true;
}

string Config::toString() const {
    ostringstream oss;
    oss << "Configuration:\n"
        << "  DMP ID: " << dmp_id_ << "\n"
        << "  API: " << api_host_ << ":" << api_port_ << "\n"
        << "  DDS Domain: " << domain_id_ << "\n"
        << "  RTPS Relay: " << rtps_relay_host_ << ":" << rtps_relay_port_ << "\n"
        << "  Security: " << (security_enabled_ ? "enabled" : "disabled") << "\n"
        << "  Purge Timeout: " << purge_timeout_seconds_ << "s\n"
        << "  Heartbeat Interval: " << heartbeat_interval_seconds_ << "s\n"
        << "  Log Level: " << log_level_ << "\n"
        << "  Log to Console: " << (log_to_console_ ? "yes" : "no") << "\n"
        << "  Log to File: " << (log_to_file_ ? "yes" : "no");
    
    if (log_to_file_) {
        oss << " (" << log_file_ << ")";
    }
    
    oss << "\n  Log to Syslog: " << (log_to_syslog_ ? "yes" : "no");
    
    if (log_to_syslog_) {
        oss << " (" << syslog_host_ << ":" << syslog_port_ << ")";
    }
    
    return oss.str();
}

string Config::trim(const string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == string::npos) {
        return "";
    }
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

void Config::setError(const string& error) {
    last_error_ = error;
    ACE_ERROR((LM_ERROR, ACE_TEXT("(%P|%t) Configuration error: %C\n"), error.c_str()));
}