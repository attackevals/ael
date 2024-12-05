<#
.SYNOPSIS
    Deploy SQL files to a database.  Includes root level exception handling.
.DESCRIPTION
    Deploy SQL files to a database.  Includes root level exception handling.
    User must have permission to perform the steps within the sql file.
.PARAMETER ServerInstance
    The SQL server instance where the SQL will be deployed.
.PARAMETER Database
    The database where the SQL will be deployed.
.EXAMPLE
    .\deploy_sql -ServerInstance hostname -Database DiscoverBackup
#>

[Cmdletbinding()]
param(
    [Parameter(Mandatory=$true)]
    [String]$ServerInstance,
    [Parameter(Mandatory=$true)]
    [String]$Database
)

$ServerInstance = hostname
$sqlFile = ".\firefox_netbnmb_creation_with_user.sql"
$Database = "master"

$scriptFile = Get-Item $PSCommandPath;
Write-Output "$($scriptFile.Name) - Start"

Write-Output "Applying scripts to server instance '$ServerInstance' on database '$Database'" $sqlFile

Write-Output "$($MyInvocation.MyCommand) -------------------- Start --------------------"

Write-Output "Processing file: $sqlFile"
Invoke-Sqlcmd -ServerInstance $ServerInstance -Database $Database -InputFile $sqlFile
Write-Output "Completed file:  $sqlFile"

Write-Output "$($scriptFile.Name) -------------------- End --------------------"
