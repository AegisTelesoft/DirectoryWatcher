cmake_minimum_required(VERSION 3.2)
project(UnitTest)

set(CMAKE_CXX_STANDARD 14)

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
    target_link_libraries(UnitTest ${CMAKE_SOURCE_DIR}/dependencies/win32/gtest/gtest.lib)
endif (WIN32)

if (UNIX)
    target_link_libraries(UnitTest gtest gtest_main) 
    target_link_libraries(UnitTest DirectoryWatcher)
endif (UNIX)

get_target_property(DirectoryWatcherType DirectoryWatcher TYPE)
if(DirectoryWatcherType STREQUAL "SHARED_LIBRARY")
add_custom_command(TARGET UnitTest POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        $<TARGET_FILE:DirectoryWatcher> $<TARGET_FILE_DIR:UnitTest>
                    COMMENT "Copying DirectoryWatcher shared lib alongside UnitTest."
                    VERBATIM)
endif()

