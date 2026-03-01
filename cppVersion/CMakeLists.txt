cmake_minimum_required(VERSION 3.15)
project(DUNEStorageBenchmark CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add executable
add_executable(dune_benchmark
    src/main.cpp
    src/DataGenerator.cpp
    src/StorageStrategy.cpp
    src/SingleFileStrategy.cpp
    src/ChunkedFileStrategy.cpp
    src/IndividualFileStrategy.cpp
    src/BenchmarkTimer.cpp
    src/SystemUtils.cpp
    src/DataValidator.cpp
)

target_include_directories(dune_benchmark PRIVATE src)

# Enable optimizations for release builds
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(MSVC)
        target_compile_options(dune_benchmark PRIVATE /O2 /GL)
        target_link_options(dune_benchmark PRIVATE /LTCG)
    else()
        target_compile_options(dune_benchmark PRIVATE -O3 -march=native)
    endif()
endif()
