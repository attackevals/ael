#!/bin/bash
#################################
# Usage:
# Execute the following command from the folder containing the FULLHOUSE.DOORED implant
# sudo ./fullhouse_cleanup.sh
#
# This script performs the following actions:
#   Ensure FULLHOUSE.DOORED process is killed
#   Delete all files associated with FULLHOUSE.DOORED:
#   - Configuration file
#   - FULLHOUSE.DOORED binary
#################################

# Ensure we are running as sudo
if [ "$EUID" -ne 0 ]; then
    echo -e "Please run this script with sudo:\n\nsudo ./fullhouse_cleanup.sh\n\n"
    exit 1
fi

# Check if the service is loaded or processes are running
keyword="com.docker.sock"
echo -e "\nKilling processes with keyword $keyword..."
result=$(pgrep -f $keyword)
if [ $? -ne 0 ]; then
    echo "No processes with $keyword running"
fi
if [ -n "$result" ]; then
    echo -e "Process found with PID(s):\n$result"
    # Ensure processes are killed
    for pid in $result; do
        echo "Terminating process with PID $pid"
        kill -9 $pid
    done
fi

#   Delete all files associated with FULLHOUSE.DOORED:
#   - Configuration file
#   - FULLHOUSE.DOORED binary
echo -e "\nRemoving all files associated with FULLHOUSE.DOORED..."

FULLHOUSE_FILES=(
    "/usr/local/bin/com.docker.sock"
    "/usr/local/bin/com.docker.sock.lock"
)

for FILE in "${FULLHOUSE_FILES[@]}"; do
    if [ -e "$FILE" ]; then
        rm -f "$FILE"
        echo "Removed file: $FILE"
    else
        echo "File does not exist: $FILE"
    fi
done

echo -e "\n################ Clean up script complete ###############"
