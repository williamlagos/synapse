#!/usr/bin/python
import os
import sys
import subprocess

CMD = "chroot"
PATH = "/opt/retro"

#if len(sys.argv) > 1: subprocess.call([CMD,PATH,"bash","-c",sys.argv[1]])
if len(sys.argv) > 1:
	os.system("%s %s bash -c \"%s\"" % (CMD,PATH,sys.argv[1]))
