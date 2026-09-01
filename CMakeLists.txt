cmake_minimum_required(VERSION 3.20)
project(MusicPlayer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Subdirectories (Ordered by dependency hierarchy)
add_subdirectory(MusicPlayerSystem)
add_subdirectory(MusicInstrument)
add_subdirectory(MusicBuilderBL)
add_subdirectory(MusicPlayerTesterApp)

# Install Application Asset (HTML Frontend) alongside the binary
install(FILES index.html
    DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Generate CMake Package Config files for 3rd-party find_package(MusicPlayerSDK)
set(CONFIG_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/MusicPlayerSDK")

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/MusicPlayerSDKConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

# Package config file referencing all exported target scripts
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/MusicPlayerSDKConfig.cmake" "
include(CMakeFindDependencyMacro)
if(WIN32)
    # Win32 platform dependencies are linked privately inside target definitions
elseif(UNIX)
    find_dependency(Threads)
endif()

include(\"\${CMAKE_CURRENT_LIST_DIR}/MusicPlayerSystemTargets.cmake\")
include(\"\${CMAKE_CURRENT_LIST_DIR}/MusicInstrumentTargets.cmake\")
include(\"\${CMAKE_CURRENT_LIST_DIR}/MusicBuilderBLTargets.cmake\")
")

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/MusicPlayerSDKConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/MusicPlayerSDKConfigVersion.cmake"
    DESTINATION ${CONFIG_INSTALL_DIR}
)

# ------------------------------------------------------------------------------
# Custom Uninstall Target
# Reads install_manifest.txt to remove all deployed binaries, headers, and assets
# ------------------------------------------------------------------------------
if(NOT TARGET uninstall)
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake" "
if(NOT EXISTS \"${CMAKE_CURRENT_BINARY_DIR}/install_manifest.txt\")
    message(FATAL_ERROR \"Cannot find install manifest: ${CMAKE_CURRENT_BINARY_DIR}/install_manifest.txt\")
endif()

file(READ \"${CMAKE_CURRENT_BINARY_DIR}/install_manifest.txt\" files)
string(REGEX REPLACE \"\\n\" \";\" files \"\${files}\")

foreach(file \${files})
    message(STATUS \"Uninstalling: \$ENV{DESTDIR}\${file}\")
    if(IS_SYMLINK \"\$ENV{DESTDIR}\${file}\" OR EXISTS \"\$ENV{DESTDIR}\${file}\")
        execute_process(
            COMMAND \"${CMAKE_COMMAND}\" -E remove \"\$ENV{DESTDIR}\${file}\"
            OUTPUT_VARIABLE rm_out
            RESULT_VARIABLE rm_retval
        )
        if(NOT \"\${rm_retval}\" STREQUAL 0)
            message(FATAL_ERROR \"Problem when removing \$ENV{DESTDIR}\${file}\")
        endif()
    else()
        message(STATUS \"File \$ENV{DESTDIR}\${file} does not exist.\")
    endif()
endforeach()
")

    add_custom_target(uninstall
        COMMAND ${CMAKE_COMMAND} -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
        COMMENT "Uninstalling all installed files..."
    )
endif()