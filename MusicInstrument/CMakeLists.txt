# Target Definition
add_library(MusicInstrument SHARED
    src/Guitar.cpp
    src/Kalimba.cpp
    src/Piano.cpp
    src/DrumKit.cpp
    src/BassGuitar.cpp
    src/Mandolin.cpp
    src/Violin.cpp
    src/Trumpet.cpp
    src/Harmonium.cpp
    src/Saxophone.cpp
)

# Build-time vs Install-time include interfaces
target_include_directories(MusicInstrument
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Compile definition for dll export
target_compile_definitions(MusicInstrument
    PRIVATE
        MUSIC_INSTRUMENT_EXPORTS
)

# Link against internal subsystem library
target_link_libraries(MusicInstrument
    PUBLIC
        MusicPlayerSystem
)

# Installation & Export Configuration
include(GNUInstallDirs)

install(TARGETS MusicInstrument
    EXPORT MusicInstrumentTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Install Header files
install(DIRECTORY include/ 
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
)

# Export target script
install(EXPORT MusicInstrumentTargets
    FILE MusicInstrumentTargets.cmake
    NAMESPACE MusicPlayerSDK::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/MusicPlayerSDK
)