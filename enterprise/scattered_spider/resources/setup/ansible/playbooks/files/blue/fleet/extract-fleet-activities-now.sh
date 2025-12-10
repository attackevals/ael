#!/bin/bash

# Quick Fleet Activity Extraction Script
# Run this manually to extract activities immediately

echo "Fleet Activity Log Extractor"
echo "============================"

# Check if Fleet containers are running
if ! docker ps | grep -q fleetdm-mysql; then
	echo "Error: Fleet MySQL container not found or not running"
	echo "Make sure Fleet is deployed and running"
	exit 1
fi

# Get container name (it might have a different suffix)
MYSQL_CONTAINER=$(docker ps --format "table {{.Names}}" | grep mysql | head -1)
echo "Using MySQL container: $MYSQL_CONTAINER"

# Create output directory
OUTPUT_DIR="./fleet-activities-export"
mkdir -p "$OUTPUT_DIR"

# Get timestamp
TIMESTAMP=$(date +"%Y%m%d_%H%M%S")
OUTPUT_FILE="$OUTPUT_DIR/fleet_activities_$TIMESTAMP.json"

echo "Extracting activities from the last 24 hours..."

# SQL query to get activities
QUERY="SELECT
    JSON_OBJECT(
        'id', id,
        'created_at', created_at,
        'user_id', user_id,
        'user_name', user_name,
        'activity_type', activity_type,
        'details', details,
        'streamed', streamed
    ) as activity_json
FROM activities
WHERE created_at >= DATE_SUB(NOW(), INTERVAL 24 HOUR)
ORDER BY created_at DESC;"

# Execute query and format output
echo "[" >"$OUTPUT_FILE"
docker exec "$MYSQL_CONTAINER" mysql -ufleet -pinsecure fleet -e "$QUERY" --batch --raw --skip-column-names |
	sed 's/$/,/' |
	sed '$s/,$//' >>"$OUTPUT_FILE"
echo "]" >>"$OUTPUT_FILE"

# Check if we got any data
if [ -s "$OUTPUT_FILE" ]; then
	ACTIVITY_COUNT=$(grep -o '"id"' "$OUTPUT_FILE" | wc -l)
	echo "Successfully exported $ACTIVITY_COUNT activities to: $OUTPUT_FILE"
	echo ""
	echo "File size: $(du -h "$OUTPUT_FILE" | cut -f1)"
	echo ""
	echo "Sample of first activity:"
	head -20 "$OUTPUT_FILE"
else
	echo "No activities found or export failed"
	rm -f "$OUTPUT_FILE"
fi
