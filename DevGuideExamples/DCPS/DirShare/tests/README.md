# DirShare Phase 2 Component Unit Tests

This directory contains unit tests for the foundational components of DirShare developed in Phase 2.

## Test Coverage

### ChecksumTest.cpp
Tests the CRC32 checksum utilities (`Checksum.h/cpp`):
- Empty data CRC32 calculation
- Known value verification (standard test vector)
- Incremental CRC32 calculation
- File-based CRC32 calculation
- Error handling for nonexistent files
- Consistency and determinism

### FileUtilsTest.cpp
Tests the file I/O utilities (`FileUtils.h/cpp`):
- File read and write operations
- File existence checking
- File size retrieval
- File deletion
- Directory detection
- Directory file listing
- Filename validation (security)
  - Safe filenames
  - Path traversal rejection
  - Absolute path rejection
  - Subdirectory rejection
- File modification time get/set

### FileMonitorTest.cpp
Tests the file monitoring component (`FileMonitor.h/cpp`):
- File creation detection
- File modification detection
- File deletion detection
- Multiple simultaneous changes
- Get all files in directory
- Get file metadata (FileMetadata struct)
- Error handling for nonexistent directories

## Building Tests

### Using MPC (Make Project Creator)

From the repository root (after running `./configure` and `source setenv.sh`):

```bash
cd DevGuideExamples/DCPS/DirShare/tests
mwc.pl -type gnuace tests.mpc
make
```

Or from the tests directory if gnuace makefiles already generated:

```bash
make
```

### Expected Build Artifacts

Three test executables will be created:
- `ChecksumTest`
- `FileUtilsTest`
- `FileMonitorTest`

## Running Tests

### Run all tests:

```bash
./run_tests.pl
```

### Run individual tests:

```bash
./ChecksumTest
./FileUtilsTest
./FileMonitorTest
```

## Test Output

Tests use a simple assertion-based framework with colored output:
- **Green ✓**: Test passed
- **Red ✗**: Test failed
- Each test reports pass/fail and detailed assertion failures

Example output:
```
╔══════════════════════════════════════════════╗
║   DirShare Phase 2 Component Unit Tests     ║
╔══════════════════════════════════════════════╗

=== Running ChecksumTest ===
Running test: crc32_empty_data
  PASS: crc32_empty_data
Running test: crc32_known_value
  PASS: crc32_known_value
...

=== Test Results ===
  Passed: 6
  Failed: 0

✓ ChecksumTest PASSED
```

## Test Philosophy

These unit tests validate the Phase 2 foundational components in isolation before they are integrated into the full DirShare application. This approach:

1. **Catches bugs early**: Issues in utilities are found before DDS integration
2. **Documents behavior**: Tests serve as executable specifications
3. **Enables refactoring**: Changes can be made with confidence
4. **Simplifies debugging**: Failures are isolated to specific components

## CI/CD Integration

The `run_tests.pl` script returns:
- Exit code **0**: All tests passed
- Exit code **1**: One or more tests failed

This makes it suitable for CI/CD pipelines:

```bash
./run_tests.pl && echo "Tests passed" || echo "Tests failed"
```

## Future Test Additions

As Phase 3+ development proceeds, additional tests should be added:
- **Integration tests**: Test DDS publishers/subscribers with DirShare logic
- **End-to-end tests**: Multi-process file synchronization scenarios
- **Performance tests**: Measure throughput and latency
- **Stress tests**: Large files, many files, rapid changes

## Dependencies

Tests depend on:
- **ACE/TAO**: For cross-platform abstractions and logging
- **OpenDDS**: For IDL-generated types (FileMetadata, etc.)
- **DirShare library**: The components being tested

Ensure environment is configured:
```bash
source ../../../setenv.sh  # From repository root
```

## Notes

- Tests create temporary files/directories in the current directory
- All temporary artifacts are cleaned up after tests complete
- Tests use simple macro-based assertions (not a full framework like GTest)
- Some tests require file system write permissions in the test directory
- FileMonitor tests use 1-second sleep to ensure timestamp changes are detectable
