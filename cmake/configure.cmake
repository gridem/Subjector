# Copyright 2015-2017 Grigory Demchenko (aka gridem)
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

option(BUILD_TESTS "Build unit tests" ON)
option(STATIC_ALL "Use static libraries" ON)
option(LOG_DEBUG "Use debug output" OFF)

if(LOG_DEBUG)
    add_definitions(-DflagLOG_DEBUG)
endif()

if("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU" OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    set(GCC_LIKE_COMPILER ON)
endif()

if(NOT MSVC AND NOT GCC_LIKE_COMPILER)
    message(FATAL_ERROR "Supports only gcc, clang or msvc compilers")
endif()

if(STATIC_ALL)
    set(Boost_USE_STATIC_LIBS ON)
    set(Boost_USE_STATIC_RUNTIME ON)
    if(MSVC)
        set(MSVC_RUNTIME static)
    endif()
    if(GCC_LIKE_COMPILER)
        set(CMAKE_EXE_LINKER_FLAGS "-static -Wl,--whole-archive -lpthread -Wl,--no-whole-archive")
    endif()
else()
    set(Boost_USE_STATIC_LIBS ON)
    set(Boost_USE_STATIC_RUNTIME ON)
    if(MSVC)
        set(MSVC_RUNTIME dynamic)
    endif()
    if(GCC_LIKE_COMPILER)
        set(CMAKE_EXE_LINKER_FLAGS)
    endif()
endif()

if(MSVC)
    include(cmake/MSVCRuntime.cmake)
    configure_msvc_runtime()
endif()

if(GCC_LIKE_COMPILER)
    add_definitions(-std=c++1z -pthread)
endif()

include(CMakeParseArguments)

function(use_boost_libraries USE_BOOST_TARGET)
    cmake_parse_arguments(USE_BOOST "" "" LIBS ${ARGN})

    set(Boost_USE_MULTITHREADED ON)
    find_package(Boost 1.62 REQUIRED COMPONENTS ${USE_BOOST_LIBS})
    target_include_directories(${USE_BOOST_TARGET} PRIVATE ${Boost_INCLUDE_DIR})
    target_link_libraries(${USE_BOOST_TARGET} ${Boost_LIBRARIES})
endfunction()

function(use_boost_asio TARGET)
    if(MINGW)
        target_link_libraries(${TARGET} Ws2_32 Mswsock)
    endif()
    if(MSVC)
        add_definitions(-D_WIN32_WINNT=0x0600)
    endif()
    add_definitions(-DBOOST_DATE_TIME_NO_LIB -DBOOST_REGEX_NO_LIB)
endfunction()
