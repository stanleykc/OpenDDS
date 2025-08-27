# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Common Development Commands

### Building OpenDDS

- **Configure**: `./configure` - Perl-based configuration script that downloads ACE/TAO dependencies
- **MPC Build**: Use MPC (MakeProject Creator) workspace files:
  - `DDS.mwc` - Core OpenDDS build
  - `DDS_TAOv2.mwc` - OpenDDS with TAO v2
  - `DDS_no_tests.mwc` - Build without tests
  - `ACE_TAO_for_OpenDDS.mwc` - ACE/TAO dependencies
- **CMake Build**: `cmake` and `make` (requires CMake 3.23+)
- **Generate build files**: `mwc.pl -type gnuace DDS.mwc` (for makefiles)

### Testing

- **Run all tests**: `perl tests/auto_run_tests.pl`
- **Test categories**:
  - DCPS tests: `perl tests/auto_run_tests.pl tests/dcps_tests.lst`
  - Security tests: `perl tests/auto_run_tests.pl tests/security/security_tests.lst`
  - Java tests: `perl tests/auto_run_tests.pl java/tests/dcps_java_tests.lst`
  - Performance tests: `perl performance-tests/bench/run_test.pl`
- **Individual test**: Navigate to test directory and run `perl run_test.pl`
- **Core CI tests**: `perl tests/auto_run_tests.pl tests/core_ci_tests.lst`

### IDL Compilation

- **OpenDDS IDL compiler**: `opendds_idl` (generates TypeSupport, DataReader, DataWriter)
- **TAO IDL compiler**: `tao_idl` (generates CORBA stubs/skeletons)

## Project Architecture

### Core Directory Structure

- **`dds/`** - Core OpenDDS implementation
  - `DCPS/` - Data-Centric Publish-Subscribe implementation
  - `InfoRepo/` - DCPSInfoRepo service for centralized discovery
  - `idl/` - IDL compiler source code
  - `monitor/` - Monitoring framework
- **`tools/`** - Utilities and tools
  - `rtpsrelay/` - RTPS relay service for cloud/firewall scenarios
  - `dissector/` - Wireshark dissector plugin
  - `scripts/` - Build and development scripts
- **`tests/`** - Test suites organized by category
- **`examples/`** and **`DevGuideExamples/`** - Sample applications
- **`performance-tests/`** - Performance and scalability tests

### Key Build Components

- **MPC (MakeProject Creator)**: Primary build system using `.mpc` files
- **CMake support**: Alternative build system (requires 3.23+)
- **ACE/TAO dependency**: Core middleware framework automatically downloaded by configure script

### Transport Protocols

OpenDDS supports multiple transport protocols:

- TCP/IP transport (`dds/DCPS/transport/tcp/`)
- UDP/IP transport (`dds/DCPS/transport/udp/`)
- IP Multicast transport (`dds/DCPS/transport/multicast/`)
- RTPS over UDP transport (`dds/DCPS/transport/rtps_udp/`)
- Shared Memory transport (`dds/DCPS/transport/shmem/`)

### Discovery Mechanisms

- **InfoRepo Discovery**: Centralized discovery using DCPSInfoRepo service
- **RTPS Discovery**: Distributed discovery following RTPS specification (`dds/DCPS/RTPS/`)
- **Static Discovery**: Pre-configured endpoint discovery

### Configuration Files

- `*.ini` files for runtime configuration (transport, discovery, logging)
- Example configurations in `DevGuideExamples/` directories

## Development Notes

### Prerequisites

- Perl (required for configure script and test runner)
- C++ compiler (GCC, Clang, Visual Studio)
- ACE/TAO (automatically handled by configure script)

### Building Process

1. Run `./configure` to set up environment and download dependencies
2. Use MPC to generate build files: `mwc.pl -type <build_type> <workspace>.mwc`
3. Build using generated makefiles or project files

### Test Organization

Tests use `.lst` files to define test suites with conditional configurations. Each test directory contains a `run_test.pl` script that can be executed independently.

- Use setenv.sh to run OpenDDS applications
