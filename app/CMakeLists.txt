cmake_minimum_required(VERSION 3.15)
project(fossredder_app)

file(GLOB_RECURSE APP_SRCS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
add_executable(fossredder ${APP_SRCS})

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
    third_party/llama.cpp/include
)

target_link_libraries(fossredder PRIVATE core_lib debug_lib services_poppler services_opencv services_tesseract ui_lib persistence_lib)

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

set(_copy_script "${CMAKE_CURRENT_BINARY_DIR}/copy_qt_plugins.cmake")
file(WRITE "${_copy_script}"
"if(NOT \"${_qt_plugins_dir}\" STREQUAL \"\")\n"
"  if(EXISTS \"${_qt_plugins_dir}/qwindows.dll\")\n"
"    file(MAKE_DIRECTORY \"${CMAKE_BINARY_DIR}/bin/platforms\")\n"
"    file(COPY \"${_qt_plugins_dir}/qwindows.dll\" DESTINATION \"${CMAKE_BINARY_DIR}/bin/platforms\")\n"
"    message(STATUS \"qwindows.dll copied to ${CMAKE_BINARY_DIR}/bin/platforms\")\n"
"  else()\n"
"    message(STATUS \"qwindows.dll not found at ${_qt_plugins_dir}\")\n"
"  endif()\n"
"else()\n"
"  message(STATUS \"qt plugins dir not set; skipping copy\")\n"
"endif()\n"
)

if(WIN32)
    add_custom_command(TARGET fossredder POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "running plugin copy script"
        COMMAND ${CMAKE_COMMAND} -P "${_copy_script}"
    )
endif()