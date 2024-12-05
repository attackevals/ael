#!/bin/bash

PASSWORD=$(openssl rand -base64 12)

echo "Encryption Password: $PASSWORD"

# Encrypt function
encrypt_file() {
    # IMPLEMENTATION REMOVED FOR PUBLIC RELEASE
    :
}

# recurse directories
recurse_directories() {
    for item in "$1"/*; do
        if [ -d "$item" ]; then
            recurse_directories "$item"
        else
            encrypt_file "$item"
        fi
    done
}

# check args
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <directory>"
    exit 1
fi

# ensure dir exists
if [ ! -d "$1" ]; then
    echo "Error: The directory provided '$1' does not exist."
    exit 1
fi

recurse_directories "$1"
