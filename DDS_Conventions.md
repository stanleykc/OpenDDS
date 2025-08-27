# DDS Coding Conventions and Style Guide

This document captures the coding conventions and style guidelines observed in the OpenDDS codebase, specifically analyzed from `tests/DCPS/XTypes`, to guide Claude Code in future development work.

## File Organization and Structure

### Header Files
- Use **include guards** with pattern: `#ifndef FILENAME_H` / `#define FILENAME_H` / `#endif`
- Include system/standard headers first, then OpenDDS headers, then local headers
- Group includes logically with blank lines between groups
- Use forward declarations where possible to minimize header dependencies

```cpp
// Example from Common.h
#ifndef COMMON_H
#define COMMON_H

#include "CommonTypeSupportImpl.h"

#include <dds/DCPS/BuiltInTopicUtils.h>
#include <dds/DCPS/DCPS_Utils.h>
// ... more OpenDDS includes

#include <string>
```

### Source Files
- Include corresponding header first, followed by other required headers
- Use consistent header grouping as in header files

## Naming Conventions

### Variables and Functions
- **snake_case** for variables: `key_value`, `expect_inconsistent_topic`, `topic_name`
- **snake_case** for function names: `write_sample`, `get_topic`, `wait_for_reader`, `check_rc`
- **camelCase** for template parameters and some specialized contexts
- **Global variables** use descriptive names: `verbose`, `expect_incompatible_qos`

### Types and Classes
- **PascalCase** for class/struct names: `DynamicWriter`, `ReadTest`, `ControlStruct`
- **PascalCase** for enum names and values: `AdditionalFieldValue`, `FINAL_STRUCT_AF`
- **Template type parameters** use single uppercase letters: `T`, `Sample`, `DataReaderType`

### IDL Conventions
- **PascalCase** for struct/union names: `PlainCdrStruct`, `ModifiedMutableStruct`
- **snake_case** for field names: `key_field`, `additional_field`, `trim_string`
- **SCREAMING_SNAKE_CASE** for enum values: `E_KEY`, `E_OCTET_FIELD`, `E_ADDITIONAL_FIELD`

### File Naming
- **PascalCase** for main files: `Publisher.cpp`, `Subscriber.cpp`, `Common.h`
- **snake_case** for configuration files: `rtps_disc.ini`, `tcp.ini`
- **lowercase** for build files: `run_test.pl`

## Code Style and Formatting

### Indentation and Spacing
- **2 spaces** for indentation (no tabs)
- **Space after comma** in parameter lists and template arguments
- **No space** before opening parentheses in function calls
- **Space around operators**: `=`, `+`, `!=`, etc.

### Braces and Line Breaks
- **Opening brace on same line** for functions, classes, and control structures
- **Closing brace on new line** aligned with the opening statement
- **Single statement if/for** blocks may omit braces (but include them for clarity)

```cpp
// Function definition style
bool wait_for_reader(bool tojoin, DataWriter_var &dw)
{
  WaitSet_var ws = new DDS::WaitSet;
  // ...
  for (int retries = 3; retries > 0 && (tojoin == (pms.current_count == 0)); --retries) {
    // ...
  }
  return success;
}
```

### Line Length and Wrapping
- **No strict line limit** observed, but keep reasonable (under 120 characters when possible)
- **Break long parameter lists** after commas, aligned with first parameter or indented
- **Break long expressions** at logical points

```cpp
// Long parameter list wrapping
void get_topic(bool& success, TypeSupport_var& ts, const DomainParticipant_var dp, 
  const std::string& topic_name, Topic_var& topic, const std::string& registered_type_name, 
  bool dynamic = false)
```

## Error Handling Patterns

### ACE_ERROR Usage
- Use `ACE_ERROR((LM_ERROR, "ERROR: descriptive message\n"))` for error reporting
- Include **ERROR:** prefix in all error messages
- Use `%C` for C-style strings, `%d` for integers, `%u` for unsigned
- Include context information in error messages

```cpp
ACE_ERROR((LM_ERROR, "ERROR: check_int32 for %C.%C: expected %d, received %d\n",
  type_name.in(), path.c_str(), expected, actual));
```

### Return Code Checking
- Use helper function `check_rc()` for consistent return code checking
- Pattern: `if (!check_rc(operation(), "context description")) { return false; }`
- Always provide descriptive context in error messages

```cpp
bool check_rc(DDS::ReturnCode_t rc, const std::string& what, bool ignore_no_data = false)
{
  if (ignore_no_data && rc == DDS::RETCODE_NO_DATA) {
    return true;
  }
  if (rc != DDS::RETCODE_OK) {
    ACE_ERROR((LM_ERROR, "ERROR: %C: %C\n", what.c_str(), OpenDDS::DCPS::retcode_to_string(rc)));
    return false;
  }
  return true;
}
```

### Success/Failure Patterns
- Use **boolean success flags** for tracking operation success across multiple steps
- **Early return** on failure conditions
- **Clean up resources** in error paths

## Template Usage

### Template Function Patterns
- Use templates for **type-safe operations** across different DDS types
- **Template specialization** for different data types (Sample, DataWriter types)
- **Template parameters** should be descriptive: `typename Sample`, `typename DataReaderType`

```cpp
template <typename Sample, typename DataWriterVar>
void write_sample_i(const TypeSupport_var& ts, const DataWriterVar& dw, const Sample& sample)
{
  CORBA::String_var name = ts->get_type_name();
  if (!dw) {
    ACE_ERROR((LM_ERROR, "ERROR: typed DataWriter for %C was null\n", name.in()));
    return;
  }
  check_rc(dw->write(sample, HANDLE_NIL), std::string("write for ") + name.in() + " failed");
}
```

### Template Member Functions
- Use **template member functions** in classes for type-generic operations
- **Consistent naming** pattern: `read_kf_struct<TopicType>()`, `write_union<TopicType>()`

## Memory Management

### CORBA Variable Types
- Use **_var types** for automatic memory management: `Topic_var`, `DataWriter_var`, `TypeSupport_var`
- Use **_duplicate()** when explicitly copying CORBA objects
- **Null checks** before using CORBA objects

### Resource Cleanup
- **Explicit cleanup** in main functions: `dp->delete_contained_entities()`, `dpf->delete_participant(dp)`
- **RAII pattern** where applicable (destructors for cleanup)

## Testing Patterns

### Test Structure
- **Separate executable** for Publisher and Subscriber
- **Command-line arguments** for test configuration (`--type`, `--verbose`, `--expect-*`)
- **Control topics** for synchronization between publisher and subscriber

### Dynamic vs Static Testing
- Support both **static typed** and **dynamic typed** operations
- Use **conditional compilation** with `#ifdef OPENDDS_SAFETY_PROFILE`
- **Template-based approach** for handling different type systems

```cpp
template <typename TopicType>
void write_kf_struct(const TypeSupport_var& ts, const DataWriter_var& dw, bool dynamic)
{
  if (dynamic) {
    DynamicWriter d(ts, dw);
    d.set_int32("key_field", key_value);
  } else {
    TopicType sample;
    sample.key_field = key_value;
    write_sample(ts, dw, sample);
  }
}
```

### Validation Patterns
- **Comprehensive status checking**: inconsistent topic, incompatible QoS, publication/subscription matching
- **Expected failure testing**: `expect_inconsistent_topic`, `expect_incompatible_qos` flags
- **Data validation**: verify received data matches expected values

## Build System Integration

### MPC Project Files
- **Separate projects** for Publisher and Subscriber
- **Dependency management** with `after += *Subscriber`
- **IDL compilation**: separate IDL_Files and TypeSupport_Files sections
- **Feature flags**: `dcps_ts_flags += -Gxtypes-complete`

### IDL Annotations
- Use **OpenDDS annotations**: `@topic`, `@key`, `@final`, `@appendable`, `@mutable`
- **ID annotations** for mutable types: `@id(1)`, `@id(2)`
- **Behavioral annotations**: `@try_construct(TRIM)`

## Constants and Configuration

### String Constants
- Define **meaningful constants**: `STRING_26`, `STRING_20` for test data
- Use **const** for read-only values
- **Descriptive naming** for enum values and constants

### Configuration Files
- **INI-based configuration** for different transport/discovery scenarios
- **Naming convention**: `rtps_disc.ini`, `tcp.ini`, `rtps_disc_security.ini`

## Documentation and Comments

### Comment Style
- **Minimal comments** in code - prefer self-documenting code
- **Section comments** for major code blocks using `//`
- **TODO comments** with specific tracking information

### Debug Output
- Use **ACE_DEBUG** for informational messages with timestamps (`%T`)
- **Conditional verbose output** controlled by command-line flags
- **Consistent message format**: operation context and timing information

```cpp
ACE_DEBUG((LM_DEBUG, "Writer starting at %T\n"));
ACE_DEBUG((LM_DEBUG, "Starting wait for reader %C -- Count = %d at %T\n",
           (tojoin ? "startup" : "shutdown"), pms.current_count));
```

This style guide ensures consistency with the existing OpenDDS codebase and facilitates maintainability and readability of future code contributions.