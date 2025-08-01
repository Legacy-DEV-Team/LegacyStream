# LegacyStream Web Interface

## Overview

The LegacyStream Web Interface provides a comprehensive, modern web-based user experience for managing and monitoring your audio streaming server. It features real-time updates, interactive controls, mobile-responsive design, and advanced analytics.

## Features

### 🌐 **Core Web Interface**
- **Custom Web UX/UI** - Mount point display on IceCast/ShoutCast ports
- **Enhanced Mount Point Display** - Server type, description, format, started time
- **Quality Indicators** - Visual quality status with CSS styling
- **Stream Details Page** - Comprehensive stream information display
- **JSON API** - Programmatic access to server and stream statistics
- **HTML5 Audio Player** - Embedded player within web interface
- **Real-time Mount Point Updates** - Dynamic mount point information
- **Server Information Display** - Location, hostname, metadata
- **URL Information** - Stream URLs, public URLs, server URLs

### 🔄 **Real-time WebSocket Updates**
- **Live Statistics** - Real-time updates without page refresh
- **WebSocket Server** - Running on port 8081 by default
- **Automatic Reconnection** - Handles connection drops gracefully
- **JSON Data Streaming** - Structured data for easy consumption
- **Client Management** - Multiple concurrent WebSocket connections

### 🎛️ **Interactive Controls**
- **Stream Management** - Start, stop, restart streams via web interface
- **Quality Settings** - Adjust stream quality (High, Medium, Low)
- **Bitrate Control** - Set stream bitrate (32-320 kbps)
- **Metadata Editing** - Update song title and artist information
- **Real-time Feedback** - Immediate response to control actions

### 📱 **Mobile Responsive Design**
- **Mobile-Optimized Interface** - Touch-friendly controls and navigation
- **Responsive Layout** - Adapts to different screen sizes
- **Mobile Navigation** - Tab-based navigation for mobile devices
- **Touch Controls** - Large buttons and touch-friendly interactions
- **Progressive Web App** - Can be installed on mobile devices

### 📊 **Advanced Analytics Dashboard**
- **Server Performance** - Real-time server metrics and statistics
- **Listener Statistics** - Detailed listener analytics and trends
- **Stream Analytics** - Individual stream performance data
- **Network Usage** - Bandwidth and data transfer statistics
- **Export Functionality** - Download analytics data as JSON
- **Interactive Charts** - Visual representation of statistics

## Pages and Routes

### Main Pages
- `/` - Main status page with server overview
- `/mountpoints` - Mount points listing and management
- `/analytics` - Advanced analytics dashboard
- `/mobile` - Mobile-optimized interface
- `/stream/{mountpoint}` - Individual stream details and controls

### API Endpoints
- `/api/stats` - Server statistics (GET)
- `/api/mountpoints` - Mount points data (GET)
- `/api/analytics` - Analytics data (GET)
- `/api/relay` - Relay statistics (GET)
- `/api/control` - Stream control (POST)

### WebSocket
- `ws://hostname:8081` - Real-time data stream

## Configuration

### Web Interface Settings
```ini
[WebInterface]
EnableWebInterface=true
EnableRealTimeUpdates=true
EnablePlayerEmbed=true
EnableStatistics=true
EnableWebSockets=true
EnableInteractiveControls=true
EnableMobileResponsive=true
EnableAnalyticsDashboard=true
UpdateInterval=1000
WebSocketPort=8081
```

### Customization
```ini
[WebInterface]
CustomTheme=default
CustomCSS=
CustomJavaScript=
CustomHTML=
```

## Usage Examples

### Accessing the Web Interface
1. Start the LegacyStream server
2. Open a web browser
3. Navigate to `http://localhost:8080`
4. The main status page will display

### Using Interactive Controls
1. Navigate to a stream page: `/stream/live`
2. Use the control buttons to start/stop/restart streams
3. Adjust quality settings using the dropdown
4. Set bitrate using the number input
5. Update metadata using the text inputs

### Mobile Interface
1. Navigate to `/mobile` on a mobile device
2. Use the tab navigation to switch between sections
3. Streams are displayed as cards with touch controls
4. Statistics are optimized for mobile viewing

### Analytics Dashboard
1. Navigate to `/analytics`
2. View real-time server performance metrics
3. Monitor listener statistics and trends
4. Export data using the export button
5. Charts update automatically every 30 seconds

### WebSocket Integration
```javascript
// Connect to WebSocket server
const ws = new WebSocket('ws://localhost:8081');

// Handle incoming messages
ws.onmessage = function(event) {
    const data = JSON.parse(event.data);
    console.log('Real-time update:', data);
};

// Send control commands
ws.send(JSON.stringify({
    action: 'controlStream',
    mountPoint: '/live',
    controlAction: 'start'
}));
```

## API Reference

### Server Statistics API
```json
{
    "server": {
        "uptime": 3600,
        "totalListeners": 150,
        "totalBytesServed": 1073741824,
        "activeMountPoints": 3,
        "peakListeners": 200
    },
    "mountPoints": [
        {
            "mountPoint": "/live",
            "listeners": 50,
            "peakListeners": 75,
            "bytesServed": 536870912,
            "uptime": 1800,
            "quality": "High",
            "isLive": true
        }
    ]
}
```

### Analytics API
```json
{
    "server": {
        "uptime": 3600,
        "totalListeners": 150,
        "totalBytesServed": 1073741824,
        "activeMountPoints": 3,
        "peakListeners": 200
    },
    "mountPoints": [
        {
            "mountPoint": "/live",
            "listeners": 50,
            "peakListeners": 75,
            "bytesServed": 536870912,
            "uptime": 1800,
            "quality": "High",
            "isLive": true
        }
    ],
    "performance": {
        "cpuUsage": 25.5,
        "memoryUsage": 512,
        "networkUsage": 1024
    }
}
```

### Control API
```json
// POST /api/control
{
    "mountPoint": "/live",
    "action": "start"
}

// Response
{
    "status": "success",
    "action": "start",
    "mountPoint": "/live"
}
```

## WebSocket Messages

### Real-time Updates
```json
{
    "type": "realTimeUpdate",
    "timestamp": "2024-01-01T12:00:00Z",
    "serverStats": { ... },
    "mountPoints": { ... },
    "analytics": { ... }
}
```

### Control Commands
```json
{
    "action": "controlStream",
    "mountPoint": "/live",
    "controlAction": "start"
}
```

### Control Response
```json
{
    "type": "controlResponse",
    "mountPoint": "/live",
    "action": "start",
    "status": "success"
}
```

## CSS Classes

### Mobile Interface
- `.mobile-container` - Main mobile container
- `.mobile-header` - Mobile header with gradient background
- `.mobile-nav` - Navigation tabs
- `.nav-btn` - Navigation button
- `.mobile-section` - Content sections
- `.stream-card` - Stream information cards

### Analytics Dashboard
- `.analytics-container` - Main analytics container
- `.analytics-header` - Dashboard header
- `.analytics-grid` - Grid layout for charts
- `.analytics-card` - Individual chart containers
- `.chart-container` - Chart display areas
- `.analytics-details` - Detailed statistics section

### Interactive Controls
- `.interactive-controls` - Control panel container
- `.control-group` - Group of related controls
- `.control-buttons` - Button container
- `.control-btn` - Individual control buttons
- `.status-indicator` - Stream status display
- `.status-dot` - Status indicator dot

## JavaScript Functions

### Mobile Interface
- `showSection(sectionId)` - Switch between mobile sections
- `controlStream(mountPoint, action)` - Send stream control commands
- `updateMobileInterface(data)` - Update mobile interface with WebSocket data

### Analytics Dashboard
- `refreshAnalytics()` - Fetch latest analytics data
- `updateCharts()` - Update all dashboard charts
- `exportData()` - Export analytics data as JSON
- `formatDuration(seconds)` - Format duration for display
- `formatBytes(bytes)` - Format bytes for display

### WebSocket Integration
- `connectWebSocket()` - Establish WebSocket connection
- `handleWebSocketMessage(event)` - Process incoming WebSocket messages
- `sendWebSocketCommand(command)` - Send commands via WebSocket

## Troubleshooting

### Common Issues

**WebSocket Connection Failed**
- Check if WebSocket server is running on port 8081
- Verify firewall settings allow WebSocket connections
- Ensure browser supports WebSocket protocol

**Mobile Interface Not Responsive**
- Check viewport meta tag is present
- Verify CSS media queries are working
- Test on different mobile devices

**Analytics Dashboard Not Loading**
- Check if analytics data is being generated
- Verify API endpoints are accessible
- Check browser console for JavaScript errors

**Interactive Controls Not Working**
- Verify WebSocket connection is established
- Check if stream manager is properly initialized
- Ensure control permissions are set correctly

### Performance Optimization

**High Memory Usage**
- Reduce update interval for real-time updates
- Limit number of concurrent WebSocket connections
- Optimize analytics data generation

**Slow Page Load**
- Enable compression for static files
- Optimize CSS and JavaScript files
- Use CDN for external resources

**WebSocket Lag**
- Increase WebSocket buffer size
- Optimize JSON data structure
- Reduce update frequency if needed

## Security Considerations

### WebSocket Security
- WebSocket server runs on separate port (8081)
- No authentication required for basic functionality
- Consider implementing authentication for production use

### API Security
- API endpoints are publicly accessible
- No rate limiting implemented
- Consider adding authentication and rate limiting

### Mobile Security
- Mobile interface uses same security as main interface
- HTTPS recommended for production deployment
- Consider implementing certificate pinning

## Future Enhancements

### Planned Features
- **Authentication System** - User login and role-based access
- **Advanced Charts** - Interactive charts with zoom and pan
- **Custom Themes** - User-selectable interface themes
- **Push Notifications** - Browser notifications for events
- **Offline Support** - Service worker for offline functionality
- **PWA Features** - Installable web app with offline capabilities

### Performance Improvements
- **WebSocket Compression** - Compress WebSocket messages
- **Caching Strategy** - Implement intelligent caching
- **Lazy Loading** - Load components on demand
- **Code Splitting** - Split JavaScript into smaller chunks

---

*Last Updated: [Current Date]*
*Version: 2.0* 