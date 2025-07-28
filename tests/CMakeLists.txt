include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()
include(GoogleTest)

add_executable(
    task_scheduler_tests
    tests_scheduler.cpp
)

target_link_libraries(
    task_scheduler_tests
    GTest::gtest_main
    # GTest::gmock_main — только если реально нужен gmock
)

# Путь к заголовкам (если нужно)
target_include_directories(task_scheduler_tests PUBLIC ${PROJECT_SOURCE_DIR}/lib)

# Регистрируем тесты автоматическим способом
gtest_discover_tests(task_scheduler_tests)