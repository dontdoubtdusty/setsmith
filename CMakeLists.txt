cmake_minimum_required(VERSION 3.13)

project(setsmith VERSION 2.5.6)
add_definitions(-DUNOFFICIAL_BUILD)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(wxWidgets 3 REQUIRED COMPONENTS core base)
find_package(Boost REQUIRED COMPONENTS regex)

find_package(PkgConfig)

# find hunspell
pkg_check_modules(HUNSPELL hunspell)
if( NOT HUNSPELL_FOUND )
	message("-- Cannot find Hunspell via pkg-config, checking directly...")
	find_path(HUNSPELL_INCLUDE_DIRS hunspell.hxx)
	message("-- HUNSPELL_INCLUDE_DIRS = ${HUNSPELL_INCLUDE_DIRS}")
	find_library(HUNSPELL_LIBRARIES NAMES hunspell libhunspell)
	message("-- HUNSPELL_LIBRARIES = ${HUNSPELL_LIBRARIES}")
	if ( ${HUNSPELL_INCLUDE_DIRS} STREQUAL "HUNSPELL_INCLUDE_DIRS-NOTFOUND" )
		message(FATAL_ERROR "Hunspell cannot be found")
	else()
		message("-- Found Hunspell at ${HUNSPELL_LIBRARIES}")
	endif()
endif()

# You will most likely get a message about being unable to open hunspell-1.7.lib because pkgconf forgets to add the actual path to
# HUNSPELL_LIBRARIES. If so, uncomment the below line and point it to the correct vcpkg root folder/library.
#set(HUNSPELL_LIBRARIES "C:\\PATH\\TO\\ROOT\\vcpkg\\installed\\${VCPKG_TARGET_TRIPLET}\\lib\\hunspell-1.7.lib")
message("-- Does this have a full path? If not, and it's just a file name, it's broken: Found Hunspell at ${HUNSPELL_LIBRARIES}")

include_directories("${PROJECT_BINARY_DIR}/src")
include_directories("${PROJECT_SOURCE_DIR}/src")
include(${wxWidgets_USE_FILE})
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${HUNSPELL_INCLUDE_DIRS})

# Setsmith executable

add_executable(setsmith WIN32)
target_link_libraries(${PROJECT_NAME} ${wxWidgets_LIBRARIES})
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})
target_link_libraries(${PROJECT_NAME} ${HUNSPELL_LIBRARIES})
if(WIN32)
  # Theme API used by src/gui/util.cpp. MSVC wxWidgets links it automatically; MinGW does not.
  target_link_libraries(${PROJECT_NAME} uxtheme)
endif()
if(MINGW)
  # GCC builds load GUI images from resource/ next to the exe instead of embedding them
  # (see load_resource_image in src/gui/util.cpp), so copy the folder after each build.
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND
    ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/resource" "${PROJECT_BINARY_DIR}/resource"
  )
endif()

file(GLOB_RECURSE sources src/*.cpp)
list(FILTER sources EXCLUDE REGEX win32_cli_wrapper.cpp)
target_sources(setsmith PRIVATE ${sources})
target_precompile_headers(setsmith PRIVATE src/util/prec.hpp)

configure_file(src/config.hpp.in src/config.hpp)

# resource file

target_sources(setsmith PRIVATE resource/win32_res.rc)

# setsmith.com: wrapper to enable command line executable on windows

if(WIN32)
  add_executable(setsmith-cli)
  target_sources(setsmith-cli PRIVATE src/cli/win32_cli_wrapper.cpp)
  add_custom_command(TARGET setsmith-cli POST_BUILD COMMAND
    ${CMAKE_COMMAND} -E copy "${PROJECT_BINARY_DIR}/setsmith-cli.exe" "${PROJECT_BINARY_DIR}/setsmith.com"
  )
endif()


# warnings
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  # Update if necessary
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wsuggest-override -Wstrict-null-sentinel -Wno-comment -Wno-unused-parameter")
endif()

# Debug Message
message("-- Does this have a full path? If not, and it's just a file name, it's broken: Found Hunspell at ${HUNSPELL_LIBRARIES}")
