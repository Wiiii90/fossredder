cmake_minimum_required(VERSION 3.16)
project(FossRedder)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 20)

# Add all source files
add_executable(fossredder
    src/main.cpp
    src/controllers/PropertyController.cpp
    src/controllers/ExpenseController.cpp
    src/controllers/PdfImportController.cpp
    src/models/Property.cpp
    src/models/Category.cpp
    src/models/Expense.cpp
    src/views/ConsoleView.cpp
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

# Use vcpkg toolchain file
set(CMAKE_TOOLCHAIN_FILE "C:/coding/.libs/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")

# Link Poppler
find_package(PkgConfig REQUIRED)
pkg_check_modules(POPPLER_CPP REQUIRED IMPORTED_TARGET poppler-cpp)
target_link_libraries(fossredder PRIVATE PkgConfig::POPPLER_CPP)

# Link Tesseract
find_package(Tesseract REQUIRED CONFIG)
target_link_libraries(fossredder PRIVATE Tesseract::libtesseract)
