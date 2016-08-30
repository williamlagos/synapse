#!/usr/bin/python
# Gamepad code integration to Kodi event server
#
# For testing compatibility for PyGame SDL2
# import pygame_sdl2 as pygame

import pygame
import subprocess,time,os
import threading

pygame.init()
j = pygame.joystick.Joystick(0)
j.init()
# print 'Initialized Joystick : %s' % j.get_name()

def get():
    out = [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]
    it = 0 #iterator
    pygame.event.pump()
    
    #Read input from the two joysticks       
    for i in range(0, j.get_numaxes()):
        out[it] = j.get_axis(i); it+=1

    #Read input from buttons
    for i in range(0, j.get_numbuttons()):
        out[it] = j.get_button(i); it+=1

    return out

def capture():
#    product = j.get_name()
#    output = subprocess.check_output(['dmesg'])
#    lines = output.split('\n')
#    line = ''
#    for l in lines:
#        if product in l and 'Product' in l:
#            line = l
#            break
#    id = line.split(':')[0].split()[-1:][0]
#    count = 0
    while True:
        btns = get()
#	print btns
	if btns[0] < 0: subprocess.call(["kodi-send","--action=Left"])
	elif btns[0] > 0: subprocess.call(["kodi-send","--action=Right"])
	elif btns[1] < 0: subprocess.call(["kodi-send","--action=Up"])
	elif btns[1] > 0: subprocess.call(["kodi-send","--action=Down"])
	elif btns[5] > 0: subprocess.call(["kodi-send","--action=Back"])
	elif btns[6] > 0: subprocess.call(["kodi-send","--action=Select"])
#	else:
#            if count >= 5:
#                os.system('echo %s > /sys/bus/usb/drivers/usb/unbind' % id)
#                os.system('echo %s > /sys/bus/usb/drivers/usb/bind' % id)
#                count = 0
#	count += 1
#	time.sleep(1)

if __name__ == "__main__": capture()

# t = threading.Thread(target=capture)
# t.daemon = True
# t.start()
