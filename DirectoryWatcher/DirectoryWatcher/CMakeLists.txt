cmake_minimum_required(VERSION 3.7)
project(DirectoryWatcher)

SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

add_library(DirectoryWatcher SHARED 
    DirectoryWatcher.cpp 
    DirectoryWatcher.h 
    CancelationToken.cpp  
    CancelationToken.h
    StatusFlag.h
    StatusFlag.tpp
)

target_include_directories(DirectoryWatcher PUBLIC ${PROJECT_SOURCE_DIR} ${LIBRARY_OUTPUT_PATH})

file(INSTALL DirectoryWatcher.h DESTINATION "${LIBRARY_OUTPUT_PATH}/include")