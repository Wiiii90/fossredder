cmake_minimum_required(VERSION 4.0.1)
project(fossredder_app LANGUAGES CXX)

# Build option: enable the QML UI. Default OFF for headless/CI builds;
# will be auto-enabled below if Qt Quick/Qml is found on the system.
option(USE_QML "Build with QML UI" OFF)

# Detect Qt early so we can auto-enable QML support and conditionally
# include the QML-only translation unit (main_qml.cpp).
find_package(Qt6 COMPONENTS Quick Qml Widgets QuickWidgets LinguistTools QUIET)
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
)

target_link_libraries(fossredder PRIVATE core_lib debug_lib services_poppler services_opencv services_tesseract ui_lib persistence_lib)

set(TRANSLATION_TS_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/i18n/fossredder_de.ts
)

set(TRANSLATION_SOURCE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/ui/src
    ${CMAKE_SOURCE_DIR}/ui/include
    ${CMAKE_SOURCE_DIR}/ui/qml
)

if(TARGET Qt6::lupdate AND TARGET Qt6::lrelease)
    set(TRANSLATION_QM_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/i18n)
    set(TRANSLATION_QM_FILES)

    foreach(TS_FILE IN LISTS TRANSLATION_TS_FILES)
        get_filename_component(TS_NAME ${TS_FILE} NAME_WE)
        set(QM_FILE ${TRANSLATION_QM_OUTPUT_DIR}/${TS_NAME}.qm)

        list(APPEND TRANSLATION_QM_FILES ${QM_FILE})

        add_custom_command(
            OUTPUT ${QM_FILE}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${TRANSLATION_QM_OUTPUT_DIR}
            COMMAND Qt6::lrelease ${TS_FILE} -qm ${QM_FILE}
            DEPENDS ${TS_FILE}
            VERBATIM
        )
    endforeach()

    add_custom_target(fossredder_translations DEPENDS ${TRANSLATION_QM_FILES})
    add_dependencies(fossredder fossredder_translations)

    add_custom_command(TARGET fossredder POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:fossredder>/i18n"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${TRANSLATION_QM_FILES} "$<TARGET_FILE_DIR:fossredder>/i18n"
        VERBATIM
    )

    add_custom_target(update_translations)
    foreach(TS_FILE IN LISTS TRANSLATION_TS_FILES)
        add_custom_command(TARGET update_translations POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory ${TRANSLATION_QM_OUTPUT_DIR}
            COMMAND Qt6::lupdate ${TRANSLATION_SOURCE_DIRS} -no-obsolete -ts ${TS_FILE}
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            VERBATIM
        )
    endforeach()
else()
    message(WARNING "Qt6 LinguistTools not found; translation catalogs will not be generated automatically")
endif()

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

# Windows: configure debugger env vars when using vcpkg
if(WIN32 AND DEFINED VCPKG_INSTALLED_DIR AND DEFINED VCPKG_TARGET_TRIPLET)
    set(_qt_bin_release "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/bin")
    set(_qt_bin_debug "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug/bin")

    if(MSVC)
        set(_dbg_env "$<$<CONFIG:Debug>:PATH=${_qt_bin_debug}$<SEMICOLON>QT_DEBUG_PLUGINS=1$<SEMICOLON>VCPKG_INSTALLED_DIR=${VCPKG_INSTALLED_DIR}$<SEMICOLON>QT_QPA_PLATFORM_PLUGIN_PATH=$<TARGET_FILE_DIR:fossredder>/platforms$<SEMICOLON>QML2_IMPORT_PATH=$<TARGET_FILE_DIR:fossredder>/qml>")
        set(_rel_env "$<$<CONFIG:Release>:PATH=${_qt_bin_release}$<SEMICOLON>VCPKG_INSTALLED_DIR=${VCPKG_INSTALLED_DIR}$<SEMICOLON>QT_QPA_PLATFORM_PLUGIN_PATH=$<TARGET_FILE_DIR:fossredder>/platforms$<SEMICOLON>QML2_IMPORT_PATH=$<TARGET_FILE_DIR:fossredder>/qml>")

        set_target_properties(fossredder PROPERTIES
            VS_DEBUGGER_ENVIRONMENT "${_dbg_env}${_rel_env}"
            VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        )
    endif()

    # Dev convenience: deploy Qt runtime + platforms/qwindows.dll + QML into the build output
    add_custom_command(TARGET fossredder POST_BUILD
        COMMAND ${CMAKE_COMMAND}
            -DTARGET_DIR="$<TARGET_FILE_DIR:fossredder>"
            -DVCPKG_INSTALLED_DIR="${VCPKG_INSTALLED_DIR}"
            -DVCPKG_TARGET_TRIPLET="${VCPKG_TARGET_TRIPLET}"
            -DBUILD_CONFIG="$<CONFIG>"
            -P "${CMAKE_SOURCE_DIR}/cmake/qtdeploy.cmake"
        VERBATIM
    )
endif()

# Install rules so 'cmake --install' stages the app for packaging
install(TARGETS fossredder
    RUNTIME DESTINATION bin
)

# Install QML folder so windeployqt can reference it (project uses ui/qml)
if(EXISTS "${CMAKE_SOURCE_DIR}/ui/qml")
    install(DIRECTORY "${CMAKE_SOURCE_DIR}/ui/qml/" DESTINATION "bin/qml")
elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/qml")
    # fallback if app-local qml folder exists
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/qml/" DESTINATION "bin/qml")
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/i18n")
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/i18n/" DESTINATION "bin/i18n")
elseif(EXISTS "${CMAKE_SOURCE_DIR}/i18n")
    install(DIRECTORY "${CMAKE_SOURCE_DIR}/i18n/" DESTINATION "bin/i18n")
endif()

if(TRANSLATION_QM_FILES)
    install(FILES ${TRANSLATION_QM_FILES} DESTINATION "bin/i18n")
endif()

# Install additional assets if present (optional)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets")
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/assets/" DESTINATION share/fossredder/assets FILES_MATCHING PATTERN "*")
endif()