cmake_minimum_required(VERSION 3.7)
project(Test)

add_subdirectory(DirectoryWatcher)

add_executable(test "UnitTests/FileSystemOperation.cpp")

target_include_directories(test PRIVATE "${PROJECT_BINARY_DIR}")

target_link_libraries(test DirectoryWatcher)

get_target_property(DirectoryWatcherType DirectoryWatcher TYPE)
if(DirectoryWatcherType STREQUAL "SHARED_LIBRARY")
  add_custom_command(TARGET test POST_BUILD
                     COMMAND ${CMAKE_COMMAND} -E copy_if_different
                         $<TARGET_FILE:DirectoryWatcher> $<TARGET_FILE_DIR:test>
                     COMMENT "Copying DirectoryWatcher shared lib alongside test."
                     VERBATIM)
endif()