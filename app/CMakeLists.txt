cmake_minimum_required(VERSION 4.0.1)
project(fossredder_app LANGUAGES CXX)

# Build option: enable the QML UI. Default OFF for headless/CI builds;
# will be auto-enabled below if Qt Quick/Qml is found on the system.
option(USE_QML "Build with QML UI" OFF)

# Detect Qt early so we can auto-enable QML support and conditionally
# include the QML-only translation unit (main_qml.cpp).
find_package(Qt6 COMPONENTS Quick Qml Widgets QuickWidgets QUIET)
if(TARGET Qt6::Quick AND TARGET Qt6::Qml)
    if(NOT USE_QML)
        set(USE_QML ON CACHE BOOL "Build with QML UI" FORCE)
    endif()
    message(STATUS "Qt6 Quick/Qml found; enabling QML support")
else()
    if(USE_QML)
        message(WARNING "USE_QML is ON but Qt Quick/Qml not found; build may fail or QML will be unavailable.")
    endif()
endif()

# Collect all source files under src/. If QML support is disabled, remove the
# QML-specific main translation unit so the target does not depend on Qt Quick.
file(GLOB_RECURSE APP_SRCS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
if(NOT USE_QML)
    list(REMOVE_ITEM APP_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/src/main_qml.cpp)
endif()

add_executable(fossredder ${APP_SRCS})

# Enable Qt meta-object tools and resource handling for this target
set_target_properties(fossredder PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)

target_compile_features(fossredder PUBLIC cxx_std_20)

if(MSVC)
    # Ensure __cplusplus reflects the chosen language standard in MSVC
    target_compile_options(fossredder PRIVATE /Zc:__cplusplus)
endif()

# Place runtime output into a predictable folder under the build tree
set_target_properties(fossredder PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin/Debug"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/Release"
)

# Only expose the app's own public include dir; other modules should export
# their include paths via their targets (core_lib, ui_lib, persistence_lib, etc.).
# Rely on target_link_libraries to provide transitive include dirs.
target_include_directories(fossredder PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    third_party/llama.cpp/include
)

target_link_libraries(fossredder PRIVATE core_lib debug_lib services_poppler services_opencv services_tesseract ui_lib persistence_lib)

# Link Qt Quick/Qml when available and define USE_QML for sources that check it
if(TARGET Qt6::Quick AND TARGET Qt6::Qml)
    target_link_libraries(fossredder PRIVATE Qt6::Quick Qt6::Qml)
    target_compile_definitions(fossredder PRIVATE USE_QML=1)
endif()

if(TARGET Qt6::Widgets)
    target_link_libraries(fossredder PRIVATE Qt6::Widgets)
endif()
if(TARGET Qt6::QuickWidgets)
    target_link_libraries(fossredder PRIVATE Qt6::QuickWidgets)
endif()

# Precompiled header (kept when available) to speed up incremental builds
if (CMAKE_VERSION VERSION_GREATER_EQUAL "4.0.1")
    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/include/pch.h")
        target_precompile_headers(fossredder PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include/pch.h")
    endif()
endif()

# Windows: post-build step to deploy Qt runtimes/QML modules when using vcpkg
if(WIN32 AND DEFINED VCPKG_INSTALLED_DIR AND DEFINED VCPKG_TARGET_TRIPLET)
    set(_qt_bin_release "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/bin")
    set(_qt_bin_debug "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug/bin")

    add_custom_command(TARGET fossredder POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Deploying Qt runtimes and QML modules (qtdeploy)"
        COMMAND ${CMAKE_COMMAND} -E env TARGET_DIR=$<TARGET_FILE_DIR:fossredder> TARGET_BINARY=$<TARGET_FILE:fossredder> QT_BIN_DEBUG=${_qt_bin_debug} QT_BIN_RELEASE=${_qt_bin_release} VCPKG_INSTALLED_DIR=${VCPKG_INSTALLED_DIR} VCPKG_TARGET_TRIPLET=${VCPKG_TARGET_TRIPLET} BUILD_CONFIG=$<CONFIG> ${CMAKE_COMMAND} -P "${CMAKE_SOURCE_DIR}/cmake/qtdeploy.cmake"
        COMMENT "Deploy Qt runtimes and QML modules (windeployqt preferred)"
    )

    if(MSVC)
        set(_dbg_env "$<$<CONFIG:Debug>:PATH=${_qt_bin_debug}$<SEMICOLON>QT_DEBUG_PLUGINS=1$<SEMICOLON>VCPKG_INSTALLED_DIR=${VCPKG_INSTALLED_DIR}$<SEMICOLON>QT_QPA_PLATFORM_PLUGIN_PATH=$<TARGET_FILE_DIR:fossredder>/platforms$<SEMICOLON>QML2_IMPORT_PATH=$<TARGET_FILE_DIR:fossredder>/qml>")
        set(_rel_env "$<$<CONFIG:Release>:PATH=${_qt_bin_release}$<SEMICOLON>VCPKG_INSTALLED_DIR=${VCPKG_INSTALLED_DIR}$<SEMICOLON>QT_QPA_PLATFORM_PLUGIN_PATH=$<TARGET_FILE_DIR:fossredder>/platforms$<SEMICOLON>QML2_IMPORT_PATH=$<TARGET_FILE_DIR:fossredder>/qml>")

        set_target_properties(fossredder PROPERTIES
            VS_DEBUGGER_ENVIRONMENT "${_dbg_env}${_rel_env}"
            VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        )
    endif()
endif()