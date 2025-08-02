@echo off
echo Copying Qt6 and OpenSSL DLLs to build directory...

REM Create build directory if it doesn't exist
if not exist "build\bin\RelWithDebInfo\" mkdir "build\bin\RelWithDebInfo\"

REM Copy Qt6 DLLs
copy "C:\Qt\6.9.1\msvc2022_64\bin\Qt6Core.dll" "build\bin\RelWithDebInfo\" >nul 2>&1
copy "C:\Qt\6.9.1\msvc2022_64\bin\Qt6Widgets.dll" "build\bin\RelWithDebInfo\" >nul 2>&1
copy "C:\Qt\6.9.1\msvc2022_64\bin\Qt6Network.dll" "build\bin\RelWithDebInfo\" >nul 2>&1
copy "C:\Qt\6.9.1\msvc2022_64\bin\Qt6WebSockets.dll" "build\bin\RelWithDebInfo\" >nul 2>&1
copy "C:\Qt\6.9.1\msvc2022_64\bin\Qt6Gui.dll" "build\bin\RelWithDebInfo\" >nul 2>&1

REM Copy OpenSSL DLLs
copy "C:\Program Files\OpenSSL-Win64\bin\libcrypto-3-x64.dll" "build\bin\RelWithDebInfo\" >nul 2>&1
copy "C:\Program Files\OpenSSL-Win64\bin\libssl-3-x64.dll" "build\bin\RelWithDebInfo\" >nul 2>&1

REM Create platforms directory and copy Qt6 platform plugins
if not exist "build\bin\RelWithDebInfo\platforms\" mkdir "build\bin\RelWithDebInfo\platforms\"
copy "C:\Qt\6.9.1\msvc2022_64\plugins\platforms\qwindows.dll" "build\bin\RelWithDebInfo\platforms\" >nul 2>&1

echo DLLs copied successfully!
echo.
echo Files in build directory:
dir "build\bin\RelWithDebInfo\*.dll"
echo.
echo LegacyStream.exe should now run without DLL errors. 