# Implementation Plan: DirShare - Distributed File Synchronization

**Branch**: `001-dirshare` | **Date**: 2025-10-30 | **Spec**: [spec.md](./spec.md)
**Input**: Feature specification from `/specs/001-dirshare/spec.md`
**User Context**: Create a command line application for each participant that does the directory sync

**Note**: This template is filled in by the `/speckit.plan` command. See `.specify/templates/commands/plan.md` for the execution workflow.

## Summary

DirShare is a command-line OpenDDS example demonstrating real-time file synchronization between multiple DDS participants. Each participant monitors a local directory and uses DDS publish-subscribe to propagate file changes (create, modify, delete) to other participants. The system uses IDL-defined data types for file metadata and content, supports both InfoRepo and RTPS discovery mechanisms, and implements last-write-wins conflict resolution based on timestamps.

## Technical Context

**Language/Version**: C++ (C++11 or later per OpenDDS requirements)
**Primary Dependencies**: OpenDDS, ACE/TAO (auto-configured via OpenDDS configure script)
**Storage**: Local file system (no database required)
**Testing**: Perl test framework (PerlDDS::TestFramework) with run_test.pl launcher
**Target Platform**: Cross-platform (Linux, macOS, Windows) - OpenDDS supported platforms
**Project Type**: Single command-line application (dirshare executable acts as both publisher and subscriber)
**Performance Goals**: File propagation within 5 seconds for files up to 10MB, support 10+ concurrent participants
**Constraints**: Files under 1GB, single directory level (no recursive subdirectories), 5-second propagation latency
**Scale/Scope**: Handle 1000+ file operations per session, support up to 100 initial files sync within 30 seconds

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

**Constitution Version**: 1.0.0 (ratified 2025-10-30)

### ✅ I. IDL-First Design - COMPLIANT

**Plan**:
- Create `DirShare.idl` defining:
  - `FileMetadata` struct with filename, size, timestamp, checksum
  - `FileContent` struct with binary payload, chunk info
  - `FileEvent` struct with operation type (CREATE/MODIFY/DELETE), path, timestamp
  - Mark appropriate structs with `@topic` annotation
  - Mark filename/path as `@key` for proper DDS keying

**Compliance**: IDL-first approach ensures type safety and leverages OpenDDS code generation.

### ✅ II. Dual Discovery/Transport Support - COMPLIANT (NON-NEGOTIABLE)

**Plan**:
- Default configuration: InfoRepo discovery + TCP transport
- RTPS configuration: RTPS discovery + RTPS/UDP transport via `rtps.ini`
- `run_test.pl` supports `--rtps` flag to switch configurations
- Static builds include conditional headers:
  ```cpp
  #if OPENDDS_DO_MANUAL_STATIC_INCLUDES
  #  include <dds/DCPS/RTPS/RtpsDiscovery.h>
  #  include <dds/DCPS/transport/rtps_udp/RtpsUdp.h>
  #endif
  ```

**Compliance**: Full support for both discovery/transport modes per constitution.

### ✅ III. Complete DDS Lifecycle - COMPLIANT

**Plan**:
- Initialize with `TheParticipantFactoryWithArgs(argc, argv)`
- Create DomainParticipant with domain ID (default 42)
- Register TypeSupport for FileMetadata, FileContent, FileEvent
- Create Topics for each data type
- Create both Publisher and Subscriber (hybrid pub/sub application)
- Use DataReaderListener for receiving file events
- Use WaitSet/StatusCondition for publication/subscription matching
- Cleanup: `delete_contained_entities()` → `delete_participant()` → `shutdown()`
- All DDS return codes checked with ACE_ERROR macros

**Compliance**: Demonstrates complete lifecycle with proper synchronization and cleanup.

### ✅ IV. Test-Driven Validation - COMPLIANT

**Plan**:
- Provide `run_test.pl` using PerlDDS::Run_Test framework
- Test launches 2+ dirshare instances with separate directories
- Validates file synchronization in both InfoRepo and RTPS modes
- Uses `--rtps` flag for RTPS testing
- Timeout: 120 seconds
- Debug levels configured for troubleshooting
- Returns 0 on success, non-zero on failure

**Compliance**: Automated testing validates both discovery modes.

### ✅ V. Standard Project Structure - COMPLIANT

**Plan**:
```
DevGuideExamples/DCPS/DirShare/
├── DirShare.idl              # Data type definitions
├── DirShare.mpc              # MPC build (3 projects: idl, lib, exe)
├── CMakeLists.txt            # CMake alternative build
├── DirShare.cpp              # Main application (publisher + subscriber)
├── FileMonitor.h/cpp         # File system monitoring logic
├── DataReaderListenerImpl.h/cpp  # DDS listener for file events
├── run_test.pl               # Perl test launcher
├── rtps.ini                  # RTPS configuration
└── README.md                 # Documentation
```

**Compliance**: Standard structure with both build systems and test infrastructure.

### Summary

**All Constitution principles: ✅ COMPLIANT**

No violations detected. The DirShare example follows all OpenDDS example standards including IDL-first design, dual discovery/transport support, complete DDS lifecycle patterns, automated testing, and standard project structure.

## Project Structure

### Documentation (this feature)

```text
specs/001-dirshare/
├── spec.md              # Feature specification
├── plan.md              # This file (/speckit.plan command output)
├── research.md          # Phase 0 output (technical research)
├── data-model.md        # Phase 1 output (IDL data structures)
├── quickstart.md        # Phase 1 output (build and run instructions)
├── contracts/           # Phase 1 output (DDS topic contracts)
│   └── topics.md        # Topic definitions and QoS policies
└── checklists/
    └── requirements.md  # Specification quality checklist
```

### Source Code (repository root)

```text
DevGuideExamples/DCPS/DirShare/
├── DirShare.idl                    # IDL data type definitions
├── DirShare.mpc                    # MPC build configuration (3 projects)
├── CMakeLists.txt                  # CMake build configuration
├── DirShare.cpp                    # Main application entry point
├── FileMonitor.h                   # File system change detection interface
├── FileMonitor.cpp                 # File system monitoring implementation
├── DataReaderListenerImpl.h       # DDS DataReader listener interface
├── DataReaderListenerImpl.cpp     # Listener implementation for file events
├── run_test.pl                     # Perl test launcher script
├── rtps.ini                        # RTPS discovery/transport configuration
└── README.md                       # Example documentation

tests/ (if comprehensive tests needed beyond run_test.pl)
└── DCPS/
    └── DirShare/
        ├── run_test.pl             # Symlink or actual test script
        └── test_scenarios/         # Additional test configurations
```

**Structure Decision**: Single command-line application structure. Each DirShare instance acts as both publisher (monitors local directory, publishes changes) and subscriber (receives changes, applies to local directory). This hybrid pub/sub pattern is common in peer-to-peer DDS applications. The application follows OpenDDS DevGuideExamples standard structure located under `DevGuideExamples/DCPS/DirShare/`.

## Complexity Tracking

**No violations detected** - Complexity Tracking table not required.

All design decisions align with OpenDDS constitution principles:
- Single executable handles both publishing and subscribing (standard DDS pattern)
- Standard file organization per constitution Section V
- No additional architectural patterns beyond DDS pub/sub
- Complexity justified by feature requirements (file sync requires file monitoring, conflict resolution, binary transfer)

## Phase 0: Research & Technical Decisions ✅ COMPLETE

**Status**: Completed 2025-10-30
**Output**: `research.md` - Technical research document

**Research Completed**:
1. ✅ File system monitoring: Polling-based approach (1-2 sec interval, cross-platform)
2. ✅ Binary data transfer: Single message <10MB, 1MB chunks >=10MB
3. ✅ Conflict resolution: DDS timestamp comparison (millisecond precision)
4. ✅ Message sizing: 1MB chunks, 16MB transport max
5. ✅ Integrity verification: CRC32 checksum

**Key Decisions**:
- Polling for simplicity and cross-platform compatibility
- Hybrid transfer strategy (single vs. chunked based on file size)
- Timestamp-based last-write-wins conflict resolution
- CRC32 for fast integrity verification

All technical unknowns resolved. Ready for design phase.

## Phase 1: Design Artifacts ✅ COMPLETE

**Status**: Completed 2025-10-30
**Artifacts Generated**:
1. ✅ `data-model.md` - IDL struct definitions (FileMetadata, FileEvent, FileContent, FileChunk, DirectorySnapshot)
2. ✅ `contracts/topics.md` - DDS topic contracts with QoS policies (4 topics defined)
3. ✅ `quickstart.md` - Build and usage instructions (MPC and CMake)

**Agent Context Update**: ✅ Updated `/Users/stanleyk/dev/OpenDDS/CLAUDE.md` with:
- C++ (C++11 or later) + OpenDDS, ACE/TAO
- Local file system storage
- Single command-line application pattern

**Design Highlights**:
- **IDL Data Model**: 5 struct types (FileMetadata, FileEvent, FileContent, FileChunk, DirectorySnapshot)
- **DDS Topics**: 4 topics with RELIABLE QoS and appropriate durability settings
- **Discovery Support**: Both InfoRepo and RTPS configurations documented
- **Testing**: Automated test scripts for both discovery modes

## Constitution Re-Check ✅ PASS

Re-verification after Phase 1 design completion:

### ✅ I. IDL-First Design - COMPLIANT
- IDL definitions in `data-model.md` with proper `@topic` and `@key` annotations
- TypeSupport generation via build system

### ✅ II. Dual Discovery/Transport Support - COMPLIANT
- InfoRepo + TCP and RTPS + UDP configurations documented
- `rtps.ini` configuration file specified
- Test script supports `--rtps` flag

### ✅ III. Complete DDS Lifecycle - COMPLIANT
- Full lifecycle documented in design artifacts
- Proper initialization, synchronization, cleanup patterns specified

### ✅ IV. Test-Driven Validation - COMPLIANT
- Test scenarios documented in `contracts/topics.md`
- `run_test.pl` launcher script planned
- Both discovery modes tested

### ✅ V. Standard Project Structure - COMPLIANT
- Standard DevGuideExamples structure defined
- Both MPC and CMake build systems planned
- README, IDL, run_test.pl, rtps.ini all specified

**All Constitution principles remain compliant after design.**

## Phase 2: Task Generation (Next Phase)

**Status**: Not started - requires `/speckit.tasks` command
**Output**: `tasks.md` - Implementation task breakdown

This phase is executed by the `/speckit.tasks` command, which will:
1. Generate dependency-ordered implementation tasks
2. Break down each requirement into actionable steps
3. Create test validation tasks
4. Organize tasks by priority and dependency

**Ready for `/speckit.tasks` command** ✅
