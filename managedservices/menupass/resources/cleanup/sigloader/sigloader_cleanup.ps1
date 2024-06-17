 # ---------------------------------------------------------------------------
 # sigloader_cleanup.ps1 - Cleans up artifacts of SigLoader pipeline

 # Copyright 2024 MITRE Engenuity. Approved for public release. Document number CT0005.
 # Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at

 # http://www.apache.org/licenses/LICENSE-2.0

 # Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.

 # This project makes use of ATT&CK®
 # ATT&CK Terms of Use - https://attack.mitre.org/resources/terms-of-use/ 
 
 # Usage: .\sigloader_cleanup.ps1
 
 # Revision History:
 
 # --------------------------------------------------------------------------- 

# Paths to all possible files
# Top level paths
$layer1_dir = "C:\Program Files\Notepad++"
$payload_dir = "C:\Windows\System32"
$ingress_dir = "C:\Users\kizumi\AppData\Local\Temp"

# Program tasking
$sigloader_task_name = "Notepad++ Script"
$sigloader_service_name = "notepad"

# List of files to be removed
$file_array = "$layer1_dir\VERSION.dll", # Layer1 File
			  "$layer1_dir\debug.txt", # Layer1 debug File
			  "$payload_dir\hkp.dll", # Layer2 file
			  "$payload_dir\win64_tools.dll", # Layer3/Final payload file
			  "$ingress_dir\VERSION.dll", # Layer1 downloaded file
			  "$ingress_dir\nhi.dll", # Layer2 downloaded file
			  "$ingress_dir\mshtmled.wpf.cfg", # Layer3/Final payload downloaded file
			  "$Env:SystemRoot\System32\skt.dll", # Layer 2 on IIS
			  "$Env:SystemRoot\Microsoft.NET\mshtml.wpf.wfx" # Layer 2/Final payload on IIS


function write($data) {
	Write-Host $data
}

function delete_files($file_arr) {
	foreach ($file in $file_arr) {
		write "Checking if $file exists..."
		if (Test-Path $file) {
			write "File found"
			Remove-Item -Path $file
		}
		else {
			write "File not found"
		}
	}
}

function delete_task($task_name) {
	write "Checking if '$task_name' task exists"
	Get-ScheduledTask -TaskName $task_name -ErrorAction SilentlyContinue -OutVariable task

	if (!$task) {
		write "Task not found."
	}
	else {
		write "Task found, deleting..."
		schtasks /delete /tn $task_name /f
	}
}

function delete_service($service_name) {
	write "Checking if '$service_name' service exists"
	Get-Service -Name $service_name -ErrorAction SilentlyContinue -OutVariable service

	if (!$service) {
		write "Service not found"
	}
	else {
		write "Service found, deleting..."
		sc.exe delete $service_name
	}
}

function stop_notepad {
	$notepad = Get-Process notepad++ -ErrorAction SilentlyContinue
	# Close notepad++
	if ($notepad) {
		write "Notepad++ is running, we need to stop it before we can delete layer1"
		$notepad.CloseMainWindow()
		Sleep 5
		# Force close if regular close didn't work
		if (!$notepad.HasExited) {
			$notepad | Stop-Process -Force
		}
	}
}

stop_notepad
delete_files $file_array
delete_task $sigloader_task_name
delete_service $sigloader_service_name
