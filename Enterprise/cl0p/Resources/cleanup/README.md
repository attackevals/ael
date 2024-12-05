# CL0P Scenario Cleanup
This directory contains cleanup scripts for the CL0P scenario.

## SDBBot Cleanup
The `sdbbot_cleanup.ps1` PowerShell script cleans up SDBBot artifacts and registry edits from installing SDBBot. The log file `mswinsdr64.log` is not removed in case users still need to reference it. This log file will need to be removed manually if needed.

Usage:
```PowerShell
.\sdbbot_cleanup.ps1
```

