#!/bin/bash

set -e

# 设置控制台字体大小
dpkg-reconfigure console-setup

# 禁用ipv6
sysctl net.ipv6.conf.all.disable_ipv6=1
echo 'net.ipv6.conf.all.disable_ipv6 = 1' > /etc/sysctl.d/01-me.conf
echo 'kernel.sched_autogroup_enabled = 0' >> /etc/sysctl.d/01-me.conf
echo 'net.ipv4.tcp_congestion_control = bbr' >> /etc/sysctl.d/01-me.conf
echo 'net.core.default_qdisc = cake' >> /etc/sysctl.d/01-me.conf
echo 'net.ipv4.tcp_ecn = 0' >> /etc/sysctl.d/01-me.conf
echo 'net.ipv4.tcp_fastopen = 3' >> /etc/sysctl.d/01-me.conf

# 设置源
cat > /etc/apt/sources.list.d/debian.sources << EOF
Types: deb
URIs: http://mirrors.tuna.tsinghua.edu.cn/debian/
Suites: sid
Components: main contrib non-free non-free-firmware
Signed-By: /usr/share/keyrings/debian-archive-keyring.gpg

Types: deb deb-src
URIs: http://mirrors.tuna.tsinghua.edu.cn/debian/
Suites: experimental
Components: main contrib non-free non-free-firmware
Signed-By: /usr/share/keyrings/debian-archive-keyring.gpg
EOF

# 更新
apt update
apt --no-install-recommends --auto-remove --purge full-upgrade

# 安装最新内核
apt -t experimental update
apt -t experimental install linux-image-amd64

# 安装kde
apt --no-install-recommends install plasma-desktop systemsettings kscreen plasma-pa wireplumber plasma-nm fonts-noto-cjk-extra fonts-noto-color-emoji dolphin kmenuedit fcitx5-pinyin kde-config-fcitx5 fcitx5-modules
# 设置面板网络代理选项
apt --no-install-recommends install kio-extras
# 终端
apt --no-install-recommends install tilix libharfbuzz-gobject0 gsettings-desktop-schemas

# 设置NewworkManager网络接管
apt autopurge ifupdown*
nano /etc/network/interfaces

# 低噪优化
systemctl disable udisks2 accounts-daemon systemd-hostnamed
systemctl mask udisks2 systemd-hostnamed polkit
systemctl --user mask plasma-polkit-agent
systemctl disable systemd-networkd.service systemd-networkd.socket
# 但是不能卸载
systemctl disable wpa_supplicant

# 禁用 walletd/密码库
mkdir -p ~/.local/share/dbus-1/services/
if [ ! -e ~/.local/share/dbus-1/services/org.kde.kwalletd6.service ]; then
cat > ~/.local/share/dbus-1/services/org.kde.kwalletd6.service << EOF
[D-BUS Service]
Name=org.kde.kwalletd6
Exec=/bin/false
EOF
fi
if [ ! -e ~/.local/share/dbus-1/services/org.kde.secretservicecompat.service ]; then
cat > ~/.local/share/dbus-1/services/org.kde.secretservicecompat.service << EOF
[D-BUS Service]
Name=org.kde.secretservicecompat
Exec=/bin/false
EOF
fi
if [ ! -e ~/.local/share/dbus-1/services/org.freedesktop.impl.portal.desktop.kwallet.service ]; then
cat > ~/.local/share/dbus-1/services/org.freedesktop.impl.portal.desktop.kwallet.service << EOF
[D-BUS Service]
Name=org.freedesktop.impl.portal.desktop.kwallet
Exec=/bin/false
EOF
fi

# 修复键盘F1 - F12不可用
if [ ! -e /etc/modprobe.d/hid_apple.conf ]; then
        echo 'options hid_apple fnmode=2' > /etc/modprobe.d/hid_apple.conf
fi
# 修复root无法登陆
if [ ! -e /etc/pam.d/kde ]; then
        cp /etc/pam.d/common-auth /etc/pam.d/kde
fi
# 允许root声音
sed -i '/^ConditionUser=!root/d' $(find /usr/lib/systemd/user -type f)

# 配置cmdline
sed -i 's/^GRUB_CMDLINE_LINUX_DEFAULT=".*"/GRUB_CMDLINE_LINUX_DEFAULT="nouveau.modeset=1 amdgpu.modeset=1 amdgpu.dcfeaturemask=0x400 lsm= selinux=0 apparmor=0 nokaslr audit=0 ima=off ima_appraise=off evm=fix no_file_caps nowatchdog nosoftlockup no_debug_objects vm_debug=- debug_pagealloc=off page_poison=off schedstats=disable traceoff_after_boot split_lock_detect=off kmemleak=off debugfs=off kfence.sample_interval=0 mitigations=off kpti=0 kvm-intel.vmentry_l1d_flush=never hardened_usercopy=off randomize_kstack_offset=0 tsa=off cfi=off kunit.enable=0 preempt=full transparent_hugepage=always mce=off nopku vsyscall=none psi=off no5lvl random.trust_cpu=on random.trust_bootloader=on mem_encrypt=off x2apic_phys skew_tick=1 cgroup_disable=cpu,cpuset,cpuacct,io,memory,devices,freezer,net_cls,perf_event,hugetlb,pids,rdma,misc,dmem,debug,pressure hibernate=no"/g' /etc/default/grub
# 不要检测其他系统
if ! grep -q '^GRUB_DISABLE_OS_PROBER=true' /etc/default/grub; then
        #sed -i '/GRUB_DISABLE_OS_PROBER/d' /etc/default/grub
        echo GRUB_DISABLE_OS_PROBER=true >> /etc/default/grub
fi
update-grub

reboot

# steam
apt --no-install-recommends install libc6:amd64 libc6:i386 libegl1:amd64 libegl1:i386 libgbm1:amd64 libgbm1:i386 libgl1-mesa-dri:amd64 libgl1-mesa-dri:i386 libgl1:amd64 libgl1:i386 steam-libs-amd64:amd64 steam-libs-i386:i386 xdg-desktop-portal xdg-desktop-portal-kde pulseaudio-utils

sleep 3
LANG="zh_CN.UTF-8" LANGUAGE="zh_CN.UTF-8" exec startplasma-wayland
