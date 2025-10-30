# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Overview

OpenDDS is an open-source C++ implementation of the Object Management Group's Data Distribution Service (DDS) specification. It provides a publish-subscribe middleware framework for distributed real-time applications. OpenDDS is built on ACE/TAO for platform portability and uses IDL (Interface Definition Language) to define data types.

## Build System

OpenDDS uses multiple build systems:

### Configuration and Setup

1. **Initial Configuration**: Run `./configure` from the repository root
   - Downloads and configures ACE/TAO dependencies automatically
   - Generates platform-specific build files
   - Creates `setenv.sh` (or `setenv.cmd` on Windows) for environment setup

2. **Environment Setup**: Source the environment file before building or running
   ```bash
   source setenv.sh
   ```

### Build Methods

#### MPC (Make Project Creator) - Primary Build System
- MPC files (`.mpc`) define project structure and dependencies
- Generate platform-specific makefiles or Visual Studio projects
- Example: `DevGuideExamples/DCPS/Messenger/Messenger.mpc`

**Building with MPC:**
```bash
# From repository root, build all of OpenDDS
make

# Build specific example
cd DevGuideExamples/DCPS/Messenger
make
```

#### CMake - Alternative Build System
- Supported for examples and modern workflows
- Uses `find_package(OpenDDS REQUIRED)`
- Example: `DevGuideExamples/DCPS/Messenger/CMakeLists.txt`

**Building with CMake:**
```bash
cd DevGuideExamples/DCPS/Messenger
mkdir build && cd build
cmake ..
cmake --build .
```

## Testing

### Running Tests

Tests use Perl launcher scripts (`run_test.pl`):

```bash
# Run example with default InfoRepo discovery and TCP transport
perl run_test.pl

# Run with RTPS discovery and RTPS/UDP transport
perl run_test.pl --rtps

# Run specific test from tests directory
cd tests/DCPS/HelloWorld
perl run_test.pl
```

### Test Framework
- Tests are located in `tests/DCPS/`
- Each test has a `run_test.pl` launcher script
- PerlDDS::Run_Test framework manages processes and validates results
- Tests typically start subscriber first, then publisher

## DDS Application Architecture

### Core Components (Publisher-Subscriber Pattern)

All DDS applications follow this structure (see `DevGuideExamples/DCPS/Messenger`):

1. **IDL Definition** (`Messenger.idl`)
   - Define data structures using IDL
   - Mark structs with `@topic` annotation
   - Mark key fields with `@key` annotation

2. **DomainParticipant** - Entry point to DDS
   ```cpp
   DDS::DomainParticipantFactory_var dpf = TheParticipantFactoryWithArgs(argc, argv);
   DDS::DomainParticipant_var participant = dpf->create_participant(domain_id, ...);
   ```

3. **TypeSupport** - Register IDL-generated types
   ```cpp
   Messenger::MessageTypeSupport_var ts = new Messenger::MessageTypeSupportImpl;
   ts->register_type(participant, "");
   ```

4. **Topic** - Named data channel
   ```cpp
   DDS::Topic_var topic = participant->create_topic("Topic Name", type_name, ...);
   ```

5. **Publisher/Subscriber** - Manages DataWriters/DataReaders
   ```cpp
   DDS::Publisher_var publisher = participant->create_publisher(...);
   DDS::Subscriber_var subscriber = participant->create_subscriber(...);
   ```

6. **DataWriter** (Publisher side)
   ```cpp
   DDS::DataWriter_var writer = publisher->create_datawriter(topic, ...);
   Messenger::MessageDataWriter_var typed_writer =
       Messenger::MessageDataWriter::_narrow(writer);
   typed_writer->write(message, DDS::HANDLE_NIL);
   ```

7. **DataReader with Listener** (Subscriber side)
   ```cpp
   DDS::DataReaderListener_var listener(new DataReaderListenerImpl);
   DDS::DataReader_var reader = subscriber->create_datareader(topic, qos, listener, ...);
   ```

### Listener Pattern
- Implement `DDS::DataReaderListener` interface (see `DataReaderListenerImpl.h/cpp`)
- Override `on_data_available()` to receive data asynchronously
- Use `take_next_sample()` or `read_next_sample()` to access data

### Synchronization Patterns
- **WaitSet**: Block until conditions are met (e.g., publication matched)
- **StatusCondition**: Monitor entity status changes
- Common pattern: Wait for subscriber/publisher matching before sending data

### Cleanup
Always cleanup in reverse order of creation:
```cpp
participant->delete_contained_entities();
dpf->delete_participant(participant);
TheServiceParticipant->shutdown();
```

## Discovery and Transport

### InfoRepo (OpenDDS-specific)
- Centralized discovery service (DCPSInfoRepo)
- Default for backward compatibility
- Tests may start InfoRepo automatically via `run_test.pl`

### RTPS (DDS-standard)
- Peer-to-peer discovery (no central service)
- Configured via INI files (e.g., `rtps.ini`)
- Use `--rtps` flag with test scripts
- Recommended for interoperability

### Transport Configuration
- TCP: OpenDDS-specific, default with InfoRepo
- RTPS/UDP: DDS-standard, required for RTPS discovery
- UDP, multicast, shared memory also available
- Configured in MPC files: `dcps_tcp`, `dcps_rtps_udp`

## Code Generation

### IDL Compilation
OpenDDS IDL compiler generates TypeSupport code:
- `*TypeSupport.idl` - DDS-standard interfaces
- `*TypeSupportImpl.h/cpp` - OpenDDS implementation
- `*C.h/cpp` - CORBA client stubs
- Build system handles this automatically

### Static vs Dynamic Linking
```cpp
#include <dds/DCPS/StaticIncludes.h>
#if OPENDDS_DO_MANUAL_STATIC_INCLUDES
#  include <dds/DCPS/RTPS/RtpsDiscovery.h>
#  include <dds/DCPS/transport/rtps_udp/RtpsUdp.h>
#endif
```
Include these for static builds to register discovery/transport plugins.

## Common Patterns

### Error Handling
Use ACE logging macros:
```cpp
ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("ERROR: %N:%l: main() - operation failed!\n")), 1);
ACE_DEBUG((LM_DEBUG, ACE_TEXT("Debug message\n")));
```

### Quality of Service (QoS)
- Use `PARTICIPANT_QOS_DEFAULT`, `TOPIC_QOS_DEFAULT`, etc. for defaults
- Modify QoS for specific needs (e.g., `RELIABLE_RELIABILITY_QOS`)
- QoS must be compatible between DataWriter and DataReader

### Return Code Checking
Always check DDS return codes:
```cpp
if (operation() != DDS::RETCODE_OK) {
  ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("ERROR: operation failed!\n")), 1);
}
```

## Project Structure

- `dds/` - Core OpenDDS implementation
- `dds/DCPS/` - Data-Centric Publish-Subscribe implementation
- `dds/DCPS/transport/` - Transport implementations (tcp, udp, rtps_udp, shmem, multicast)
- `DevGuideExamples/` - Example applications demonstrating OpenDDS features
- `tests/DCPS/` - Regression and feature tests
- `tools/` - IDL compiler and utilities
- `ACE_wrappers/`, `TAO/` - Dependencies (generated by configure)

## Dependencies

- **ACE/TAO**: Platform abstraction and CORBA ORB (auto-downloaded by configure)
- **Perl**: Required for build scripts, code generation, and test runners
- **IDL Compiler**: `tao_idl` (from TAO) and `opendds_idl`

## Development Workflow

1. Define data types in IDL files
2. Create MPC project file referencing the IDL
3. Implement Publisher and Subscriber applications
4. Create `run_test.pl` launcher script
5. Build using make or CMake
6. Test with both InfoRepo and RTPS configurations

## Running Examples

The Messenger example demonstrates the complete workflow:
```bash
cd DevGuideExamples/DCPS/Messenger

# Ensure environment is set
source ../../../setenv.sh

# Run with InfoRepo discovery
perl run_test.pl

# Run with RTPS discovery
perl run_test.pl --rtps

# Or build and run with CMake
mkdir build && cd build
cmake .. && cmake --build .
perl ../run_test.pl
```
