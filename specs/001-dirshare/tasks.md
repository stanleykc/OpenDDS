# Tasks: DirShare - Distributed File Synchronization

**Input**: Design documents from `/specs/001-dirshare/`
**Prerequisites**: plan.md ✅, spec.md ✅, research.md ✅, data-model.md ✅, contracts/topics.md ✅

**Tests**: NOT EXPLICITLY REQUESTED in spec.md. Basic test automation via run_test.pl is included per OpenDDS constitution.

**Organization**: Tasks are grouped by user story to enable independent implementation and testing of each story.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: Which user story this task belongs to (e.g., US1, US2, US3)
- Include exact file paths in descriptions

## Path Conventions

- **OpenDDS Example Structure**: `DevGuideExamples/DCPS/DirShare/` at repository root
- Test scripts: `run_test.pl` in example directory
- Configuration: `rtps.ini` for RTPS discovery mode

---

## Phase 1: Setup (Shared Infrastructure) ✅ COMPLETE

**Purpose**: Create project directory structure and basic files per OpenDDS constitution

- [x] T001 Create directory `DevGuideExamples/DCPS/DirShare/` in repository root
- [x] T002 [P] Create IDL file `DevGuideExamples/DCPS/DirShare/DirShare.idl` with data structures from data-model.md
- [x] T003 [P] Create MPC project file `DevGuideExamples/DCPS/DirShare/DirShare.mpc` with three projects (idl, lib, exe)
- [x] T004 [P] Create CMake build file `DevGuideExamples/DCPS/DirShare/CMakeLists.txt` for alternative build support
- [x] T005 [P] Create RTPS configuration `DevGuideExamples/DCPS/DirShare/rtps.ini` per contracts/topics.md
- [x] T006 [P] Create README.md in `DevGuideExamples/DCPS/DirShare/README.md` with build and usage instructions

---

## Phase 2: Foundational (Blocking Prerequisites) ✅ COMPLETE

**Purpose**: Core DDS infrastructure and utility components that ALL user stories depend on

**⚠️ CRITICAL**: No user story work can begin until this phase is complete

- [x] T007 Create FileMonitor interface in `DevGuideExamples/DCPS/DirShare/FileMonitor.h` with polling-based directory scanning
- [x] T008 Implement FileMonitor in `DevGuideExamples/DCPS/DirShare/FileMonitor.cpp` with 1-2 second polling interval
- [x] T009 [P] Create utility functions for CRC32 checksum calculation in `DevGuideExamples/DCPS/DirShare/Checksum.h`
- [x] T010 [P] Implement CRC32 functions in `DevGuideExamples/DCPS/DirShare/Checksum.cpp` per research.md decisions
- [x] T011 [P] Create utility functions for file I/O operations in `DevGuideExamples/DCPS/DirShare/FileUtils.h`
- [x] T012 [P] Implement file I/O utilities in `DevGuideExamples/DCPS/DirShare/FileUtils.cpp` (read, write, timestamp preservation)
- [x] T013 Create main application skeleton in `DevGuideExamples/DCPS/DirShare/DirShare.cpp` with DDS initialization
- [x] T014 Add command-line argument parsing to `DevGuideExamples/DCPS/DirShare/DirShare.cpp` (shared_directory, DDS options)
- [x] T015 Add DomainParticipant creation with TheParticipantFactoryWithArgs in `DirShare.cpp`
- [x] T016 [P] Register FileEvent TypeSupport in `DirShare.cpp`
- [x] T017 [P] Register FileContent TypeSupport in `DirShare.cpp`
- [x] T018 [P] Register FileChunk TypeSupport in `DirShare.cpp`
- [x] T019 [P] Register DirectorySnapshot TypeSupport in `DirShare.cpp`
- [x] T020 [P] Create FileEvents topic in `DirShare.cpp` with QoS from contracts/topics.md
- [x] T021 [P] Create FileContent topic in `DirShare.cpp` with QoS from contracts/topics.md
- [x] T022 [P] Create FileChunks topic in `DirShare.cpp` with QoS from contracts/topics.md
- [x] T023 [P] Create DirectorySnapshot topic in `DirShare.cpp` with QoS from contracts/topics.md
- [x] T024 Create Publisher entity in `DirShare.cpp` for publishing file changes
- [x] T025 Create Subscriber entity in `DirShare.cpp` for receiving remote changes
- [x] T026 Add WaitSet and StatusCondition setup for publication/subscription matching in `DirShare.cpp`
- [x] T027 Add proper DDS cleanup sequence in `DirShare.cpp` (delete_contained_entities, delete_participant, shutdown)

**Checkpoint**: Foundation ready - DDS infrastructure initialized, topics created, user story implementation can now begin

### Phase 2 Testing ✅ COMPLETE

**Purpose**: Validate Phase 2 components in isolation before DDS integration

- [x] T027a Create unit test for Checksum utilities (`tests/ChecksumTest.cpp`)
- [x] T027b Create unit test for FileUtils utilities (`tests/FileUtilsTest.cpp`)
- [x] T027c Create unit test for FileMonitor (`tests/FileMonitorTest.cpp`)
- [x] T027d Create MPC project for tests (`tests/tests.mpc`)
- [x] T027e Create test runner script (`tests/run_tests.pl`)
- [x] T027f Update DirShare.mpc to create shared library for tests
- [x] T027g Document test framework and usage (`tests/README.md`)

**Test Coverage**:
- Checksum: 6 tests (empty data, known values, incremental, file-based, error handling)
- FileUtils: 11 tests (read/write, file operations, validation, timestamps)
- FileMonitor: 7 tests (change detection, metadata, error handling)

**How to run tests**:
```bash
cd DevGuideExamples/DCPS/DirShare/tests
mwc.pl -type gnuace tests.mpc && make
./run_tests.pl
```

---

## Phase 3: User Story 1 - Initial Directory Synchronization (Priority: P1) 🎯 MVP

**Goal**: Enable two DirShare instances to synchronize existing files when establishing a sharing session

**Independent Test**: Start two DirShare instances in directories with different pre-existing files, verify both directories converge to contain all files from both sources

### Implementation for User Story 1

- [ ] T028 [P] [US1] Create DirectorySnapshot DataWriter in `DirShare.cpp` with QoS policies
- [ ] T029 [P] [US1] Create DirectorySnapshot DataReader with listener in `DirShare.cpp`
- [ ] T030 [US1] Implement DirectorySnapshot listener interface in `DevGuideExamples/DCPS/DirShare/SnapshotListenerImpl.h`
- [ ] T031 [US1] Implement DirectorySnapshot listener on_data_available in `DevGuideExamples/DCPS/DirShare/SnapshotListenerImpl.cpp`
- [ ] T032 [US1] Add directory scanning function to FileMonitor to generate list of FileMetadata for all local files
- [ ] T033 [US1] Implement publish DirectorySnapshot on startup in `DirShare.cpp` (scan local directory, publish to topic)
- [ ] T034 [US1] Implement snapshot comparison logic in SnapshotListenerImpl to detect missing files
- [ ] T035 [US1] Add file request handling when missing files detected (trigger file transfer)
- [ ] T036 [P] [US1] Create FileContent DataWriter in `DirShare.cpp` for small file transfers
- [ ] T037 [P] [US1] Create FileContent DataReader with listener in `DirShare.cpp`
- [ ] T038 [US1] Implement FileContent listener interface in `DevGuideExamples/DCPS/DirShare/FileContentListenerImpl.h`
- [ ] T039 [US1] Implement FileContent listener on_data_available in `DevGuideExamples/DCPS/DirShare/FileContentListenerImpl.cpp` with checksum verification
- [ ] T040 [US1] Add small file publishing logic (<10MB threshold) to DirShare.cpp when requested
- [ ] T041 [US1] Add timestamp preservation when writing received files in FileContentListenerImpl
- [ ] T042 [P] [US1] Create FileChunk DataWriter in `DirShare.cpp` for large file transfers
- [ ] T043 [P] [US1] Create FileChunk DataReader with listener in `DirShare.cpp`
- [ ] T044 [US1] Implement FileChunk listener interface in `DevGuideExamples/DCPS/DirShare/FileChunkListenerImpl.h`
- [ ] T045 [US1] Implement chunk reassembly logic in `DevGuideExamples/DCPS/DirShare/FileChunkListenerImpl.cpp` with buffer management
- [ ] T046 [US1] Add chunk publishing logic (1MB chunks) to DirShare.cpp for files >=10MB
- [ ] T047 [US1] Add file checksum verification after reassembly in FileChunkListenerImpl
- [ ] T048 [US1] Add error handling for checksum mismatches in all listeners
- [ ] T049 [US1] Add ACE logging for initial synchronization events (snapshot sent/received, files transferred)

**Checkpoint**: Initial directory synchronization complete - two instances synchronize existing files on startup

---

## Phase 4: User Story 2 - Real-Time File Creation Propagation (Priority: P1)

**Goal**: Automatically propagate newly created files to all participants during active session

**Independent Test**: With active session, create new file in one directory, verify it appears in other directory within 5 seconds

### Implementation for User Story 2

- [ ] T050 [P] [US2] Create FileEvent DataWriter in `DirShare.cpp` with QoS policies
- [ ] T051 [P] [US2] Create FileEvent DataReader with listener in `DirShare.cpp`
- [ ] T052 [US2] Implement FileEvent listener interface in `DevGuideExamples/DCPS/DirShare/FileEventListenerImpl.h`
- [ ] T053 [US2] Implement FileEvent listener on_data_available in `DevGuideExamples/DCPS/DirShare/FileEventListenerImpl.cpp`
- [ ] T054 [US2] Add file creation detection to FileMonitor.cpp (compare current scan with previous scan state)
- [ ] T055 [US2] Implement publish FileEvent(CREATE) when new file detected in FileMonitor
- [ ] T056 [US2] Add FileEvent CREATE handling in FileEventListenerImpl (trigger file content transfer)
- [ ] T057 [US2] Connect FileEvent CREATE to FileContent/FileChunk request logic
- [ ] T058 [US2] Add validation that file doesn't already exist locally before writing in FileEventListenerImpl
- [ ] T059 [US2] Add ACE logging for file creation events (detected, published, received, applied)

**Checkpoint**: Real-time file creation working - new files propagate automatically within 5 seconds

---

## Phase 5: User Story 3 - Real-Time File Modification Propagation (Priority: P1)

**Goal**: Propagate file modifications efficiently (only changed files transmitted) to all participants

**Independent Test**: Modify existing file, verify only that file is retransmitted and change propagates within 5 seconds

### Implementation for User Story 3

- [ ] T060 [US3] Add file modification detection to FileMonitor.cpp (compare file size, timestamp, or checksum)
- [ ] T061 [US3] Implement publish FileEvent(MODIFY) when file change detected in FileMonitor
- [ ] T062 [US3] Add FileEvent MODIFY handling in FileEventListenerImpl (check timestamp for conflict resolution)
- [ ] T063 [US3] Implement timestamp comparison logic using DDS source_timestamp in FileEventListenerImpl
- [ ] T064 [US3] Add logic to overwrite local file only if remote timestamp is newer in FileEventListenerImpl
- [ ] T065 [US3] Add ACE logging for modification events with timestamp comparisons
- [ ] T066 [US3] Add instrumentation to verify only modified files are transferred (not all files)

**Checkpoint**: File modification propagation working - changes detected and transmitted efficiently with timestamp-based ordering

---

## Phase 6: User Story 4 - Real-Time File Deletion Propagation (Priority: P2)

**Goal**: Automatically delete files on all participants when deleted on one machine

**Independent Test**: Delete file on one machine, verify it's removed from other machines within 5 seconds

### Implementation for User Story 4

- [ ] T067 [US4] Add file deletion detection to FileMonitor.cpp (file present in previous scan, absent in current)
- [ ] T068 [US4] Implement publish FileEvent(DELETE) when file deletion detected in FileMonitor
- [ ] T069 [US4] Add FileEvent DELETE handling in FileEventListenerImpl (check timestamp, delete local file)
- [ ] T070 [US4] Add timestamp comparison for delete events (respect last-write-wins for delete vs modify conflicts)
- [ ] T071 [US4] Add error handling for deletion failures (file in use, permission denied) in FileEventListenerImpl
- [ ] T072 [US4] Add ACE logging for deletion events (detected, published, received, applied)

**Checkpoint**: File deletion propagation working - deletions propagate automatically with conflict resolution

---

## Phase 7: User Story 5 - Concurrent Modification Conflict Resolution (Priority: P2)

**Goal**: Resolve conflicts when same file modified on multiple machines simultaneously using last-write-wins

**Independent Test**: Modify same file on two machines at nearly same time, verify all participants converge to version with latest timestamp

### Implementation for User Story 5

- [ ] T073 [US5] Refine timestamp comparison logic in FileEventListenerImpl to handle millisecond precision
- [ ] T074 [US5] Add tie-breaker logic for identical timestamps (use participant GUID or checksum) in FileEventListenerImpl
- [ ] T075 [US5] Add test scenario for concurrent modifications in `run_test.pl` (modify same file simultaneously)
- [ ] T076 [US5] Add ACE logging for conflict resolution decisions (which version won, why)
- [ ] T077 [US5] Add verification that all participants converge to same final state after conflict

**Checkpoint**: Conflict resolution working - simultaneous modifications resolved deterministically

---

## Phase 8: User Story 6 - Metadata Transfer and Preservation (Priority: P3)

**Goal**: Transfer and preserve file metadata (timestamps, size) along with file content

**Independent Test**: Create file with specific metadata, transfer via DirShare, verify metadata preserved on receiving side

### Implementation for User Story 6

- [ ] T078 [P] [US6] Verify FileMetadata includes size, timestamp in all FileEvent publications
- [ ] T079 [P] [US6] Verify FileContent includes size, timestamp, checksum in publications
- [ ] T080 [P] [US6] Verify FileChunk includes file_size, timestamp, checksums in publications
- [ ] T081 [US6] Add timestamp preservation logic to FileContentListenerImpl using ACE or filesystem API
- [ ] T082 [US6] Add timestamp preservation logic to FileChunkListenerImpl after reassembly
- [ ] T083 [US6] Add metadata validation in all listeners (size matches actual data, timestamps reasonable)
- [ ] T084 [US6] Add ACE logging for metadata preservation (original vs preserved timestamps)

**Checkpoint**: Metadata preservation working - timestamps and file properties maintained across transfers

---

## Phase 9: Polish & Cross-Cutting Concerns

**Purpose**: Testing, documentation, and OpenDDS constitution compliance

- [ ] T085 [P] Create test script `DevGuideExamples/DCPS/DirShare/run_test.pl` using PerlDDS::TestFramework
- [ ] T086 [P] Add InfoRepo mode test to run_test.pl (default mode with DCPSInfoRepo)
- [ ] T087 [P] Add RTPS mode test to run_test.pl (--rtps flag, uses rtps.ini)
- [ ] T088 [P] Add test scenario for basic file sync (2 participants, create file, verify propagation)
- [ ] T089 [P] Add test scenario for large file transfer (>=10MB, verify chunking and reassembly)
- [ ] T090 [P] Add test scenario for file deletion (create, sync, delete, verify deletion propagates)
- [ ] T091 Add error handling for missing directory argument in DirShare.cpp
- [ ] T092 Add validation that specified directory exists and is writable in DirShare.cpp
- [ ] T093 Add help message (-h, --help) to DirShare.cpp showing usage and options
- [ ] T094 Add static build support with conditional OPENDDS_DO_MANUAL_STATIC_INCLUDES in DirShare.cpp
- [ ] T095 [P] Update README.md with build instructions (MPC and CMake)
- [ ] T096 [P] Update README.md with usage examples (InfoRepo and RTPS modes)
- [ ] T097 [P] Add troubleshooting section to README.md
- [ ] T098 Code review for ACE error handling patterns (ACE_ERROR_RETURN, ACE_DEBUG)
- [ ] T099 Code review for proper DDS return code checking (all operations check RETCODE_OK)
- [ ] T100 Verify both MPC and CMake builds compile successfully
- [ ] T101 Run run_test.pl in InfoRepo mode and verify PASS
- [ ] T102 Run run_test.pl in RTPS mode (--rtps) and verify PASS
- [ ] T103 [P] Manual test: Run quickstart.md validation scenarios
- [ ] T104 Memory leak check with valgrind or similar tool (optional)

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: No dependencies - can start immediately
- **Foundational (Phase 2)**: Depends on Phase 1 completion - BLOCKS all user stories
- **User Stories (Phases 3-8)**: All depend on Phase 2 (Foundational) completion
  - US1 (Phase 3): Initial Sync - Must complete first (foundation for other stories)
  - US2 (Phase 4): File Creation - Depends on US1 (needs initial sync working)
  - US3 (Phase 5): File Modification - Depends on US1 (needs file transfer infrastructure)
  - US4 (Phase 6): File Deletion - Depends on US1 and US3 (needs event handling)
  - US5 (Phase 7): Conflict Resolution - Depends on US3 (needs modification handling)
  - US6 (Phase 8): Metadata Preservation - Enhances US1-US4 (can be integrated throughout)
- **Polish (Phase 9)**: Depends on all user stories being complete

### User Story Dependencies

- **User Story 1 (P1)**: Must complete first - provides core file transfer infrastructure
- **User Story 2 (P1)**: Depends on US1 - reuses file transfer mechanisms
- **User Story 3 (P1)**: Depends on US1 - reuses file transfer + adds modification detection
- **User Story 4 (P2)**: Depends on US1 and US3 - reuses event system
- **User Story 5 (P2)**: Depends on US3 - enhances modification handling
- **User Story 6 (P3)**: Can be integrated throughout US1-US5 - cross-cutting concern

### Within Each User Story

**User Story 1 (Initial Sync)**:
1. Snapshot publisher/subscriber setup (T028-T031) - parallel
2. Directory scanning (T032-T035) - sequential
3. FileContent publisher/subscriber (T036-T041) - parallel within pairs
4. FileChunk publisher/subscriber (T042-T049) - parallel within pairs

**User Story 2 (File Creation)**:
1. FileEvent publisher/subscriber setup (T050-T053) - parallel
2. Creation detection and publishing (T054-T055) - sequential
3. Event handling (T056-T059) - sequential

**User Story 3 (File Modification)**:
1. Modification detection (T060-T061) - sequential
2. Event handling with timestamp logic (T062-T066) - sequential

**User Story 4 (File Deletion)**:
1. Deletion detection (T067-T068) - sequential
2. Event handling (T069-T072) - sequential

**User Story 5 (Conflict Resolution)**:
1. Enhanced timestamp logic (T073-T074) - sequential
2. Testing and validation (T075-T077) - sequential

**User Story 6 (Metadata Preservation)**:
1. Metadata validation (T078-T083) - parallel where possible
2. Logging (T084) - sequential

**Polish Phase**:
- All test script tasks (T085-T090) - parallel
- Error handling tasks (T091-T094) - parallel
- Documentation tasks (T095-T097) - parallel
- Validation tasks (T098-T104) - sequential

### Parallel Opportunities

**Within Phase 1 (Setup)**:
- T002, T003, T004, T005, T006 can all run in parallel (different files)

**Within Phase 2 (Foundational)**:
- T009-T010 (Checksum) parallel with T011-T012 (FileUtils)
- T016-T019 (TypeSupport registration) - all parallel
- T020-T023 (Topic creation) - all parallel

**Within User Stories**:
- DataWriter and DataReader creation tasks within same story
- Multiple listener implementation files (header vs cpp)
- Different utility functions

**Cross-Story Parallelization** (if team capacity):
- After US1 completes, US2 and US4 could potentially be worked on in parallel
- US6 (metadata) can be integrated while working on other stories

---

## Parallel Example: Phase 2 (Foundational)

```bash
# Launch utility implementations together:
Task: "Create utility functions for CRC32 checksum calculation in Checksum.h"
Task: "Implement CRC32 functions in Checksum.cpp"
Task: "Create utility functions for file I/O operations in FileUtils.h"
Task: "Implement file I/O utilities in FileUtils.cpp"

# Launch TypeSupport registration together:
Task: "Register FileEvent TypeSupport in DirShare.cpp"
Task: "Register FileContent TypeSupport in DirShare.cpp"
Task: "Register FileChunk TypeSupport in DirShare.cpp"
Task: "Register DirectorySnapshot TypeSupport in DirShare.cpp"

# Launch Topic creation together:
Task: "Create FileEvents topic in DirShare.cpp"
Task: "Create FileContent topic in DirShare.cpp"
Task: "Create FileChunks topic in DirShare.cpp"
Task: "Create DirectorySnapshot topic in DirShare.cpp"
```

---

## Parallel Example: User Story 1

```bash
# Launch snapshot publisher/subscriber together:
Task: "Create DirectorySnapshot DataWriter in DirShare.cpp"
Task: "Create DirectorySnapshot DataReader with listener in DirShare.cpp"

# Launch FileContent publisher/subscriber together:
Task: "Create FileContent DataWriter in DirShare.cpp"
Task: "Create FileContent DataReader with listener in DirShare.cpp"

# Launch FileChunk publisher/subscriber together:
Task: "Create FileChunk DataWriter in DirShare.cpp"
Task: "Create FileChunk DataReader with listener in DirShare.cpp"
```

---

## Implementation Strategy

### MVP First (User Stories 1-3 Only)

**Why US1-3 as MVP**: These three P1 stories provide complete basic file synchronization (initial sync + create + modify). This is the minimum viable product for DirShare.

1. Complete Phase 1: Setup (T001-T006)
2. Complete Phase 2: Foundational (T007-T027) ⚠️ CRITICAL BLOCKING PHASE
3. Complete Phase 3: User Story 1 - Initial Directory Sync (T028-T049)
4. **STOP and VALIDATE**: Test initial sync between two instances
5. Complete Phase 4: User Story 2 - File Creation (T050-T059)
6. **STOP and VALIDATE**: Test file creation propagation
7. Complete Phase 5: User Story 3 - File Modification (T060-T066)
8. **STOP and VALIDATE**: Test modification propagation with conflict detection
9. Minimal Phase 9 tasks: T085-T102 (testing)
10. **MVP READY**: DirShare demonstrates core OpenDDS pub/sub for file synchronization

### Incremental Delivery

1. **Foundation (Phases 1-2)**: DDS infrastructure ready
2. **MVP (Phases 3-5)**: Initial sync + create + modify → Deploy/Demo
3. **Enhanced (Phase 6)**: Add file deletion → Deploy/Demo
4. **Robust (Phase 7)**: Improve conflict resolution → Deploy/Demo
5. **Complete (Phase 8)**: Full metadata preservation → Deploy/Demo
6. **Production-Ready (Phase 9)**: Polish, testing, documentation → Final Release

### Sequential Development (Recommended for OpenDDS Example)

Given the interdependencies in DirShare:

1. Phase 1: Setup → Foundation for everything
2. Phase 2: Foundational → MUST complete before ANY user stories
3. Phase 3: US1 (Initial Sync) → MUST complete before other stories (provides file transfer)
4. Phase 4: US2 (File Creation) → Builds on US1
5. Phase 5: US3 (File Modification) → Builds on US1
6. Phase 6: US4 (File Deletion) → Builds on US1 and US3
7. Phase 7: US5 (Conflict Resolution) → Enhances US3
8. Phase 8: US6 (Metadata) → Enhances all previous stories
9. Phase 9: Polish → Final validation

**Rationale**: Unlike web applications where user stories might be independent, DirShare's user stories build on shared DDS infrastructure, making sequential development more natural.

---

## Task Summary

**Total Tasks**: 104
**Tasks by Phase**:
- Phase 1 (Setup): 6 tasks
- Phase 2 (Foundational): 21 tasks (BLOCKING)
- Phase 3 (US1 - Initial Sync): 22 tasks
- Phase 4 (US2 - File Creation): 10 tasks
- Phase 5 (US3 - File Modification): 7 tasks
- Phase 6 (US4 - File Deletion): 6 tasks
- Phase 7 (US5 - Conflict Resolution): 5 tasks
- Phase 8 (US6 - Metadata Preservation): 7 tasks
- Phase 9 (Polish): 20 tasks

**Tasks by User Story**:
- US1 (P1): 22 tasks (Initial Directory Synchronization)
- US2 (P1): 10 tasks (File Creation Propagation)
- US3 (P1): 7 tasks (File Modification Propagation)
- US4 (P2): 6 tasks (File Deletion Propagation)
- US5 (P2): 5 tasks (Conflict Resolution)
- US6 (P3): 7 tasks (Metadata Preservation)

**Parallel Opportunities Identified**:
- Phase 1: 5 tasks can run in parallel
- Phase 2: 12 tasks can run in parallel (after initial setup)
- Each user story: 4-8 tasks can run in parallel (DataWriter/DataReader pairs, utilities)
- Phase 9: 10 tasks can run in parallel (documentation, tests)

**Suggested MVP Scope**:
- Phases 1-5 (Setup + Foundational + US1 + US2 + US3)
- Total MVP tasks: 66 out of 104 (63%)
- Provides: Initial sync, file creation, file modification with conflict detection
- OpenDDS constitution compliant: IDL-first, dual discovery, complete lifecycle

---

## Notes

- All tasks follow OpenDDS constitution principles (IDL-first, dual discovery, proper lifecycle)
- [P] tasks = different files, no dependencies within phase
- [Story] labels map tasks to user stories for traceability
- File paths are absolute from repository root
- Verify DDS return codes (RETCODE_OK) for all operations
- Use ACE logging macros (ACE_ERROR, ACE_DEBUG) consistently
- Test both InfoRepo and RTPS modes per constitution requirement
- Commit after each logical task group (e.g., listener implementation, topic creation)
- Stop at checkpoints to validate independently before proceeding
