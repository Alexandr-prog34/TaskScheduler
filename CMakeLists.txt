cmake_minimum_required(VERSION 3.12)

project(
    labwork9
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)

# Подключаем директорию lib, чтобы .h файлы были доступны.
# (если нужно, можно оставить, как у вас уже было)
include_directories(lib)

# Сборка вашего основного приложения
add_subdirectory(bin)

# Включаем CTest (для запуска тестов)
enable_testing()

# Добавляем папку с нашими тестами
add_subdirectory(tests)
