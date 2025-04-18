cmake_minimum_required(VERSION 3.16)
project(FossRedder)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)

# Add all source files
add_executable(fossredder
    src/main.cpp
    src/controllers/PropertyController.cpp
    src/controllers/ExpenseController.cpp
    src/controllers/PdfImportController.cpp
    src/views/ConsoleView.cpp
    src/models/Property.cpp
    src/models/Category.cpp
    src/models/Expense.cpp
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

# Use vcpkg toolchain file (adjust the path to your vcpkg installation)
set(CMAKE_TOOLCHAIN_FILE "C:/coding/.libs/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")

# Use PkgConfig to find and link Poppler
find_package(PkgConfig REQUIRED)
pkg_check_modules(POPPLER_CPP REQUIRED IMPORTED_TARGET poppler-cpp)
target_link_libraries(fossredder PRIVATE PkgConfig::POPPLER_CPP)

