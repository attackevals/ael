# Protections Test 5

## Features

C++ executable that emulates ransomware behavior by enumerating and encrypting files in a specified directory.

Note: the encryption implementations have been removed for public release

## Build

Test5 can be built in Debug or Release mode using the included
`CMakePreset.json` configurations.

### Dependencies

- `CMake` version `3.26`
- `CMakePresets.json` version `6` support (Ninja Multi-Config)

### Quickstart

#### Command Line

Build both Debug and Release configurations of test5. This will also kick off the GoogleTest unit tests.

```PowerShell
cd Resources\test5
cmake.exe --workflow --preset cicd-debug
cmake.exe --workflow --preset cicd-release
```

For simplicity, all of the generated artifacts can be bundled into the
top-level `install/` directory using the CMake installation facility.

```PowerShell
cmake.exe --install ./build --config release
cmake.exe --install ./build --config debug
```

You can then run Test5 from the command line:

```PowerShell
.\install\Debug\test5.exe

.\install\Release\test5.exe
```

#### Usage & Arguments

All test5 arguments are positional. Test 5 requires a starting file path and a verb `encrypt/decrypt`.
- `[Path to starting directory/]`
- `encrypt|decrypt` verb

Example 1 (start in Users/Public and recursively encrypt):

```cmd
test5.exe C:\Users\Public\ encrypt 
```

Example 2 (start in Users/Public and recursively decrypt):

```cmd
test5.exe C:\Users\Public\ decrypt 
```

## Install

Installing test5 simply copies all of the artifacts from both Release
and Debug builds into the top-level `install/` directory. This is simply for
ease of use and distributability.

The final product should be a top-level directory named `install/` with the
following layout:

```PowerShell

test5/
├─ install/
   ├─ Debug/
   │  ├─ main.exe
   │  ├─ main.pdb
   ├─ Release/
      ├─ main.exe
```

## Test 🧪

### Unit Tests

GoogleTest unit tests will automatically run when using the `cicd-debug` and `cicd-release` presets

You can also run them ad-hoc by running:

```PowerShell
ctest.exe --preset all
```

## Troubleshoot

test5 does not generate any logging or stdout to reduce ioc's that are not relevant to the test.
If issues appear while trying to encrypt/decrypt, ensure that the path provide has a trailing slash
(eg. `C:\Users\Public\`).

## CTI

n/a

## Other References

- TBD
