cmake_minimum_required(VERSION 2.8)

project(SubstringSearcher)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS: "${CMAKE_CXX_FLAGS} -Wall -std=c++17 -pedantic")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address,undefined -D_GLIBCXX_DEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

find_package(Qt5Widgets REQUIRED)

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/src/interface)
include_directories(${CMAKE_SOURCE_DIR}/src/library)

add_subdirectory(src/interface)
add_subdirectory(src/library)

add_executable(Searcher
        src/main.cpp)

target_link_libraries(Searcher
        SearcherLib
        SearcherUI)

qt5_use_modules(Searcher Widgets)

