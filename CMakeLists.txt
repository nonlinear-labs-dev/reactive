cmake_minimum_required(VERSION 3.16.3)

project(reactive)

set(CMAKE_CXX_STANDARD 20)

set(PUBLIC_HEADERS
    include/Computations.h
    include/Deferrer.h
    include/Latch.h
    include/Var.h
)

add_library(reactive STATIC
    ${PUBLIC_HEADERS}
    src/Computation.h
    src/Computation.cpp
    src/Computations.cpp
    src/ComputationsImpl.cpp
    src/ComputationsImpl.h
    src/Deferrable.h
    src/Deferrer.cpp
    src/Invalidateable.h
    src/Var.cpp
)

target_include_directories(reactive PUBLIC ./src ./include)

if (ENABLE_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif ()