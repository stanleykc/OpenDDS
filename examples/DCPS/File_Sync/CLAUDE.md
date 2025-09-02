# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

File_Sync is a peer-to-peer file synchronization application built on OpenDDS that provides secure, automatic synchronization of text files across multiple computers on a LAN. This is an example application demonstrating OpenDDS capabilities in C++.

## Architecture Overview

### Core Components
- **FileSyncApplication**: Main application orchestrator that manages OpenDDS participants, topics, and coordinates all components
- **ConfigurationManager**: Handles configuration from files and command line arguments
- **FileSystemMonitor**: Monitors source directories for file changes (using platform-appropriate APIs)
- **FileManager**: Handles file I/O operations, chunking, hashing, and atomic writes
- **DDS Publishers/Subscribers**: Handles OpenDDS communication for file metadata and content chunks

### OpenDDS Data Model
The application uses two primary DDS topics defined in `idl/FileSync.idl`:
- **FileMetadata Topic**: Lightweight control messages announcing file changes (create/modify/delete)
- **FileChunk Topic**: Bulk data transfer for file content, split into manageable chunks

### Peer-to-Peer Architecture  
No central server - each instance acts as both publisher and subscriber in a shared OpenDDS domain. Security is handled via DDS-Security with UnityAuth for authentication and encryption.

## Build System and Commands

### Environment Setup
```bash
# Ensure OpenDDS environment is set up
source /path/to/OpenDDS/setenv.sh
```

### Build Commands
```bash
# Generate Makefiles using MPC (Make Project Creator)
mpc.pl -type gnuace FileSync.mpc

# Build main application
make

# Build unit tests
make file_sync_tests

# Clean build artifacts
make clean
```

### Development Commands
```bash
# Run the main application
./file_sync --help
./file_sync --source /path/to/source --dest /path/to/dest --domain 42
./file_sync --config file_sync.conf --verbose

# Run unit tests
./file_sync_tests

# Generate IDL type support (done automatically by MPC)
opendds_idl idl/FileSync.idl
```

## Configuration

### Command Line Options
- `-c, --config FILE`: Configuration file path (default: file_sync.conf)
- `-s, --source DIR`: Source directory to monitor
- `-d, --dest DIR`: Destination directory for synchronized files  
- `-D, --domain ID`: DDS domain ID (default: 42)
- `--daemon`: Run as daemon/service
- `-v, --verbose`: Enable verbose logging
- `-h, --help`: Show help message

### Configuration File Format
Uses INI format with sections for directories, DDS, security, sync, and logging settings. See `config/file_sync.conf.example` for template.

## Key Development Patterns

### OpenDDS Integration
- Uses ACE/TAO macros and patterns (ACE_TMAIN, ACE_Get_Opt)
- MPC build system for OpenDDS integration and IDL compilation
- DDS participant, topic, publisher, and subscriber lifecycle management
- Reliable QoS for guaranteed delivery, with different policies for metadata vs. chunks

### File Operations
- Atomic file writes using temporary files to prevent corruption
- SHA-256 hash validation for data integrity
- Configurable chunk sizes for efficient transfer
- Platform-appropriate file system monitoring

### Error Handling
- Comprehensive error classification and logging
- Graceful degradation and recovery mechanisms
- Conflict resolution that never overwrites local changes
- Data integrity validation at multiple levels

## Testing

### Unit Tests
Located in `tests/unit/` directory:
- `test_ConfigurationManager.cpp`: Configuration loading and validation
- `test_FileManager.cpp`: File I/O operations, hashing, atomic writes

### Running Tests
```bash
# Build and run unit tests
make file_sync_tests
./file_sync_tests
```

### Test Structure
Uses a simple test framework within the unit test files. Tests cover core functionality like configuration parsing, file operations, and basic DDS integration.

## Current Implementation Status

**Phase 1.1 - Project Structure**: ✅ COMPLETED  
**Phase 1.3 - Configuration Management**: ✅ COMPLETED  
**Phase 3.1 - Basic File I/O**: ✅ COMPLETED  

**In Progress**: Phase 1.2 - Basic OpenDDS Integration (topic creation and data flow)

See `todo/tasks.md` for complete implementation roadmap and current progress.

## Security Considerations

- DDS-Security integration with certificate-based authentication
- TLS/DTLS encryption for all data transmission
- Access control policies at topic level
- Secure storage for private keys and certificates
- Conflict resolution prevents data loss (never overwrites local changes)

## Dependencies

- **OpenDDS**: Core DDS implementation with security support
- **ACE/TAO**: Middleware foundation (included with OpenDDS)
- **C++17 Standard Library**: For modern C++ features and std::filesystem
- **MPC**: Make Project Creator for build system integration
- **OpenSSL**: For cryptographic operations (SHA-256 hashing, security)

## Common Development Tasks

### Adding New Components
1. Add header/source files to MPC project in `FileSync.mpc`
2. Follow existing patterns for OpenDDS integration
3. Add corresponding unit tests in `tests/unit/`
4. Update documentation as needed

### Testing Changes
1. Build and run unit tests: `make file_sync_tests && ./file_sync_tests`
2. Test basic functionality: `./file_sync --help`
3. Verify OpenDDS integration doesn't break existing functionality

### Debugging
- Use `--verbose` flag for detailed logging
- Check OpenDDS participant and topic creation
- Verify configuration file parsing
- Monitor DDS communication patterns