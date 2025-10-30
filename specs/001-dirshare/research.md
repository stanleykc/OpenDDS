# Technical Research: DirShare Implementation

**Feature**: DirShare - Distributed File Synchronization
**Date**: 2025-10-30
**Purpose**: Resolve technical unknowns and establish implementation patterns

## Research Area 1: File System Monitoring

### Decision: Polling-Based Approach with Periodic Directory Scanning

**Rationale**:
- **Simplicity**: Polling is straightforward to implement and debug across all platforms
- **Cross-platform**: Works identically on Linux, macOS, and Windows without platform-specific code
- **Example-appropriate**: For a DevGuideExamples demonstration, simplicity and teachability outweigh efficiency
- **Testability**: Predictable behavior makes automated testing more reliable
- **Resource usage**: For monitoring a single directory with moderate file counts, polling overhead is acceptable

**Implementation Approach**:
- Poll directory every 1-2 seconds using standard C++ filesystem APIs (C++17 `<filesystem>` or ACE filesystem utilities)
- Track file metadata (size, modification time) in memory map
- Compare current state with previous state to detect changes
- Generate FileEvent for each detected change

**Alternatives Considered**:

| Approach | Pros | Cons | Why Rejected |
|----------|------|------|--------------|
| Platform-specific APIs (inotify/FSEvents/ReadDirectoryChangesW) | Real-time detection, low CPU | Complex, platform-dependent code | Adds unnecessary complexity for an example |
| ACE Reactor with file descriptors | Integrated with ACE event loop | Linux-only (inotify-based) | Not cross-platform |
| Third-party library (efsw, FileWatcher) | Maintained abstraction | External dependency | Violates OpenDDS example simplicity |

**References**:
- ACE Filesystem utilities: `ACE_Dirent`, `ACE_stat`
- C++17 filesystem: `std::filesystem::directory_iterator`, `std::filesystem::last_write_time`

---

## Research Area 2: Binary Data Transfer in DDS

### Decision: Single-Message Transfer for Small Files, Sequence-Based Chunking for Large Files

**Rationale**:
- **DDS message limits**: OpenDDS typically handles messages up to several MB efficiently, but larger messages may cause performance issues
- **Simplicity first**: Most files in typical use cases are under 10MB, use simple approach for these
- **Scalability**: Support larger files (up to 1GB limit) via chunking when needed
- **DDS sequence types**: IDL sequences provide natural mechanism for variable-length data

**Implementation Approach**:

**For files < 10MB (threshold configurable)**:
```idl
struct FileContent {
  @key string filename;
  sequence<octet> data;  // Entire file content
  unsigned long long size;
  unsigned long checksum;
};
```

**For files >= 10MB**:
```idl
struct FileChunk {
  @key string filename;
  @key unsigned long chunk_id;  // Chunk sequence number
  sequence<octet> data;          // Chunk data (1MB per chunk)
  unsigned long total_chunks;    // Total chunks for this file
  unsigned long long file_size;
  unsigned long checksum;        // Checksum of complete file
};
```

**Transfer Strategy**:
1. Check file size before transfer
2. If < 10MB: Send single FileContent message
3. If >= 10MB: Split into 1MB chunks, send as FileChunk sequence
4. Receiver reassembles chunks before writing to disk
5. Verify checksum after complete transfer

**Alternatives Considered**:

| Approach | Pros | Cons | Why Rejected |
|----------|------|------|--------------|
| Always chunk everything | Consistent code path | Overhead for small files | Overengineered for common case |
| Use DDS large data features | Built-in support | OpenDDS version-dependent | May not be available in all versions |
| External file transfer (HTTP/FTP) | Optimized for files | Defeats DDS example purpose | Not demonstrating DDS capabilities |

**References**:
- OpenDDS Developer's Guide: "Sequences" (unbounded sequences for binary data)
- DDS specification: IDL sequence types
- Message size considerations: Default MAX_MESSAGE_SIZE typically 64KB-16MB depending on transport

---

## Research Area 3: Conflict Resolution Implementation

### Decision: Timestamp Comparison with Millisecond Precision

**Rationale**:
- **DDS built-in timestamps**: DDS provides source timestamps for all samples
- **Deterministic**: Timestamp-based ordering provides clear, reproducible conflict resolution
- **Spec requirement**: Feature specification explicitly calls for "last modification takes precedence"
- **Clock sync assumption**: Specification assumes reasonably synchronized clocks (within seconds)

**Implementation Approach**:

```cpp
// Use DDS source timestamp from SampleInfo
void DataReaderListenerImpl::on_data_available(DDS::DataReader_ptr reader) {
  FileEvent event;
  DDS::SampleInfo info;

  if (reader->take_next_sample(event, info) == DDS::RETCODE_OK) {
    if (info.valid_data) {
      // Check for existing file
      if (localFileExists(event.filename)) {
        DDS::Time_t local_mod_time = getLocalModTime(event.filename);

        // Compare DDS source timestamp with local modification time
        if (compareTimestamps(info.source_timestamp, local_mod_time) > 0) {
          // Remote version is newer, apply change
          applyFileChange(event);
        } else {
          // Local version is newer, ignore remote change
          ACE_DEBUG((LM_INFO, "Ignoring older remote version of %C\n", event.filename.c_str()));
        }
      } else {
        // File doesn't exist locally, apply change
        applyFileChange(event);
      }
    }
  }
}
```

**Edge Cases Handled**:
- Clock skew within a few seconds: Acceptable per spec assumptions
- Simultaneous edits (same millisecond): Deterministic ordering via tie-breaker (e.g., participant ID or GUID)
- Deleted then recreated files: Timestamp distinguishes operations

**Alternatives Considered**:

| Approach | Pros | Cons | Why Rejected |
|----------|------|------|--------------|
| Vector clocks | Handles network partitions | Complex for example | Overengineered; not in spec |
| CRDTs (Conflict-free Replicated Data Types) | Automatic merging | Very complex | Not applicable to binary files |
| Manual user prompts | User decides | Not automated | Violates "automatic propagation" requirement |
| Checksums for content equality | Detects identical edits | Doesn't resolve conflicts | Complementary, not alternative |

**References**:
- DDS specification: `source_timestamp` in SampleInfo structure
- OpenDDS: `DDS::Time_t` representation (seconds + nanoseconds since epoch)

---

## Research Area 4: OpenDDS Message Size Limits and Chunking Best Practices

### Decision: 1MB Chunk Size with Configurable Transport Settings

**Rationale**:
- **Transport defaults**: OpenDDS TCP transport default max message size is 2MB (configurable)
- **Network efficiency**: 1MB chunks balance overhead vs. granularity
- **Memory usage**: Reasonable buffer size for most systems
- **DDS reliability**: Smaller chunks reduce retransmission cost on packet loss

**Implementation Approach**:

**INI Configuration** (for both InfoRepo and RTPS modes):
```ini
[common]
DCPSGlobalTransportConfig=$file

[config/tcp_config]
transports=tcp
max_message_size=16777216  # 16MB to handle large chunks

[transport/tcp]
transport_type=tcp
max_packet_size=8192       # 8KB packets

[config/rtps_config]
transports=rtps_udp
max_message_size=16777216  # 16MB consistent with TCP

[transport/rtps_udp]
transport_type=rtps_udp
local_address=0.0.0.0:0
```

**Chunk Management**:
- Define constant: `const size_t CHUNK_SIZE = 1024 * 1024; // 1MB`
- File size threshold: `const size_t SINGLE_MESSAGE_THRESHOLD = 10 * 1024 * 1024; // 10MB`
- Calculate chunks: `size_t num_chunks = (file_size + CHUNK_SIZE - 1) / CHUNK_SIZE;`
- Use keyed topics for chunks (filename + chunk_id as composite key)

**Memory Management**:
- Read/write files in chunks to avoid loading entire large files
- Use ACE memory pools if needed for frequent allocations
- Release chunk buffers immediately after sending/receiving

**Alternatives Considered**:

| Approach | Pros | Cons | Why Rejected |
|----------|------|------|--------------|
| Smaller chunks (256KB) | Lower memory, better streaming | More overhead | Diminishing returns |
| Larger chunks (10MB) | Less overhead | May hit transport limits | Risk of exceeding defaults |
| Adaptive chunk sizing | Optimizes per file | Complex logic | Overengineered for example |
| No chunking | Simplest | Can't handle large files | Violates spec requirement |

**References**:
- OpenDDS Developer's Guide: "Configuring Transports"
- OpenDDS configuration: `DCPSChunks` and `max_message_size` parameters
- DDS RTPS specification: Fragmentation and reassembly

---

## Research Area 5: File Integrity Verification Methods

### Decision: CRC32 Checksum for Speed and Simplicity

**Rationale**:
- **Performance**: CRC32 is extremely fast (1-2 GB/s on modern CPUs)
- **Built-in support**: Many libraries include CRC32 (zlib, ACE)
- **Sufficient for use case**: Detects transmission errors and corruption; not for cryptographic security
- **Low overhead**: 4 bytes per file, negligible storage/transmission cost
- **Industry standard**: Used in ZIP, PNG, Ethernet, many file formats

**Implementation Approach**:

```cpp
#include <ace/OS_NS_string.h>
// Use ACE or zlib CRC32 implementation

unsigned long calculate_crc32(const char* data, size_t length) {
  unsigned long crc = 0xFFFFFFFF;
  for (size_t i = 0; i < length; ++i) {
    crc = crc32_table[(crc ^ data[i]) & 0xFF] ^ (crc >> 8);
  }
  return crc ^ 0xFFFFFFFF;
}

// Verify file after transfer
bool verify_file_integrity(const std::string& filename, unsigned long expected_crc) {
  std::ifstream file(filename, std::ios::binary);
  std::vector<char> buffer(1024 * 1024); // 1MB buffer

  unsigned long calculated_crc = 0xFFFFFFFF;
  while (file.read(buffer.data(), buffer.size()) || file.gcount() > 0) {
    calculated_crc = calculate_crc32_incremental(buffer.data(), file.gcount(), calculated_crc);
  }
  calculated_crc ^= 0xFFFFFFFF;

  return calculated_crc == expected_crc;
}
```

**Integrity Check Flow**:
1. Sender calculates CRC32 before transmitting file
2. CRC32 included in FileMetadata or FileContent message
3. Receiver calculates CRC32 after reassembling/writing file
4. If mismatch: Log error, optionally request retransmission
5. If match: Confirm successful transfer

**Alternatives Considered**:

| Approach | Pros | Cons | Why Rejected |
|----------|------|------|--------------|
| MD5 hash | Stronger than CRC32 | Slower, overkill for non-crypto | Not needed for error detection |
| SHA-256 | Cryptographic strength | Much slower | Not a security application |
| Size-only check | Fastest | Doesn't detect corruption | Insufficient for reliability |
| No verification | Simplest | Can't detect errors | Violates spec requirement (FR-017) |

**References**:
- ACE: `ACE::crc32()` if available
- zlib: `crc32()` function (widely available)
- RFC 1952: GZIP file format specification (CRC32 usage)
- Ethernet: IEEE 802.3 CRC-32

---

## Summary of Technical Decisions

| Area | Decision | Rationale |
|------|----------|-----------|
| File Monitoring | Polling (1-2 sec interval) | Cross-platform simplicity, example-appropriate |
| Data Transfer | Single message <10MB, 1MB chunks >=10MB | Balance simplicity and scalability |
| Conflict Resolution | DDS timestamp comparison (millisecond precision) | Deterministic, leverages DDS built-in timestamps |
| Message Sizing | 1MB chunks, 16MB transport max | Efficient, within OpenDDS defaults |
| Integrity Verification | CRC32 checksum | Fast, sufficient, industry standard |

**No Unresolved Issues**: All technical unknowns from Technical Context have been researched and decided.

**Ready for Phase 1**: Design artifacts (data-model.md, contracts, quickstart) can now be generated based on these research decisions.
