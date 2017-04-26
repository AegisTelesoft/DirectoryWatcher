cmake_minimum_required(VERSION 3.7)
project(UnitTest)

add_subdirectory(DirectoryWatcher)

include_directories(
    ${CMAKE_SOURCE_DIR}/build/DirectoryWatcher/lib/include 
    ${CMAKE_SOURCE_DIR}/dependencies/include
)

if (UNIX)
    link_directories(${CMAKE_BINARY_DIR}/DirectoryWatcher/lib)
    link_directories(${CMAKE_SOURCE_DIR}/dependencies/linux/gtest)
endif (UNIX)

add_executable(UnitTest "UnitTests/Main.cpp" "UnitTests/FileSystemOperation.cpp")

if (WIN32)
    target_link_libraries(UnitTest DirectoryWatcher)
    target_link_libraries(UnitTest ${CMAKE_SOURCE_DIR}/dependencies/lib/gtest.lib)
endif (WIN32)


if (UNIX)
    target_link_libraries(UnitTest gtest gtest_main) 
    target_link_libraries(UnitTest DirectoryWatcher)
endif (UNIX)

 
