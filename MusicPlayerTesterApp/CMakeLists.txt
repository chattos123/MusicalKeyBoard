add_executable(MusicPlayerTesterApp
    src/main.cpp
    src/HttpServer.cpp
    src/HttpUtils.cpp
)

target_include_directories(MusicPlayerTesterApp
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

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
elseif(UNIX)
    find_package(Threads REQUIRED)
    target_link_libraries(MusicPlayerTesterApp
        PRIVATE
            Threads::Threads
    )
endif()

include(GNUInstallDirs)
install(TARGETS MusicPlayerTesterApp
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)