# Configuration options for LockBit

# Configure stubbed dependencies for a given component. Can be set to ON or
# OFF via environment variables.
option(LOCKBIT_TEST_CONFIG "Use the test configuration when buiding LockBit" OFF)
option(LOCKBIT_DEBUG_LOGGING "Enable debug logging when building LockBit" ON)

# Get locations for resources (plaintext and XORed)
if (LOCKBIT_TEST_CONFIG)
    message(STATUS "[LockBit] Using test configuration")
    set(ConfigResourcePath "${CMAKE_SOURCE_DIR}/config/test_config.json")
else ()
    message(STATUS "[LockBit] Using release configuration")
    set(ConfigResourcePath "${CMAKE_SOURCE_DIR}/config/config.json")
endif()
unset(LOCKBIT_TEST_CONFIG CACHE)

set(ResourceXorKeyHex "21be1e89c74b5641e6de86bc1f828ae505baab5b84c19411b0a0587c28f0eab1f77fa79774d19816a9ebaec7d265d1629cbb43eb3d0d8156182b08bb5d57f3a0b06ea7b8696fff3615d78a4aa89c58e5e549714b3daaf1732a0f307ab0545fd9cab2949a2056904c6be6a32dbbae92979cad1f142de9016ca5826ed79bcd1032")
set(ConfigResourcePathXor "${ConfigResourcePath}.xor")
set(IconResourcePath "${CMAKE_SOURCE_DIR}/resources/icon.ico")
set(IconResourcePathXor "${IconResourcePath}.xor")
set(WallpaperResourcePath "${CMAKE_SOURCE_DIR}/resources/desktop.bmp")
set(WallpaperResourcePathXor "${WallpaperResourcePath}.xor")
set(UnitTestConfigResourcePath "${CMAKE_SOURCE_DIR}/config/unit_test_config.json")
set(UnitTestConfigResourcePathXor "${UnitTestConfigResourcePath}.xor")

# XOR resources
execute_process(
    COMMAND powershell.exe -file xor_file.ps1 -key "${ResourceXorKeyHex}" -infile "${ConfigResourcePath}" -outfile "${ConfigResourcePathXor}"
    COMMAND powershell.exe -file xor_file.ps1 -key "${ResourceXorKeyHex}" -infile "${UnitTestConfigResourcePath}" -outfile "${UnitTestConfigResourcePathXor}"
    COMMAND powershell.exe -file xor_file.ps1 -key "${ResourceXorKeyHex}" -infile "${IconResourcePath}" -outfile "${IconResourcePathXor}"
    COMMAND powershell.exe -file xor_file.ps1 -key "${ResourceXorKeyHex}" -infile "${WallpaperResourcePath}" -outfile "${WallpaperResourcePathXor}"
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/src"
)
