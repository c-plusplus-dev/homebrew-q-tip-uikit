#!/usr/bin/env bash

set -euo pipefail

# Paths
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

QBUDGET="$HOME/CLionProjects/QBudget"
TOOLCHAIN="$QBUDGET/cmake/mingw-x64.cmake"

VCPKG="$HOME/GitHub/vcpkg"
VCPKG_INSTALLED="$VCPKG/installed/x64-mingw-dynamic"

INSTALL_PREFIX="$QBUDGET/Windows-Dependencies"

# Build configuration
BUILD_TYPE="Release"
BUILD_DIR="$ROOT/build-windows"

while [[ $# -gt 0 ]]; do
    case "$1" in
        -debug)
            BUILD_TYPE="Debug"
            BUILD_DIR="$ROOT/build-windows-debug"
            ;;
        -h|--help)
            echo "Usage: ./build-windows.sh [-debug]"
            echo
            echo "Options:"
            echo "  -debug    Build with debug symbols and no optimizations"
            echo "  -h        Show this help"
            exit 0
            ;;
        *)
            echo "Error: Unknown argument: $1"
            echo "Usage: ./build-windows.sh [-debug]"
            exit 1
            ;;
    esac
    shift
done

# Validate dependencies
for path in "$TOOLCHAIN" "$VCPKG_INSTALLED" "$INSTALL_PREFIX"; do
    if [[ ! -e "$path" ]]; then
        echo "Error: Required path does not exist: $path"
        exit 1
    fi
done

echo "=================================="
echo " Building QTipUIKit for Windows x64"
echo " Configuration: $BUILD_TYPE"
echo "=================================="

CMAKE_ARGS=(
    -S "$ROOT"
    -B "$BUILD_DIR"
    -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN"
    "-DCMAKE_PREFIX_PATH=$INSTALL_PREFIX;$VCPKG_INSTALLED"
    "-DCMAKE_FIND_ROOT_PATH=$INSTALL_PREFIX;$VCPKG_INSTALLED"
    "-DCMAKE_INSTALL_PREFIX=$INSTALL_PREFIX"
    "-DCMAKE_BUILD_TYPE=$BUILD_TYPE"
)

if [[ "$BUILD_TYPE" == "Debug" ]]; then
    CMAKE_ARGS+=(
        -DCMAKE_CXX_FLAGS_DEBUG="-g3 -O0"
    )
fi

cmake "${CMAKE_ARGS[@]}"

cmake --build "$BUILD_DIR" \
    --target QTipUIKit \
    --config "$BUILD_TYPE" \
    --parallel

cmake --install "$BUILD_DIR" --config "$BUILD_TYPE"

echo
echo "QTipUIKit successfully built and installed!"
echo "Configuration: $BUILD_TYPE"
echo "Build directory: $BUILD_DIR"
echo "Install directory: $INSTALL_PREFIX"
