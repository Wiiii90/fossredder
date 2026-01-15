cmake_minimum_required(VERSION 4.0.1)
project(persistence)

file(GLOB_RECURSE SRCS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
add_library(persistence_lib STATIC ${SRCS})

target_include_directories(persistence_lib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/core/include>
)

target_link_libraries(persistence_lib PUBLIC core_lib)

target_compile_features(persistence_lib PUBLIC cxx_std_20)

find_package(SQLite3 QUIET)
if (SQLite3_FOUND)
    target_link_libraries(persistence_lib PRIVATE SQLite::SQLite3)
else()
    find_library(SQLITE3_LIBRARY sqlite3)
    if(SQLITE3_LIBRARY)
        target_link_libraries(persistence_lib PRIVATE ${SQLITE3_LIBRARY})
    endif()
endif()

if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.16")
    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/include/persistence/pch.h")
        target_precompile_headers(persistence_lib PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include/persistence/pch.h")
    endif()
endif()

install(TARGETS persistence_lib
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION include)
