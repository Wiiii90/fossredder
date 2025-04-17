cmake_minimum_required(VERSION 3.16)
project(FossRedder)

set(CMAKE_CXX_STANDARD 17)

# Add all source files
add_executable(fossredder
    src/main.cpp
    src/controllers/PropertyController.cpp
    src/views/ConsoleView.cpp
    src/models/Property.cpp
    src/models/Category.cpp
)

# Set the output directory for binaries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/build)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/build)

# Include directories
target_include_directories(fossredder PRIVATE
    include
    include/models
    include/controllers
    include/views
)
