# Executable Definition
add_executable(MusicPlayerTesterApp
    src/main.cpp
    src/HttpUtils.cpp
)

target_include_directories(MusicPlayerTesterApp
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link Domain Business Logic Library
target_link_libraries(MusicPlayerTesterApp
    PRIVATE
        MusicBuilderBL
)

if(WIN32)
    target_link_libraries(MusicPlayerTesterApp
        PRIVATE
            ws2_32
            shell32
    )
endif()

# Installation Configuration for Application Binary
include(GNUInstallDirs)

install(TARGETS MusicPlayerTesterApp
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)