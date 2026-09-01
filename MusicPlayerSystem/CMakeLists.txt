# Target Definition
add_library(MusicPlayerSystem SHARED
    src/WindowsMusicSystem.cpp
)

# Build-time vs Install-time include interfaces
target_include_directories(MusicPlayerSystem
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Compile definition for dll export (Verify macro name in MusicPlayerSystemExport.h)
target_compile_definitions(MusicPlayerSystem
    PRIVATE
        MUSIC_PLAYER_SYSTEM_EXPORTS
)

# System / Platform Dependencies
if(WIN32)
    target_link_libraries(MusicPlayerSystem 
        PRIVATE 
            ole32 
            uuid 
            ksuser
    )
elseif(UNIX)
    find_package(Threads REQUIRED)
    target_link_libraries(MusicPlayerSystem 
        PRIVATE 
            Threads::Threads
    )
endif()

# Installation & Export Configuration
include(GNUInstallDirs)

install(TARGETS MusicPlayerSystem
    EXPORT MusicPlayerSystemTargets
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
install(EXPORT MusicPlayerSystemTargets
    FILE MusicPlayerSystemTargets.cmake
    NAMESPACE MusicPlayerSDK::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/MusicPlayerSDK
)