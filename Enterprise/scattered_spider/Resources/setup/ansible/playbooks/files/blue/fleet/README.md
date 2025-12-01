# Fleet Activity Log Exporter for Splunk

This solution extracts Fleet activity logs directly from the MySQL database and formats them for Splunk ingestion, bypassing the need for Fleet Premium licensing.

## Files Created

1. **extract-fleet-activities-now.sh** - Manual extraction script for immediate use
2. **fleet_activity_exporter.py** - Automated Python script with error handling
3. **fleet-activity-exporter.sh** - Simple bash version
4. **fleet-activity-exporter.service** - Systemd service file
5. **fleet-activity-exporter.timer** - Systemd timer for hourly execution
6. **configure-fleet-activity-exporter.yml** - Ansible playbook for deployment

## Quick Start (Manual Extraction)

1. SSH to your Fleet server (b3-mdm-srv1)
2. Copy the `extract-fleet-activities-now.sh` script to the server
3. Run it:

   ```bash
   ./extract-fleet-activities-now.sh
   ```

This will create a `fleet-activities-export/` directory with JSON files containing the last 24 hours of activities.

## Automated Deployment with Ansible

This will:

- Install the Python exporter script
- Set up systemd service and timer for hourly execution
- Configure log rotation
- Test the initial extraction

## Manual Setup (Alternative)

1. Copy files to the Fleet server:

   ```bash
   scp fleet_activity_exporter.py user@fleet-server:/opt/scripts/
   scp fleet-activity-exporter.service /etc/systemd/system/
   scp fleet-activity-exporter.timer /etc/systemd/system/
   ```

2. Enable the service:

   ```bash
   sudo systemctl daemon-reload
   sudo systemctl enable fleet-activity-exporter.timer
   sudo systemctl start fleet-activity-exporter.timer
   ```

## Database Schema

The script extracts from the `activities` table with these fields:

- `id` - Unique activity ID
- `created_at` - Timestamp of the activity
- `user_id` - ID of the user who performed the action
- `user_name` - Name of the user
- `activity_type` - Type of activity (e.g., "created_pack", "deleted_query")
- `details` - JSON details of the activity
- `streamed` - Whether the activity was streamed

## Output Format

Activities are exported as JSON arrays:

```json
[
  {
    "id": "123",
    "created_at": "2024-01-15 10:30:00",
    "user_id": "1",
    "user_name": "admin",
    "activity_type": "created_query",
    "details": "{\"query_name\":\"test_query\"}",
    "streamed": "0"
  }
]
```

## Splunk Configuration

Configure Splunk Universal Forwarder to monitor the output directory:

```ini
# Fleet Logs (if applicable)
[monitor:////var/log/fleet-activities//*.json]
sourcetype = _json
disabled = 0
crcSalt = <SOURCE>
```

## Customization

### Change Export Frequency

Edit the timer file to change from hourly:

```ini
# For every 30 minutes
OnCalendar=*:0/30

# For daily at 2 AM
OnCalendar=daily
```

### Change Time Range

Modify the Python script or pass hours as argument:

```bash
python3 fleet_activity_exporter.py 6  # Last 6 hours
```

### Database Connection

Update these variables in the scripts if your setup differs:

- `mysql_container` - Docker container name
- `db_name` - Database name (default: "fleet")
- `db_user` - Database user (default: "fleet")
- `db_pass` - Database password (default: "insecure")

## Troubleshooting

### Check if Fleet containers are running

```bash
docker ps | grep fleet
```

### Test database connection

```bash
docker exec fleetdm-mysql-1 mysql -ufleet -pinsecure fleet -e "SELECT COUNT(*) FROM activities;"
```

### Check systemd service status

```bash
sudo systemctl status fleet-activity-exporter.timer
sudo journalctl -u fleet-activity-exporter.service
```

### Verify output files

```bash
ls -la /var/log/fleet-activities/
```

## Security Notes

- The database password is hardcoded as "insecure" in your current setup
- Output files are readable by all users (644 permissions) for Splunk access

## Performance Considerations

- The script only queries the last hour by default to minimize database load
- Old export files are automatically cleaned up after 24 hours
