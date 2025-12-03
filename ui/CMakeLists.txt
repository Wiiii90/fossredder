cmake_minimum_required(VERSION 3.15)
project(ui)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Find Qt Quick/Qml only
find_package(Qt6 COMPONENTS Quick Qml QUIET)

file(GLOB_RECURSE SRCS CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/**/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.qrc)

# Also include public headers so AUTOMOC picks up Q_OBJECT macros in headers
file(GLOB_RECURSE HDRS CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/**/*.h)

# Add QML resources if present
set(QML_QRC "${CMAKE_CURRENT_SOURCE_DIR}/qml/qml.qrc")
if(EXISTS ${QML_QRC})
    list(APPEND SRCS ${QML_QRC})
endif()

add_library(ui_lib STATIC ${SRCS} ${HDRS})

target_include_directories(ui_lib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(ui_lib PUBLIC cxx_std_20)

# Log which Qt components were found for easier debugging
if(TARGET Qt6::Quick AND TARGET Qt6::Qml)
    message(STATUS "Qt6 Quick and Qml found; enabling USE_QML for ui_lib")
    target_link_libraries(ui_lib PUBLIC Qt6::Quick Qt6::Qml)
    target_compile_definitions(ui_lib PUBLIC USE_QML=1)
else()
    message(STATUS "Qt6 Quick/Qml not found; QML features disabled in ui_lib")
endif()

# link project libs
target_link_libraries(ui_lib PUBLIC core_lib debug_lib)

# Precompiled header
if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.16")
    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/include/pch.h")
        target_precompile_headers(ui_lib PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include/pch.h")
    endif()
endif()

install(TARGETS ui_lib
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION include)
