#!/usr/bin/python
# When running in bash:
# /opt/scripts/switcher.py $1

import time,subprocess,sys,os

FNULL = open(os.devnull,'w')

if len(sys.argv) > 1:
	# Stop Kodi service.
	subprocess.call(["systemctl","stop","kodi"])
	subprocess.call(["systemctl","stop","kodi-gamepad"])
	subprocess.call(["systemctl","start",sys.argv[1]])
	while True:
		time.sleep(1)
		retcode = subprocess.call(["systemctl","status",sys.argv[1]],
					  stdout=FNULL,stderr=subprocess.STDOUT)
		if retcode != 0: break
	subprocess.call(["systemctl","start","kodi"])
