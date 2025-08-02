# LegacyStream Deployment Guide

## Running the Application

The LegacyStream application has been successfully compiled and should now run without DLL errors.

### Location
The executable is located at: `build\bin\RelWithDebInfo\LegacyStream.exe`

### Required DLL Files
The following DLL files have been copied to the build directory and are required for the application to run:

#### Qt6 Libraries:
- `Qt6Core.dll` - Core Qt6 functionality
- `Qt6Widgets.dll` - GUI widgets
- `Qt6Network.dll` - Network functionality
- `Qt6WebSockets.dll` - WebSocket support
- `Qt6Gui.dll` - GUI framework

#### Qt6 Platform Plugins:
- `platforms/qwindows.dll` - Windows platform plugin (required for GUI)

#### OpenSSL Libraries:
- `libcrypto-3-x64.dll` - OpenSSL cryptography
- `libssl-3-x64.dll` - OpenSSL SSL/TLS

### Automatic DLL Copying

The DLL copying is now **fully integrated into the build process**. Every time you build the application, the required DLLs are automatically copied to the build directory.

#### Build Process Integration
The CMakeLists.txt includes a post-build command that automatically copies all necessary DLLs and plugins:
- Qt6Core.dll
- Qt6Widgets.dll  
- Qt6Network.dll
- Qt6WebSockets.dll
- Qt6Gui.dll
- libcrypto-3-x64.dll
- libssl-3-x64.dll
- platforms/qwindows.dll (Windows platform plugin)

#### Manual Copy (Fallback)
If you need to copy DLLs manually, you can still use the batch script:
```cmd
copy_dlls.bat
```

### Running the Application

1. **From Command Line:**
   ```cmd
   cd build\bin\RelWithDebInfo
   LegacyStream.exe
   ```

2. **From File Explorer:**
   Navigate to `build\bin\RelWithDebInfo\` and double-click `LegacyStream.exe`

### Troubleshooting

If you encounter DLL errors:

1. **Rebuild the application:** Run `cmake --build build --config RelWithDebInfo` - this will automatically copy all DLLs
2. **Check DLL presence:** Ensure all required DLL files are in the same directory as `LegacyStream.exe`
3. **Manual copy (if needed):** Run `copy_dlls.bat` as a fallback option

### Application Features

The LegacyStream application includes:
- ✅ **GUI Interface** - Complete Qt6-based user interface
- ✅ **Application Icon** - Professional audio streaming icon
- ✅ **HTTP Server functionality**
- ✅ **Stream Management**
- ✅ **SSL/TLS Support** - Certificate management interface
- ✅ **Web Interface**
- ✅ **Statistics and Monitoring**
- ✅ **Relay Management**
- ✅ **HLS Generation**

### Notes

- The application is compiled in Release mode with debug information (`RelWithDebInfo`)
- Protocol servers (IceCast/SHOUTcast) are currently disabled until implementation
- The application should start and be ready to accept streaming connections 