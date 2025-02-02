#
# D++ (DPP), The Lightweight C++ Discord Library
#
# Copyright 2021 Craig Edwards <support@brainbox.cc>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

cmake_minimum_required (VERSION 3.12)

if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
      CACHE STRING "")
endif()

include("cmake/colour.cmake")

file(READ ${CMAKE_CURRENT_SOURCE_DIR}/include/dpp/version.h version_h)
if (NOT version_h MATCHES "DPP_VERSION_SHORT ([0-9][0-9])([0-9][0-9])([0-9][0-9])")
	message(FATAL_ERROR "Cannot get DPP_VERSION_SHORT from version.h")
endif ()
math(EXPR DPP_VERSION_MAJOR ${CMAKE_MATCH_1})
math(EXPR DPP_VERSION_MINOR ${CMAKE_MATCH_2})
math(EXPR DPP_VERSION_PATCH ${CMAKE_MATCH_3})
string(CONCAT DPP_VERSION ${DPP_VERSION_MAJOR}.${DPP_VERSION_MINOR}.${DPP_VERSION_PATCH})


project(libdpp VERSION ${DPP_VERSION} LANGUAGES CXX)

include("cmake/FindSodium.cmake")
include("cmake/FindOpus.cmake")

if (DEFINED OPUS_FOUND)
	if (DEFINED sodium_VERSION_STRING)
		add_compile_definitions(HAVE_VOICE)
		message("-- Sodium ${Green}${sodium_VERSION_STRING}${ColourReset}")
		set(HAVE_VOICE 1)
	endif()
endif()

if (HAVE_VOICE)
	message("-- Detected ${Green}libsodium${ColourReset} and ${Green}libopus${ColourReset}. VOICE support will be ${Green}enabled${ColourReset}")
else(HAVE_VOICE)
	message("-- Could not detect ${Green}libsodium${ColourReset} and/or ${Green}libopus${ColourReset}. VOICE support will be ${Red}disabled${ColourReset}")
endif(HAVE_VOICE)

if (WIN32)
	link_libraries("C:\\vckpg\\installed\\x64-windows\\lib\\libssl.lib")
	link_libraries("C:\\vckpg\\installed\\x64-windows\\lib\\libcrypto.lib")
	link_libraries("C:\\vckpg\\installed\\x64-windows\\lib\\zlib.lib")
	include_directories("C:\\vckpg\\installed\\x64-windows\\include")
	set(ZLIB_LIBRARY "C:\\vckpg\\installed\\x64-windows\\lib")
	set(ZLIB_INCLUDE_DIR "C:\\vckpg\\installed\\x64-windows\\include")
endif()

aux_source_directory("src" coresrc)

add_executable(test ${coresrc})

string(ASCII 27 Esc)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)

target_compile_features(test PRIVATE cxx_std_17)

include_directories( "include" )

target_link_libraries(test)

find_package(Git QUIET)

if(NOT GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
	message(FATAL_ERROR "You are using a git version of D++ but do not have git installed. Install git (not 'gh') and try again.")
endif()

if (EXISTS "${PROJECT_SOURCE_DIR}/.git")
	message("-- Building ${Green}git${ColourReset} version. ${Green}Be aware git versions may be unstable!${ColourReset}")
else()
	message("-- Building ${Green}stable${ColourReset} version ${Green}${DPP_VERSION}${ColourReset}")
endif()

if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
# Update submodules as needed
        message(STATUS "Submodule update")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                        RESULT_VARIABLE GIT_SUBMOD_RESULT)
        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
        endif()
endif()

if(NOT EXISTS "${PROJECT_SOURCE_DIR}/deps/json/CMakeLists.txt" OR NOT EXISTS "${PROJECT_SOURCE_DIR}/deps/fmt/CMakeLists.txt")
    message(FATAL_ERROR "The submodules were not downloaded! Please update submodules and try again.")
endif()

if (UNIX OR MSYS)
	find_program(LDCONFIG_EXECUTABLE "ldconfig")
endif()

set(BUILD_SHARED_LIBS 1 CACHE BOOL "Build fmt::format as a shared library")
add_subdirectory(deps/fmt)
add_subdirectory(deps/json)

include_directories("deps/fmt/include")
include_directories("deps/json/single_include")
if (WIN32)
	include_directories("C:/Program Files/OpenSSL-Win64/include")
endif (WIN32)

execute_process(COMMAND "g++ -c deps/json/single_include/nlohmann/json.hpp -o deps/json/single_include/nlohmann/json.hpp.gch")

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -pthread -g -O2 -fPIC -rdynamic")
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O2")

set (modules_dir "src")
file(GLOB subdirlist ${modules_dir}/dpp)
foreach (fullmodname ${subdirlist})
        get_filename_component(modname ${fullmodname} NAME)
        set (modsrc "")
	file(GLOB modsrc ${modules_dir}/dpp/*.cpp ${modules_dir}/dpp/events/*.cpp)
        add_library(${modname} SHARED ${modsrc})
if (WIN32)
	target_link_libraries(${modname} PUBLIC nlohmann_json::nlohmann_json fmt "C:\\vckpg\\installed\\x64-windows\\lib\\libssl.lib" "C:\\vckpg\\installed\\x64-windows\\lib\\libcrypto.lib" "C:\\vckpg\\installed\\x64-windows\\lib\\zlib.lib")
else()
	target_link_libraries(${modname} PUBLIC ssl crypto nlohmann_json::nlohmann_json fmt z)
endif()
if (HAVE_VOICE)
	target_link_libraries(${modname} PUBLIC sodium opus)
endif()
endforeach(fullmodname)

target_compile_features(dpp PRIVATE cxx_std_17)
target_link_libraries(test PUBLIC dpp fmt)

# message("Type ${Green}make -j${ColourReset} to build D++ then ${Green}sudo make install${ColourReset}")

add_custom_command(TARGET dpp
	COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/PostBuild.cmake)

# Installation

include(GNUInstallDirs)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/deps/fmt/include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/deps/json/include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libdpp.so DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(TARGETS fmt
 ARCHIVE DESTINATION lib
 LIBRARY DESTINATION lib
 RUNTIME DESTINATION bin
 COMPONENT fmt
)
install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -DLDCONFIG_EXECUTABLE=${LDCONFIG_EXECUTABLE} -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/PostInstall.cmake)")

