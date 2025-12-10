#!/usr/bin/env python3
"""
Fleet Activity Log Exporter
Extracts activity logs from Fleet's MySQL database for Splunk ingestion
"""

import json
import os
import subprocess
import sys
from datetime import datetime, timedelta


class FleetActivityExporter:
    def __init__(self):
        self.mysql_container = "fleetdm-mysql-1"
        self.db_name = "fleet"
        self.db_user = "fleet"
        self.db_pass = "insecure"
        self.output_dir = "/var/log/fleet-activities"

    def ensure_output_dir(self):
        """Create output directory if it doesn't exist"""
        os.makedirs(self.output_dir, exist_ok=True)

    def execute_mysql_query(self, query):
        """Execute MySQL query via docker exec"""
        cmd = [
            "docker",
            "exec",
            self.mysql_container,
            "mysql",
            f"-u{self.db_user}",
            f"-p{self.db_pass}",
            self.db_name,
            "-e",
            query,
            "--batch",
            "--raw",
        ]

        try:
            result = subprocess.run(cmd, capture_output=True, text=True, check=True)
            return result.stdout.strip()
        except subprocess.CalledProcessError as e:
            print(f"Error executing MySQL query: {e}")
            print(f"stderr: {e.stderr}")
            return None

    def get_activities(self, hours_back=1):
        """Get activity logs from the last N hours"""
        query = f"""
        SELECT
            id,
            created_at,
            user_id,
            user_name,
            activity_type,
            details
        FROM activities
        WHERE created_at >= DATE_SUB(NOW(), INTERVAL {hours_back} HOUR)
        ORDER BY created_at DESC;
        """

        return self.execute_mysql_query(query)

    def parse_mysql_output(self, output):
        """Parse MySQL tab-separated output to JSON"""
        if not output:
            return []

        lines = output.split("\n")
        if len(lines) < 2:
            return []

        headers = lines[0].split("\t")
        activities = []

        for line in lines[1:]:
            if not line.strip():
                continue

            values = line.split("\t")
            activity = {}

            for i, header in enumerate(headers):
                value = values[i] if i < len(values) else ""
                # Handle NULL values
                if value == "NULL":
                    value = None
                activity[header] = value

            activities.append(activity)

        return activities

    def export_to_file(self, activities, filename=None):
        """Export activities to JSON file"""
        if filename is None:
            timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
            filename = f"{self.output_dir}/fleet_activities_{timestamp}.json"

        with open(filename, "w") as f:
            json.dump(activities, f, indent=2, default=str)

        # Set permissions for Splunk
        os.chmod(filename, 0o644)
        return filename

    def cleanup_old_files(self, days_to_keep=1):
        """Remove old activity files"""
        cutoff_time = datetime.now() - timedelta(days=days_to_keep)

        for filename in os.listdir(self.output_dir):
            if filename.startswith("fleet_activities_") and filename.endswith(".json"):
                filepath = os.path.join(self.output_dir, filename)
                file_time = datetime.fromtimestamp(os.path.getmtime(filepath))

                if file_time < cutoff_time:
                    os.remove(filepath)
                    print(f"Removed old file: {filename}")

    def run(self, hours_back=1):
        """Main execution function"""
        self.ensure_output_dir()

        print(f"Extracting Fleet activities from last {hours_back} hour(s)...")

        # Get activities
        raw_output = self.get_activities(hours_back)
        if raw_output is None:
            print("Failed to retrieve activities")
            return False

        # Parse to JSON
        activities = self.parse_mysql_output(raw_output)

        if not activities:
            print("No activities found")
            return True

        # Export to file
        output_file = self.export_to_file(activities)
        print(f"Exported {len(activities)} activities to: {output_file}")

        # Cleanup old files
        self.cleanup_old_files()

        return True


if __name__ == "__main__":
    hours_back = 1
    if len(sys.argv) > 1:
        try:
            hours_back = int(sys.argv[1])
        except ValueError:
            print("Usage: python3 fleet_activity_exporter.py [hours_back]")
            sys.exit(1)

    exporter = FleetActivityExporter()
    success = exporter.run(hours_back)
    sys.exit(0 if success else 1)
