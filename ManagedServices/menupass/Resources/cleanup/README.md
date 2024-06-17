# Cleanup

## Menupass Scenario

### Setup
To remove artifacts, run the [Cleanup Scripts](../cleanup/) as from the Kali hosts. You can also run the cleanup scripts on each target as described in their respective directories.

1. From the Kali Linux machine (`176.59.17.11`):
    ```
    # from menuPass directory
    xfreerdp +clipboard /u:DOMAIN\\\evals_domain_admin /p:"DuapQj7k8Va8U1X27rw6" /v:10.10.10.4 /drive:X,Resources/cleanup
    ```
1. From the RDP session, open powershell in administrative mode
1. Run the cleanup commands for each implant listed below that you need to cleanup
1. Sign out of the RDP session when finished.

### SODAMASTER
* ```
  \\tsclient\X\SodaMaster\sodamaster_cleanup.ps1 -target 10.10.20.11 -user USERNAME -restart
  ```

### SIGLOADER
* ```
  \\tsclient\X\SigLoader\sigloader_cleanup.ps1 -target 10.10.20.11 -user USERNAME
  ```
* ```
  \\tsclient\X\SigLoader\sigloader_cleanup.ps1 -target 10.10.10.9 -user USERNAME
  ```
* ```
  \\tsclient\X\Sigloader\sigloader_cleanup.ps1 -target 10.10.10.4 -user USERNAME
  ```

