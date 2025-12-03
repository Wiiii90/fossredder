# Copies debug or release Qt runtime DLLs (if present) from vcpkg_installed into the target dir.
# Usage: cmake -P CopyQtDlls.cmake -DTARGET_DIR=... -DQT_BIN_DEBUG=... -DQT_BIN_RELEASE=...

if(NOT DEFINED TARGET_DIR)
    if(DEFINED ENV{TARGET_DIR})
        set(TARGET_DIR $ENV{TARGET_DIR})
    endif()
endif()

if(NOT DEFINED QT_BIN_DEBUG)
    if(DEFINED ENV{QT_BIN_DEBUG})
        set(QT_BIN_DEBUG $ENV{QT_BIN_DEBUG})
    endif()
endif()

if(NOT DEFINED QT_BIN_RELEASE)
    if(DEFINED ENV{QT_BIN_RELEASE})
        set(QT_BIN_RELEASE $ENV{QT_BIN_RELEASE})
    endif()
endif()

if(NOT DEFINED TARGET_DIR)
    message(FATAL_ERROR "TARGET_DIR not specified")
endif()

# Primary DLL name patterns to copy (Qt runtime and common dependencies)
set(_patterns
    "Qt6Quickd.dll"
    "Qt6Qmld.dll"
    "Qt6Cored.dll"
    "Qt6Networkd.dll"
    "Qt6Guid.dll"
    "Qt6Widgetsd.dll"
    "Qt6QmlMetad.dll"
    "Qt6QmlModelsd.dll"
    "Qt6OpenGLd.dll"
    "icudtd*.dll"
    "icuind*.dll"
    "icuucd*.dll"
    "zlibd1.dll"
    "double-conversion.dll"
    "pcre2-16d.dll"
    "zstd.dll"
    "brotlicommon.dll"
    "brotlidec.dll"
    "libcrypto-3-x64.dll"
    "libpng16d.dll"
    "freetyped.dll"
)

# Choose debug bin if present, otherwise release bin
if(DEFINED QT_BIN_DEBUG AND EXISTS "${QT_BIN_DEBUG}")
    set(_src_dir ${QT_BIN_DEBUG})
else()
    set(_src_dir ${QT_BIN_RELEASE})
endif()

if(NOT EXISTS "${_src_dir}")
    message(STATUS "CopyQtDlls: source dir ${_src_dir} does not exist; skipping copy")
    return()
endif()

# Keep track of which concrete DLL filenames we copied to TARGET_DIR
set(_copied_dlls)

# Copy each matching pattern into TARGET_DIR
foreach(_pat IN LISTS _patterns)
    file(GLOB _matched "${_src_dir}/${_pat}")
    foreach(_src IN LISTS _matched)
        get_filename_component(_name ${_src} NAME)
        message(STATUS "CopyQtDlls: copying ${_src} -> ${TARGET_DIR}/${_name}")
        file(COPY "${_src}" DESTINATION "${TARGET_DIR}")
        list(APPEND _copied_dlls ${_name})
    endforeach()
endforeach()

# Ensure TARGET_DIR contains copied DLLs (some deployments rely on DLLs next to executables)
if(NOT _copied_dlls)
    message(STATUS "CopyQtDlls: no DLLs matched for copy; skipping plugin-side copy")
else()
    # Find all plugin DLLs under TARGET_DIR/qml (plugins are often in subfolders like qml/QtQuick)
    file(GLOB_RECURSE _plugin_files RELATIVE "${TARGET_DIR}" "${TARGET_DIR}/qml/*.dll")
    foreach(_plugin_rel IN LISTS _plugin_files)
        set(_plugin_abs "${TARGET_DIR}/${_plugin_rel}")
        get_filename_component(_plugindir ${_plugin_abs} DIRECTORY)
        message(STATUS "CopyQtDlls: detected plugin dir ${_plugindir}")
        foreach(_dllname IN LISTS _copied_dlls)
            # prefer already-copied target DLLs, otherwise use src dir
            if(EXISTS "${TARGET_DIR}/${_dllname}")
                set(_src_for_plugin "${TARGET_DIR}/${_dllname}")
            else()
                set(_src_for_plugin "${_src_dir}/${_dllname}")
            endif()
            if(EXISTS "${_src_for_plugin}")
                get_filename_component(_name_only ${_src_for_plugin} NAME)
                message(STATUS "CopyQtDlls: copying ${_src_for_plugin} -> ${_plugindir}/${_name_only}")
                file(COPY "${_src_for_plugin}" DESTINATION "${_plugindir}")
            endif()
        endforeach()
    endforeach()
endif()

# Also ensure QML modules directory exists (some plugins expect specific relative layout)
set(_qml_src_dir "${_src_dir}/../Qt6/qml")
if(NOT EXISTS "${_qml_src_dir}")
    set(_qml_src_dir "${QT_BIN_RELEASE}/../Qt6/qml")
endif()
if(EXISTS "${_qml_src_dir}")
    message(STATUS "CopyQtDlls: copying QML modules from ${_qml_src_dir} to ${TARGET_DIR}/qml")
    file(COPY "${_qml_src_dir}" DESTINATION "${TARGET_DIR}/qml")
endif()
