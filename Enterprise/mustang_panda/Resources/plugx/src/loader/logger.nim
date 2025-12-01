import std/[streams, strformat, times]

var logFileStream: owned FileStream = nil
var initialized = false

proc logMsg(msg: string, prefix: string) =
    if logFileStream.isNil:
        return

    let timestamp = now().utc.format("yyyy-MM-dd'T'HH:mm:ss")
    let combined = &"[{timestamp}] {prefix} {msg}\n"
    logFileStream.write(combined)
    logFileStream.flush()

proc LogDebug*(msg: string) =
    logMsg(msg, "[DEBUG]")

proc LogInfo*(msg: string) =
    logMsg(msg, " [INFO]")

proc LogWarn*(msg: string) =
    logMsg(msg, " [WARN]")

proc LogError*(msg: string) =
    logMsg(msg, "[ERROR]")

proc InitializeLogger*(filename: string): bool =
    if initialized:
        return initialized

    logFileStream = newFileStream(filename, fmAppend)
    if logFileStream.isNil:
        return false
    else:
        initialized = true
        LogInfo("==================")
        LogInfo("Initialized logger")
        LogInfo("==================")
        return true

proc CloseLogger*() =
    if not logFileStream.isNil:
        logFileStream.close()
