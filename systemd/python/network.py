#!/usr/bin/python
import subprocess,os
import NetworkManager

ssids = []

while True:
	for dev in NetworkManager.NetworkManager.GetDevices():
	    if dev.DeviceType != NetworkManager.NM_DEVICE_TYPE_WIFI: continue
	    for ap in dev.SpecificDevice().GetAccessPoints(): ssids.append(ap.Ssid)

	connections = os.listdir("/etc/NetworkManager/system-connections")

	for con in connections:
		if con in ssids: subprocess.call(["systemctl","start","create_ap"]

# out = subprocess.check_output(["ifconfig","wlan0"])
# while True:
#	if 'inet' not in out: subprocess.call(["systemctl","start","create_ap"]) 
