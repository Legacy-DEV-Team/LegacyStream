add_library(LegacyStreamGUI STATIC
    MainWindow.cpp
    OverviewWidget.cpp
    StreamsWidget.cpp
    ListenersWidget.cpp
    CodecsWidget.cpp
    RelaysWidget.cpp
    SSLWidget.cpp
    SettingsWidget.cpp
    LogsWidget.cpp
    StatsWidget.cpp
)

target_include_directories(LegacyStreamGUI PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(LegacyStreamGUI PUBLIC
    Qt6::Core
    Qt6::Widgets
    Qt6::Charts
    LegacyStreamCore
)

target_compile_features(LegacyStreamGUI PUBLIC cxx_std_17)