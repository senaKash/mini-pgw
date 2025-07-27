cmake_minimum_required(VERSION 3.16)

# Название проекта для тестов
project(mini_pgw_tests)

# Включаем возможность использования Google Test через FetchContent (если включены тесты)
option(BUILD_TESTS "Build unit tests" ON)

if (BUILD_TESTS)
  enable_testing()

  # Указываем директории для заголовочных файлов
  include_directories(
    ${CMAKE_SOURCE_DIR}/include  # Путь для всех заголовочных файлов проекта
    ${CMAKE_SOURCE_DIR}/include/pgw  # Путь для заголовков в папке pgw
    ${CMAKE_BINARY_DIR}/_deps/sqlite3_ext-src  # Путь к заголовочным файлам SQLite3
  )

  # Найдём все cpp файлы, начинающиеся с "test_"
  file(GLOB TEST_SOURCES ${CMAKE_SOURCE_DIR}/tests/test_*.cpp)

  # Убедитесь, что добавлены все исходные файлы для реализации
  file(GLOB SOURCES
    ${CMAKE_SOURCE_DIR}/src/server/cdr_writer.cpp
    ${CMAKE_SOURCE_DIR}/src/server/in_memory_session_store.cpp
    ${CMAKE_SOURCE_DIR}/src/server/sqlite_session_store.cpp  # Добавляем сюда
    ${CMAKE_SOURCE_DIR}/src/server/blacklist.cpp
    # добавьте другие исходные файлы, если они необходимы
  )

  # Создаем исполняемый файл для тестов, включая исходники
  add_executable(mini_pgw_tests ${TEST_SOURCES} ${SOURCES})

  # Добавляем исходники SQLite3
  target_sources(mini_pgw_tests
    PRIVATE
      ${CMAKE_BINARY_DIR}/_deps/sqlite3_ext-src/sqlite3.c  # Исходник SQLite3
  )

  # Линкуем gtest и другие глобально настроенные библиотеки
  target_link_libraries(mini_pgw_tests
    PRIVATE
      gtest
      gtest_main
      Threads::Threads
      spdlog::spdlog
      nlohmann_json::nlohmann_json
      httplib::httplib
      ${sqlite3_BINARY_DIR}/libsqlite3.a  # Линкуем с SQLite3
  )

  # Настройка автоматического запуска тестов
  include(GoogleTest)
  gtest_discover_tests(mini_pgw_tests)
endif()
