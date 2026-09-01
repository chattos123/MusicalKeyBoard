add_library(MusicBuilderBL STATIC
    src/ThreadPool.cpp
    src/DrumLoopEngine.cpp
    src/InstrumentManager.cpp
)

target_include_directories(MusicBuilderBL PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(MusicBuilderBL PUBLIC
    MusicInstrument
    MusicPlayerSystem
)