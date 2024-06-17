cmake_minimum_required(VERSION 3.24)
project(InfoStealer VERSION 1.0.0 LANGUAGES CSharp)

set(CMAKE_SKIP_INSTALL_RULES TRUE)

execute_process(COMMAND dotnet restore InfoStealer.sln)
execute_process(COMMAND dotnet publish InfoStealer.sln -r win-x86 -p:PlatformTarget=x86 -p:PublishSingleFile=true --sc false -c Release -p:IncludeNativeLibrariesForSelfExtract=true)
