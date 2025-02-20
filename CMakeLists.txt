cmake_minimum_required(VERSION 3.16.3)

project(reactive)

set(CMAKE_CXX_STANDARD 20)

set(PUBLIC_HEADERS
        include/reactive/Computation.h
        include/reactive/Computations.h
        include/reactive/Deferrer.h
        include/reactive/Deferrable.h
        include/reactive/Invalidateable.h
        include/reactive/Latch.h
        include/reactive/Var.h
)

add_library(reactive STATIC
        ${PUBLIC_HEADERS}
        src/Computation.cpp
        src/Computations.cpp
        src/ComputationsImpl.cpp
        src/ComputationsImpl.h
        src/Deferrer.cpp
        src/Var.cpp
)

#if(CMAKE_BUILD_TYPE MATCHES "Debug")
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
#    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
#endif()

target_include_directories(reactive PRIVATE ./src)
target_include_directories(reactive PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

if (ENABLE_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif ()