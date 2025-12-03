cmake_minimum_required(VERSION 3.15)
project(fossredder_app)

option(ENABLE_QML_FALLBACK "Enable QWidget fallback when QML is unavailable" ON)

file(GLOB_RECURSE APP_SRCS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
add_executable(fossredder ${APP_SRCS})

if(ENABLE_QML_FALLBACK)
    target_compile_definitions(fossredder PRIVATE QML_FALLBACK_ENABLED=1)
endif()

set_target_properties(fossredder PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin"
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
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/ui/include>
    third_party/llama.cpp/include
)

target_link_libraries(fossredder PRIVATE core_lib debug_lib services_poppler services_opencv services_tesseract ui_lib persistence_lib)

# Try find Qt Quick/Qml and link if present
find_package(Qt6 COMPONENTS Quick Qml QUIET)
if(TARGET Qt6::Quick AND TARGET Qt6::Qml)
    target_link_libraries(fossredder PRIVATE Qt6::Quick Qt6::Qml)
    target_compile_definitions(fossredder PRIVATE USE_QML=1)
    message(STATUS "Linking Qt6 Quick/Qml to fossredder and enabling USE_QML")
endif()

if(TARGET Qt6::Widgets)
    target_link_libraries(fossredder PRIVATE Qt6::Widgets)
endif()

if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.16")
    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/include/pch.h")
        target_precompile_headers(fossredder PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include/pch.h")
    endif()
endif()

if(TARGET llama)
    target_link_libraries(fossredder PRIVATE llama)
endif()

if(TARGET nlohmann_json::nlohmann_json)
    target_link_libraries(fossredder PRIVATE nlohmann_json::nlohmann_json)
endif()

if(TARGET spdlog::spdlog)
    target_link_libraries(fossredder PRIVATE spdlog::spdlog)
endif()

if(NOT DEFINED _qt_plugins_dir)
    if(DEFINED QT_PLUGINS_DIR)
        set(_qt_plugins_dir "${QT_PLUGINS_DIR}")
    elseif(DEFINED VCPKG_INSTALLED_DIR AND DEFINED VCPKG_TARGET_TRIPLET)
        set(_qt_plugins_dir "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/Qt6/plugins/platforms")
    else()
        set(_qt_plugins_dir "")
    endif()
endif()

message(STATUS "Qt plugin directory: ${_qt_plugins_dir}")

# Long-term: do not copy entire vcpkg bin tree. Rely on vcpkg applocal to deploy required DLLs.
# Copy only the configuration-appropriate Qt platform plugins and QML modules.
if(WIN32)
    # Platforms dir: choose debug or release variant at build time
    if(DEFINED VCPKG_INSTALLED_DIR AND DEFINED VCPKG_TARGET_TRIPLET)
        set(_qt_qml_release "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/Qt6/qml")
        set(_qt_qml_debug "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug/Qt6/qml")
        set(_qt_plugins_release "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/Qt6/plugins/platforms")
        set(_qt_plugins_debug "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug/Qt6/plugins/platforms")
        set(_qt_bin_release "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/bin")
        set(_qt_bin_debug "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug/bin")

        add_custom_command(TARGET fossredder POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E echo "Copying Qt platform plugins (config-aware)"
            COMMAND ${CMAKE_COMMAND} -E remove_directory "$<TARGET_FILE_DIR:fossredder>/platforms"
            COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:fossredder>/platforms"
            # copy_directory supports generator expressions for the source path
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "$<$<CONFIG:Debug>:${_qt_plugins_debug}>$<$<NOT:$<CONFIG:Debug>>:${_qt_plugins_release}>"
                "$<TARGET_FILE_DIR:fossredder>/platforms"
            COMMENT "Copy Qt platform plugins for current configuration"
        )

        # Copy QML modules for the active configuration (Debug uses debug qml if present)
        add_custom_command(TARGET fossredder POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E echo "Copying QML modules (config-aware)"
            COMMAND ${CMAKE_COMMAND} -E remove_directory "$<TARGET_FILE_DIR:fossredder>/qml"
            COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:fossredder>/qml"
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "$<$<CONFIG:Debug>:${_qt_qml_debug}>$<$<NOT:$<CONFIG:Debug>>:${_qt_qml_release}>"
                "$<TARGET_FILE_DIR:fossredder>/qml"
            COMMENT "Copy QML modules for current configuration"
        )

        # Copy Qt runtime DLLs (quick/qml) if present in vcpkg installed tree
        add_custom_command(TARGET fossredder POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E echo "Copying Qt runtime DLLs from vcpkg_installed if available"
            COMMAND ${CMAKE_COMMAND} -E env TARGET_DIR=$<TARGET_FILE_DIR:fossredder> QT_BIN_DEBUG=${_qt_bin_debug} QT_BIN_RELEASE=${_qt_bin_release} ${CMAKE_COMMAND} -P "${CMAKE_SOURCE_DIR}/cmake/CopyQtDlls.cmake"
            COMMENT "Copy Qt runtime DLLs (Qt6Quickd/Qt6Qmld/Qt6Cored etc) if present"
        )

        # If windeployqt is available in vcpkg_installed, run it to ensure full deployment of QML modules and dependent DLLs
        set(_windeployqt1 "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/tools/qt/bin/windeployqt.exe")
        set(_windeployqt2 "${VCPKG_INSTALLED_DIR}/tools/qt/bin/windeployqt.exe")
        if(EXISTS ${_windeployqt1})
            set(_windeployqt ${_windeployqt1})
        elseif(EXISTS ${_windeployqt2})
            set(_windeployqt ${_windeployqt2})
        else()
            set(_windeployqt "")
        endif()

        if(EXISTS ${_windeployqt})
            add_custom_command(TARGET fossredder POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E echo "Running windeployqt to deploy Qt runtimes and QML modules"
                COMMAND ${_windeployqt} --dir "$<TARGET_FILE_DIR:fossredder>" --qmldir "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/Qt6/qml" "$<TARGET_FILE:fossredder>"
                COMMENT "windeployqt deployment"
            )
        else()
            add_custom_command(TARGET fossredder POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E echo "windeployqt not found in vcpkg_installed; skipping windeployqt step"
            )
        endif()

        # Run vcpkg applocal to copy required runtime DLLs from vcpkg_installed into the target directory.
        # This ensures the executable has all native dependencies available without modifying PATH at debug time.
        if(DEFINED VCPKG_INSTALLED_DIR AND DEFINED VCPKG_TARGET_TRIPLET)
            set(_applocal_ps1 "${CMAKE_SOURCE_DIR}/third_party/vcpkg/scripts/buildsystems/msbuild/applocal.ps1")
            if(EXISTS ${_applocal_ps1})
                add_custom_command(TARGET fossredder POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E echo "Running vcpkg applocal to deploy runtime dependencies"
                    COMMAND powershell.exe -NoProfile -ExecutionPolicy Bypass -File "${_applocal_ps1}" -targetBinary "$<TARGET_FILE:fossredder>" -installedDir "$<$<CONFIG:Debug>:${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug/bin>$<$<NOT:$<CONFIG:Debug>>:${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/bin>"
                    COMMENT "vcpkg applocal deploy runtime dependencies"
                )
            else()
                add_custom_command(TARGET fossredder POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E echo "applocal.ps1 not found at ${_applocal_ps1}; skipping applocal step"
                )
            endif()
        endif()
    else()
        # Fallback: copy from previously resolved _qt_plugins_dir if set
        add_custom_command(TARGET fossredder POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E echo "Copying qwindows.dll from ${_qt_plugins_dir} (legacy)"
            COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:fossredder>/platforms"
            COMMAND ${CMAKE_COMMAND} -E copy "${_qt_plugins_dir}/qwindows.dll" "$<TARGET_FILE_DIR:fossredder>/platforms/"
        )
    endif()
endif()

# Generate a debug launcher batch to prepend vcpkg debug bin to PATH when running locally
if(WIN32 AND DEFINED VCPKG_INSTALLED_DIR AND DEFINED VCPKG_TARGET_TRIPLET)
    configure_file(
        "${CMAKE_SOURCE_DIR}/cmake/RunDebug.bat.in"
        "${CMAKE_BINARY_DIR}/bin/RunDebug.bat"
        @ONLY
    )

    if(MSVC)
        # Configure Visual Studio debugger environment so vcpkg debug/release bin is prepended to PATH
        # and QT_DEBUG_PLUGINS is set for Debug builds. Use generator expressions with $<SEMICOLON>.
        set(_vcpkg_debug_bin "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug/bin")
        set(_vcpkg_release_bin "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/bin")

        # Try to locate MSVC debug CRT directory containing vcruntime140d.dll (debug_nonredist/x64)
        set(_msvc_debug_dir "")
        # Build a list of candidate roots using environment variables (avoid parentheses in CMake var names)
        set(_possible_roots)
        if(NOT "$ENV{ProgramFiles}" STREQUAL "")
            list(APPEND _possible_roots "$ENV{ProgramFiles}/Microsoft Visual Studio/2022/Community/VC/Redist/MSVC")
            list(APPEND _possible_roots "$ENV{ProgramFiles}/Microsoft Visual Studio/*/VC/Redist/MSVC")
        endif()
        foreach(_root IN LISTS _possible_roots)
            file(GLOB _candidates RELATIVE "${CMAKE_SOURCE_DIR}" "${_root}/*/debug_nonredist/x64/*")
            foreach(_cand IN LISTS _candidates)
                # absolute path
                get_filename_component(_cand_abs "${_root}/${_cand}" ABSOLUTE)
                if(EXISTS "${_cand_abs}/vcruntime140d.dll")
                    set(_msvc_debug_dir "${_cand_abs}")
                    break()
                endif()
            endforeach()
            if(NOT "${_msvc_debug_dir}" STREQUAL "")
                break()
            endif()
        endforeach()

        set(_dbg_env "$<$<CONFIG:Debug>:PATH=${_vcpkg_debug_bin}$<SEMICOLON>QT_DEBUG_PLUGINS=1$<SEMICOLON>VCPKG_INSTALLED_DIR=${VCPKG_INSTALLED_DIR}>")
        set(_rel_env "$<$<CONFIG:Release>:PATH=${_vcpkg_release_bin}$<SEMICOLON>VCPKG_INSTALLED_DIR=${VCPKG_INSTALLED_DIR}>")

        set_target_properties(fossredder PROPERTIES
            VS_DEBUGGER_ENVIRONMENT "${_dbg_env}${_rel_env}"
            VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:fossredder>"
        )
    endif()
endif()

# Note: Do NOT copy the entire VCPKG bin folder here (can be very large).
# vcpkg's applocal deployment (applocal.ps1) is invoked automatically during the link step
# and should copy only required DLL dependencies. If additional files are missing,
# adjust vcpkg installs or the applocal hooks.

# End of app/CMakeLists.txt