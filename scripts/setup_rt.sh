#!/bin/bash

# MEX-HAL Real-Time Setup Script
set -e

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}MEX-HAL Real-Time Configuration Script${NC}"
echo "========================================"
echo ""

# Check if running as root
if [ "$EUID" -ne 0 ]; then 
    echo -e "${RED}Error: This script must be run as root (sudo)${NC}"
    exit 1
fi

# Check kernel version and RT patch
echo -e "${YELLOW}Checking kernel configuration...${NC}"
KERNEL_VERSION=$(uname -r)
echo "Current kernel: $KERNEL_VERSION"

if uname -v | grep -q "PREEMPT RT"; then
    echo -e "${GREEN}✓ PREEMPT RT patch detected${NC}"
else
    echo -e "${YELLOW}⚠ Warning: PREEMPT RT patch not detected${NC}"
    echo "Consider installing a PREEMPT RT kernel for optimal real-time performance"
    echo "Visit: https://wiki.linuxfoundation.org/realtime/start"
fi

# Configure CPU governor for performance
echo ""
echo -e "${YELLOW}Configuring CPU governor for performance...${NC}"
for cpu in /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor; do
    if [ -f "$cpu" ]; then
        echo "performance" > "$cpu" 2>/dev/null || echo -e "${YELLOW}⚠ Could not set performance governor for $cpu${NC}"
    fi
done
echo -e "${GREEN}✓ CPU governor configured${NC}"

# Disable CPU frequency scaling
echo ""
echo -e "${YELLOW}Disabling CPU frequency scaling...${NC}"
for cpu in /sys/devices/system/cpu/cpu*/cpufreq/scaling_max_freq; do
    if [ -f "$cpu" ]; then
        MAX_FREQ=$(cat "$cpu")
        MIN_FREQ_FILE="${cpu/scaling_max_freq/scaling_min_freq}"
        echo "$MAX_FREQ" > "$MIN_FREQ_FILE" 2>/dev/null || echo -e "${YELLOW}⚠ Could not disable frequency scaling for $cpu${NC}"
    fi
done
echo -e "${GREEN}✓ CPU frequency scaling disabled${NC}"

# Configure IRQ affinity
echo ""
echo -e "${YELLOW}Configuring IRQ affinity...${NC}"
# Move all IRQs to CPU 0, leaving other CPUs for real-time tasks
for irq in /proc/irq/*/smp_affinity; do
    echo "1" > "$irq" 2>/dev/null || true
done
echo -e "${GREEN}✓ IRQ affinity configured${NC}"

# Configure system limits
echo ""
echo -e "${YELLOW}Configuring system limits for real-time...${NC}"

LIMITS_FILE="/etc/security/limits.d/99-realtime.conf"
cat > "$LIMITS_FILE" << 'LIMITS'
# Real-time limits for MEX-HAL
* soft rtprio 99
* hard rtprio 99
* soft memlock unlimited
* hard memlock unlimited
* soft nice -20
* hard nice -20
LIMITS

echo -e "${GREEN}✓ System limits configured in $LIMITS_FILE${NC}"

# Configure kernel parameters
echo ""
echo -e "${YELLOW}Configuring kernel parameters...${NC}"

SYSCTL_FILE="/etc/sysctl.d/99-realtime.conf"
cat > "$SYSCTL_FILE" << 'SYSCTL'
# Real-time kernel parameters for MEX-HAL

# Disable swap
vm.swappiness = 0

# Increase max locked memory
kernel.shmmax = 268435456
kernel.shmall = 268435456

# Real-time throttling
kernel.sched_rt_period_us = 1000000
kernel.sched_rt_runtime_us = 950000

# Reduce timer tick for better latency
kernel.timer_migration = 0
SYSCTL

sysctl -p "$SYSCTL_FILE" >/dev/null 2>&1 || echo -e "${YELLOW}⚠ Some sysctl parameters may require a reboot${NC}"
echo -e "${GREEN}✓ Kernel parameters configured in $SYSCTL_FILE${NC}"

# Disable unnecessary services
echo ""
echo -e "${YELLOW}Checking for unnecessary services...${NC}"

SERVICES_TO_DISABLE=(
    "bluetooth"
    "cups"
    "avahi-daemon"
)

for service in "${SERVICES_TO_DISABLE[@]}"; do
    if systemctl is-active --quiet "$service" 2>/dev/null; then
        echo "Disabling $service..."
        systemctl stop "$service" >/dev/null 2>&1 || true
        systemctl disable "$service" >/dev/null 2>&1 || true
        echo -e "${GREEN}✓ Disabled $service${NC}"
    fi
done

# Create udev rules for GPIO, SPI, I2C access
echo ""
echo -e "${YELLOW}Creating udev rules for hardware access...${NC}"

UDEV_FILE="/etc/udev/rules.d/99-mex-hal.rules"
cat > "$UDEV_FILE" << 'UDEV'
# MEX-HAL hardware access rules

# GPIO access
SUBSYSTEM=="gpio", GROUP="gpio", MODE="0660"
SUBSYSTEM=="gpio*", PROGRAM="/bin/sh -c 'chown -R root:gpio /sys/class/gpio && chmod -R 770 /sys/class/gpio; chown -R root:gpio /sys/devices/virtual/gpio && chmod -R 770 /sys/devices/virtual/gpio; chown -R root:gpio /sys$devpath && chmod -R 770 /sys$devpath'"

# SPI access
SUBSYSTEM=="spidev", GROUP="spi", MODE="0660"

# I2C access
SUBSYSTEM=="i2c-dev", GROUP="i2c", MODE="0660"

# PWM access
SUBSYSTEM=="pwm*", GROUP="gpio", MODE="0660"

# ADC/IIO access
SUBSYSTEM=="iio", GROUP="iio", MODE="0660"
UDEV

# Create groups if they don't exist
groupadd -f gpio
groupadd -f spi
groupadd -f i2c
groupadd -f iio

# Reload udev rules
udevadm control --reload-rules >/dev/null 2>&1 || true
udevadm trigger >/dev/null 2>&1 || true

echo -e "${GREEN}✓ Udev rules configured in $UDEV_FILE${NC}"

# Summary
echo ""
echo -e "${GREEN}========================================"
echo "Real-time configuration complete!"
echo "========================================${NC}"
echo ""
echo "Next steps:"
echo "1. Add your user to the necessary groups:"
echo "   sudo usermod -a -G gpio,spi,i2c,iio \$USER"
echo ""
echo "2. Reboot your system for all changes to take effect:"
echo "   sudo reboot"
echo ""
echo "3. After reboot, verify RT configuration:"
echo "   ./check_dependencies.sh"
echo ""
echo -e "${YELLOW}Note: For optimal real-time performance, consider:${NC}"
echo "  - Installing a PREEMPT RT kernel if not already installed"
echo "  - Isolating CPUs for real-time tasks (isolcpus kernel parameter)"
echo "  - Using CPU affinity for your real-time applications"
echo ""

exit 0
