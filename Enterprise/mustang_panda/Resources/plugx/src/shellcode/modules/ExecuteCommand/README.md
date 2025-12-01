## ExecuteCommand Module

This module executes commands sent by the C2 server.

### Default Dispatch Code

`0x1000`

### Required Arguments

A single argument (the command) is required for the module to execute. There is no default argument.

To execute a console command prepend the command with `cmd.exe /c`.

whoami example: `cmd.exe /c whoami`
