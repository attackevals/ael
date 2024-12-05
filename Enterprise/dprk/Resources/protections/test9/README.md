# Protections Test 9

`test9` is an AppleScript intended to be run as `root` that will attempt to
dump the System Keychain.

Of particular note is success in retrieving the machine account's plaintext
password after the victim machine has been joined to the domain.

## Usage Examples

### Requirements

* Sonoma M1
* Victim host must be domain joined
* Remote desktop/VNC access to the victim host
* Terminal running as root

### Basic Execution

`test9` takes in no arguments and will attempt to dump the System Keychain.

* In Terminal, elevate to root:

    ```zsh
    sudo su
    ```

* Execute the test9 AppleScript:

    ```zsh
    osascript test9.scpt
    ```

* View the output:

    ```zsh
    cat /tmp/kc.txt
    ```

## Cleanup 🧹

Remove the AppleScript and the output file:

```bash
rm test9.scpt /tmp/kc.txt
```
