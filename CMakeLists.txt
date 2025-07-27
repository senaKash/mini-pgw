cmake_minimum_required(VERSION 3.16)
project(mini-pgw VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# -----------------------------
# 1) FetchContent helper
# -----------------------------
include(FetchContent)

# -----------------------------
# 2) nlohmann::json
# -----------------------------
find_package(nlohmann_json 3.11.2 QUIET)
if (NOT nlohmann_json_FOUND)
  FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.2
  )
  FetchContent_MakeAvailable(nlohmann_json)
endif()

# -----------------------------
# 3) cpp-httplib (header-only)
# -----------------------------
find_package(cpp-httplib 0.23.1 QUIET)
if (NOT cpp_httplib_FOUND)
    FetchContent_Declare(
        cpp_httplib
        GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
        GIT_TAG v0.23.1
    )
    FetchContent_MakeAvailable(cpp_httplib)
endif()

# -----------------------------
# 4) spdlog
# -----------------------------
find_package(spdlog 1.11.0 QUIET)
if (NOT spdlog_FOUND)
  FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.15.3
  )
  FetchContent_MakeAvailable(spdlog)
endif()

# -----------------------------
# 5) Threads (для std::thread / pthread)
# -----------------------------
find_package(Threads REQUIRED)

# -----------------------------
# 6) googletest (для тестов)
# -----------------------------
option(BUILD_TESTS "Build unit tests" ON)
if (BUILD_TESTS)
  find_package(GTest 1.14.0 QUIET)
  if (NOT GTest_FOUND)
    FetchContent_Declare(
      googletest
      GIT_REPOSITORY https://github.com/google/googletest.git
      GIT_TAG release-1.14.0
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
  endif()
  enable_testing()
endif()

# -----------------------------
# 7) SQLite3
# -----------------------------
#FetchContent_Declare(
#  sqlite3
#  GIT_REPOSITORY https://github.com/sjinks/sqlite3-cmake.git
#  GIT_TAG v3.49.1
#)
#FetchContent_MakeAvailable(sqlite3)
find_package(SQLite3 REQUIRED)
message(STATUS "Using system SQLite3: ${SQLite3_INCLUDE_DIRS} / ${SQLite3_LIBRARIES}")

# -----------------------------
# 8) Добавление подпроектов
# -----------------------------
add_subdirectory(src/server)
add_subdirectory(src/client)
add_subdirectory(tests)

