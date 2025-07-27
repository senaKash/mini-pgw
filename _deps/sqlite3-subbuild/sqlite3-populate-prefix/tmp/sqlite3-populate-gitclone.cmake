# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

if(EXISTS "/root/mini-pgw/build/_deps/sqlite3-subbuild/sqlite3-populate-prefix/src/sqlite3-populate-stamp/sqlite3-populate-gitclone-lastrun.txt" AND EXISTS "/root/mini-pgw/build/_deps/sqlite3-subbuild/sqlite3-populate-prefix/src/sqlite3-populate-stamp/sqlite3-populate-gitinfo.txt" AND
  "/root/mini-pgw/build/_deps/sqlite3-subbuild/sqlite3-populate-prefix/src/sqlite3-populate-stamp/sqlite3-populate-gitclone-lastrun.txt" IS_NEWER_THAN "/root/mini-pgw/build/_deps/sqlite3-subbuild/sqlite3-populate-prefix/src/sqlite3-populate-stamp/sqlite3-populate-gitinfo.txt")
  message(STATUS
    "Avoiding repeated git clone, stamp file is up to date: "
    "'/root/mini-pgw/build/_deps/sqlite3-subbuild/sqlite3-populate-prefix/src/sqlite3-populate-stamp/sqlite3-populate-gitclone-lastrun.txt'"
  )
  return()
endif()

execute_process(
  COMMAND ${CMAKE_COMMAND} -E rm -rf "/root/mini-pgw/build/_deps/sqlite3-src"
  RESULT_VARIABLE error_code
)
if(error_code)
  message(FATAL_ERROR "Failed to remove directory: '/root/mini-pgw/build/_deps/sqlite3-src'")
endif()

# try the clone 3 times in case there is an odd git clone issue
set(error_code 1)
set(number_of_tries 0)
while(error_code AND number_of_tries LESS 3)
  execute_process(
    COMMAND "/usr/bin/git"
            clone --no-checkout --config "advice.detachedHead=false" "https://github.com/sjinks/sqlite3-cmake.git" "sqlite3-src"
    WORKING_DIRECTORY "/root/mini-pgw/build/_deps"
    RESULT_VARIABLE error_code
  )
  math(EXPR number_of_tries "${number_of_tries} + 1")
endwhile()
if(number_of_tries GREATER 1)
  message(STATUS "Had to git clone more than once: ${number_of_tries} times.")
endif()
if(error_code)
  message(FATAL_ERROR "Failed to clone repository: 'https://github.com/sjinks/sqlite3-cmake.git'")
endif()

execute_process(
  COMMAND "/usr/bin/git"
          checkout "v3.49.1" --
  WORKING_DIRECTORY "/root/mini-pgw/build/_deps/sqlite3-src"
  RESULT_VARIABLE error_code
)
if(error_code)
  message(FATAL_ERROR "Failed to checkout tag: 'v3.49.1'")
endif()

set(init_submodules TRUE)
if(init_submodules)
  execute_process(
    COMMAND "/usr/bin/git" 
            submodule update --recursive --init 
    WORKING_DIRECTORY "/root/mini-pgw/build/_deps/sqlite3-src"
    RESULT_VARIABLE error_code
  )
endif()
if(error_code)
  message(FATAL_ERROR "Failed to update submodules in: '/root/mini-pgw/build/_deps/sqlite3-src'")
endif()

# Complete success, update the script-last-run stamp file:
#
execute_process(
  COMMAND ${CMAKE_COMMAND} -E copy "/root/mini-pgw/build/_deps/sqlite3-subbuild/sqlite3-populate-prefix/src/sqlite3-populate-stamp/sqlite3-populate-gitinfo.txt" "/root/mini-pgw/build/_deps/sqlite3-subbuild/sqlite3-populate-prefix/src/sqlite3-populate-stamp/sqlite3-populate-gitclone-lastrun.txt"
  RESULT_VARIABLE error_code
)
if(error_code)
  message(FATAL_ERROR "Failed to copy script-last-run stamp file: '/root/mini-pgw/build/_deps/sqlite3-subbuild/sqlite3-populate-prefix/src/sqlite3-populate-stamp/sqlite3-populate-gitclone-lastrun.txt'")
endif()
