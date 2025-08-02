cmake_minimum_required(VERSION 3.16)
project(LegacyStream VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Fix for MSVC and Qt6
if(MSVC)
    add_compile_options(/Zc:__cplusplus /permissive-)
endif()

# Configure Vulkan SDK path
set(Vulkan_INCLUDE_DIR "C:/VulkanSDK/1.4.321.1/Include")
set(Vulkan_LIBRARY_DIR "C:/VulkanSDK/1.4.321.1/Lib")

# Configure Qt6 path (adjust this path to match your Qt6 installation)
set(Qt6_DIR "C:/Qt/6.9.1/msvc2022_64/lib/cmake/Qt6")

# Set CMAKE_PREFIX_PATH to help find Vulkan
list(APPEND CMAKE_PREFIX_PATH "C:/VulkanSDK/1.4.321.1")

# Find Qt6 components
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network WebSockets)

# Find OpenSSL
find_package(OpenSSL REQUIRED)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Core module
set(LEGACYSTREAM_CORE_SOURCES
    src/core/Configuration.cpp
    src/core/ServerManager.cpp
    src/core/PerformanceManager.cpp
    src/core/Logger.cpp
)

set(LEGACYSTREAM_CORE_HEADERS
    include/core/Configuration.h
    include/core/ServerManager.h
    include/core/PerformanceManager.h
    include/core/Logger.h
)

# GUI module
set(LEGACYSTREAM_GUI_SOURCES
    src/gui/MainWindow.cpp
    src/gui/SSLWidget.cpp
)

set(LEGACYSTREAM_GUI_HEADERS
    include/gui/MainWindow.h
    include/gui/SSLWidget.h
)

# SSL module
set(LEGACYSTREAM_SSL_SOURCES
    src/ssl/CertificateManager.cpp
)

set(LEGACYSTREAM_SSL_HEADERS
    include/ssl/CertificateManager.h
)

# Streaming module - handled in subdirectory

# Protocols module
set(LEGACYSTREAM_PROTOCOLS_SOURCES
    src/protocols/ProtocolManager.cpp
)

set(LEGACYSTREAM_PROTOCOLS_HEADERS
    include/protocols/ProtocolManager.h
)

# Codecs module
set(LEGACYSTREAM_CODECS_SOURCES
    src/codecs/CodecManager.cpp
)

set(LEGACYSTREAM_CODECS_HEADERS
    include/codecs/CodecManager.h
)

# Main application
set(LEGACYSTREAM_MAIN_SOURCES
    src/main.cpp
)

# Create libraries
add_library(LegacyStreamCore STATIC
    ${LEGACYSTREAM_CORE_SOURCES}
    ${LEGACYSTREAM_CORE_HEADERS}
)

add_library(LegacyStreamGUI STATIC
    ${LEGACYSTREAM_GUI_SOURCES}
    ${LEGACYSTREAM_GUI_HEADERS}
)
set_target_properties(LegacyStreamGUI PROPERTIES AUTOMOC ON)

add_library(LegacyStreamSSL STATIC
    ${LEGACYSTREAM_SSL_SOURCES}
    ${LEGACYSTREAM_SSL_HEADERS}
)

# LegacyStreamStreaming library is created in src/streaming/CMakeLists.txt

add_library(LegacyStreamProtocols STATIC
    ${LEGACYSTREAM_PROTOCOLS_SOURCES}
    ${LEGACYSTREAM_PROTOCOLS_HEADERS}
)

add_library(LegacyStreamCodecs STATIC
    ${LEGACYSTREAM_CODECS_SOURCES}
    ${LEGACYSTREAM_CODECS_HEADERS}
)

# Link Qt libraries to all modules
target_link_libraries(LegacyStreamCore Qt6::Core Qt6::Network)
target_link_libraries(LegacyStreamGUI Qt6::Core Qt6::Widgets Qt6::Network)
target_link_libraries(LegacyStreamSSL Qt6::Core OpenSSL::SSL OpenSSL::Crypto)
# LegacyStreamStreaming linking is handled in src/streaming/CMakeLists.txt
target_link_libraries(LegacyStreamProtocols Qt6::Core Qt6::Network)
target_link_libraries(LegacyStreamCodecs Qt6::Core)

# Create main executable
add_executable(LegacyStream ${LEGACYSTREAM_MAIN_SOURCES})

# Add resource file (optional - only if icon files exist)
set(APP_RESOURCES assets/icons/resources.qrc)
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${APP_RESOURCES})
    # Check if icon files exist
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/assets/icons/app_icon.png AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/assets/icons/app_icon.ico)
        qt_add_resources(LEGACYSTREAM_RESOURCES ${APP_RESOURCES})
        target_sources(LegacyStream PRIVATE ${LEGACYSTREAM_RESOURCES})
        message(STATUS "Application icon found and will be included")
    else()
        message(STATUS "Application icon files not found - icon will be default")
    endif()
else()
    message(STATUS "Resource file not found - icon will be default")
endif()

# Link all libraries to main executable
target_link_libraries(LegacyStream
    LegacyStreamCore
    LegacyStreamGUI
    LegacyStreamSSL
    LegacyStreamStreaming
    LegacyStreamProtocols
    LegacyStreamCodecs
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    Qt6::WebSockets
    OpenSSL::SSL
    OpenSSL::Crypto
)

# Set properties for Windows
if(WIN32)
    set_target_properties(LegacyStream PROPERTIES
        WIN32_EXECUTABLE TRUE
        VS_DEBUGGER_ENVIRONMENT "PATH=${CMAKE_BINARY_DIR}/bin"
    )
    
    # Set application icon
set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/assets/icons/app_icon.ico")
if(EXISTS ${APP_ICON_RESOURCE_WINDOWS})
    set_target_properties(LegacyStream PROPERTIES 
        WIN32_EXECUTABLE TRUE
        OUTPUT_NAME "LegacyStream"
    )
    
    # Set the icon for the Windows executable
    if(WIN32)
        set_target_properties(LegacyStream PROPERTIES
            LINK_FLAGS "/SUBSYSTEM:WINDOWS"
        )
        
        # Create a Windows resource file for the icon
        set(RESOURCE_FILE "${CMAKE_CURRENT_BINARY_DIR}/app_icon.rc")
        file(WRITE ${RESOURCE_FILE} "IDI_ICON1 ICON \"${APP_ICON_RESOURCE_WINDOWS}\"")
        target_sources(LegacyStream PRIVATE ${RESOURCE_FILE})
    endif()
endif()
    
    # Add post-build command to copy Qt6 DLLs and platform plugins
    add_custom_command(TARGET LegacyStream POST_BUILD
        # Create platforms directory
        COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:LegacyStream>/platforms
        # Copy Qt6 DLLs
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "C:/Qt/6.9.1/msvc2022_64/bin/Qt6Core.dll"
            "C:/Qt/6.9.1/msvc2022_64/bin/Qt6Widgets.dll"
            "C:/Qt/6.9.1/msvc2022_64/bin/Qt6Network.dll"
            "C:/Qt/6.9.1/msvc2022_64/bin/Qt6WebSockets.dll"
            "C:/Qt/6.9.1/msvc2022_64/bin/Qt6Gui.dll"
            "C:/Program Files/OpenSSL-Win64/bin/libcrypto-3-x64.dll"
            "C:/Program Files/OpenSSL-Win64/bin/libssl-3-x64.dll"
            $<TARGET_FILE_DIR:LegacyStream>
        # Copy Qt6 platform plugins
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "C:/Qt/6.9.1/msvc2022_64/plugins/platforms/qwindows.dll"
            $<TARGET_FILE_DIR:LegacyStream>/platforms/
        COMMENT "Copying Qt6 DLLs, OpenSSL DLLs, and platform plugins to build directory"
    )
endif()

# Install rules
install(TARGETS LegacyStream
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# Install headers
install(DIRECTORY include/
    DESTINATION include/LegacyStream
    FILES_MATCHING PATTERN "*.h"
)

# Install documentation
install(FILES README.md LICENSE
    DESTINATION share/doc/LegacyStream
)

# Enable testing
enable_testing()

# Add subdirectories for module-specific CMakeLists.txt files
add_subdirectory(src/streaming) 