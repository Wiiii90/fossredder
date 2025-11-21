cmake_minimum_required(VERSION 3.15)
project(services_tesseract)

file(GLOB_RECURSE SRCS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
add_library(services_tesseract STATIC ${SRCS})

target_include_directories(services_tesseract PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(services_tesseract PUBLIC cxx_std_20)

find_package(Tesseract REQUIRED CONFIG)
if(Tesseract_FOUND)
    target_link_libraries(services_tesseract PUBLIC Tesseract::libtesseract)
endif()

# Link to api so API headers are available
target_link_libraries(services_tesseract PUBLIC api)

# link to core/debug
target_link_libraries(services_tesseract PUBLIC core_lib debug_lib)

# Precompiled header
if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.16")
    if (EXISTS "${CMAKE_SOURCE_DIR}/include/pch.h")
        target_precompile_headers(services_tesseract PRIVATE "${CMAKE_SOURCE_DIR}/include/pch.h")
    endif()
endif()

install(TARGETS services_tesseract ARCHIVE DESTINATION lib LIBRARY DESTINATION lib RUNTIME DESTINATION bin)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION include)
