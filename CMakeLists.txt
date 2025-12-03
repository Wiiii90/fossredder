cmake_minimum_required(VERSION 4.0.1)
project(FossRedder)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 20)

# Default vcpkg variables to repository-relative locations when not provided
if(NOT DEFINED VCPKG_TARGET_TRIPLET)
    set(VCPKG_TARGET_TRIPLET "x64-windows" CACHE STRING "vcpkg target triplet")
endif()
if(NOT DEFINED VCPKG_INSTALLED_DIR)
    set(VCPKG_INSTALLED_DIR "${CMAKE_SOURCE_DIR}/vcpkg_installed" CACHE PATH "vcpkg installed dir")
endif()

# Diagnostic messages to help with repository moves
message(STATUS "CMake configure: CMAKE_SOURCE_DIR=${CMAKE_SOURCE_DIR}")
message(STATUS "CMake configure: CMAKE_BINARY_DIR=${CMAKE_BINARY_DIR}")
if(EXISTS "${CMAKE_BINARY_DIR}/CMakeCache.txt")
    file(READ "${CMAKE_BINARY_DIR}/CMakeCache.txt" _cmake_cache_contents)
    string(REPLACE "\\\\" "/" _cache_norm "${_cmake_cache_contents}")
    string(REPLACE "\\" "/" _src_norm "${CMAKE_SOURCE_DIR}")
    if(NOT _cache_norm MATCHES _src_norm)
        message(WARNING "Existing CMakeCache.txt does not appear to reference the current source directory. If you moved the repository, remove the build directory and re-run CMake (or run bootstrap.ps1 -CleanBuild). This project will attempt to use repository-relative vcpkg paths by default.")
    endif()
endif()

# 🔧 Erzwinge konsistente CRT unter MSVC
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# Enable automatic Qt processing in case Qt is found by subprojects
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Do not require Qt globally here; let UI module detect Quick/Qml/Widgets as needed
find_package(Qt6 COMPONENTS Widgets QUIET)
if(TARGET Qt6::Widgets)
    message(STATUS "Top-level: Qt6::Widgets available (will be used by subprojects)")
else()
    message(STATUS "Top-level: Qt6::Widgets not found; UI module will attempt to locate Qt as needed")
endif()

# API module must be available early
add_subdirectory(api)

# Add core and debug modules early
add_subdirectory(core)
add_subdirectory(debug)

# Add service modules
add_subdirectory(services/poppler)
add_subdirectory(services/opencv)
add_subdirectory(services/tesseract)

# Add persistence and ui modules
add_subdirectory(persistence)
add_subdirectory(ui)

# Add app subdirectory
add_subdirectory(app)

# Third-party llama (moved to third_party)
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

# Setze Zielverzeichnis für die Plugins
set(QT_PLUGINS_DIR "${CMAKE_BINARY_DIR}/vcpkg_installed/x64-windows/Qt6/plugins/platforms")
set(DEPLOY_DIR "${CMAKE_BINARY_DIR}/platforms")

# Dummy LlamaModelTest post-build
if(TARGET LlamaModelTest)
    add_custom_command(TARGET LlamaModelTest POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "LlamaModelTest built"
    )
endif()