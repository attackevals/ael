###############################################################################
# PLUG X LOADER
###############################################################################

###############################################################################
# Build loader DLL
#
# Example Nim CMakeLists file used for reference:
# https://github.com/stoneface86/example-nim-cmake/blob/master/src/nim/CMakeLists.txt
###############################################################################
# Use MSVC as the compiler
set(NIM_CC "--cc:vcc")

# name of the project file to build, in this case it is our fib module
set_property(GLOBAL PROPERTY NIM_NAME "libcurl")
get_property(NIM_NAME GLOBAL PROPERTY NIM_NAME)
set(NIM_SRC "${NIM_NAME}.nim")
# if you have more modules other than NIM_PROJECT, include them here so
# that the library will be rebuilt when you make changes to them.
set(NIM_DEPS "")

# set the nimcache in our build folder
set(NIMCACHE "${CMAKE_CURRENT_BINARY_DIR}/nimcache")

set(NIM_LIB "${CMAKE_STATIC_LIBRARY_PREFIX}${NIM_NAME}$<$<CONFIG:Debug>:_debug>.dll")
set_property(GLOBAL PROPERTY NIM_LIB_PATH "${CMAKE_CURRENT_BINARY_DIR}/${NIM_LIB}")
get_property(NIM_LIB_PATH GLOBAL PROPERTY NIM_LIB_PATH)
set(NIM_PDB --debugger:native)
set_property(GLOBAL PROPERTY NIM_PDB_PATH ${NIM_NAME}$<$<CONFIG:Debug>:_debug>.pdb)
set(NIM_RELEASE $<$<CONFIG:Release>:-d:release>)

# build the library via nimble
add_custom_command(
    OUTPUT "${NIM_LIB_PATH}"
    COMMAND nim c --app:lib --nomain ${NIM_CC} --cpu:amd64 ${NIM_PDB}
    --out:"${NIM_LIB_PATH}" --outdir:"${CMAKE_CURRENT_BINARY_DIR}" --nimcache:"${NIMCACHE}"
    ${NIM_RELEASE} "${NIM_SRC}"
    DEPENDS "${NIM_SRC}" "${NIM_DEPS}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)
add_custom_target(
    libcurl
    DEPENDS "${NIM_LIB_PATH}"
    COMMENT "Building Nim DLL"
)
