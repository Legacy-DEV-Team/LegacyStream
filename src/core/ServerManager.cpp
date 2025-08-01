#include "core/ServerManager.h"
#include "core/Configuration.h"
#include "core/Logger.h"
#include "streaming/HttpServer.h"
#include "streaming/StreamManager.h"
#include "streaming/RelayManager.h"
#include "streaming/MetadataManager.h"
#include "streaming/WebInterface.h"
#include "streaming/StatisticRelayManager.h"
#include "ssl/SSLManager.h"
#include "ssl/CertificateManager.h"
#include "streaming/HLSGenerator.h"
#include "protocols/IceCastServer.h"
#include "protocols/SHOUTcastServer.h"

#include <QLoggingCategory>
#include <QTimer>
#include <QDateTime>
#include <QNetworkAccessManager>
#include <QThread>

Q_LOGGING_CATEGORY(serverManager, "serverManager")

namespace LegacyStream {

ServerManager& ServerManager::instance()
{
    static ServerManager instance;
    return instance;
}

ServerManager::ServerManager()
    : QObject(nullptr)
    , m_statsTimer(new QTimer(this))
    , m_startTime(QDateTime::currentSecsSinceEpoch())
    , m_networkManager(std::make_unique<QNetworkAccessManager>())
{
    qCDebug(serverManager) << "ServerManager created";
}

ServerManager::~ServerManager()
{
    shutdown();
}

bool ServerManager::initialize()
{
    if (m_initialized.load()) {
        qCWarning(serverManager) << "ServerManager already initialized";
        return true;
    }

    qCInfo(serverManager) << "Initializing ServerManager";

    try {
        initializeComponents();
        setupStatisticsTimer();
        
        m_initialized.store(true);
        qCInfo(serverManager) << "ServerManager initialized successfully";
        return true;
    }
    catch (const std::exception& e) {
        qCCritical(serverManager) << "Failed to initialize ServerManager:" << e.what();
        return false;
    }
}

void ServerManager::initializeComponents()
{
    auto& config = Configuration::instance();
    
    // Initialize SSL manager first
    m_sslManager = std::make_unique<SSLManager>();
    m_certificateManager = std::make_unique<SSL::CertificateManager>();
    
    // Initialize HTTP server
    m_httpServer = std::make_unique<HttpServer>();
    m_httpServer->setSSLManager(m_sslManager.get());
    m_httpServer->setMaxConnections(config.maxConnections());
    
    // Initialize stream manager
    m_streamManager = std::make_unique<StreamManager>();
    m_httpServer->setStreamManager(m_streamManager.get());
    
    // Initialize relay manager
    m_relayManager = std::make_unique<RelayManager>();
    m_relayManager->setStreamManager(m_streamManager.get());
    
    // Initialize metadata manager
    m_metadataManager = std::make_unique<MetadataManager>();
    
    // Initialize HLS generator
    m_hlsGenerator = std::make_unique<HLSGenerator>();
    m_hlsGenerator->setStreamManager(m_streamManager.get());
    
    // Initialize web interface
    m_webInterface = std::make_unique<WebInterface::WebInterface>();
    m_webInterface->initialize(m_httpServer.get(), m_streamManager.get(), m_statisticRelayManager.get());
    
    // Initialize statistic relay manager
    m_statisticRelayManager = std::make_unique<StatisticRelay::StatisticRelayManager>();
    m_statisticRelayManager->initialize(m_streamManager.get());
    
    // Initialize protocol servers
    if (config.iceCastEnabled()) {
        m_iceCastServer = std::make_unique<Protocols::IceCastServer>();
        m_iceCastServer->setStreamManager(m_streamManager.get());
    }
    
    if (config.shoutCastEnabled()) {
        m_shoutCastServer = std::make_unique<Protocols::SHOUTcastServer>();
        m_shoutCastServer->setStreamManager(m_streamManager.get());
    }
    
    // Connect signals
    connect(m_httpServer.get(), &HttpServer::connectionAccepted,
            this, &ServerManager::listenerConnected);
    connect(m_httpServer.get(), &HttpServer::connectionClosed,
            this, &ServerManager::listenerDisconnected);
    connect(m_httpServer.get(), &HttpServer::errorOccurred,
            this, &ServerManager::handleServerError);
    
    connect(m_streamManager.get(), &StreamManager::streamConnected,
            this, &ServerManager::streamConnected);
    connect(m_streamManager.get(), &StreamManager::streamDisconnected,
            this, &ServerManager::streamDisconnected);
    
    // Connect web interface signals
    connect(m_webInterface.get(), &WebInterface::WebInterface::mountPointAdded,
            this, &ServerManager::streamConnected);
    connect(m_webInterface.get(), &WebInterface::WebInterface::mountPointRemoved,
            this, &ServerManager::streamDisconnected);
    
    // Connect statistic relay signals
    connect(m_statisticRelayManager.get(), &StatisticRelay::StatisticRelayManager::relayConnected,
            this, [this](const QString& name, const QString& type) {
                qCInfo(serverManager) << "Statistic relay connected:" << name << "(" << type << ")";
            });
    connect(m_statisticRelayManager.get(), &StatisticRelay::StatisticRelayManager::relayError,
            this, [this](const QString& name, const QString& error) {
                qCWarning(serverManager) << "Statistic relay error:" << name << "-" << error;
            });
}

void ServerManager::setupStatisticsTimer()
{
    m_statsTimer->setInterval(1000); // Update every second
    connect(m_statsTimer, &QTimer::timeout, this, &ServerManager::updateStats);
}

bool ServerManager::startServers()
{
    if (m_isRunning.load()) {
        qCWarning(serverManager) << "Servers already running";
        return true;
    }
    
    if (!m_initialized.load()) {
        qCWarning(serverManager) << "ServerManager not initialized";
        return false;
    }

    qCInfo(serverManager) << "Starting servers";

    auto& config = Configuration::instance();
    
    // Start HTTP server
    if (!m_httpServer->startServer(config.httpPort(), config.httpsPort())) {
        qCCritical(serverManager) << "Failed to start HTTP server";
        return false;
    }
    
    // Start protocol servers
    if (m_iceCastServer && !m_iceCastServer->start()) {
        qCWarning(serverManager) << "Failed to start IceCast server";
    }
    
    if (m_shoutCastServer && !m_shoutCastServer->start()) {
        qCWarning(serverManager) << "Failed to start SHOUTcast server";
    }
    
    // Start relay manager
    if (config.relayEnabled()) {
        m_relayManager->start();
    }
    
    // Start statistic relay manager
    if (config.statisticRelayEnabled()) {
        m_statisticRelayManager->start();
    }
    
    // Start HLS generator
    if (config.hlsEnabled()) {
        m_hlsGenerator->start();
    }
    
    // Start statistics timer
    m_statsTimer->start();
    
    m_isRunning.store(true);
    m_startTime = QDateTime::currentSecsSinceEpoch();
    
    qCInfo(serverManager) << "Servers started successfully";
    emit serverStarted();
    
    return true;
}

void ServerManager::stopServers()
{
    if (!m_isRunning.load()) {
        qCWarning(serverManager) << "Servers not running";
        return;
    }

    qCInfo(serverManager) << "Stopping servers";

    // Stop statistics timer
    m_statsTimer->stop();
    
    // Stop HLS generator
    if (m_hlsGenerator) {
        m_hlsGenerator->stop();
    }
    
    // Stop relay manager
    if (m_relayManager) {
        m_relayManager->stop();
    }
    
    // Stop statistic relay manager
    if (m_statisticRelayManager) {
        m_statisticRelayManager->stop();
    }
    
    // Stop protocol servers
    if (m_shoutCastServer) {
        m_shoutCastServer->stop();
    }
    
    if (m_iceCastServer) {
        m_iceCastServer->stop();
    }
    
    // Stop HTTP server
    if (m_httpServer) {
        m_httpServer->stopServer();
    }
    
    m_isRunning.store(false);
    
    qCInfo(serverManager) << "Servers stopped";
    emit serverStopped();
}

void ServerManager::restartServers()
{
    qCInfo(serverManager) << "Restarting servers";
    stopServers();
    
    // Wait a moment for cleanup
    QThread::msleep(100);
    
    startServers();
}

void ServerManager::shutdown()
{
    if (m_isRunning.load()) {
        stopServers();
    }
    
    // Clean up components
    m_webInterface.reset();
    m_statisticRelayManager.reset();
    m_hlsGenerator.reset();
    m_shoutCastServer.reset();
    m_iceCastServer.reset();
    m_metadataManager.reset();
    m_relayManager.reset();
    m_streamManager.reset();
    m_httpServer.reset();
    m_certificateManager.reset();
    m_sslManager.reset();
    
    m_initialized.store(false);
    
    qCInfo(serverManager) << "ServerManager shut down";
}

ServerManager::ServerStats ServerManager::getStats() const
{
    ServerStats stats;
    stats.uptime = QDateTime::currentSecsSinceEpoch() - m_startTime;
    
    if (m_httpServer) {
        auto httpStats = m_httpServer->getStats();
        stats.totalConnections = httpStats.totalConnections.load();
        stats.currentListeners = httpStats.activeConnections.load();
        stats.totalBytesServed = httpStats.bytesServed.load();
    }
    
    if (m_streamManager) {
        stats.activeStreams = m_streamManager->getActiveStreamCount();
    }
    
    // Get system stats (simplified - would need platform-specific implementation)
    stats.cpuUsage = 0.0; // TODO: Implement CPU usage monitoring
    stats.memoryUsage = 0.0; // TODO: Implement memory usage monitoring
    
    return stats;
}

void ServerManager::updateStats()
{
    m_currentStats = getStats();
    emit statsUpdated(m_currentStats);
}

void ServerManager::handleServerError(const QString& error)
{
    qCWarning(serverManager) << "Server error:" << error;
    emit serverError(error);
}

} // namespace LegacyStream