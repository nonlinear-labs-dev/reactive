cmake_minimum_required(VERSION 3.16.3)

if(NOT TARGET Catch2WithMain)
    Include(FetchContent)

    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.4.0 # or a later release
    )

    FetchContent_MakeAvailable(Catch2)
    list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
    include(CTest)
    include(Catch)
endif()

add_executable(reactive-tests
    LatchTests.cpp
    ReactiveTests.cpp
)

target_include_directories(reactive-tests PUBLIC ${CATCH_LIB_INCLUDE_DIRS} ./ ../src ../include)
target_link_libraries(reactive-tests PRIVATE reactive Catch2::Catch2WithMain)
catch_discover_tests(reactive-tests)
