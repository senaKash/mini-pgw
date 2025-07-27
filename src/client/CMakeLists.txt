# src/client/CMakeLists.txt

# Собираем библиотеку клиента
add_library(pgw_client_lib
  client.cpp
)

# Делаем include-директорию (общие заголовки) доступной и клиенту, и тому, кто будет линковаться с pgw_client_lib
target_include_directories(pgw_client_lib
  PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src/client
)

# Линкуем внешние библиотеки: эти интерфейс‑таргеты прокидывают свои include‑директории
target_link_libraries(pgw_client_lib
  PUBLIC   # PUBLIC, чтобы include‑dirs nlohmann и spdlog пробросились дальше
    nlohmann_json::nlohmann_json
    spdlog::spdlog
    Threads::Threads
)

# Создаём исполняемый файл
add_executable(pgw_client main.cpp)

# Напишите, что pgw_client зависит от библиотеки pgw_client_lib
target_link_libraries(pgw_client
  PRIVATE
    pgw_client_lib
)

