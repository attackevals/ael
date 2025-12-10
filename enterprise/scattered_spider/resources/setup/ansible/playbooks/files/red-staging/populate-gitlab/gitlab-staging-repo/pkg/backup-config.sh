#!/bin/bash

# Backup AirByte configuration files

BACKUP_DIR="/backups/airbyte-configs"
TIMESTAMP=$(date +"%Y%m%d%H%M%S")

echo "Creating backup directory..."
mkdir -p $BACKUP_DIR

# Replace the path to AirByte config with your file path
echo "Backing up Airbyte configuration..."
tar -czvf $BACKUP_DIR/airbyte-config-backup-$TIMESTAMP.tar.gz /path/to/airbyte/config

if [ $? -eq 0 ]; then
	echo "Backup completed successfully at $TIMESTAMP."
else
	echo "Backup failed."
fi
