#!/bin/bash
#
# install.sh - Setup development environment for Anagram Chain Finder
#
# This script checks for required dependencies, installs missing ones,
# and builds the project.
#
# Usage: ./install.sh [options]
#   --no-docker    Skip Docker installation
#   --no-arm       Skip ARM cross-compiler installation
#   --help         Show this help message
#
# Supported systems: Ubuntu/Debian (apt-based)
#

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Default options
INSTALL_DOCKER=true
INSTALL_ARM=true

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --no-docker)
            INSTALL_DOCKER=false
            shift
            ;;
        --no-arm)
            INSTALL_ARM=false
            shift
            ;;
        --help)
            head -15 "$0" | tail -11
            exit 0
            ;;
        *)
            echo -e "${RED}Unknown option: $1${NC}"
            exit 1
            ;;
    esac
done

echo ""
echo -e "${BLUE}======================================${NC}"
echo -e "${BLUE}  Anagram Chain Finder - Setup Script${NC}"
echo -e "${BLUE}======================================${NC}"
echo ""

# Function to check if command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Function to print status
print_status() {
    local status=$2
    if [ "$status" = "ok" ]; then
        echo -e "${GREEN}[OK]${NC} $1"
    elif [ "$status" = "skip" ]; then
        echo -e "${YELLOW}[SKIP]${NC} $1"
    elif [ "$status" = "install" ]; then
        echo -e "${YELLOW}[INSTALLING]${NC} $1"
    else
        echo -e "${RED}[MISSING]${NC} $1"
    fi
}

# Check if running on Debian/Ubuntu
if ! command_exists apt-get; then
    echo -e "${RED}Error: This script requires apt-get (Debian/Ubuntu)${NC}"
    echo "For other systems, please install dependencies manually:"
    echo "  - GCC (build-essential)"
    echo "  - Make"
    echo "  - ARM cross-compiler (gcc-arm-linux-gnueabihf)"
    echo "  - QEMU (qemu-user)"
    echo "  - Docker (optional)"
    exit 1
fi

echo "Checking dependencies..."
echo ""

# Track if we need sudo
NEED_SUDO=false

# Check GCC
if command_exists gcc; then
    GCC_VERSION=$(gcc --version | head -1 | awk '{print $4}')
    print_status "GCC $GCC_VERSION" "ok"
else
    print_status "GCC" "missing"
    NEED_SUDO=true
fi

# Check Make
if command_exists make; then
    MAKE_VERSION=$(make --version | head -1 | awk '{print $3}')
    print_status "Make $MAKE_VERSION" "ok"
else
    print_status "Make" "missing"
    NEED_SUDO=true
fi

# Check ARM cross-compiler
if [ "$INSTALL_ARM" = true ]; then
    if command_exists arm-linux-gnueabihf-gcc; then
        print_status "ARM cross-compiler" "ok"
    else
        print_status "ARM cross-compiler" "missing"
        NEED_SUDO=true
    fi
fi

# Check QEMU
if [ "$INSTALL_ARM" = true ]; then
    if command_exists qemu-arm; then
        print_status "QEMU user-mode" "ok"
    else
        print_status "QEMU user-mode" "missing"
        NEED_SUDO=true
    fi
fi

# Check Docker
if [ "$INSTALL_DOCKER" = true ]; then
    if command_exists docker; then
        print_status "Docker" "ok"
    else
        print_status "Docker" "missing"
        NEED_SUDO=true
    fi
fi

# Check clang-format
if command_exists clang-format; then
    print_status "clang-format" "ok"
else
    print_status "clang-format" "missing"
    NEED_SUDO=true
fi

echo ""

# Install missing dependencies
if [ "$NEED_SUDO" = true ]; then
    echo "Installing missing dependencies..."
    echo ""
    
    # Update package list
    sudo apt-get update -qq
    
    # Install build essentials
    if ! command_exists gcc || ! command_exists make; then
        print_status "Installing build-essential..." "install"
        sudo apt-get install -y build-essential
    fi
    
    # Install ARM toolchain
    if [ "$INSTALL_ARM" = true ]; then
        if ! command_exists arm-linux-gnueabihf-gcc; then
            print_status "Installing ARM cross-compiler..." "install"
            sudo apt-get install -y gcc-arm-linux-gnueabihf
        fi
        
        if ! command_exists qemu-arm; then
            print_status "Installing QEMU..." "install"
            sudo apt-get install -y qemu-user
        fi
    fi
    
    # Install Docker
    if [ "$INSTALL_DOCKER" = true ]; then
        if ! command_exists docker; then
            print_status "Installing Docker..." "install"
            curl -fsSL https://get.docker.com -o /tmp/get-docker.sh
            sudo sh /tmp/get-docker.sh
            sudo usermod -aG docker "$USER"
            rm /tmp/get-docker.sh
            echo -e "${YELLOW}Note: You may need to log out and back in for Docker group membership${NC}"
        fi
    fi
    
    # Install clang-format
    if ! command_exists clang-format; then
        print_status "Installing clang-format..." "install"
        sudo apt-get install -y clang-format
    fi
    
    echo ""
fi

# Build the project
echo "Building project..."
echo ""

# Clean previous build
make clean 2>/dev/null || true

# Build native binary
make
print_status "Native build: bin/anagram_chain" "ok"

# Build ARM binary if toolchain available
if command_exists arm-linux-gnueabihf-gcc; then
    make arm
    print_status "ARM build: bin/anagram_chain_arm" "ok"
else
    print_status "ARM build" "skip"
fi

# Run tests
echo ""
echo "Running tests..."
make test

echo ""
echo -e "${BLUE}======================================${NC}"
echo -e "${GREEN}  Setup Complete!${NC}"
echo -e "${BLUE}======================================${NC}"
echo ""
echo "Quick test:"
echo "  ./bin/anagram_chain tests/data/example.txt abck"
echo ""
echo "Expected output:"
echo "  Found 1 chain(s) of length 4:"
echo "  abck->abcek->abcelk->baclekt"
echo ""
echo "For more information, see README.md"
echo ""
