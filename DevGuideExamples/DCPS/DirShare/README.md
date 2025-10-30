# DirShare - Distributed File Synchronization Example

## Overview

DirShare is an OpenDDS example demonstrating real-time file synchronization between multiple DDS participants. Each participant monitors a local directory and uses DDS publish-subscribe to propagate file changes (create, modify, delete) to other participants.

## Features

- **Initial Directory Synchronization**: When participants join, existing files are automatically synchronized
- **Real-Time File Propagation**: File creation, modification, and deletion events propagate automatically
- **Conflict Resolution**: Last-write-wins based on timestamps
- **Large File Support**: Files up to 1GB with automatic chunking (1MB chunks for files >=10MB)
- **Integrity Verification**: CRC32 checksums ensure file integrity
- **Dual Discovery Support**: Both InfoRepo and RTPS discovery mechanisms
- **Cross-Platform**: Works on Linux, macOS, and Windows

## Prerequisites

1. **OpenDDS Configured**: Run `./configure` from OpenDDS repository root
2. **Environment Sourced**: Run `source setenv.sh` (or `setenv.cmd` on Windows)
3. **Perl Installed**: Required for test scripts

```bash
# From OpenDDS repository root
./configure
source setenv.sh
make  # Build OpenDDS core (first time only)
```

## Building

### Option 1: MPC Build (Primary)

```bash
cd DevGuideExamples/DCPS/DirShare

# Generate makefiles (first time or after .mpc changes)
mwc.pl -type gnuace

# Build
make

# Verify build
ls -lh dirshare
```

### Option 2: CMake Build (Alternative)

```bash
cd DevGuideExamples/DCPS/DirShare
mkdir build && cd build

# Configure
cmake ..

# Build
cmake --build .

# Verify build
ls -lh dirshare
```

## Running Tests

### InfoRepo Mode (Default)

```bash
cd DevGuideExamples/DCPS/DirShare
perl run_test.pl
```

### RTPS Mode

```bash
perl run_test.pl --rtps
```

## Usage

### Basic Usage (RTPS Mode - Recommended)

RTPS mode is the simplest way to get started as it doesn't require a central InfoRepo server.

**Terminal 1: Start first participant**
```bash
mkdir /tmp/dirshare_a
echo "Hello from A" > /tmp/dirshare_a/fileA.txt
./dirshare -DCPSConfigFile rtps.ini /tmp/dirshare_a
```

**Terminal 2: Start second participant**
```bash
mkdir /tmp/dirshare_b
echo "Hello from B" > /tmp/dirshare_b/fileB.txt
./dirshare -DCPSConfigFile rtps.ini /tmp/dirshare_b
```

**Result**: Both directories will contain both files within a few seconds.

### InfoRepo Mode

**Terminal 1: Start DCPSInfoRepo**
```bash
$DDS_ROOT/bin/DCPSInfoRepo -o repo.ior
```

**Terminal 2: Start first participant**
```bash
./dirshare -DCPSInfoRepo file://repo.ior /tmp/dirshare_a
```

**Terminal 3: Start second participant**
```bash
./dirshare -DCPSInfoRepo file://repo.ior /tmp/dirshare_b
```

### RTPS Mode (No Central Server)

**Terminal 1: First participant**
```bash
./dirshare -DCPSConfigFile rtps.ini /tmp/dirshare_a
```

**Terminal 2: Second participant**
```bash
./dirshare -DCPSConfigFile rtps.ini /tmp/dirshare_b
```

## Command-Line Options

```
Usage: dirshare [OPTIONS] <shared_directory>

Arguments:
  shared_directory      Path to directory to synchronize

OpenDDS Options:
  -DCPSInfoRepo <ior>   InfoRepo IOR (file://path or corbaloc://...)
  -DCPSConfigFile <ini> Configuration file (e.g., rtps.ini for RTPS mode)
  -d <domain_id>        DDS domain ID (default: 42)
  -ORBDebugLevel <n>    ORB debug level (0-10)

DirShare Options:
  -v, --verbose         Enable verbose logging
  -h, --help            Show this help message

Examples:
  # InfoRepo mode
  dirshare -DCPSInfoRepo file://repo.ior /tmp/myshare

  # RTPS mode
  dirshare -DCPSConfigFile rtps.ini /tmp/myshare

  # Custom domain
  dirshare -DCPSConfigFile rtps.ini -d 50 /tmp/myshare
```

## Testing Real-Time Synchronization

### File Creation
```bash
# With DirShare running in /tmp/dirshare_a and /tmp/dirshare_b
echo "New file" > /tmp/dirshare_a/newfile.txt

# Check propagation (should appear within 5 seconds)
ls -l /tmp/dirshare_b/newfile.txt
```

### File Modification
```bash
echo "Updated content" >> /tmp/dirshare_a/fileA.txt

# Verify propagation
tail /tmp/dirshare_b/fileA.txt
```

### File Deletion
```bash
rm /tmp/dirshare_a/fileA.txt

# Verify propagation (should disappear within 5 seconds)
ls /tmp/dirshare_b/fileA.txt  # Should not exist
```

### Large File Transfer
```bash
# Create 50MB test file
dd if=/dev/urandom of=/tmp/dirshare_a/large.bin bs=1M count=50

# Monitor transfer (should complete within 30 seconds)
watch ls -lh /tmp/dirshare_b/large.bin
```

## Architecture

### Data Types (IDL)

- **FileMetadata**: File properties (name, size, timestamp, checksum)
- **FileEvent**: File operation notifications (CREATE/MODIFY/DELETE)
- **FileContent**: Small file content (<10MB)
- **FileChunk**: Large file chunks (1MB chunks for files >=10MB)
- **DirectorySnapshot**: Initial directory state for synchronization

### DDS Topics

- `DirShare_FileEvents`: File operation notifications
- `DirShare_FileContent`: Small file transfers
- `DirShare_FileChunks`: Large file chunked transfers
- `DirShare_DirectorySnapshot`: Initial directory snapshots

### Components

- **FileMonitor**: Polls directory for changes (1-2 second interval)
- **Checksum**: CRC32 integrity verification
- **FileUtils**: File I/O and timestamp preservation
- **Listeners**: DDS DataReader listeners for receiving remote changes

## Limitations

- **File Size**: Maximum 1GB per file
- **Directory Depth**: Single directory level (no recursive subdirectories)
- **Propagation Latency**: Target 5 seconds for files up to 10MB
- **Symbolic Links**: Ignored (not synchronized)

## Troubleshooting

### Files Not Synchronizing

1. Enable debug logging:
   ```bash
   export DCPS_debug_level=4
   ./dirshare -DCPSConfigFile rtps.ini /tmp/dirshare_a
   ```

2. Check for "publication matched" and "subscription matched" messages

3. Verify directory permissions:
   ```bash
   ls -ld /tmp/dirshare_a /tmp/dirshare_b
   ```

### Checksum Errors

- Ensure file is not being modified during transfer
- Check network quality (packet loss)
- Retry transfer (delete and recreate file)

### High Memory Usage

- Limit concurrent transfers of large files
- Restart DirShare if memory leak suspected

## Performance Expectations (LAN, 1 Gbps)

| Operation | File Size | Expected Time |
|-----------|-----------|---------------|
| Create/Propagate | 1 MB | < 1 second |
| Create/Propagate | 10 MB | < 5 seconds |
| Create/Propagate | 100 MB | < 30 seconds |
| Modify/Propagate | 1 MB | < 2 seconds |
| Delete/Propagate | Any | < 1 second |
| Initial Sync | 100 files (1 MB each) | < 30 seconds |

## Further Reading

- **OpenDDS Developer's Guide**: `$DDS_ROOT/docs/OpenDDS_Developer_Guide.pdf`
- **Design Documents**: `/specs/001-dirshare/` in repository
- **OpenDDS Website**: https://opendds.org/
- **DDS Specification**: https://www.omg.org/spec/DDS/

## License

This example is part of OpenDDS and follows the OpenDDS license.
