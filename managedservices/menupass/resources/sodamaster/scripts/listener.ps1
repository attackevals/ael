# ---------------------------------------------------------------------------
# listener.ps1 - Starts a local listener for testing SodaMaster callbacks

# Copyright 2024 MITRE Engenuity. Approved for public release. Document number CT0005.
# Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at

# http://www.apache.org/licenses/LICENSE-2.0

# Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.

# This project makes use of ATT&CK�
# ATT&CK Terms of Use - https://attack.mitre.org/resources/terms-of-use/ 

# Usage: .\listener.ps1; Start-listener -TCPPort 8080

# Revision History:

 # Resources: 
 # https://powershellisfun.com/2022/08/10/create-tcp-udp-port-listener-using-powershell/

# --------------------------------------------------------------------------- 

function Start-listener {
    param (
        [parameter(Mandatory = $false, HelpMessage = "Enter the tcp port you want to use to listen on, for example 3389")]
        [ValidatePattern('^[0-9]+$')]
        [ValidateRange(0, 65535)]
        [int]$TCPPort
    )
    
    #Test if TCP port is already listening port before starting listener
    if ($TCPPort) {
        $Global:ProgressPreference = 'SilentlyContinue' #Hide GUI output
        $testtcpport = Test-NetConnection -ComputerName localhost -Port $TCPPort -WarningAction SilentlyContinue -ErrorAction Stop
        if ($testtcpport.TcpTestSucceeded -ne $True) {
            Write-Host ("TCP port {0} is available, continuing..." -f $TCPPort) -ForegroundColor Green
        }
        else {
            Write-Warning ("TCP Port {0} is already listening, aborting..." -f $TCPPort)
            return
        }
        #Start TCP Server
        #Used procedure from https://riptutorial.com/powershell/example/18117/tcp-listener
        $ipendpoint = new-object System.Net.IPEndPoint([ipaddress]::any, $TCPPort) 
        $listener = new-object System.Net.Sockets.TcpListener $ipendpoint
        $listener.start()
        Write-Host ("Now listening on TCP port {0}, press Escape to stop listening" -f $TCPPort) -ForegroundColor Green
        while ( $true ) {
            if ($host.ui.RawUi.KeyAvailable) {
                $key = $host.ui.RawUI.ReadKey("NoEcho,IncludeKeyUp,IncludeKeyDown")
                if ($key.VirtualKeyCode -eq 27 ) { 
                    $listener.stop()
                    Write-Host ("Stopped listening on TCP port {0}" -f $TCPPort) -ForegroundColor Green
                    return
                }
            }
        }
    }
}