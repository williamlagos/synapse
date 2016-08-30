#!/usr/bin/python
import subprocess,os,time
import pyudev as udev
import pygame

pygame.init()
# Verify if the joystick is connected
if pygame.joystick.get_count() > 0:
	j = pygame.joystick.Joystick(0)
	j.init()

	# Get device ID to rebind first USB controller
	dev = None
	context = udev.Context()
	for d in context.list_devices(subsystem='input',DEVNAME='/dev/input/js0'): dev = d
	devsplit = dev['DEVPATH'].split('/')
	for d in devsplit:
		if '-' in d and '.' in d:
			id = d
			break

	os.system('echo %s > /sys/bus/usb/drivers/usb/unbind' % id)
	os.system('echo %s > /sys/bus/usb/drivers/usb/bind' % id)
else:
	exit(0)
