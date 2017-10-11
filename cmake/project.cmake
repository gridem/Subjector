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

macro(my_project MY_PROJECT)
    project(${MY_PROJECT} CXX C)
    include(cmake/configure.cmake)

    file(GLOB_RECURSE ${MY_PROJECT}_hdrs include/*)
    file(GLOB_RECURSE ${MY_PROJECT}_srcs src/*)

    add_library(${MY_PROJECT} ${${MY_PROJECT}_hdrs} ${${MY_PROJECT}_srcs})
    target_include_directories(${MY_PROJECT} PUBLIC include)

    if(BUILD_TESTS)
        add_subdirectory(tests)
    endif()
endmacro()

macro(add_uts)
    enable_testing()

    file(GLOB ut_srcs *.cpp *.h)

    add_executable(tests ${ut_srcs})
    target_link_libraries(tests ${PROJECT_NAME})
    use_boost_libraries(tests LIBS unit_test_framework)
    add_test(tests tests)
endmacro()
