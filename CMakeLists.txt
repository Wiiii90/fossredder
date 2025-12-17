cmake_minimum_required(VERSION 4.0.1)
project(FossRedder)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 20)

# Default vcpkg variables to repository-relative locations when not provided
if(NOT DEFINED VCPKG_TARGET_TRIPLET)
    set(VCPKG_TARGET_TRIPLET "x64-windows" CACHE STRING "vcpkg target triplet")
endif()
if(NOT DEFINED VCPKG_INSTALLED_DIR)
    # Default to the bundled third_party vcpkg installation used in this workspace
    set(VCPKG_INSTALLED_DIR "${CMAKE_SOURCE_DIR}/third_party/vcpkg/installed" CACHE PATH "vcpkg installed dir")
endif()

# Informative configure messages
message(STATUS "CMake configure: CMAKE_SOURCE_DIR=${CMAKE_SOURCE_DIR}")
message(STATUS "CMake configure: CMAKE_BINARY_DIR=${CMAKE_BINARY_DIR}")

# Ensure consistent MSVC runtime selection when using MSVC toolset
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# Enable automatic Qt processing in subprojects
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Generate QML lsp ini to help IDE/QML language server resolve imports
set(QT_QML_GENERATE_QMLLS_INI ON)

# Do not require Qt globally here; let UI module detect Quick/Qml/Widgets as needed
find_package(Qt6 COMPONENTS Widgets QUIET)
if(TARGET Qt6::Widgets)
    message(STATUS "Top-level: Qt6::Widgets available (will be used by subprojects)")
else()
    message(STATUS "Top-level: Qt6::Widgets not found; UI module will attempt to locate Qt as needed")
endif()

add_subdirectory(api)
add_subdirectory(core)
add_subdirectory(debug)
add_subdirectory(services/poppler)
add_subdirectory(services/opencv)
add_subdirectory(services/tesseract)
add_subdirectory(persistence)
add_subdirectory(ui)
add_subdirectory(app)

# Third-party llama
option(BUILD_THIRD_PARTY_LLAMACPP "Build third_party/llama.cpp subproject" OFF)
if(BUILD_THIRD_PARTY_LLAMACPP)
    if(EXISTS "${CMAKE_SOURCE_DIR}/third_party/llama.cpp/CMakeLists.txt")
        add_subdirectory(third_party/llama.cpp)
    elseif(EXISTS "${CMAKE_SOURCE_DIR}/extern/llama.cpp/CMakeLists.txt")
        add_subdirectory(extern/llama.cpp)
    endif()
else()
    message(STATUS "Skipping third_party/llama.cpp (BUILD_THIRD_PARTY_LLAMACPP=OFF)")
endif()

# Third-party packages
find_package(Tesseract QUIET CONFIG)
find_package(ICU QUIET COMPONENTS uc i18n)
find_package(nlohmann_json QUIET CONFIG)
find_package(spdlog QUIET CONFIG)
find_package(OpenCV QUIET)

# ONNX Runtime (manual link via vcpkg)
set(_ORT_ROOT "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}")
find_path(ONNXRUNTIME_INCLUDE_DIR
    NAMES onnxruntime_cxx_api.h
    PATHS "${_ORT_ROOT}/include" "${_ORT_ROOT}/include/onnxruntime"
    NO_DEFAULT_PATH)
find_library(ONNXRUNTIME_LIBRARY
    NAMES onnxruntime
    PATHS "${_ORT_ROOT}/lib" "${_ORT_ROOT}/lib/manual-link"
    NO_DEFAULT_PATH)