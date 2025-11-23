#!/bin/bash

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}MEX-HAL Dependency and Configuration Check${NC}"
echo "============================================"
echo ""

# Track overall status
ALL_OK=true

# Function to check command existence
check_command()
{
    local cmd=$1
    local name=$2
    if command -v "$cmd" &> /dev/null; then
        echo -e "${GREEN}✓${NC} $name is installed"
        return 0
    else
        echo -e "${RED}✗${NC} $name is NOT installed"
        ALL_OK=false
        return 1
    fi
}

# Function to check library
check_library()
{
    local lib=$1
    local name=$2
    if ldconfig -p | grep -q "$lib"; then
        echo -e "${GREEN}✓${NC} $name is available"
        return 0
    else
        echo -e "${RED}✗${NC} $name is NOT available"
        ALL_OK=false
        return 1
    fi
}

# Check build tools
echo -e "${YELLOW}Build Tools:${NC}"
check_command "g++" "C++ compiler (g++)"
check_command "cmake" "CMake build system"
check_command "make" "GNU Make"
check_command "pkg-config" "pkg-config"
echo ""

# Check required libraries
echo -e "${YELLOW}Required Libraries:${NC}"
check_library "libpthread" "POSIX Threads"
check_library "librt" "Real-time library"
check_library "libxml2" "LibXML2"
echo ""

# Check kernel configuration
echo -e "${YELLOW}Kernel Configuration:${NC}"
KERNEL_VERSION=$(uname -r)
echo "Kernel version: $KERNEL_VERSION"

if uname -v | grep -q "PREEMPT RT"; then
    echo -e "${GREEN}✓${NC} PREEMPT RT patch detected"
elif uname -v | grep -q "PREEMPT"; then
    echo -e "${YELLOW}⚠${NC} Preemptible kernel (CONFIG_PREEMPT) - RT patch recommended for best performance"
else
    echo -e "${RED}✗${NC} No preemption detected - PREEMPT RT patch strongly recommended"
    ALL_OK=false
fi
echo ""

# Check CPU governor
echo -e "${YELLOW}CPU Configuration:${NC}"
GOVERNOR=$(cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor 2>/dev/null || echo "unknown")
if [ "$GOVERNOR" = "performance" ]; then
    echo -e "${GREEN}✓${NC} CPU governor set to performance"
else
    echo -e "${YELLOW}⚠${NC} CPU governor is '$GOVERNOR' (recommended: performance)"
    echo "  Run: sudo ./scripts/setup_rt.sh to configure"
fi

# Count available CPUs
CPU_COUNT=$(nproc)
echo "Available CPUs: $CPU_COUNT"
echo ""

# Check hardware device access
echo -e "${YELLOW}Hardware Device Access:${NC}"

# Check GPIO
if [ -d "/sys/class/gpio" ]; then
    echo -e "${GREEN}✓${NC} GPIO sysfs interface available"
    if [ -w "/sys/class/gpio/export" ] 2>/dev/null; then
        echo -e "${GREEN}✓${NC} GPIO export writable (proper permissions)"
    else
        echo -e "${YELLOW}⚠${NC} GPIO export not writable (may need udev rules or group membership)"
    fi
else
    echo -e "${YELLOW}⚠${NC} GPIO sysfs interface not found"
fi

# Check SPI
SPI_DEVICES=$(ls /dev/spidev* 2>/dev/null | wc -l)
if [ "$SPI_DEVICES" -gt 0 ]; then
    echo -e "${GREEN}✓${NC} SPI devices found: $SPI_DEVICES device(s)"
else
    echo -e "${YELLOW}⚠${NC} No SPI devices found in /dev"
fi

# Check I2C
I2C_DEVICES=$(ls /dev/i2c-* 2>/dev/null | wc -l)
if [ "$I2C_DEVICES" -gt 0 ]; then
    echo -e "${GREEN}✓${NC} I2C devices found: $I2C_DEVICES device(s)"
else
    echo -e "${YELLOW}⚠${NC} No I2C devices found in /dev"
fi

# Check PWM
if [ -d "/sys/class/pwm" ]; then
    PWM_CHIPS=$(ls -d /sys/class/pwm/pwmchip* 2>/dev/null | wc -l)
    echo -e "${GREEN}✓${NC} PWM interface available: $PWM_CHIPS chip(s)"
else
    echo -e "${YELLOW}⚠${NC} PWM sysfs interface not found"
fi

# Check ADC/IIO
if [ -d "/sys/bus/iio/devices" ]; then
    IIO_DEVICES=$(ls -d /sys/bus/iio/devices/iio:device* 2>/dev/null | wc -l)
    echo -e "${GREEN}✓${NC} IIO (ADC) interface available: $IIO_DEVICES device(s)"
else
    echo -e "${YELLOW}⚠${NC} IIO (ADC) interface not found"
fi
echo ""

# Check user groups
echo -e "${YELLOW}User Groups (for current user: $USER):${NC}"
GROUPS_LIST=$(groups)

check_group() {
    local group=$1
    if echo "$GROUPS_LIST" | grep -q "\b$group\b"; then
        echo -e "${GREEN}✓${NC} Member of '$group' group"
        return 0
    else
        echo -e "${YELLOW}⚠${NC} Not a member of '$group' group"
        echo "  Run: sudo usermod -a -G $group $USER"
        return 1
    fi
}

check_group "gpio" || true
check_group "spi" || true
check_group "i2c" || true
check_group "iio" || true
echo ""

# Check real-time limits
echo -e "${YELLOW}Real-Time Configuration:${NC}"
RT_PRIO=$(ulimit -r 2>/dev/null || echo "0")
if [ "$RT_PRIO" -ge 99 ]; then
    echo -e "${GREEN}✓${NC} Real-time priority limit: $RT_PRIO"
else
    echo -e "${YELLOW}⚠${NC} Real-time priority limit: $RT_PRIO (recommended: 99)"
    echo "  Run: sudo ./scripts/setup_rt.sh to configure"
fi

MEMLOCK=$(ulimit -l 2>/dev/null || echo "0")
if [ "$MEMLOCK" = "unlimited" ] || [ "$MEMLOCK" -gt 1000000 ]; then
    echo -e "${GREEN}✓${NC} Memory lock limit: $MEMLOCK"
else
    echo -e "${YELLOW}⚠${NC} Memory lock limit: $MEMLOCK KB (recommended: unlimited)"
    echo "  Run: sudo ./scripts/setup_rt.sh to configure"
fi
echo ""

# Check if project is built
echo -e "${YELLOW}Build Status:${NC}"
if [ -f "build/libhal.a" ]; then
    echo -e "${GREEN}✓${NC} MEX-HAL library built"
    BUILD_SIZE=$(du -h build/libhal.a | cut -f1)
    echo "  Library size: $BUILD_SIZE"
else
    echo -e "${RED}✗${NC} MEX-HAL library not built"
    echo "  Run: mkdir -p build && cd build && cmake .. && make"
    ALL_OK=false
fi
echo ""

# Summary
echo "============================================"
if [ "$ALL_OK" = true ]; then
    echo -e "${GREEN}All critical dependencies are satisfied!${NC}"
    exit 0
else
    echo -e "${YELLOW}Some dependencies or configurations are missing.${NC}"
    echo "Please review the items marked with ✗ above."
    echo ""
    echo "To install missing dependencies on Ubuntu/Debian:"
    echo "  sudo apt-get update"
    echo "  sudo apt-get install build-essential cmake pkg-config libxml2-dev"
    echo ""
    echo "To configure real-time settings:"
    echo "  sudo ./scripts/setup_rt.sh"
    echo ""
    exit 1
fi
