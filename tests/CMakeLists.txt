# tests/CMakeLists.txt

cmake_minimum_required(VERSION 3.16)

# Название проекта для тестов
project(mini_pgw_tests)

option(BUILD_TESTS "Build unit tests" ON)

if (BUILD_TESTS)
  enable_testing()

  # Указываем директории для заголовочных файлов
  include_directories( 
    ${CMAKE_SOURCE_DIR}/include/pgw      
  )

  # Найдём все cpp файлы, начинающиеся с "test_"
  file(GLOB TEST_SOURCES ${CMAKE_SOURCE_DIR}/tests/test_*.cpp)

  # Создаем исполняемый файл для тестов
  add_executable(mini_pgw_tests ${TEST_SOURCES})

  
  target_link_libraries(mini_pgw_tests
    PRIVATE
      pgw_server_lib
      gtest
      gtest_main
      Threads::Threads
      spdlog::spdlog
      nlohmann_json::nlohmann_json
      ${SQLite3_LIBRARIES}
  )

  # Настройка автоматического запуска тестов
  include(GoogleTest)
  gtest_discover_tests(mini_pgw_tests)
endif()
