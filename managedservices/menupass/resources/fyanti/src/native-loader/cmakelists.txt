add_library(
	native-loader SHARED
	"main.cpp"
	"error.cpp"
	"string.cpp"
)

add_dependencies(
    native-loader
    managed-loader-header
)

target_link_options(
	native-loader PRIVATE
	/PDBALTPATH:test.pdb
)

target_link_libraries(
	native-loader PRIVATE
	managed-loader-header
)

target_include_directories(
  native-loader
  INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}"
)

add_srdi_target(
	native-loader-srdi
	native-loader
	execute_assembly
)
