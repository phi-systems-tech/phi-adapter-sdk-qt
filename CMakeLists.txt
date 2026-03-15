cmake_minimum_required(VERSION 3.21)

project(phi-adapter-sdk-qt
    VERSION 0.1.3
    DESCRIPTION "Qt event-loop backend for phi-adapter-sdk"
    LANGUAGES CXX
)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(PHI_ADAPTER_SDK_QT_CMAKE_INSTALL_DIR "lib/cmake/phi-adapter-sdk-qt")

find_package(Qt6 REQUIRED COMPONENTS Core)

set(PHI_ADAPTER_SDK_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../phi-adapter-sdk" CACHE PATH
    "Path to local phi-adapter-sdk checkout. If not found, find_package(phi-adapter-sdk) is used."
)
option(PHI_ADAPTER_SDK_QT_USE_LOCAL_ADAPTER_SDK
    "Use local ../phi-adapter-sdk checkout when available"
    ON
)

if(NOT TARGET phi::adapter-sdk)
    if(PHI_ADAPTER_SDK_QT_USE_LOCAL_ADAPTER_SDK AND EXISTS "${PHI_ADAPTER_SDK_SOURCE_DIR}/CMakeLists.txt")
        message(STATUS "Using local phi-adapter-sdk from: ${PHI_ADAPTER_SDK_SOURCE_DIR}")
        add_subdirectory("${PHI_ADAPTER_SDK_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/_deps/phi-adapter-sdk")
    else()
        message(STATUS "Using installed phi-adapter-sdk via find_package")
        find_package(phi-adapter-sdk CONFIG REQUIRED)
    endif()
endif()

if(NOT TARGET phi::adapter-sdk)
    message(FATAL_ERROR "phi-adapter-sdk target phi::adapter-sdk not found")
endif()

add_library(phi_adapter_sdk_qt SHARED
    src/qt/instance_execution_backend_qt.cpp
)
add_library(phi::adapter-sdk-qt ALIAS phi_adapter_sdk_qt)

set_target_properties(phi_adapter_sdk_qt PROPERTIES
    EXPORT_NAME adapter-sdk-qt
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

target_compile_features(phi_adapter_sdk_qt PUBLIC cxx_std_20)

target_include_directories(phi_adapter_sdk_qt
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(phi_adapter_sdk_qt
    PRIVATE
        Qt6::Core
        phi::adapter-sdk
)

install(
    DIRECTORY include/phi/adapter/sdk/qt
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/phi/adapter/sdk
    FILES_MATCHING
        PATTERN "*.h"
)

install(
    TARGETS phi_adapter_sdk_qt
    EXPORT phi-adapter-sdk-qtTargets
)

install(
    EXPORT phi-adapter-sdk-qtTargets
    FILE phi-adapter-sdk-qtTargets.cmake
    NAMESPACE phi::
    DESTINATION ${PHI_ADAPTER_SDK_QT_CMAKE_INSTALL_DIR}
)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/phi-adapter-sdk-qtConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/phi-adapter-sdk-qtConfig.cmake
    INSTALL_DESTINATION ${PHI_ADAPTER_SDK_QT_CMAKE_INSTALL_DIR}
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/phi-adapter-sdk-qtConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/phi-adapter-sdk-qtConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/phi-adapter-sdk-qtConfigVersion.cmake
    DESTINATION ${PHI_ADAPTER_SDK_QT_CMAKE_INSTALL_DIR}
)
