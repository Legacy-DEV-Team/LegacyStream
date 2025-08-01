add_library(LegacyStreamSSL STATIC
    CertificateManager.cpp
    SSLManager.cpp
    AcmeClient.cpp
    CloudflareApi.cpp
    CryptoUtils.cpp
)

target_include_directories(LegacyStreamSSL PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(LegacyStreamSSL PUBLIC
    Qt6::Core
    Qt6::Network
    OpenSSL::SSL
    OpenSSL::Crypto
    LegacyStreamCore
)

target_compile_features(LegacyStreamSSL PUBLIC cxx_std_17)