#!/bin/bash
dnf autoremove
dnf --setopt=install_weak_deps=0 upgrade
dnf autoremove
#dnf --setopt=install_weak_deps=False install open-vm-tools-desktop
#dnf --setopt=install_weak_deps=False install java-latest-openjdk-headless python3 python3-pip mysql-server
dnf --setopt=install_weak_deps=0 install ncurses util-linux glibc-langpack-zh vim tree gedit gcc gcc-c++ openssh make gdb git