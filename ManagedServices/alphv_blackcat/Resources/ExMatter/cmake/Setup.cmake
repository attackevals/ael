include(FetchContent)

# Setup ConfuserEx2
set(confuser-TAG "v1.6.0") 
message(STATUS "Downloading ConfuserEx (${confuser-TAG})")

FetchContent_Declare(
	confuser
	URL "https://github.com/mkaring/ConfuserEx/releases/download/${confuser-TAG}/ConfuserEx-CLI.zip"
	URL_HASH MD5=8482B6A5CC4C708947131C84E2110934
	CONFIGURE_COMMAND ""
	BUILD_COMMAND ""
)
FetchContent_MakeAvailable(confuser)

# Download ILMerge
message(STATUS "Downloading ILMerge")

FetchContent_Declare(
  ilmerge
  GIT_REPOSITORY https://github.com/dotnet/ILMerge.git
  GIT_TAG master
)
FetchContent_MakeAvailable(ilmerge)

# create directories for ILMerge to place merged exe
file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/ExMatter/Debug")
file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/ExMatter/Release")