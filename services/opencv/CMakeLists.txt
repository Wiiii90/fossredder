cmake_minimum_required(VERSION 3.15)
project(services_opencv)

file(GLOB_RECURSE SRCS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cxx ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc)
add_library(services_opencv STATIC ${SRCS})

target_include_directories(services_opencv PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(services_opencv PUBLIC cxx_std_20)

find_package(OpenCV REQUIRED)
if(OpenCV_FOUND)
    target_include_directories(services_opencv PUBLIC ${OpenCV_INCLUDE_DIRS})
    target_link_libraries(services_opencv PUBLIC ${OpenCV_LIBS})
endif()

# Link to api so API headers (namespaced types) are available
target_link_libraries(services_opencv PUBLIC api)

# Link to core/debug if available
target_link_libraries(services_opencv PUBLIC core_lib debug_lib)

# Precompiled header
if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.16")
    if (EXISTS "${CMAKE_SOURCE_DIR}/include/pch.h")
        target_precompile_headers(services_opencv PRIVATE "${CMAKE_SOURCE_DIR}/include/pch.h")
    endif()
endif()

install(TARGETS services_opencv ARCHIVE DESTINATION lib LIBRARY DESTINATION lib RUNTIME DESTINATION bin)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION include)
