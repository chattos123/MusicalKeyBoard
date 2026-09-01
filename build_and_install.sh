#!/usr/bin/env bash
#
# build_and_install.sh - Multi-action build, install, packaging, and uninstallation automation
#

set -euo pipefail

# -----------------------------
# Default Configuration
# -----------------------------
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"
CONFIG="Release"
INSTALL_PREFIX="${SCRIPT_DIR}/install"
RUN_APP=false
ACTION="build"

# -----------------------------
# Formatting Helpers
# -----------------------------
print_cyan() {
    echo -e "\033[1;36m$1\033[0m"
}

print_green() {
    echo -e "\033[1;32m$1\033[0m"
}

print_yellow() {
    echo -e "\033[1;33m$1\033[0m"
}

print_red() {
    echo -e "\033[1;31m$1\033[0m"
}

show_usage() {
    cat << EOF
========================================================================
 Music Player SDK & Synthesizer Automation Engine
========================================================================

Usage:
  $(basename "$0") [ACTION] [OPTIONS]

Actions:
  build         Configure and compile all targets (SDK libs + App) [Default].
  install       Build targets and deploy to installation destination.
  rebuild       Wipe build directory, recompile from scratch, and install.
  clean         Remove build artifacts and local install directories.
  uninstall     Remove all deployed files and empty folders via install_manifest.txt.

Options:
  -c, --config <Config>       Build type: Debug | Release | RelWithDebInfo (Default: Release).
  -p, --prefix <Path>         Destination installation directory (Default: ./install).
  -b, --build-dir <Path>      Custom build output directory (Default: ./build).
  -r, --run                   Launch the application upon successful build/install.
  -h, --help, -?, --h         Display this help message.

Examples:
  # Standard build and install to ./install
  ./$(basename "$0") install

  # Rebuild in Debug mode and run immediately
  ./$(basename "$0") rebuild --config Debug --run

  # Install to a system directory
  ./$(basename "$0") install --prefix "/c/Program Files/SMusicSystem"

  # Uninstall deployed application and SDK
  ./$(basename "$0") uninstall --prefix "/c/Program Files/SMusicSystem"

  # Clean build artifacts
  ./$(basename "$0") clean
========================================================================
EOF
}

# -----------------------------
# Path Resolution Helper
# -----------------------------
resolve_path() {
    local target="$1"
    if [ -d "$target" ]; then
        (cd "$target" && pwd)
    else
        local parent
        parent="$(mkdir -p "$(dirname "$target")" && cd "$(dirname "$target")" && pwd)"
        echo "${parent}/$(basename "$target")"
    fi
}

# -----------------------------
# Action Implementations
# -----------------------------
do_clean() {
    print_yellow "==> Cleaning build and local install artifacts..."
    if [ -d "${BUILD_DIR}" ]; then
        rm -rf "${BUILD_DIR}"
        echo "  - Removed: ${BUILD_DIR}"
    fi
    if [ -d "${INSTALL_PREFIX}" ]; then
        rm -rf "${INSTALL_PREFIX}"
        echo "  - Removed: ${INSTALL_PREFIX}"
    fi
    print_green "Clean completed successfully."
}

do_configure() {
    print_green "==> Configuring CMake (${CONFIG})..."
    
    local CMAKE_GEN_ARGS=()
    if command -v ninja >/dev/null 2>&1; then
        CMAKE_GEN_ARGS+=("-G" "Ninja")
    fi

    cmake -S "${SCRIPT_DIR}" \
          -B "${BUILD_DIR}" \
          -DCMAKE_BUILD_TYPE="${CONFIG}" \
          -DCMAKE_INSTALL_PREFIX="${INSTALL_PREFIX}" \
          "${CMAKE_GEN_ARGS[@]}"
}

do_build() {
    if [ ! -f "${BUILD_DIR}/CMakeCache.txt" ]; then
        do_configure
    fi
    print_green "==> Compiling MusicPlayer targets (${CONFIG})..."
    
    local CPU_CORES
    CPU_CORES="$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)"
    
    cmake --build "${BUILD_DIR}" --config "${CONFIG}" --parallel "${CPU_CORES}"
}

do_install() {
    do_build
    print_green "==> Deploying SDK, Libraries, and Application to: ${INSTALL_PREFIX}..."
    cmake --install "${BUILD_DIR}" --config "${CONFIG}"

    print_cyan "=========================================================="
    print_green " Build & Installation Completed Successfully!"
    print_cyan "=========================================================="
    echo "Installed Layout:"
    echo "  - Executable:   ${INSTALL_PREFIX}/bin/MusicPlayerTesterApp*"
    echo "  - Frontend UI:  ${INSTALL_PREFIX}/bin/index.html"
    echo "  - Headers:      ${INSTALL_PREFIX}/include/"
    echo "  - Libraries:    ${INSTALL_PREFIX}/lib/"
    echo "  - CMake SDK:    ${INSTALL_PREFIX}/lib/cmake/MusicPlayerSDK/"
    echo ""
}

do_uninstall() {
    local MANIFEST="${BUILD_DIR}/install_manifest.txt"
    if [ ! -f "${MANIFEST}" ]; then
        print_red "Error: Cannot uninstall. '${MANIFEST}' not found."
        print_yellow "Ensure the project has been configured and installed before attempting uninstallation."
        exit 1
    fi

    print_yellow "==> Removing installed files listed in ${MANIFEST}..."
    while IFS= read -r file || [ -n "$file" ]; do
        file="${file//$'\r'/}"
        if [ -f "${file}" ] || [ -L "${file}" ]; then
            echo "  - Removing: ${file}"
            rm -f "${file}"
        fi
    done < "${MANIFEST}"

    # Remove lingering empty directories (from deepest to root)
    local sub_dirs=(
        "${INSTALL_PREFIX}/lib/cmake/MusicPlayerSDK"
        "${INSTALL_PREFIX}/lib/cmake"
        "${INSTALL_PREFIX}/lib"
        "${INSTALL_PREFIX}/include/MusicBuilderBL"
        "${INSTALL_PREFIX}/include/MusicInstrument"
        "${INSTALL_PREFIX}/include/MusicPlayerSystem"
        "${INSTALL_PREFIX}/include"
        "${INSTALL_PREFIX}/bin"
        "${INSTALL_PREFIX}"
    )

    for dir in "${sub_dirs[@]}"; do
        if [ -d "${dir}" ] && [ -z "$(ls -A "${dir}" 2>/dev/null)" ]; then
            echo "  - Removing empty folder: ${dir}"
            rmdir "${dir}" 2>/dev/null || true
        fi
    done

    print_green "Uninstallation completed successfully."
}

do_run() {
    local APP_BIN=""
    local candidate_paths=(
        "${INSTALL_PREFIX}/bin/MusicPlayerTesterApp.exe"
        "${INSTALL_PREFIX}/bin/MusicPlayerTesterApp"
        "${BUILD_DIR}/bin/MusicPlayerTesterApp.exe"
        "${BUILD_DIR}/bin/MusicPlayerTesterApp"
        "${BUILD_DIR}/MusicPlayerTesterApp/MusicPlayerTesterApp.exe"
        "${BUILD_DIR}/MusicPlayerTesterApp/MusicPlayerTesterApp"
    )

    for candidate in "${candidate_paths[@]}"; do
        if [ -f "${candidate}" ]; then
            APP_BIN="${candidate}"
            break
        fi
    done

    if [ -n "${APP_BIN}" ]; then
        print_cyan "==> Launching: ${APP_BIN}"
        (cd "$(dirname "${APP_BIN}")" && "./$(basename "${APP_BIN}")")
    else
        print_red "Error: Application binary not found. Please build or install first."
        exit 1
    fi
}

# -----------------------------
# Argument Parsing
# -----------------------------
while [ $# -gt 0 ]; do
    case "$1" in
        build|clean|rebuild|install|uninstall)
            ACTION="$1"
            shift
            ;;
        -c|--config)
            CONFIG="$2"
            shift 2
            ;;
        -p|--prefix)
            INSTALL_PREFIX="$(resolve_path "$2")"
            shift 2
            ;;
        -b|--build-dir)
            BUILD_DIR="$(resolve_path "$2")"
            shift 2
            ;;
        -r|--run)
            RUN_APP=true
            shift
            ;;
        -h|--help|-help|--h|-\?)
            show_usage
            exit 0
            ;;
        *)
            print_red "Unknown parameter or option: $1"
            show_usage
            exit 1
            ;;
    esac
done

# Normalize paths
BUILD_DIR="$(resolve_path "${BUILD_DIR}")"
INSTALL_PREFIX="$(resolve_path "${INSTALL_PREFIX}")"

# -----------------------------
# Execution Flow
# -----------------------------
print_cyan "=========================================================="
print_cyan " Music Player SDK & Synthesizer Automation Engine (${ACTION})"
print_cyan "=========================================================="

case "${ACTION}" in
    clean)
        do_clean
        ;;
    build)
        do_build
        if [ "${RUN_APP}" = true ]; then do_run; fi
        ;;
    rebuild)
        do_clean
        do_install
        if [ "${RUN_APP}" = true ]; then do_run; fi
        ;;
    install)
        do_install
        if [ "${RUN_APP}" = true ]; then do_run; fi
        ;;
    uninstall)
        do_uninstall
        ;;
esac