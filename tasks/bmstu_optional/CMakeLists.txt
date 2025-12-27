message(STATUS "Running tasks/bmstu_optional/CMakeLists.txt")

# Отключить сканирование зависимостей если есть проблемы
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
get_filename_component(NAME_EXECUTABLE ${CMAKE_CURRENT_SOURCE_DIR} NAME)

# Найти все файлы в task_optional
file(GLOB TASK_FILES 
    "${CMAKE_CURRENT_SOURCE_DIR}/task_optional/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/task_optional/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/task_optional/*.hpp"
)

message(STATUS "Optional sources: ${TASK_FILES}")

# Создать исполняемый файл
add_executable(${NAME_EXECUTABLE} ${TASK_FILES})

# Подключить зависимости
target_link_libraries(
    ${NAME_EXECUTABLE}
    GTest::gtest_main
)

# Включить в общее тестирование
gtest_discover_tests(${NAME_EXECUTABLE})