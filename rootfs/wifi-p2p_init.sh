#!/usr/bin/bash

iw phy phy0 interface add p2p0 type __p2pcl

sleep 1

ip link set p2p0 up
ip addr add 192.168.3.11/24 dev p2p0

wpa_supplicant -i p2p0 -D nl80211 -c /etc/netctl/wpa_supplicant.conf -B -f /var/log/wpa_supplicant.log

udhcpd /etc/udhcpd.conf
