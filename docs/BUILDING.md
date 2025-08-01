# Building LegacyStream on Windows

This guide covers building LegacyStream from source on Windows using Visual Studio and CMake.

## Prerequisites

### Required Software

1. **Visual Studio 2019 or 2022**
   - Install with "Desktop development with C++" workload
   - Ensure Windows 10/11 SDK is included
   - MSVC v142 or v143 compiler toolset

2. **CMake 3.20 or later**
   - Download from [cmake.org](https://cmake.org/download/)
   - Add to PATH during installation

3. **Qt 6.2 or later**
   - Download Qt Online Installer
   - Install Qt 6.2+ with MSVC 2019/2022 64-bit components
   - Include Qt Creator (optional but recommended)

4. **Git**
   - For cloning the repository and submodules

### Optional Dependencies

1. **vcpkg** (Recommended for OpenSSL)
   ```bash
   git clone https://github.com/Microsoft/vcpkg.git
   cd vcpkg
   ./bootstrap-vcpkg.bat
   ./vcpkg integrate install
   ```

2. **Ninja** (For faster builds)
   - Download from [ninja-build.org](https://ninja-build.org/)
   - Add to PATH

## Setting Up Dependencies

### 1. Install OpenSSL

#### Using vcpkg (Recommended)
```bash
vcpkg install openssl:x64-windows
```

#### Using Pre-built Binaries
1. Download OpenSSL from [slproweb.com](https://slproweb.com/products/Win32OpenSSL.html)
2. Install to `C:\OpenSSL-Win64`
3. Add `C:\OpenSSL-Win64\bin` to PATH

### 2. Setup Qt Environment
```bash
# Add Qt to PATH (adjust path to your Qt installation)
set PATH=C:\Qt\6.4.0\msvc2019_64\bin;%PATH%
set CMAKE_PREFIX_PATH=C:\Qt\6.4.0\msvc2019_64
```

### 3. Audio Codec Libraries
Create an `external` directory in the project root and place codec libraries:

```
external/
├── lame/
│   ├── include/lame/lame.h
│   └── lib/libmp3lame.lib
├── faac/
│   ├── include/faac.h
│   └── lib/libfaac.lib
├── ogg/
│   ├── include/ogg/ogg.h
│   └── lib/libogg.lib
├── vorbis/
│   ├── include/vorbis/codec.h
│   └── lib/libvorbis.lib
├── opus/
│   ├── include/opus/opus.h
│   └── lib/libopus.lib
└── flac/
    ├── include/FLAC/stream_decoder.h
    └── lib/libFLAC.lib
```

**Note**: You can download pre-built libraries or build them from source.

## Building LegacyStream

### Method 1: Command Line Build

1. **Clone the Repository**
   ```bash
   git clone https://github.com/yourorg/legacystream.git
   cd legacystream
   git submodule update --init --recursive
   ```

2. **Create Build Directory**
   ```bash
   mkdir build
   cd build
   ```

3. **Configure with CMake**
   ```bash
   # For Visual Studio 2022
   cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release
   
   # For Visual Studio 2019
   cmake .. -G "Visual Studio 16 2019" -A x64 -DCMAKE_BUILD_TYPE=Release
   
   # With vcpkg
   cmake .. -G "Visual Studio 17 2022" -A x64 ^
     -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake ^
     -DCMAKE_BUILD_TYPE=Release
   ```

4. **Build the Project**
   ```bash
   # Build Release configuration
   cmake --build . --config Release
   
   # Or build specific target
   cmake --build . --config Release --target LegacyStream
   ```

### Method 2: Visual Studio IDE

1. **Generate Solution File**
   ```bash
   cmake .. -G "Visual Studio 17 2022" -A x64
   ```

2. **Open in Visual Studio**
   - Open `LegacyStream.sln` in Visual Studio
   - Set build configuration to Release
   - Build solution (Ctrl+Shift+B)

### Method 3: Qt Creator

1. **Open CMakeLists.txt**
   - Launch Qt Creator
   - File → Open File or Project
   - Select `CMakeLists.txt` in project root

2. **Configure Kit**
   - Select appropriate kit (MSVC 64-bit)
   - Configure CMake settings
   - Build project (Ctrl+B)

## Build Configuration Options

### CMake Options

```bash
# Enable debug build
cmake .. -DCMAKE_BUILD_TYPE=Debug

# Specify Qt installation
cmake .. -DCMAKE_PREFIX_PATH=C:/Qt/6.4.0/msvc2019_64

# Enable static linking
cmake .. -DBUILD_STATIC=ON

# Disable specific features
cmake .. -DENABLE_SSL=OFF -DENABLE_HLS=OFF

# Custom codec library paths
cmake .. -DLAME_ROOT=C:/lame -DFAAC_ROOT=C:/faac
```

### Environment Variables

```bash
# Qt installation path
set CMAKE_PREFIX_PATH=C:\Qt\6.4.0\msvc2019_64

# OpenSSL path (if not using vcpkg)
set OPENSSL_ROOT_DIR=C:\OpenSSL-Win64

# Custom library paths
set LAME_ROOT=C:\Libraries\lame
set FAAC_ROOT=C:\Libraries\faac
```

## Building Dependencies from Source

### Building LAME (MP3)
```bash
# Download LAME source
curl -O https://downloads.sourceforge.net/lame/lame-3.100.tar.gz
tar -xzf lame-3.100.tar.gz
cd lame-3.100

# Build with MSVC
nmake -f Makefile.MSVC comp=msvc
```

### Building FAAC (AAC)
```bash
git clone https://github.com/knik0/faac.git
cd faac
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

### Building Ogg/Vorbis
```bash
# Using vcpkg (easiest)
vcpkg install libogg:x64-windows
vcpkg install libvorbis:x64-windows

# Or build manually
git clone https://github.com/xiph/ogg.git
git clone https://github.com/xiph/vorbis.git
# Follow standard CMake build process
```

## Troubleshooting

### Common Build Issues

#### 1. Qt Not Found
```
Error: Could NOT find Qt6 (missing: Qt6_DIR)
```
**Solution**: Set CMAKE_PREFIX_PATH to Qt installation directory
```bash
cmake .. -DCMAKE_PREFIX_PATH=C:/Qt/6.4.0/msvc2019_64
```

#### 2. OpenSSL Not Found
```
Error: Could NOT find OpenSSL
```
**Solution**: Install OpenSSL or set OPENSSL_ROOT_DIR
```bash
# Using vcpkg
vcpkg install openssl:x64-windows

# Or set path manually
cmake .. -DOPENSSL_ROOT_DIR=C:/OpenSSL-Win64
```

#### 3. Codec Libraries Missing
```
Error: Could NOT find LAME
```
**Solution**: Install codec libraries or disable them
```bash
# Place libraries in external/ directory or
cmake .. -DENABLE_MP3=OFF -DENABLE_AAC=OFF
```

#### 4. Windows SDK Issues
```
Error: Windows Kits directory not found
```
**Solution**: Install Windows 10/11 SDK through Visual Studio Installer

#### 5. CMake Version Too Old
```
Error: CMake 3.20 or higher is required
```
**Solution**: Update CMake from [cmake.org](https://cmake.org/download/)

### Performance Build Optimizations

#### Release Build with Optimizations
```bash
cmake .. -G "Visual Studio 17 2022" -A x64 ^
  -DCMAKE_BUILD_TYPE=Release ^
  -DCMAKE_CXX_FLAGS_RELEASE="/O2 /Ob2 /DNDEBUG /GL" ^
  -DCMAKE_EXE_LINKER_FLAGS_RELEASE="/LTCG"
```

#### Profile-Guided Optimization (PGO)
```bash
# First build for profiling
cmake .. -DCMAKE_CXX_FLAGS="/GL /wd4996" ^
  -DCMAKE_EXE_LINKER_FLAGS="/LTCG:PGI"

# Run application with typical workload
./Release/LegacyStream.exe --generate-profile

# Rebuild with profile data
cmake --build . --config Release
```

## Deployment

### Creating Installer
```bash
# Build installer package
cmake --build . --config Release --target package

# Or use CPack directly
cpack -G NSIS
```

### Deploying Qt Libraries
```bash
# Use Qt deployment tool
cd Release
windeployqt.exe LegacyStream.exe
```

### Manual Deployment
Copy required DLLs to application directory:
- Qt6Core.dll, Qt6Gui.dll, Qt6Widgets.dll, Qt6Network.dll
- OpenSSL DLLs (libssl-1_1-x64.dll, libcrypto-1_1-x64.dll)
- Visual C++ Redistributable
- Codec library DLLs

## Development Workflow

### Recommended IDE Setup
1. **Visual Studio** with Qt VS Tools extension
2. **Qt Creator** for Qt-specific development
3. **Visual Studio Code** with CMake extension

### Debugging
```bash
# Debug build
cmake .. -DCMAKE_BUILD_TYPE=Debug

# Enable detailed logging
set QT_LOGGING_RULES="*.debug=true"
```

### Testing
```bash
# Build with tests
cmake .. -DBUILD_TESTS=ON

# Run tests
cmake --build . --config Release --target test
```

## Continuous Integration

### GitHub Actions Example
```yaml
name: Windows Build

on: [push, pull_request]

jobs:
  build:
    runs-on: windows-latest
    
    steps:
    - uses: actions/checkout@v3
    
    - name: Install Qt
      uses: jurplel/install-qt-action@v3
      with:
        version: '6.4.0'
        arch: 'win64_msvc2019_64'
    
    - name: Install OpenSSL
      run: |
        vcpkg install openssl:x64-windows
    
    - name: Configure CMake
      run: |
        cmake -B build -G "Visual Studio 17 2022" -A x64 ^
          -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
    
    - name: Build
      run: cmake --build build --config Release
    
    - name: Test
      run: ctest --test-dir build -C Release
```

This comprehensive build guide should help you successfully compile LegacyStream on Windows with all its dependencies and features.