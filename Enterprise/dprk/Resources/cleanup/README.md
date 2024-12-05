# DPRK Scenario Cleanup

This cleanup instruction set assumes the cleanup scripts are available to
download from the Kali attack machine. Either the evalsC2server Simple File
Server handler or a simple Python HTTP server to serve the files will suffice
for this purpose.

1. In the victim macOS machine, open a Terminal
1. Elevate to sudo, providing the user's password when prompted:

    ```zsh
    sudo su
    ```

1. To cleanup STRATOFEAR:
    1. Download the [STRATOFEAR cleanup script](./stratofearCleanUp.sh) to
    the /Library/PrivilegedHelperTools directory
        - evalsC2server Simple File Server handler, configured to serve the DPRK/Resources/cleanup folder:

            ```zsh
            curl -o stratofearCleanUp.sh http://b0ring-t3min4l.kr/files/stratofearCleanUp.sh
            ```

        - Simple Python HTTP server (`python3 -m http.server`), started in the DPRK/Resources/cleanup folder:

            ```zsh
            curl -o stratofearCleanUp.sh http://223.246.0.70:8000/stratofearCleanUp.sh
            ```

    1. Execute the STRATOFEAR cleanup script:

        ```zsh
        cd /Library/PrivilegedHelperTools
        chmod +x stratofearCleanUp.sh
        ./stratofearCleanUp.sh
        ```

1. To cleanup FULLHOUSE.DOORED:
    1. Download the [FULLHOUSE.DOORED cleanup script](./fullhouse_cleanup.sh)
    to /usr/local/bin/
        - evalsC2server Simple File Server handler, configured to serve the DPRK/Resources/cleanup folder:

        ```zsh
        curl -o fullhouse_cleanup.sh http://b0ring-t3min4l.kr/files/fullhouse_cleanup.sh
        ```

        - Simple Python HTTP server (`python3 -m http.server`), started in the DPRK/Resources/cleanup folder:

            ```zsh
            curl -o fullhouse_cleanup.sh http://223.246.0.70:8000/fullhouse_cleanup.sh
            ```

    1. Execute the FULLHOUSE.DOORED cleanup script:

        ```zsh
        cd /usr/local/bin
        chmod +x fullhouse_cleanup.sh
        ./fullhouse_cleanup.sh
        ```

**NOTE:** This cleanup instruction set *does not* remove the debug log files created by the implants.
