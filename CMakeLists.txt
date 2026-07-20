cmake_minimum_required(VERSION 3.16)

project(NanoMatch VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(include)

add_executable(NanoMatch
    src/main.cpp
    src/orderbook.cpp
    src/FlatHashMap.cpp
    src/FastIngestionPipeline.cpp
)

add_executable(NanoBenchmark
    src/benchmark.cpp
    src/orderbook.cpp
    src/FastIngestionPipeline.cpp
    src/FlatHashMap.cpp
)

add_executable(Generator
    src/ordergenerator.cpp
)
if(MSVC)
    target_compile_options(NanoMatch PRIVATE /O2)
else()
    target_compile_options(NanoMatch PRIVATE -O3 -Wall -Wextra)
endif()