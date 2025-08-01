cmake_minimum_required(VERSION 3.20)
project(LegacyStream VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Platform-specific configurations
if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0601)  # Windows 7+
    add_definitions(-DNOMINMAX)
    add_definitions(-DWIN32_LEAN_AND_MEAN)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

# Build type configuration
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler-specific flags
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /permissive- /std:c++17")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /DNDEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Od /Zi /DEBUG")
endif()

# Find required packages
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network Sql Charts)
find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/external)

# External dependencies configuration
set(EXTERNAL_DIR ${CMAKE_SOURCE_DIR}/external)

# Add subdirectories
add_subdirectory(src/core)
add_subdirectory(src/gui)
add_subdirectory(src/protocols)
add_subdirectory(src/codecs)
add_subdirectory(src/ssl)
add_subdirectory(src/streaming)

# Main executable
add_executable(LegacyStream
    src/main.cpp
)

target_link_libraries(LegacyStream PRIVATE
    LegacyStreamCore
    LegacyStreamGUI
    LegacyStreamProtocols
    LegacyStreamCodecs
    LegacyStreamSSL
    LegacyStreamStreaming
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    Qt6::Sql
    Qt6::Charts
    OpenSSL::SSL
    OpenSSL::Crypto
    ZLIB::ZLIB
)

if(WIN32)
    target_link_libraries(LegacyStream PRIVATE
        ws2_32
        winmm
        mfplat
        mf
        mfreadwrite
        mfuuid
        ole32
        propsys
    )
endif()

# Set working directory for Visual Studio
set_target_properties(LegacyStream PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)

# Installation
install(TARGETS LegacyStream
    RUNTIME DESTINATION bin
)

# Copy Qt DLLs for Windows deployment
if(WIN32)
    get_target_property(QT_QMAKE_EXECUTABLE Qt6::qmake IMPORTED_LOCATION)
    get_filename_component(QT_WINDEPLOYQT_EXECUTABLE ${QT_QMAKE_EXECUTABLE} PATH)
    set(QT_WINDEPLOYQT_EXECUTABLE "${QT_WINDEPLOYQT_EXECUTABLE}/windeployqt.exe")

    add_custom_command(TARGET LegacyStream POST_BUILD
        COMMAND ${QT_WINDEPLOYQT_EXECUTABLE} $<TARGET_FILE:LegacyStream>
        COMMENT "Deploying Qt libraries")
endif()

# CPack configuration for installer
set(CPACK_PACKAGE_NAME "LegacyStream")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "High-Performance Audio Streaming Server")
set(CPACK_PACKAGE_VENDOR "LegacyStream")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "LegacyStream")

if(WIN32)
    set(CPACK_GENERATOR "NSIS")
    set(CPACK_NSIS_DISPLAY_NAME "LegacyStream Audio Server")
    set(CPACK_NSIS_PACKAGE_NAME "LegacyStream")
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
endif()

include(CPack)