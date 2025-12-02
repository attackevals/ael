# Mustang Panda Resources

This directory contains all source code, malware implementations, and infrastructure components required to execute the Mustang Panda adversary emulation scenario.

## Overview

The Mustang Panda emulation replicates the tactics, techniques, and procedures (TTPs) of this China-based cyber espionage group. The scenario includes custom implementations of PlugX and ToneShell malware, along with supporting tools for reconnaissance and lateral movement. All components are provided as source code and must be built before use.

## Directory Structure

### Core Infrastructure

- **[controlServer/](controlServer/)** - Go-based C2 server with REST API for managing PlugX and ToneShell implant sessions
- **[setup/](setup/)** - Infrastructure deployment and configuration documentation
- **[payloads/](payloads/)** - Payload files including spearphishing documents, VBS scripts, and MSC files

### Malware Implementations

- **[plugx/](plugx/)** - Custom PlugX malware implementation in C++
  - Modular shellcode architecture with multiple capabilities
  - MSI-based deployment mechanism with Nim-based loader
  - Protections Test 5 variant with separate MSI installer and sideloaded DLL
  - Unit testing framework using Google Test
- **[toneshell/](toneshell/)** - ToneShell malware implementation in C++
  - Deployment via DLL sideloading
  - Protections Test 4 variant with dropper and modified C2 encryption
  - Shellcode-based execution
  - Unit testing framework using Google Test

### Supporting Tools

- **[sharpnbtscan/](sharpnbtscan/)** - .NET-based NetBIOS scanner for network reconnaissance
- **[email_generation/](email_generation/)** - Scripts for generating and sending spearphishing emails

### Supporting Resources

- **[Images/](Images/)** - Image assets for documentation and operations
- **[assets/](assets/)** - Scenario diagrams, operational flow charts, and visual documentation

## Getting Started

1. **Review the scenario** - Start with the [Scenario Overview](../CTI_Emulation_Resources/Mustang_Panda_Scenario_Overview.md) to understand the attack flow
2. **Set up infrastructure** - Follow the [setup documentation](setup/) to prepare the environment
3. **Build malware components**:
   - Build [PlugX](plugx/) following the CMake build instructions
   - Build [ToneShell](toneshell/) following the CMake build instructions
   - Compile [SharpNBTScan](sharpnbtscan/) using Visual Studio or dotnet CLI
4. **Configure C2 server** - Set up the [control server](controlServer/) with PlugX and ToneShell handlers
5. **Execute emulation** - Follow the [Emulation Plan](../Emulation_Plan/Mustang_Panda_Scenario.md) for step-by-step execution

## Key Techniques Emulated

- **Initial Access**: Spearphishing with malicious attachments (MSC, DOCX, HTML)
- **Execution**: DLL side-loading, MSI installation, VBS scripts
- **Persistence**: Registry Run keys, service installation
- **Defense Evasion**: DLL side-loading with legitimate binaries, encrypted payloads
- **Discovery**: Network scanning, system enumeration
- **Command and Control**: Custom PlugX and ToneShell protocols
- **Collection**: File and data collection via VBS scripts

## Build Requirements

### PlugX and ToneShell

- CMake 3.26+
- Visual Studio 2019+ or compatible C++ compiler
- Windows SDK
- Nim compiler (for PlugX loader)
- Google Test framework (for unit tests)

### Control Server

- Go 1.18+

### SharpNBTScan

- .NET Framework 4.5+ or .NET Core 3.1+

### Scripts and Tools

- Python 3.x

## Malware Architecture

### PlugX

PlugX is implemented as a modular malware with:

- **Loader**: Nim-based initial loader with RC4 encryption
- **Shellcode**: Position-independent code with module registry
- **Modules**: Pluggable capabilities (C2 comms, command execution, keylogging, file operations)
- **Deployment**: MSI installer with DLL side-loading

### ToneShell

ToneShell features:

- **C2 Comms**: TCP-based C2 channel that supports task execution and file uploads/downloads.
- **Defense Evason**: Sandbox checks, process injection to obfuscate execution flow.
- **Shellcode**: Core malicious functionality
- **Deployment**: DLL sideloading

## Important Notes

- All executables have been removed from this repository for security reasons
- You must build all components from source following the documentation
- The malware implementations are for authorized security testing only
- Ensure you have proper authorization before using these tools
- Review the [Liability / Responsible Usage](../README.md#liability--responsible-usage) section

## Testing

Both PlugX and ToneShell include comprehensive unit test suites:

```shell
# Build and run PlugX tests
cd plugx
cmake --workflow --preset cicd-debug
cmake --workflow --preset cicd-release

# Build and run ToneShell tests
cd toneshell
cmake --workflow --preset cicd-debug
cmake --workflow --preset cicd-release
```

## Documentation

Each subdirectory contains its own README with specific build instructions, usage examples, and technical details:

- [PlugX Build Instructions](plugx/README.md)
- [ToneShell Build Instructions](toneshell/README.md)
- [Control Server Documentation](controlServer/README.md)

## Support

For questions or issues:

- Email: evals@mitre.org
- Review the main [Mustang Panda README](../README.md) for additional resources
