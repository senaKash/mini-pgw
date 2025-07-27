# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/root/mini-pgw/build/_deps/sqlite3_ext-src"
  "/root/mini-pgw/build/_deps/sqlite3_ext-build"
  "/root/mini-pgw/build/_deps/sqlite3_ext-subbuild/sqlite3_ext-populate-prefix"
  "/root/mini-pgw/build/_deps/sqlite3_ext-subbuild/sqlite3_ext-populate-prefix/tmp"
  "/root/mini-pgw/build/_deps/sqlite3_ext-subbuild/sqlite3_ext-populate-prefix/src/sqlite3_ext-populate-stamp"
  "/root/mini-pgw/build/_deps/sqlite3_ext-subbuild/sqlite3_ext-populate-prefix/src"
  "/root/mini-pgw/build/_deps/sqlite3_ext-subbuild/sqlite3_ext-populate-prefix/src/sqlite3_ext-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/root/mini-pgw/build/_deps/sqlite3_ext-subbuild/sqlite3_ext-populate-prefix/src/sqlite3_ext-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/root/mini-pgw/build/_deps/sqlite3_ext-subbuild/sqlite3_ext-populate-prefix/src/sqlite3_ext-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
