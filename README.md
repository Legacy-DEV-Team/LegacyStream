# LegacyStream Audio Streaming Server

A high-performance, Windows-based audio streaming server with full IceCast and SHOUTcast compatibility, supporting multiple codecs and SSL/TLS with Let's Encrypt integration.

## Features

### Core Features
- **Protocol Compatibility**: Full IceCast and SHOUTcast protocol support
- **Multi-Codec Support**: MP3, AAC, AAC+, Ogg Vorbis, Opus, and lossless Ogg FLAC
- **In-Stream Metadata**: ICY metadata support for all codecs
- **Configurable Latency**: 1-60 seconds with minimal relay latency
- **High Performance**: Optimized for up to 100 million concurrent listeners using Windows IOCP
- **HTTP Live Streaming (HLS)**: Mobile and Wi-Fi optimized streaming

### Advanced Features
- **Stream Relays**: On-demand mirroring of existing streams
- **Fallback Streams**: Automatic failover to backup streams/files with seamless recovery
- **SSL/TLS Support**: Manual certificates, Let's Encrypt, and Cloudflare integration
- **Real-time Statistics**: Comprehensive listener and performance monitoring

### GUI Application
- **Modern Qt Interface**: User-friendly configuration and monitoring
- **SSL Certificate Management**: Visual certificate status and auto-renewal
- **Port Configuration**: Custom HTTP/HTTPS port settings
- **System Tray Support**: Minimize to tray with notifications

## Requirements

### Development Environment
- **Visual Studio 2019** or later (with C++ workload)
- **CMake 3.20** or later
- **Qt 6.2** or later
- **OpenSSL 1.1.1** or later

### Runtime Dependencies
- **Windows 7** or later (x64)
- **Visual C++ Redistributable** (latest)

## Quick Start

### 1. Clone and Setup
```bash
git clone https://github.com/yourorg/legacystream.git
cd legacystream
mkdir build
cd build
```

### 2. Configure with CMake
```bash
# Configure for Visual Studio
cmake .. -G "Visual Studio 16 2019" -A x64 -DCMAKE_BUILD_TYPE=Release

# Or for Visual Studio 2022
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release
```

### 3. Build
```bash
# Build using CMake
cmake --build . --config Release

# Or open LegacyStream.sln in Visual Studio and build
```

### 4. Run
```bash
# GUI mode
./Release/LegacyStream.exe

# Daemon mode
./Release/LegacyStream.exe --daemon --port 8000 --ssl-port 8443
```

## Project Structure

```
LegacyStream/
├── CMakeLists.txt              # Main build configuration
├── src/
│   ├── main.cpp               # Application entry point
│   ├── core/                  # Core server functionality
│   │   ├── ServerManager.cpp  # Main server coordinator
│   │   ├── Configuration.cpp  # Settings management
│   │   └── Logger.cpp         # Logging system
│   ├── gui/                   # Qt GUI components
│   │   ├── MainWindow.cpp     # Main application window
│   │   ├── OverviewWidget.cpp # Dashboard widget
│   │   └── SSLWidget.cpp      # SSL management interface
│   ├── streaming/             # HTTP server and streaming
│   │   ├── HttpServer.cpp     # High-performance HTTP server
│   │   ├── HLSGenerator.cpp   # HLS segment generation
│   │   └── StreamBuffer.cpp   # Audio stream buffering
│   ├── protocols/             # Protocol implementations
│   │   ├── IceCastServer.cpp  # IceCast protocol handler
│   │   └── SHOUTcastServer.cpp # SHOUTcast protocol handler
│   ├── codecs/                # Audio codec support
│   │   ├── MP3Codec.cpp       # MP3 encoding/decoding
│   │   ├── AACCodec.cpp       # AAC encoding/decoding
│   │   └── OggCodec.cpp       # Ogg Vorbis/FLAC/Opus
│   └── ssl/                   # SSL/TLS management
│       ├── CertificateManager.cpp # Certificate lifecycle
│       ├── AcmeClient.cpp     # Let's Encrypt client
│       └── CloudflareApi.cpp  # Cloudflare DNS integration
├── include/                   # Header files
├── external/                  # Third-party dependencies
└── docs/                     # Documentation
```

## Configuration

### Basic Server Settings
```ini
[Server]
HttpPort=8000
HttpsPort=8443
MaxConnections=100000
MaxStreams=1000
DefaultLatency=5

[SSL]
Enabled=true
AutoRenew=true
LetsEncryptEmail=admin@example.com
LetsEncryptDomains=stream.example.com,radio.example.com
```

### SSL Certificate Setup

#### Manual Certificate
1. Go to **SSL/TLS** tab in the GUI
2. Click **Upload Certificate**
3. Select your certificate (.crt) and private key (.key) files
4. Enter password if required

#### Let's Encrypt (Automatic)
1. Enter your email address
2. Add domain names
3. Click **Generate Certificate**
4. Certificates auto-renew 30 days before expiry

#### Cloudflare Integration
1. Get API token from Cloudflare dashboard
2. Find your Zone ID
3. Enter credentials in SSL settings
4. DNS challenges handled automatically

## Performance Optimization

### Windows IOCP
LegacyStream uses Windows I/O Completion Ports for maximum scalability:
- Non-blocking I/O operations
- Efficient thread utilization
- Supports 100M+ concurrent connections

### Memory Management
- Lock-free data structures where possible
- Memory pools for frequent allocations
- Configurable buffer sizes

### Threading Strategy
- I/O threads: Handle network operations
- Worker threads: Process audio data
- GUI thread: User interface (separate process possible)

## Building from Source

### Prerequisites Setup

#### Install Qt 6
1. Download Qt Online Installer
2. Install Qt 6.2+ with MSVC compiler
3. Add Qt bin directory to PATH

#### Install OpenSSL
```bash
# Using vcpkg (recommended)
vcpkg install openssl:x64-windows

# Or download pre-built binaries
# Add to CMAKE_PREFIX_PATH
```

#### Audio Codec Libraries
Place codec libraries in `external/` directory:
- LAME for MP3 (libmp3lame.lib)
- FAAC for AAC (libfaac.lib)
- Ogg/Vorbis libraries
- Opus library
- FLAC library

### Advanced Build Options

#### Debug Build
```bash
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Debug
cmake --build . --config Debug
```

#### Custom Qt Installation
```bash
cmake .. -DCMAKE_PREFIX_PATH="C:/Qt/6.4.0/msvc2019_64"
```

#### Static Linking
```bash
cmake .. -DBUILD_STATIC=ON
```

## API Reference

### Stream Management
```cpp
// Connect a new stream source
streamManager->addSource("/live", StreamSource::IceCast);

// Set fallback file
streamManager->setFallback("/live", "fallback.mp3");

// Get stream statistics
auto stats = streamManager->getStreamStats("/live");
```

### SSL Certificate Management
```cpp
// Request Let's Encrypt certificate
certificateManager->requestLetsEncryptCertificate(
    {"stream.example.com"}, 
    "admin@example.com"
);

// Check certificate status
auto info = certificateManager->getCertificateInfo();
qDebug() << "Days until expiry:" << info.daysUntilExpiry;
```

## Monitoring and Statistics

### Real-time Metrics
- Current listeners per stream
- Bandwidth usage
- CPU and memory utilization
- Connection statistics
- Error rates

### Logging
- Configurable log levels (DEBUG, INFO, WARN, ERROR)
- Automatic log rotation
- Export functionality

## Troubleshooting

### Common Issues

#### Port Already in Use
```
Error: Failed to bind to port 8000
Solution: Change port in settings or stop conflicting service
```

#### SSL Certificate Errors
```
Error: Certificate validation failed
Solution: Check certificate validity and domain matching
```

#### High Memory Usage
```
Solution: Reduce buffer sizes or max connections in configuration
```

### Performance Tuning

#### For High Listener Counts
- Increase `MaxConnections`
- Adjust `BufferSize` (64KB recommended)
- Monitor CPU usage and adjust thread counts

#### For Low Latency
- Reduce `DefaultLatency` to 1-3 seconds
- Use smaller HLS segments
- Optimize network settings

## Contributing

1. Fork the repository
2. Create a feature branch
3. Follow coding standards (see CODING_STYLE.md)
4. Add tests for new functionality
5. Submit a pull request

## License

LegacyStream is licensed under the MIT License. See LICENSE file for details.

## Support

- **Documentation**: [docs/](docs/)
- **Issues**: GitHub Issues
- **Discussions**: GitHub Discussions
- **Email**: support@legacystream.com

## Acknowledgments

- Qt Framework
- OpenSSL Project
- LAME MP3 Encoder
- Xiph.Org Foundation (Ogg, Vorbis, FLAC, Opus)
- Let's Encrypt / ACME protocol
- Cloudflare API