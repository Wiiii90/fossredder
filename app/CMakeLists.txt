cmake_minimum_required(VERSION 3.15)
project(fossredder_app)

option(ENABLE_QML_FALLBACK "Enable QWidget fallback when QML is unavailable" ON)
option(ENABLE_RUNTIME_FALLBACK "Enable runtime PATH/VCPKG fallback for debugging (local only)" OFF)

file(GLOB_RECURSE APP_SRCS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
add_executable(fossredder ${APP_SRCS})

# Require modern C++ standard for this target
target_compile_features(fossredder PUBLIC cxx_std_20)

if(MSVC)
    # Ensure __cplusplus reports correct standard to headers
    target_compile_options(fossredder PRIVATE /Zc:__cplusplus)
endif()

if(ENABLE_QML_FALLBACK)
    target_compile_definitions(fossredder PRIVATE QML_FALLBACK_ENABLED=1)
endif()

if(ENABLE_RUNTIME_FALLBACK)
    target_compile_definitions(fossredder PRIVATE ENABLE_RUNTIME_FALLBACK=1)
endif()

set_target_properties(fossredder PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin/Debug"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/Release"
)

target_include_directories(fossredder PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/core/include>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/debug/include>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/services/poppler/include>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/services/opencv/include>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/services/tesseract/include>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/ui/include>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/persistence/include>
    third_party/llama.cpp/include
)

target_link_libraries(fossredder PRIVATE core_lib debug_lib services_poppler services_opencv services_tesseract ui_lib persistence_lib)

find_package(Qt6 COMPONENTS Quick Qml Widgets QuickWidgets QUIET)
if(TARGET Qt6::Quick AND TARGET Qt6::Qml)
    target_link_libraries(fossredder PRIVATE Qt6::Quick Qt6::Qml)
    target_compile_definitions(fossredder PRIVATE USE_QML=1)
    message(STATUS "Linking Qt6 Quick/Qml to fossredder and enabling USE_QML")
endif()

if(TARGET Qt6::Widgets)
    target_link_libraries(fossredder PRIVATE Qt6::Widgets)
endif()

if(TARGET Qt6::QuickWidgets)
    target_link_libraries(fossredder PRIVATE Qt6::QuickWidgets)
endif()

if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.16")
    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/include/pch.h")
        target_precompile_headers(fossredder PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include/pch.h")
    endif()
endif()

# post-build deployment
if(WIN32 AND DEFINED VCPKG_INSTALLED_DIR AND DEFINED VCPKG_TARGET_TRIPLET)
    set(_qt_bin_release "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/bin")
    set(_qt_bin_debug "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug/bin")

    add_custom_command(TARGET fossredder POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Deploying Qt runtimes and QML modules (qtdeploy)"
        COMMAND ${CMAKE_COMMAND} -E env TARGET_DIR=$<TARGET_FILE_DIR:fossredder> TARGET_BINARY=$<TARGET_FILE:fossredder> QT_BIN_DEBUG=${_qt_bin_debug} QT_BIN_RELEASE=${_qt_bin_release} VCPKG_INSTALLED_DIR=${VCPKG_INSTALLED_DIR} VCPKG_TARGET_TRIPLET=${VCPKG_TARGET_TRIPLET} BUILD_CONFIG=$<CONFIG> ${CMAKE_COMMAND} -P "${CMAKE_SOURCE_DIR}/cmake/qtdeploy.cmake"
        COMMENT "Deploy Qt runtimes and QML modules (windeployqt preferred)"
    )

    if(MSVC)
        # Make VS launches behave like manual start from the output directory.
        # We deploy to <exeDir>/platforms and <exeDir>/qml.
        set(_dbg_env "$<$<CONFIG:Debug>:PATH=${_qt_bin_debug}$<SEMICOLON>QT_DEBUG_PLUGINS=1$<SEMICOLON>VCPKG_INSTALLED_DIR=${VCPKG_INSTALLED_DIR}$<SEMICOLON>QT_QPA_PLATFORM_PLUGIN_PATH=$<TARGET_FILE_DIR:fossredder>/platforms$<SEMICOLON>QML2_IMPORT_PATH=$<TARGET_FILE_DIR:fossredder>/qml>")
        set(_rel_env "$<$<CONFIG:Release>:PATH=${_qt_bin_release}$<SEMICOLON>VCPKG_INSTALLED_DIR=${VCPKG_INSTALLED_DIR}$<SEMICOLON>QT_QPA_PLATFORM_PLUGIN_PATH=$<TARGET_FILE_DIR:fossredder>/platforms$<SEMICOLON>QML2_IMPORT_PATH=$<TARGET_FILE_DIR:fossredder>/qml>")

        # VS does not expand generator expressions in VS_DEBUGGER_WORKING_DIRECTORY.
        # Use a concrete per-config folder matching RUNTIME_OUTPUT_DIRECTORY_<CONFIG>.
        set_target_properties(fossredder PROPERTIES
            VS_DEBUGGER_ENVIRONMENT "${_dbg_env}${_rel_env}"
            VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        )
    endif()
endif()