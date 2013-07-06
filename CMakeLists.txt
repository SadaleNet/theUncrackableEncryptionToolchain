if(NOT UNIX)
	message(FATAL_ERROR "Sorry, this cmake file is for unix only. To build for Windows, please use The Uncrackable Encryption Toolchain [WINDOWS].cbp")
endif(NOT UNIX)

cmake_minimum_required (VERSION 2.6)
project (TheUncrackableEncryptionToolchain)

include("LibFindMacros.cmake")

# Initialize CXXFLAGS.
set(CMAKE_CXX_FLAGS "-Wall -std=c++0x")


# The version number.
set (TheUncrackableEncryptionToolchain_VERSION_MAJOR 1)
set (TheUncrackableEncryptionToolchain_VERSION_MINOR 0)

# add the binary tree to the search path for include files
include_directories("${PROJECT_SOURCE_DIR}")

#external linking

# Use pkg-config to get hints about paths
libfind_pkg_check_modules(botan-1.10_PKGCONF botan-1.10)

# Include dir
find_path(botan-1.10_INCLUDE_DIR NAMES botan.h PATHS ${botan-1.10_PKGCONF_INCLUDE_DIRS} )

libfind_process(botan-1.10)

include_directories(${botan-1.10_PKGCONF_INCLUDE_DIRS})
include_directories(boost)


macro (addExecutable binDir name)
	add_executable(${binDir}${name} ${name}.cpp)
	target_link_libraries(${binDir}${name} botan-1.10)
	target_link_libraries(${binDir}${name} boost_regex-mt)
	target_link_libraries(${binDir}${name} boost_system)
	target_link_libraries(${binDir}${name} boost_filesystem)
endmacro (addExecutable)

set(binDir "./build/")

# add the executables
addExecutable(${binDir} dummy)
addExecutable(${binDir} crypter)
addExecutable(${binDir} injector)
addExecutable(${binDir} redate)
	target_link_libraries(${binDir}redate rt)
addExecutable(${binDir} nuke)
addExecutable(${binDir} testCases)


set(wxWidgets_USE_LIBS)
find_package(wxWidgets)
include("${wxWidgets_USE_FILE}")
add_executable(${binDir}GUI GUIApp.cpp GUI.cpp)
# and for each of your dependant executable/library targets:
target_link_libraries(${binDir}GUI ${wxWidgets_LIBRARIES})
target_link_libraries(${binDir}GUI boost_system)

