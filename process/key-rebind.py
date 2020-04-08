#!/usr/bin/python2
# 
# When running on bash:
# /opt/scripts/key-rebind.py
import subprocess,os,time
import pyudev as udev

# Get device ID to rebind first USB controller
dev = None
context = udev.Context()
for d in context.list_devices(subsystem='input',DEVNAME='/dev/input/event0'): dev = d
devsplit = dev['DEVPATH'].split('/')
for d in devsplit:
	if '-' in d and '.' in d:
		id = d
		break

os.system('echo %s > /sys/bus/usb/drivers/usb/unbind' % id)
os.system('echo %s > /sys/bus/usb/drivers/usb/bind' % id)
