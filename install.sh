#!/bin/bash
#
# Embedded Anagram Chain Demo - Installation Script
#
# This script installs all dependencies and initializes the project.
#
# Usage:
#   ./install.sh           - Install all dependencies
#   ./install.sh --help    - Show help
#

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

print_step() {
    echo -e "${GREEN}[*]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[!]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

show_help() {
    echo "Embedded Anagram Chain Demo - Installation Script"
    echo ""
    echo "Usage:"
    echo "  ./install.sh           - Install all dependencies"
    echo "  ./install.sh --help    - Show this help"
    echo ""
    echo "This script will:"
    echo "  1. Initialize git submodules (FreeRTOS-Kernel)"
    echo "  2. Install required packages (requires sudo)"
    echo "  3. Verify the installation"
    echo ""
    echo "Required packages:"
    echo "  - build-essential       (GCC, make)"
    echo "  - gcc-arm-none-eabi     (ARM cross-compiler)"
    echo "  - libnewlib-arm-none-eabi (ARM C library)"
    echo "  - binutils-arm-none-eabi (ARM binutils: objdump, size, etc)"
    echo "  - qemu-system-arm       (ARM system emulator)"
    echo "  - gdb-multiarch         (Debugger for ARM)"
    echo "  - clang-format          (Code formatter)"
    echo "  - clang-tidy            (Static analyzer)"
    echo "  - cppcheck              (Static analyzer)"
    echo "  - git                   (Version control)"
    echo "  - docker.io             (Optional, for Docker builds)"
}

init_submodules() {
    print_step "Initializing git submodules..."
    
    if [ ! -d ".git" ]; then
        print_error "Not a git repository. Please clone the repository first."
        exit 1
    fi
    
    git submodule update --init --recursive
    
    if [ -d "arm/freertos/FreeRTOS-Kernel" ]; then
        echo "  FreeRTOS-Kernel: OK"
    else
        print_error "Failed to initialize FreeRTOS-Kernel submodule"
        exit 1
    fi
}

install_packages() {
    print_step "Installing required packages..."
    
    # Check if running on Debian/Ubuntu
    if ! command -v apt-get &> /dev/null; then
        print_warning "apt-get not found. Please install packages manually:"
        echo "  - build-essential"
        echo "  - gcc-arm-none-eabi"
        echo "  - libnewlib-arm-none-eabi"
        echo "  - binutils-arm-none-eabi"
        echo "  - qemu-system-arm"
        echo "  - gdb-multiarch"
        echo "  - clang-format"
        echo "  - git"
        return
    fi
    
    sudo apt-get update
    
    # Core build tools
    print_step "Installing build tools..."
    sudo apt-get install -y \
        build-essential \
        git \
        make
    
    # ARM cross-compilation toolchain
    print_step "Installing ARM toolchain..."
    sudo apt-get install -y \
        gcc-arm-none-eabi \
        libnewlib-arm-none-eabi \
        binutils-arm-none-eabi
    
    # Emulation and debugging
    print_step "Installing QEMU and GDB..."
    sudo apt-get install -y \
        qemu-system-arm \
        gdb-multiarch
    
    # Code quality tools
    print_step "Installing code quality tools..."
    sudo apt-get install -y \
        clang-format \
        clang-tidy \
        cppcheck || print_warning "Some code quality tools not installed (optional)"
    
    # Docker is optional
    print_step "Installing Docker (optional)..."
    sudo apt-get install -y docker.io || print_warning "Docker installation failed (optional)"
    
    # Add user to docker group (requires logout/login)
    if command -v docker &> /dev/null; then
        sudo usermod -aG docker $USER 2>/dev/null || true
        print_warning "Added user to docker group. Log out and back in to use Docker without sudo."
    fi
}

verify_installation() {
    print_step "Verifying installation..."
    
    local all_ok=true
    local debug_ok=true
    
    echo ""
    echo "Build Tools:"
    
    # Check GCC
    if command -v gcc &> /dev/null; then
        echo "  [OK] GCC: $(gcc --version | head -1)"
    else
        echo "  [FAIL] GCC not found"
        all_ok=false
    fi
    
    # Check Make
    if command -v make &> /dev/null; then
        echo "  [OK] Make: $(make --version | head -1)"
    else
        echo "  [FAIL] Make not found"
        all_ok=false
    fi
    
    # Check Git
    if command -v git &> /dev/null; then
        echo "  [OK] Git: $(git --version)"
    else
        echo "  [FAIL] Git not found"
        all_ok=false
    fi
    
    echo ""
    echo "ARM Toolchain:"
    
    # Check ARM GCC
    if command -v arm-none-eabi-gcc &> /dev/null; then
        echo "  [OK] ARM GCC: $(arm-none-eabi-gcc --version | head -1)"
    else
        echo "  [FAIL] arm-none-eabi-gcc not found"
        all_ok=false
    fi
    
    # Check ARM objdump
    if command -v arm-none-eabi-objdump &> /dev/null; then
        echo "  [OK] ARM objdump: available"
    else
        echo "  [WARN] arm-none-eabi-objdump not found (optional)"
    fi
    
    # Check ARM size
    if command -v arm-none-eabi-size &> /dev/null; then
        echo "  [OK] ARM size: available"
    else
        echo "  [WARN] arm-none-eabi-size not found (optional)"
    fi
    
    echo ""
    echo "Emulation & Debugging:"
    
    # Check QEMU
    if command -v qemu-system-arm &> /dev/null; then
        echo "  [OK] QEMU: $(qemu-system-arm --version | head -1)"
    else
        echo "  [FAIL] qemu-system-arm not found"
        all_ok=false
    fi
    
    # Check GDB multiarch
    if command -v gdb-multiarch &> /dev/null; then
        echo "  [OK] GDB multiarch: $(gdb-multiarch --version | head -1)"
    else
        echo "  [WARN] gdb-multiarch not found (required for ARM debugging)"
        debug_ok=false
    fi
    
    echo ""
    echo "Code Quality:"
    
    # Check clang-format
    if command -v clang-format &> /dev/null; then
        echo "  [OK] clang-format: $(clang-format --version)"
    else
        echo "  [WARN] clang-format not found (optional)"
    fi
    
    # Check clang-tidy
    if command -v clang-tidy &> /dev/null; then
        echo "  [OK] clang-tidy: $(clang-tidy --version | head -1)"
    else
        echo "  [WARN] clang-tidy not found (optional)"
    fi
    
    # Check cppcheck
    if command -v cppcheck &> /dev/null; then
        echo "  [OK] cppcheck: $(cppcheck --version)"
    else
        echo "  [WARN] cppcheck not found (optional)"
    fi
    
    echo ""
    echo "Docker (optional):"
    
    # Check Docker
    if command -v docker &> /dev/null; then
        echo "  [OK] Docker: $(docker --version)"
    else
        echo "  [WARN] Docker not found (optional, for containerized builds)"
    fi
    
    echo ""
    echo "Submodules:"
    
    # Check FreeRTOS submodule
    if [ -f "arm/freertos/FreeRTOS-Kernel/tasks.c" ]; then
        echo "  [OK] FreeRTOS-Kernel: initialized"
    else
        echo "  [FAIL] FreeRTOS-Kernel submodule not initialized"
        all_ok=false
    fi
    
    echo ""
    echo "=========================================="
    
    if [ "$all_ok" = true ]; then
        print_step "Installation complete!"
        echo ""
        echo "You can now build the project:"
        echo "  make              - Build PC version"
        echo "  make arm-baremetal - Build ARM bare-metal version"
        echo "  make arm-freertos  - Build ARM FreeRTOS version"
        echo "  make test         - Run PC tests"
        echo "  make test-all     - Run tests on all platforms"
        echo "  make help         - Show all targets"
        echo ""
        if [ "$debug_ok" = false ]; then
            print_warning "ARM debugging requires gdb-multiarch. Install with:"
            echo "  sudo apt-get install gdb-multiarch"
        fi
        echo ""
        echo "To start debugging in VSCode:"
        echo "  1. Open this folder in VSCode"
        echo "  2. Press F5 and select a debug configuration"
    else
        echo ""
        print_error "Some required components are missing."
        echo "Please install them and run this script again."
        exit 1
    fi
}

# Main
case "${1:-}" in
    --help|-h)
        show_help
        exit 0
        ;;
    *)
        echo "=========================================="
        echo "  Embedded Anagram Chain Demo - Installation"
        echo "=========================================="
        echo ""
        
        init_submodules
        install_packages
        verify_installation
        ;;
esac
