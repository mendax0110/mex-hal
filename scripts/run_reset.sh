#!/bin/bash
# Reset system after MEX-HAL real-time setup
set -e

echo "=== Resetting system configuration to defaults ==="

sudo rm -f /etc/security/limits.d/99-realtime.conf
sudo rm -f /etc/sysctl.d/99-realtime.conf
sudo rm -f /etc/udev/rules.d/99-mex-hal.rules

sudo sysctl --system || true

for cpu in /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor; do
    [ -f "$cpu" ] && echo "ondemand" > "$cpu" 2>/dev/null || echo "powersave" > "$cpu" 2>/dev/null || true
done

for irq in /proc/irq/*/smp_affinity; do
    echo "ff" > "$irq" 2>/dev/null || true
done

for s in bluetooth cups avahi-daemon; do
    sudo systemctl enable --now "$s" >/dev/null 2>&1 || true
done

sudo udevadm control --reload-rules
sudo udevadm trigger

echo "=== Reset complete. Please reboot to finalize changes. ==="
