cmake_minimum_required(VERSION 3.7)
project(UnitTest)

add_subdirectory(DirectoryWatcher)

add_executable(unitTest "UnitTests/main.cpp" "UnitTests/FileSystemOperation.cpp")

target_include_directories(unitTest PRIVATE "${PROJECT_BINARY_DIR}")
include_directories(${CMAKE_SOURCE_DIR}/dependencies/include)

target_link_libraries(unitTest ${CMAKE_SOURCE_DIR}/dependencies/lib/gtest.lib)
target_link_libraries(unitTest DirectoryWatcher)

get_target_property(DirectoryWatcherType DirectoryWatcher TYPE)
if(DirectoryWatcherType STREQUAL "SHARED_LIBRARY")
  add_custom_command(TARGET unitTest POST_BUILD
                     COMMAND ${CMAKE_COMMAND} -E copy_if_different
                         $<TARGET_FILE:DirectoryWatcher> $<TARGET_FILE_DIR:unitTest>
                     COMMENT "Copying DirectoryWatcher shared lib alongside unitTest."
                     VERBATIM)
endif()