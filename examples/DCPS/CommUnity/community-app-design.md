# CommUnity Application Design Document

## 1. Executive Summary

The CommUnity Application is a federated data sharing platform built on OpenDDS that enables organizations to securely share and consume Human Services Data Specification (HSDS) directories. The system consists of two main components per organization: a Writer application for publishing local data and a Reader application for consuming federated data.

## 2. System Architecture Overview

### 2.1 High-Level Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    Shared Infrastructure                      │
│  ┌──────────────────┐            ┌──────────────────┐      │
│  │  DDS Permissions │            │   RtpsRelay      │      │
│  │  Manager (DPM)   │            │   Service        │      │
│  └──────────────────┘            └──────────────────┘      │
└────────────┬──────────────────────────┬────────────────────┘
             │                          │
   ┌─────────┴─────────┐      ┌────────┴─────────┐
   │  Organization A   │      │  Organization B   │
   │                   │      │                   │
   │  ┌─────────────┐  │      │  ┌─────────────┐ │
   │  │   Writer    │  │      │  │   Writer    │ │
   │  │ Application │  │      │  │ Application │ │
   │  └─────────────┘  │      │  └─────────────┘ │
   │                   │      │                   │
   │  ┌─────────────┐  │      │  ┌─────────────┐ │
   │  │   Reader    │  │      │  │   Reader    │ │
   │  │ Application │  │      │  │ Application │ │
   │  └─────────────┘  │      │  └─────────────┘ │
   └───────────────────┘      └───────────────────┘
```

### 2.2 Component Responsibilities

- **DPM**: Certificate authority for identity and permissions management
- **RtpsRelay**: Message forwarding across organizational boundaries
- **Writer Application**: Publishes organization's HSDS data to DDS topics
- **Reader Application**: Subscribes to and aggregates federated HSDS data

## 3. CommUnityWriter Application Design

### 3.1 Core Components

#### 3.1.1 HTTP API Server
- **Framework**: Use a lightweight HTTP server (e.g., Boost.Beast or Pistache)
- **Endpoints**:
  - `POST /api/v1/hsds/{table}` - Submit data for a specific HSDS table
  - `PUT /api/v1/hsds/{table}/{id}` - Update specific record
  - `DELETE /api/v1/hsds/{table}/{id}` - Remove specific record
  - `GET /api/v1/health` - Health check endpoint
  - `GET /api/v1/status` - Connection and publishing status

#### 3.1.2 DDS Publisher Module
```cpp
class DdsPublisher {
private:
    DDS::DomainParticipant_ptr participant_;
    std::map<std::string, DDS::DataWriter_ptr> writers_;
    SecurityConfiguration security_config_;
    
public:
    void initialize(const Config& config);
    void publishSample(const std::string& topic, const HsdsData& data);
    void setupSecurity(const std::string& dpm_id);
};
```

#### 3.1.3 Data Validation Layer
```cpp
class HsdsValidator {
public:
    bool validateOrganization(const Organization& org);
    bool validateService(const Service& service);
    bool validateLocation(const Location& location);
    // Additional validation methods for each HSDS table
    
private:
    bool checkRequiredFields(const HsdsBase& data);
    bool validateRelationships(const HsdsBase& data);
};
```

### 3.2 Configuration Schema
```yaml
writer:
  organization_id: "org_unique_id"
  dpm_id: "dpm_assigned_id"
  
  api:
    host: "0.0.0.0"
    port: 8080
    auth_token: "secure_token_here"
    
  dds:
    domain_id: 0
    rtps_relay:
      host: "relay.community.org"
      port: 4444
    
  security:
    identity_ca: "/path/to/identity_ca.pem"
    identity_cert: "/path/to/identity_cert.pem"
    identity_key: "/path/to/identity_key.pem"
    permissions_ca: "/path/to/permissions_ca.pem"
    permissions_file: "/path/to/permissions.xml"
    governance_file: "/path/to/governance.xml"
    
  data:
    purge_timeout_seconds: 3600
    heartbeat_interval_seconds: 30
```

### 3.3 Security Implementation

#### 3.3.1 DDS Security Plugin Configuration
```cpp
void configureSecurity(DDS::DomainParticipant_ptr participant) {
    DDS::PropertySeq properties;
    
    // Identity configuration
    properties.push_back(DDS::Property_t(
        "dds.sec.auth.identity_ca",
        config.security.identity_ca
    ));
    properties.push_back(DDS::Property_t(
        "dds.sec.auth.identity_certificate",
        config.security.identity_cert
    ));
    properties.push_back(DDS::Property_t(
        "dds.sec.auth.private_key",
        config.security.identity_key
    ));
    
    // Access control configuration
    properties.push_back(DDS::Property_t(
        "dds.sec.access.permissions_ca",
        config.security.permissions_ca
    ));
    properties.push_back(DDS::Property_t(
        "dds.sec.access.permissions",
        config.security.permissions_file
    ));
    properties.push_back(DDS::Property_t(
        "dds.sec.access.governance",
        config.security.governance_file
    ));
    
    // Enable security
    properties.push_back(DDS::Property_t(
        "dds.sec.auth.plugin",
        "builtin.PKI-DH"
    ));
    properties.push_back(DDS::Property_t(
        "dds.sec.access.plugin",
        "builtin.Access-Permissions"
    ));
}
```

#### 3.3.2 Non-Repudiation Implementation
```cpp
class NonRepudiationPlugin : public DDS::DataWriterListener {
private:
    std::string dpm_id_;
    
public:
    void on_before_write(DDS::DataWriter_ptr writer, void* data) {
        // Inject DPM ID into every sample
        HsdsBase* hsds_data = static_cast<HsdsBase*>(data);
        hsds_data->source_dpm_id = dpm_id_;
        hsds_data->timestamp = getCurrentTimestamp();
        hsds_data->signature = generateSignature(hsds_data);
    }
};
```

## 4. CommUnityReader Application Design

### 4.1 Core Components

#### 4.1.1 HTTP API Server
- **Endpoints**:
  - `GET /api/v1/hsds/{table}` - Retrieve all records from table
  - `GET /api/v1/hsds/{table}/{id}` - Retrieve specific record
  - `GET /api/v1/hsds/{table}?source={dpm_id}` - Filter by source
  - `GET /api/v1/health` - Health check
  - `GET /api/v1/sources` - List all data sources

#### 4.1.2 DDS Subscriber Module
```cpp
class DdsSubscriber {
private:
    DDS::DomainParticipant_ptr participant_;
    std::map<std::string, DDS::DataReader_ptr> readers_;
    FederatedDataStore data_store_;
    
public:
    void initialize(const Config& config);
    void subscribeToTopic(const std::string& topic_name);
    void on_data_available(DDS::DataReader_ptr reader);
    
private:
    void processIncomingSample(const HsdsData& data);
    bool validateSource(const std::string& dpm_id);
};
```

#### 4.1.3 Federated Data Store
```cpp
class FederatedDataStore {
private:
    // Map: table_name -> record_id -> source_dpm_id -> record
    std::map<std::string, 
             std::map<std::string, 
                      std::map<std::string, HsdsRecord>>> data_;
    std::mutex data_mutex_;
    
public:
    void upsertRecord(const std::string& table, 
                     const HsdsRecord& record);
    void removeRecord(const std::string& table, 
                     const std::string& id, 
                     const std::string& source);
    std::vector<HsdsRecord> getTableData(const std::string& table);
    void purgeStaleData(int timeout_seconds);
};
```

### 4.2 Data Aggregation Strategy

```cpp
class DataAggregator {
public:
    enum ConflictResolution {
        MOST_RECENT,      // Use most recently updated
        PRIORITY_SOURCE,  // Prefer specific sources
        MERGE             // Attempt to merge fields
    };
    
    HsdsRecord resolveConflicts(
        const std::vector<HsdsRecord>& records,
        ConflictResolution strategy
    );
    
private:
    HsdsRecord mergeRecords(const std::vector<HsdsRecord>& records);
    HsdsRecord selectMostRecent(const std::vector<HsdsRecord>& records);
};
```

## 5. HSDS Topic Definitions (IDL)

### 5.1 Base Types
```idl
module HSDS {
    struct Timestamp {
        long seconds;
        long nanoseconds;
    };
    
    struct BaseRecord {
        string id;
        string source_dpm_id;
        Timestamp last_updated;
        string signature;
    };
```

### 5.2 Organization Topic
```idl
    struct Organization : BaseRecord {
        string name;
        string alternate_name;
        string description;
        string email;
        string url;
        string tax_status;
        string tax_id;
        string year_incorporated;
        string legal_status;
    };
    
    #pragma keylist Organization id source_dpm_id
```

### 5.3 Service Topic
```idl
    struct Service : BaseRecord {
        string organization_id;
        string program_id;
        string name;
        string alternate_name;
        string description;
        string url;
        string email;
        string status;
        string interpretation_services;
        string application_process;
        string wait_time;
        string fees;
        string accreditations;
        string licenses;
    };
    
    #pragma keylist Service id source_dpm_id
```

### 5.4 Location Topic
```idl
    struct Location : BaseRecord {
        string organization_id;
        string name;
        string alternate_name;
        string description;
        string transportation;
        double latitude;
        double longitude;
    };
    
    #pragma keylist Location id source_dpm_id
```

## 6. Implementation Guidelines

### 6.1 Error Handling Strategy
```cpp
class ErrorHandler {
public:
    enum ErrorLevel {
        WARNING,
        ERROR,
        CRITICAL
    };
    
    void handleDdsError(const DDS::ReturnCode_t& code);
    void handleApiError(const std::exception& e);
    void handleSecurityError(const SecurityException& e);
    
private:
    void logError(ErrorLevel level, const std::string& message);
    void notifyMonitoring(const Error& error);
    bool shouldRetry(const Error& error);
};
```

### 6.2 Monitoring and Observability
```cpp
class MetricsCollector {
private:
    std::atomic<uint64_t> messages_published_;
    std::atomic<uint64_t> messages_received_;
    std::atomic<uint64_t> api_requests_;
    std::atomic<uint64_t> errors_;
    
public:
    void recordPublish(const std::string& topic);
    void recordReceive(const std::string& topic);
    void recordApiRequest(const std::string& endpoint);
    void recordError(const std::string& error_type);
    
    json getMetrics() const;
};
```

### 6.3 Logging Configuration
```yaml
logging:
  level: INFO  # DEBUG, INFO, WARN, ERROR
  outputs:
    - type: console
      format: json
    - type: file
      path: /var/log/community/app.log
      rotation:
        max_size: 100MB
        max_files: 10
    - type: syslog
      host: localhost
      port: 514
```

## 7. Deployment Considerations

### 7.1 Docker Container Structure
```dockerfile
FROM ubuntu:22.04

# Install OpenDDS dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    libssl-dev \
    libboost-all-dev

# Install OpenDDS
COPY --from=opendds-builder /opt/OpenDDS /opt/OpenDDS
ENV DDS_ROOT=/opt/OpenDDS
ENV PATH=$DDS_ROOT/bin:$PATH

# Copy application
COPY build/community-writer /usr/local/bin/
COPY config/ /etc/community/

# Security certificates volume
VOLUME ["/etc/community/certs"]

EXPOSE 8080

CMD ["community-writer", "-c", "/etc/community/config.yaml"]
```

### 7.2 High Availability Configuration
```yaml
ha:
  mode: active-standby
  heartbeat_interval: 5
  failover_timeout: 30
  state_sync:
    enabled: true
    method: shared-storage  # or 'dds-based'
    storage_path: /shared/community/state
```

### 7.3 Resource Requirements
```yaml
resources:
  writer:
    cpu: 2 cores
    memory: 4GB
    disk: 20GB
    network: 100Mbps
    
  reader:
    cpu: 4 cores
    memory: 8GB
    disk: 50GB
    network: 100Mbps
```

## 8. Testing Strategy

### 8.1 Unit Test Structure
```cpp
TEST(WriterApplication, PublishValidData) {
    MockDdsPublisher publisher;
    WriterApp app(publisher);
    
    Organization org;
    org.id = "test-org-1";
    org.name = "Test Organization";
    
    EXPECT_CALL(publisher, publish(_)).Times(1);
    ASSERT_TRUE(app.publishOrganization(org));
}
```

### 8.2 Integration Test Scenarios
1. **Multi-organization data federation**
2. **Security permission enforcement**
3. **Network partition recovery**
4. **Data conflict resolution**
5. **Performance under load**

### 8.3 Performance Benchmarks
- Target throughput: 1000 messages/second
- Target latency: < 100ms for local network
- Target federation size: 100 organizations
- Target data volume: 1M records per table

## 9. Migration and Rollback Plan

### 9.1 Data Migration
```cpp
class DataMigrator {
public:
    void migrateFromLegacySystem(const LegacyDataSource& source);
    void validateMigration(const ValidationRules& rules);
    void rollbackMigration(const Checkpoint& checkpoint);
    
private:
    Checkpoint createCheckpoint();
    void transformData(const LegacyFormat& old, HsdsFormat& new);
};
```

### 9.2 Version Compatibility
- Maintain backward compatibility for 2 major versions
- Use IDL versioning for topic evolution
- Implement graceful degradation for missing features

## 10. Security Considerations

### 10.1 Threat Model
- **Unauthorized data access**: Mitigated by DDS security plugins
- **Data tampering**: Mitigated by signatures and non-repudiation
- **Denial of Service**: Mitigated by rate limiting and resource quotas
- **Man-in-the-middle**: Mitigated by TLS and certificate validation

### 10.2 Compliance Requirements
- HIPAA compliance for health-related data
- GDPR compliance for EU organizations
- SOC 2 Type II certification readiness

## 11. API Examples

### 11.1 Writer API Usage
```bash
# Submit organization data
curl -X POST https://writer.org.local:8080/api/v1/hsds/organization \
  -H "Authorization: Bearer ${AUTH_TOKEN}" \
  -H "Content-Type: application/json" \
  -d '{
    "id": "org-123",
    "name": "Community Services",
    "description": "Local community support services"
  }'
```

### 11.2 Reader API Usage
```bash
# Get all services from federated data
curl https://reader.org.local:8081/api/v1/hsds/service

# Get services from specific source
curl https://reader.org.local:8081/api/v1/hsds/service?source=org-abc
```

## 12. Development Roadmap

### Phase 1: Core Implementation (Weeks 1-4)
- Basic Writer and Reader applications
- DDS integration
- Simple HTTP APIs

### Phase 2: Security Implementation (Weeks 5-6)
- DPM integration
- Security plugin configuration
- Non-repudiation implementation

### Phase 3: Production Hardening (Weeks 7-8)
- Error handling and recovery
- Monitoring and metrics
- Performance optimization

### Phase 4: Advanced Features (Weeks 9-12)
- Conflict resolution strategies
- Advanced querying capabilities
- Administrative dashboard

## Appendix A: Configuration Templates

Complete configuration templates for both Writer and Reader applications are available in the `/config/templates` directory of the repository.

## Appendix B: IDL Definitions

Complete IDL definitions for all HSDS tables are available in the `/idl` directory of the repository.

## Appendix C: API Documentation

Detailed OpenAPI specifications for both Writer and Reader REST APIs are available in the `/api/openapi` directory.