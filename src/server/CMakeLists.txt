cmake_minimum_required(VERSION 3.16)

# Поддержка потоков
find_package(Threads REQUIRED)


# Собираем библиотеку из серверных модулей
add_library(pgw_server_lib
  config.cpp
  session_manager.cpp
  udp_server.cpp
  http_api.cpp
  cdr_writer.cpp
  blacklist.cpp
  in_memory_session_store.cpp
  sqlite_session_store.cpp
)

# Указываем, где искать наши заголовки (include/pgw)
target_include_directories(pgw_server_lib
  PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${SQLite3_INCLUDE_DIRS}
)

# Линкуем внешние зависимости PUBLIC, чтобы они достались исполняемому
target_link_libraries(pgw_server_lib
  PUBLIC
    nlohmann_json::nlohmann_json
    httplib::httplib
    spdlog::spdlog
    Threads::Threads
    ${SQLite3_LIBRARIES}
)

# Создаём исполняемый файл
add_executable(pgw_server main.cpp)

# Линкуем с библиотекой — все PUBLIC-зависимости автоматически подтянутся
target_link_libraries(pgw_server
  PRIVATE
    pgw_server_lib
)
