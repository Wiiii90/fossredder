cmake_minimum_required(VERSION 3.15)
project(core)

file(GLOB_RECURSE CORE_SRCS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_library(core_lib STATIC ${CORE_SRCS})

target_include_directories(core_lib PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/debug/include>
)

if(TARGET api)
    target_link_libraries(core_lib PUBLIC api)
endif()

if(TARGET debug_lib)
    target_link_libraries(core_lib PUBLIC debug_lib)
endif()

target_compile_features(core_lib PUBLIC cxx_std_20)

if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.16")
    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/include/core/pch.h")
        target_precompile_headers(core_lib PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include/core/pch.h")
    endif()
endif()

install(TARGETS core_lib
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION include)
