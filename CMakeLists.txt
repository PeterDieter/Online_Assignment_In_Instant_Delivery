cmake_minimum_required(VERSION 3.5)

project(onlineAssignment)

set(CMAKE_CXX_STANDARD 11)

# List all source files
set(SRC_FILES
    main.cpp
    Environment.cpp
    Data.cpp
)

# List all header files
set(HDR_FILES
    Environment.h
    Data.h
    Matrix.h
    xorshift128.h
)

find_package(Torch REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")


# Create an executable target
add_executable(onlineAssignment ${SRC_FILES} ${HDR_FILES})

target_link_libraries(onlineAssignment "${TORCH_LIBRARIES}")
set_property(TARGET onlineAssignment PROPERTY CXX_STANDARD 14)