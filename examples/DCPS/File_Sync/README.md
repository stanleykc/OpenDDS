# File_Sync - OpenDDS File Synchronization Application

File_Sync is a peer-to-peer file synchronization application built on OpenDDS that provides secure, automatic synchronization of text files across multiple computers on a LAN.

## Current Status

**Phase 1.1 - Project Structure Setup: COMPLETED**

The foundational project structure has been established with:

- ✅ MPC build system with proper OpenDDS integration
- ✅ IDL compilation setup for FileSync.idl  
- ✅ Basic application skeleton with main.cpp
- ✅ Basic logging infrastructure (using std::cout for now)
- ✅ Unit test framework structure

## Project Structure

```
File_Sync/
├── FileSync.mpc           # MPC build configuration
├── README.md              # This file
├── design.md              # Detailed design document
├── requirements.md        # Software requirements specification
├── todo/
│   └── tasks.md          # Implementation task tracking
├── idl/
│   └── FileSync.idl      # DDS data type definitions
├── src/
│   ├── main.cpp          # Application entry point
│   ├── FileSyncApplication.{h,cpp}     # Main application class
│   ├── ConfigurationManager.{h,cpp}    # Configuration management
│   ├── FileSystemMonitor.{h,cpp}       # File system monitoring
│   ├── FileManager.{h,cpp}            # File I/O operations
│   └── dds/              # DDS-related classes
│       ├── FileMetadataPublisher.{h,cpp}
│       ├── FileMetadataSubscriber.{h,cpp}
│       ├── FileChunkPublisher.{h,cpp}
│       └── FileChunkSubscriber.{h,cpp}
├── config/
│   └── file_sync.conf.example         # Example configuration
└── tests/
    └── unit/             # Unit tests
        ├── test_main.cpp
        ├── test_ConfigurationManager.cpp
        └── test_FileManager.cpp
```

## Building

### Prerequisites

1. OpenDDS built and installed with security support
2. ACE/TAO (automatically included with OpenDDS)
3. MPC (Make Project Creator)
4. C++17 compliant compiler

### Build Steps

1. **Set up OpenDDS environment:**
   ```bash
   cd /path/to/OpenDDS
   source setenv.sh
   ```

2. **Generate Makefiles:**
   ```bash
   cd examples/DCPS/File_Sync
   mpc.pl -type gnuace FileSync.mpc
   ```

3. **Build the application:**
   ```bash
   make
   ```

4. **Build and run tests:**
   ```bash
   make file_sync_tests
   ./file_sync_tests
   ```

## Current Functionality

This is currently a **foundational skeleton**. The application can:

- Parse command line arguments and configuration files
- Initialize DDS participant and join a domain
- Create stub DDS publishers and subscribers
- Provide basic file I/O operations
- Run a simple main loop (currently just sleeps)
- Gracefully shutdown when interrupted

**Note:** File synchronization, monitoring, and DDS data exchange are not yet implemented - these are placeholder stubs.

## Usage

```bash
# Basic usage with command line arguments
./file_sync --source /path/to/source --dest /path/to/dest --domain 42

# Using configuration file
cp config/file_sync.conf.example file_sync.conf
# Edit file_sync.conf with your settings
./file_sync --config file_sync.conf

# Help
./file_sync --help
```

## Configuration

Copy `config/file_sync.conf.example` to `file_sync.conf` and modify:

```ini
[directories]
source_dir=/path/to/monitor
dest_dir=/path/to/sync

[dds]
domain_id=42

[sync]
chunk_size=65536
max_file_size=104857600
excluded_patterns=*.tmp,*.swp,*~
```

## Testing

Run the unit tests to verify basic functionality:

```bash
./file_sync_tests
```

Current tests cover:
- ConfigurationManager: Loading config files, validation, command-line overrides
- FileManager: File I/O operations, hashing, atomic writes, directory management

## Next Steps

The next phase (1.2) will implement basic OpenDDS integration:

1. Create and register FileMetadata and FileChunk topics
2. Implement basic DataWriter and DataReader creation  
3. Add graceful shutdown handling
4. Test that participants can join domain and discover peers

See `todo/tasks.md` for the complete implementation roadmap.

## Architecture

File_Sync uses a peer-to-peer architecture where each instance:

- Monitors a source directory for file changes
- Publishes changes via OpenDDS topics (FileMetadata and FileChunk)
- Subscribes to changes from other peers
- Synchronizes received changes to destination directory
- Handles conflicts safely without data loss

For detailed design information, see `design.md`.

## Dependencies

- OpenDDS (with DDS-Security support recommended)
- ACE/TAO middleware
- C++17 standard library
- Platform-specific file system monitoring APIs (future)
- OpenSSL (for SHA-256 hashing - future enhancement)

## License

This is example code for demonstrating OpenDDS capabilities.