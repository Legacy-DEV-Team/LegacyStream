# LegacyStream Web Interface

## Overview

LegacyStream includes a comprehensive web interface that runs on the streaming ports to display mount point information for IceCast and SHOUTcast protocols. This interface provides real-time statistics, stream management, and an embedded audio player for each mount point.

## Features

### 🎵 Real-time Mount Point Display
- **Enhanced Stream Information**: Shows all active mount points with server type, protocol, format, quality, and listener count
- **Detailed Stream Data**: Server type, description, format, started time, bitrate, quality, listeners and more
- **Current Track Display**: Real-time metadata showing current song, artist, album, and genre
- **Stream Status**: Visual indicators for live streams vs fallback content
- **Listener Statistics**: Peak listeners, current listeners, and bytes served
- **Server Information**: Server name, genre, IRC, ICQ, AIM, and contact details
- **URL Information**: Direct stream URLs, public URLs, and server URLs

### 🎧 Embedded Audio Player
- **HTML5 Audio Player**: Built-in player for each mount point
- **Direct Stream URLs**: Easy access to stream URLs for external players
- **Copy URL Functionality**: One-click copying of stream URLs
- **Mobile Responsive**: Optimized for mobile devices and tablets

### 📊 Real-time Statistics
- **Server Overview**: Total streams, listeners, bytes served, and uptime
- **Auto-refresh**: Statistics update every 5 seconds
- **Visual Dashboard**: Modern, responsive design with gradient backgrounds
- **Protocol Icons**: Visual indicators for IceCast (❄️) and SHOUTcast (📢)

### 🎨 Customizable Interface
- **Custom CSS**: Ability to inject custom stylesheets
- **Custom JavaScript**: Support for custom client-side functionality
- **Custom HTML**: Custom header and footer content
- **Theme Support**: Multiple theme options (planned)

## URL Structure

### Main Pages
- **Root (`/`)**: Main dashboard with server overview and active streams
- **Mount Points (`/mounts`)**: Detailed view of all mount points
- **Status (`/status`)**: Server status and statistics

### Stream Pages
- **Stream Details (`/{mountpoint}`)**: Individual stream page with player and enhanced information
- **Stream Status (`/{mountpoint}/status`)**: Detailed stream information including server details

## Enhanced Mount Point Information

The web interface now displays comprehensive information for each mount point:

### Mount Points Table
- **Server Type**: The type of server (e.g., "LegacyStream Audio Server")
- **Protocol**: IceCast or SHOUTcast with visual icons
- **Format**: Audio format (MP3, AAC, etc.)
- **Quality**: Stream quality indicator (High, Medium, Low, Standard)
- **Listeners**: Current listener count
- **Started**: When the stream started
- **Current Song**: Currently playing track
- **Status**: Live or Fallback status

### Stream Details Page
- **Stream Overview**: Server type, description, quality, and status
- **Technical Information**: Protocol, format, codec, bitrate, sample rate, channels
- **Timing Information**: Started time, uptime
- **Listener Information**: Current, peak, and maximum listeners
- **Server Information**: Server name, genre, IRC, ICQ, AIM contact details
- **URL Information**: Direct stream, public, and server URLs
- **Current Track**: Song, artist, album, and genre information
- **API Endpoints (`/api/*`)**: JSON endpoints for programmatic access

### API Endpoints
- **`/api/stats`**: Server statistics in JSON format
- **`/api/mounts`**: All mount points information
- **`/api/mounts/{mountpoint}`**: Specific mount point details

## Interface Design

### Modern UI Components

#### Header Section
```html
<header class="header">
    <h1>🎵 LegacyStream Audio Server</h1>
    <p>High-Performance Audio Streaming Server</p>
</header>
```

#### Statistics Widget
```html
<div class="stats-container">
    <div class="stat-item">
        <div class="stat-value">5</div>
        <div class="stat-label">Active Streams</div>
    </div>
    <div class="stat-item">
        <div class="stat-value">1,247</div>
        <div class="stat-label">Total Listeners</div>
    </div>
    <!-- More stats... -->
</div>
```

#### Mount Points Table
```html
<table>
    <thead>
        <tr>
            <th>Mount Point</th>
            <th>Protocol</th>
            <th>Codec</th>
            <th>Listeners</th>
            <th>Current Song</th>
            <th>Status</th>
            <th>Actions</th>
        </tr>
    </thead>
    <tbody>
        <!-- Dynamic content -->
    </tbody>
</table>
```

### Responsive Design

The web interface is fully responsive and optimized for:
- **Desktop**: Full-featured interface with all statistics
- **Tablet**: Optimized layout with touch-friendly controls
- **Mobile**: Simplified view with essential information

### Color Scheme
- **Primary**: Gradient blues (#667eea to #764ba2)
- **Success**: Green (#27ae60) for live streams
- **Warning**: Red (#e74c3c) for fallback streams
- **Background**: Glassmorphism with backdrop blur

## Integration with Streaming Protocols

### IceCast Integration
- **Protocol Detection**: Automatic detection of IceCast streams
- **Metadata Support**: ICY metadata extraction and display
- **Mount Point Management**: Dynamic mount point registration
- **Listener Tracking**: Real-time listener count updates

### SHOUTcast Integration
- **v1/v2 Support**: Full compatibility with SHOUTcast protocols
- **Legacy Client Support**: Maintains compatibility with older clients
- **Directory Services**: Integration with SHOUTcast directory
- **Metadata Injection**: Real-time metadata updates

## Configuration

### Web Interface Settings
```ini
[WebInterface]
Enabled=true
UpdateInterval=1000
EnableRealTimeUpdates=true
EnablePlayerEmbed=true
EnableStatistics=true
CustomTheme=default
CustomCSS=
CustomJavaScript=
CustomHTML=
```

### Mount Point Configuration
```ini
[MountPoints]
/live=icecast
/radio=shoutcast
/jazz=icecast
/classical=shoutcast
```

## API Reference

### Mount Point Information Structure
```json
{
    "mountPoint": "/live",
    "protocol": "icecast",
    "codec": "mp3",
    "bitrate": "128000",
    "sampleRate": "44100",
    "channels": "2",
    "currentSong": "Bohemian Rhapsody",
    "currentArtist": "Queen",
    "currentAlbum": "A Night at the Opera",
    "currentGenre": "Rock",
    "listeners": 45,
    "peakListeners": 67,
    "bytesServed": 1234567890,
    "uptime": 3600,
    "isLive": true,
    "hasFallback": true,
    "fallbackFile": "fallback.mp3",
    "lastUpdate": "2024-01-01T12:00:00Z"
}
```

### Server Statistics Structure
```json
{
    "serverName": "LegacyStream Audio Server",
    "serverVersion": "1.0.0",
    "totalMountPoints": 5,
    "totalListeners": 1247,
    "totalBytesServed": "1234567890",
    "serverUptime": 86400,
    "formattedUptime": "1d 0h 0m 0s",
    "formattedBytesServed": "1.15 GB"
}
```

## Customization

### Custom CSS Example
```css
/* Custom theme for dark mode */
body {
    background: linear-gradient(135deg, #1a1a1a 0%, #2d2d2d 100%);
    color: #ffffff;
}

.header {
    background: rgba(255, 255, 255, 0.1);
    backdrop-filter: blur(20px);
}

.stat-item {
    background: linear-gradient(135deg, #4a90e2 0%, #357abd 100%);
}
```

### Custom JavaScript Example
```javascript
// Custom analytics tracking
function trackStreamPlay(mountPoint) {
    gtag('event', 'stream_play', {
        'stream_name': mountPoint,
        'protocol': getStreamProtocol(mountPoint)
    });
}

// Custom player controls
function addCustomControls() {
    const audio = document.querySelector('audio');
    if (audio) {
        // Add custom controls
    }
}
```

## Security Features

### Input Validation
- **HTML Escaping**: All user-generated content is properly escaped
- **URL Validation**: Mount point URLs are validated before processing
- **XSS Protection**: Comprehensive protection against cross-site scripting

### Access Control
- **IP-based Restrictions**: Configurable IP allow/deny lists
- **Rate Limiting**: Protection against abuse and DDoS
- **SSL/TLS Support**: Full HTTPS support with certificate management

## Performance Optimization

### Caching Strategy
- **Static Content**: CSS, JavaScript, and images are cached
- **Dynamic Content**: JSON API responses are cached briefly
- **HTML Generation**: Templates are pre-compiled for efficiency

### Memory Management
- **Efficient Data Structures**: Optimized for high-frequency updates
- **Connection Pooling**: Reuse of HTTP connections
- **Buffer Management**: Efficient handling of large responses

## Troubleshooting

### Common Issues

#### Web Interface Not Loading
```
Error: Web interface not accessible
Solution: Check if web interface is enabled in configuration
```

#### Mount Points Not Displaying
```
Error: No mount points shown
Solution: Verify stream manager is running and streams are active
```

#### Statistics Not Updating
```
Error: Statistics remain static
Solution: Check update interval and ensure timer is running
```

### Debug Information
- **Log Level**: Set to DEBUG for detailed web interface logs
- **Network Monitoring**: Monitor HTTP requests and responses
- **Performance Metrics**: Track response times and memory usage

## Future Enhancements

### Planned Features
1. **Advanced Analytics**: Detailed listener demographics and behavior
2. **Custom Themes**: Multiple pre-built themes and theme builder
3. **Mobile App**: Companion mobile application
4. **WebSocket Support**: Real-time updates via WebSocket
5. **Plugin System**: Extensible interface with plugin support

### Performance Improvements
1. **CDN Integration**: Global content distribution
2. **Compression**: Gzip/Brotli compression for faster loading
3. **Image Optimization**: WebP support and lazy loading
4. **Service Worker**: Offline support and caching

## Conclusion

The LegacyStream web interface provides a comprehensive, modern, and user-friendly way to monitor and interact with audio streams. It combines real-time statistics, embedded players, and customizable design to create a professional streaming server experience.

The interface is designed to be both functional for administrators and accessible for end users, making it an essential component of the LegacyStream audio streaming server. 