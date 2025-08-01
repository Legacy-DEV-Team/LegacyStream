# Statistic Relay System

## Overview

The Statistic Relay System allows LegacyStream to relay stream statistics to external Shoutcast and Icecast servers. This enables integration with existing streaming infrastructure and provides real-time statistics updates to external monitoring systems.

## Features

- **Shoutcast Relay**: Relay statistics to Shoutcast v1/v2 servers
- **Icecast Relay**: Relay statistics to Icecast servers
- **Real-time Updates**: Automatic statistics updates at configurable intervals
- **Multiple Relays**: Support for multiple relay configurations
- **Error Handling**: Robust error handling and reconnection logic
- **JSON API**: RESTful API for relay management and monitoring

## Configuration

### Server Authentication and Location Settings

Before configuring relays, you should set up the server's authentication and location settings:

```ini
[Server]
# Server location and hostname settings
Location=My Radio Station
Hostname=stream.mystation.com

# Authentication settings for sources and relays
SourcePassword=source123
RelayPassword=relay456
AdminUsername=admin
AdminPassword=admin123
```

**Server Settings:**
- `Location`: Human-readable location description
- `Hostname`: Domain name or IP address of your server (used in relay URLs)
- `SourcePassword`: Password for stream sources
- `RelayPassword`: Default password for relay authentication
- `AdminUsername`: Default admin username for Icecast relays
- `AdminPassword`: Default admin password for Icecast relays

### Basic Configuration

The statistic relay system is configured through the main configuration file:

```ini
[StatisticRelay]
Enabled=true
UpdateInterval=30
MaxRelays=50
```

### Relay Configuration

#### Shoutcast Relay

```ini
[ShoutcastRelay]
Name=my_shoutcast_relay
Host=shoutcast.example.com
Port=8000
Password=admin
SID=1
MountPoint=/live
Enabled=true
UpdateInterval=30
```

**Parameters:**
- `Name`: Unique identifier for the relay
- `Host`: Shoutcast server hostname or IP
- `Port`: Shoutcast server port (default: 8000)
- `Password`: Admin password for authentication (if empty, uses server's relay password)
- `SID`: Stream ID (for Shoutcast v2)
- `MountPoint`: Local mount point to relay
- `Enabled`: Enable/disable this relay
- `UpdateInterval`: Update interval in seconds

#### Icecast Relay

```ini
[IcecastRelay]
Name=my_icecast_relay
Host=icecast.example.com
Port=8000
Username=admin
Password=admin
Alias=/live
MountPoint=/live
Enabled=true
UpdateInterval=30
```

**Parameters:**
- `Name`: Unique identifier for the relay
- `Host`: Icecast server hostname or IP
- `Port`: Icecast server port (default: 8000)
- `Username`: Admin username for authentication (if empty, uses server's admin username)
- `Password`: Admin password for authentication (if empty, uses server's admin password)
- `Alias`: Remote mount point alias (e.g., "/live")
- `MountPoint`: Local mount point to relay
- `Enabled`: Enable/disable this relay
- `UpdateInterval`: Update interval in seconds

### Server Hostname Integration

The server's hostname setting is automatically used in relay URLs when sending statistics:

- **Shoutcast**: The hostname is used in the `url`, `serverurl`, and `streamurl` parameters
- **Icecast**: The hostname is used in the `server_url` parameter

This ensures that external servers receive the correct server URL for your stream, enabling proper integration with directory listings and monitoring systems.

## API Reference

### Relay Statistics API

**Endpoint:** `GET /api/relays`

**Response:**
```json
{
  "shoutcastRelays": [
    {
      "name": "my_shoutcast_relay",
      "type": "shoutcast",
      "host": "shoutcast.example.com",
      "port": 8000,
      "enabled": true,
      "mountPoint": "/live",
      "connected": true,
      "statistics": {
        "currentListeners": 5,
        "peakListeners": 10,
        "bytesServed": "1024000",
        "uptime": 3600,
        "currentSong": "Song Title",
        "currentArtist": "Artist Name",
        "lastUpdate": "2024-01-01T12:00:00Z"
      }
    }
  ],
  "icecastRelays": [
    {
      "name": "my_icecast_relay",
      "type": "icecast",
      "host": "icecast.example.com",
      "port": 8000,
      "enabled": true,
      "alias": "/live",
      "mountPoint": "/live",
      "connected": true,
      "statistics": {
        "currentListeners": 3,
        "peakListeners": 8,
        "bytesServed": "512000",
        "uptime": 1800,
        "currentSong": "Song Title",
        "currentArtist": "Artist Name",
        "lastUpdate": "2024-01-01T12:00:00Z"
      }
    }
  ],
  "totalRelays": 2,
  "activeRelays": 2
}
```

## Programmatic Interface

### C++ API

```cpp
#include "streaming/StatisticRelayManager.h"

// Get the statistic relay manager
auto& relayManager = ServerManager::instance().statisticRelayManager();

// Add a Shoutcast relay
StatisticRelay::ShoutcastRelayConfig shoutcastConfig;
shoutcastConfig.host = "shoutcast.example.com";
shoutcastConfig.port = 8000;
shoutcastConfig.password = "admin";
shoutcastConfig.sid = "1";
shoutcastConfig.mountPoint = "/live";
shoutcastConfig.enabled = true;
shoutcastConfig.updateInterval = 30;

relayManager->addShoutcastRelay("my_relay", shoutcastConfig);

// Add an Icecast relay
StatisticRelay::IcecastRelayConfig icecastConfig;
icecastConfig.host = "icecast.example.com";
icecastConfig.port = 8000;
icecastConfig.username = "admin";
icecastConfig.password = "admin";
icecastConfig.alias = "/live";
icecastConfig.mountPoint = "/live";
icecastConfig.enabled = true;
icecastConfig.updateInterval = 30;

relayManager->addIcecastRelay("my_icecast_relay", icecastConfig);

// Get relay statistics
QJsonObject stats = relayManager->getRelayStatisticsJson();
```

### Signals and Slots

```cpp
// Connect to relay events
connect(relayManager, &StatisticRelay::StatisticRelayManager::relayConnected,
        this, [](const QString& name, const QString& type) {
    qDebug() << "Relay connected:" << name << "(" << type << ")";
});

connect(relayManager, &StatisticRelay::StatisticRelayManager::relayError,
        this, [](const QString& name, const QString& error) {
    qWarning() << "Relay error:" << name << "-" << error;
});

connect(relayManager, &StatisticRelay::StatisticRelayManager::statisticsRelayed,
        this, [](const QString& name, const QString& type) {
    qDebug() << "Statistics relayed to:" << name << "(" << type << ")";
});
```

## Protocol Details

### Shoutcast Protocol

The statistic relay sends POST requests to the Shoutcast admin interface:

**URL:** `http://host:port/admin.cgi`

**Parameters:**
- `mode=updinfo`: Update stream information
- `pass=password`: Admin password
- `sid=stream_id`: Stream ID (v2 only)
- `song=song_title`: Current song title
- `listeners=current_listeners`: Current listener count
- `maxlisteners=peak_listeners`: Peak listener count
- `bitrate=bitrate`: Stream bitrate
- `samplerate=sample_rate`: Sample rate
- `channels=channels`: Number of channels

### Icecast Protocol

The statistic relay sends POST requests to the Icecast admin interface:

**URL:** `http://host:port/admin/stats.xml`

**Parameters:**
- `username=admin`: Admin username
- `password=admin`: Admin password
- `mount=/live`: Mount point alias
- `listeners=current_listeners`: Current listener count
- `peak_listeners=peak_listeners`: Peak listener count
- `bytes_served=bytes_served`: Bytes served
- `uptime=uptime`: Stream uptime
- `current_song=song_title`: Current song title
- `current_artist=artist_name`: Current artist
- `codec=codec`: Audio codec
- `bitrate=bitrate`: Stream bitrate

## Error Handling

The statistic relay system includes comprehensive error handling:

- **Network Errors**: Automatic retry with exponential backoff
- **Authentication Errors**: Logged and reported
- **Invalid Configuration**: Validation and error reporting
- **Connection Timeouts**: Configurable timeout settings
- **Server Errors**: HTTP error code handling

## Performance Considerations

- **Update Intervals**: Configurable to balance accuracy vs. server load
- **Connection Pooling**: Efficient network connection management
- **Asynchronous Processing**: Non-blocking relay operations
- **Memory Management**: Efficient data structures and cleanup
- **Thread Safety**: Thread-safe operations for concurrent access

## Security

- **Authentication**: Secure credential handling
- **HTTPS Support**: SSL/TLS encryption for sensitive data
- **Input Validation**: Comprehensive parameter validation
- **Error Sanitization**: Safe error message handling

## Monitoring and Logging

### Log Categories

- `statisticRelay`: Main relay operations
- `statisticRelay.network`: Network communication
- `statisticRelay.auth`: Authentication events
- `statisticRelay.stats`: Statistics processing

### Log Levels

- **DEBUG**: Detailed operation information
- **INFO**: General relay status
- **WARNING**: Non-critical issues
- **ERROR**: Relay failures and errors

## Troubleshooting

### Common Issues

1. **Connection Refused**
   - Check server hostname and port
   - Verify firewall settings
   - Ensure server is running

2. **Authentication Failed**
   - Verify username/password
   - Check admin credentials
   - Ensure proper permissions

3. **Statistics Not Updating**
   - Check update interval settings
   - Verify mount point configuration
   - Review server logs for errors

4. **High CPU Usage**
   - Reduce update frequency
   - Check for network issues
   - Monitor relay count

### Debug Commands

```bash
# Check relay status
curl http://localhost:8000/api/relays

# View relay logs
tail -f logs/statisticRelay.log

# Test relay connectivity
telnet shoutcast.example.com 8000
```

## Future Enhancements

- **WebSocket Support**: Real-time relay status updates
- **Advanced Authentication**: OAuth and API key support
- **Load Balancing**: Multiple relay endpoints
- **Statistics Aggregation**: Combined relay statistics
- **Custom Protocols**: Support for additional streaming protocols
- **Relay Templates**: Predefined relay configurations
- **Automated Discovery**: Auto-discovery of relay endpoints 