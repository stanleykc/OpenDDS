# File_Sync: Design Document
Version 1.0

## 1. Overview

File_Sync is a peer-to-peer file synchronization application built on OpenDDS that provides secure, automatic synchronization of text files across multiple computers on a LAN. The design prioritizes data safety, security, ease of use, and automation.

### 1.1 Design Principles
1. **Safety First**: All operations prioritize data integrity and protection from loss
2. **Secure by Default**: DDS-Security with UnityAuth ensures encrypted, authenticated communication
3. **Peer-to-Peer**: No central server; all instances are equal participants
4. **Atomic Operations**: File writes are atomic to prevent corruption
5. **Conflict-Safe**: Local changes are never overwritten; conflicts create separate files

## 2. System Architecture

### 2.1 High-Level Architecture
```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   File_Sync     │    │   File_Sync     │    │   File_Sync     │
│   Instance A    │◄──►│   Instance B    │◄──►│   Instance C    │
│                 │    │                 │    │                 │
│ ┌─────────────┐ │    │ ┌─────────────┐ │    │ ┌─────────────┐ │
│ │Source Dir   │ │    │ │Source Dir   │ │    │ │Source Dir   │ │
│ │Monitor      │ │    │ │Monitor      │ │    │ │Monitor      │ │
│ └─────────────┘ │    │ └─────────────┘ │    │ └─────────────┘ │
│ ┌─────────────┐ │    │ ┌─────────────┐ │    │ ┌─────────────┐ │
│ │Dest Dir     │ │    │ │Dest Dir     │ │    │ │Dest Dir     │ │
│ │Sync         │ │    │ │Sync         │ │    │ │Sync         │ │
│ └─────────────┘ │    │ └─────────────┘ │    │ └─────────────┘ │
└─────────────────┘    └─────────────────┘    └─────────────────┘
         │                       │                       │
         └───────────────────────┼───────────────────────┘
                                 │
                    ┌─────────────────────┐
                    │   OpenDDS Domain    │
                    │                     │
                    │  FileMetadata Topic │
                    │  FileChunk Topic    │
                    │                     │
                    │  DDS-Security       │
                    │  (UnityAuth)        │
                    └─────────────────────┘
```

### 2.2 Core Components

#### 2.2.1 FileSyncApplication
Main application class that orchestrates all components:
- Initializes OpenDDS participant and topics
- Manages configuration
- Coordinates between file monitor and DDS publishers/subscribers
- Handles graceful shutdown

#### 2.2.2 FileSystemMonitor
Monitors source directory for changes:
- Uses platform-appropriate file system watching APIs (inotify on Linux, ReadDirectoryChangesW on Windows)
- Detects create, modify, delete events
- Provides callback interface for change notifications
- Handles recursive directory monitoring

#### 2.2.3 FileMetadataPublisher
Publishes file metadata changes:
- Creates FileMetadata samples for file events
- Calculates SHA-256 hashes of file content
- Publishes metadata before chunk data

#### 2.2.4 FileChunkPublisher
Publishes file content in chunks:
- Reads files and splits into configurable chunk sizes (default: 64KB)
- Publishes ordered sequence of FileChunk samples
- Ensures all chunks are sent reliably

#### 2.2.5 FileMetadataSubscriber
Receives and processes metadata from peers:
- Subscribes to FileMetadata topic
- Initiates file reconstruction process
- Handles deletion notifications
- Manages conflict detection

#### 2.2.6 FileChunkSubscriber
Receives and reassembles file chunks:
- Subscribes to FileChunk topic
- Collects chunks for each file_id
- Reassembles files in correct order
- Verifies hash integrity before writing

#### 2.2.7 FileManager
Handles file I/O operations:
- Atomic file writes using temporary files
- Hash calculation and verification
- Conflict resolution file naming
- Directory creation and management

#### 2.2.8 ConfigurationManager
Manages application configuration:
- Reads configuration from files and command line
- Validates settings
- Provides access to DDS domain settings and security credentials

## 3. OpenDDS Data Model

### 3.1 IDL Definitions

```cpp
// FileSync.idl
module FileSync {
    
    // File metadata for control messages
    #pragma DCPS_DATA_TYPE "FileSync::FileMetadata"
    #pragma DCPS_DATA_KEY "FileSync::FileMetadata file_id"
    struct FileMetadata {
        string file_id;         // Relative path from source directory (key)
        long long mod_time;     // Modification timestamp (seconds since epoch)
        string file_hash;       // SHA-256 hash of complete file content
        boolean is_deleted;     // True if file was deleted
        string publisher_id;    // Unique identifier for publishing peer
    };
    
    // File content chunks for bulk data transfer
    #pragma DCPS_DATA_TYPE "FileSync::FileChunk"
    #pragma DCPS_DATA_KEY "FileSync::FileChunk file_id, sequence_number"
    struct FileChunk {
        string file_id;         // Matches FileMetadata.file_id (part of key)
        long sequence_number;   // Chunk sequence number (part of key)
        long total_chunks;      // Total number of chunks for this file
        sequence<octet> data;   // Raw chunk data (max 64KB)
        string file_hash;       // SHA-256 hash (redundant for verification)
    };
};
```

### 3.2 QoS Policies

#### FileMetadata Topic QoS:
- **Reliability**: RELIABLE (guaranteed delivery)
- **Durability**: TRANSIENT_LOCAL (late joiners receive recent metadata)
- **History**: KEEP_LAST with depth=1 per key
- **Liveliness**: AUTOMATIC with lease_duration=30s

#### FileChunk Topic QoS:
- **Reliability**: RELIABLE (guaranteed delivery)
- **Durability**: VOLATILE (chunks not persistent)
- **History**: KEEP_ALL (ensure all chunks received)
- **Resource Limits**: max_samples=10000, max_instances=1000

## 4. File Synchronization Process

### 4.1 Publishing Workflow

```
File Change Detected
        │
        ▼
Calculate SHA-256 Hash
        │
        ▼
Publish FileMetadata
        │
        ▼
Read File & Split into Chunks
        │
        ▼
Publish FileChunk Samples
(in sequence order)
        │
        ▼
Complete
```

### 4.2 Subscribing Workflow

```
Receive FileMetadata
        │
        ▼
   is_deleted?
    ┌─────┼─────┐
    │ Yes │ No  │
    ▼     ▼     ▼
Delete    Prepare to
File      Collect Chunks
    │            │
    ▼            ▼
Complete    Collect All FileChunks
               │
               ▼
          Reassemble File
               │
               ▼
          Verify Hash
               │
           ┌───┼───┐
           │ OK │ Bad│
           ▼   ▼   ▼
      Write   Discard
      File    & Log Error
           │
           ▼
      Complete
```

### 4.3 Conflict Detection and Resolution

When receiving a file update:

1. **Check if local file exists and has been modified**
   - Compare local file hash with last known synchronized hash
   - If hashes differ, a local modification has occurred

2. **Conflict Resolution**
   - Never overwrite locally modified files
   - Save incoming version with conflict suffix: `filename (conflicted copy from hostname YYYY-MM-DD HH-MM-SS).ext`
   - Log conflict for user awareness

3. **State Tracking**
   - Maintain database/file of last synchronized hash for each file
   - Update after successful synchronization operations

## 5. Security Design

### 5.1 DDS-Security Configuration

The application uses OpenDDS Security with UnityAuth:

- **Authentication**: Mutual certificate-based authentication
- **Access Control**: Topic-based permissions
- **Encryption**: AES-256 for all data transmission
- **Key Management**: Automatic key exchange and rotation

### 5.2 Security Files Structure
```
security/
├── identity_ca.pem          # Certificate Authority
├── permissions_ca.pem       # Permissions CA
├── identity_certificate.pem # Node identity certificate
├── identity_private_key.pem # Node private key
├── permissions.xml          # Access control permissions
└── governance.xml           # Security governance policy
```

### 5.3 Governance Policy
- All topics require authentication and encryption
- Only authenticated participants can join domain
- Access control enforced at topic level

## 6. Configuration Management

### 6.1 Configuration File Format (file_sync.conf)
```ini
[directories]
source_dir=/path/to/source
dest_dir=/path/to/destination

[dds]
domain_id=42
dcps_config_file=dds_config.ini

[security]
identity_ca=/path/to/identity_ca.pem
permissions_ca=/path/to/permissions_ca.pem
identity_certificate=/path/to/identity_certificate.pem
identity_private_key=/path/to/identity_private_key.pem
permissions_file=/path/to/permissions.xml
governance_file=/path/to/governance.xml

[sync]
chunk_size=65536
max_file_size=104857600
excluded_patterns=*.tmp,*.swp,*~,.DS_Store

[logging]
level=info
file=/var/log/file_sync.log
```

### 6.2 Command Line Interface
```bash
file_sync [OPTIONS]

Options:
  -c, --config FILE     Configuration file path (default: file_sync.conf)
  -s, --source DIR      Source directory to monitor
  -d, --dest DIR        Destination directory for synchronized files
  -D, --domain ID       DDS domain ID (default: 42)
  --daemon              Run as daemon/service
  --verbose             Enable verbose logging
  --help                Show help message
```

## 7. Error Handling and Resilience

### 7.1 Error Categories and Responses

#### Network/DDS Errors:
- **Participant creation failure**: Log error, exit with code 1
- **Topic creation failure**: Log error, exit with code 1
- **Temporary network issues**: Continue operation, rely on DDS reliability QoS

#### File System Errors:
- **Source directory inaccessible**: Log error, exit with code 1
- **Destination directory creation failure**: Log error, attempt to create, exit if unsuccessful
- **File read errors**: Log warning, skip file, continue monitoring
- **File write errors**: Log error, discard incoming file, continue operation

#### Data Integrity Errors:
- **Hash mismatch**: Discard file, log error, request retransmission
- **Incomplete chunk reception**: Wait with timeout, log warning if timeout exceeded
- **Malformed data**: Log error, discard, continue operation

### 7.2 Recovery Mechanisms

#### Startup Recovery:
- Compare source and destination directories on startup
- Publish metadata for all files in source directory
- Subscribe to existing metadata to identify missed changes

#### Runtime Recovery:
- Periodic integrity checks (configurable interval)
- Automatic retransmission for failed transfers
- Graceful handling of peer disconnections/reconnections

## 8. Performance Considerations

### 8.1 Scalability Targets
- Support 1000+ files per directory
- Handle file sizes up to 100MB
- Support 5-10 concurrent peers
- Process 100+ file changes per minute

### 8.2 Optimizations

#### Memory Management:
- Stream-based file reading for large files
- Configurable chunk sizes to balance memory usage and network efficiency
- Release chunks after successful transmission

#### Network Efficiency:
- Batch small file operations
- Use DDS batching for chunk transmission
- Implement backpressure to prevent network saturation

#### CPU Optimization:
- Asynchronous hash calculation
- Parallel chunk processing
- Efficient file system monitoring using OS-native APIs

## 9. Build System and Dependencies

### 9.1 CMake Build Structure
```
File_Sync/
├── CMakeLists.txt           # Main build configuration
├── src/
│   ├── CMakeLists.txt
│   ├── main.cpp
│   ├── FileSyncApplication.cpp/h
│   ├── FileSystemMonitor.cpp/h
│   ├── FileManager.cpp/h
│   ├── ConfigurationManager.cpp/h
│   └── dds/
│       ├── FileMetadataPublisher.cpp/h
│       ├── FileMetadataSubscriber.cpp/h
│       ├── FileChunkPublisher.cpp/h
│       └── FileChunkSubscriber.cpp/h
├── idl/
│   ├── CMakeLists.txt
│   └── FileSync.idl
├── config/
│   ├── file_sync.conf.example
│   ├── dds_config.ini.example
│   └── security/
├── tests/
│   ├── CMakeLists.txt
│   ├── unit/
│   └── integration/
└── docs/
    ├── setup.md
    └── troubleshooting.md
```

### 9.2 Dependencies
- **OpenDDS**: Core DDS implementation and security features
- **OpenSSL**: Cryptographic hash functions and TLS support
- **C++17 Standard Library**: std::filesystem for cross-platform file operations
- **Platform-specific**: File system monitoring APIs (inotify, etc.)

### 9.3 Build Requirements
- CMake 3.20+
- C++17 compliant compiler (GCC 8+, Clang 7+, MSVC 2019+)
- OpenDDS installation with security support
- OpenSSL 1.1+ or 3.0+

## 10. Testing Strategy

### 10.1 Unit Tests
- Individual component functionality
- Mock DDS interfaces for isolated testing
- File system operation validation
- Configuration parsing and validation

### 10.2 Integration Tests
- Multi-peer synchronization scenarios
- Network partition and recovery
- Large file transfers
- Concurrent modification handling
- Security authentication and authorization

### 10.3 Performance Tests
- Throughput measurement with various file sizes
- Memory usage profiling
- Network bandwidth utilization
- Latency measurements for small file changes

### 10.4 Security Tests
- Certificate validation
- Unauthorized access attempts
- Man-in-the-middle attack simulation
- Data tampering detection

## 11. Deployment and Operations

### 11.1 Installation
1. Build application using CMake
2. Create configuration directory structure
3. Generate security certificates using provided scripts
4. Configure source and destination directories
5. Start application as service/daemon

### 11.2 Monitoring and Logging
- Structured logging with configurable levels
- Metrics for file synchronization rates
- Network connectivity status
- Error rate monitoring
- Performance metrics (latency, throughput)

### 11.3 Maintenance
- Log rotation configuration
- Certificate renewal procedures
- Database maintenance for synchronization state
- Backup and recovery procedures

This design provides a comprehensive foundation for implementing a secure, reliable, and efficient file synchronization system using OpenDDS.