#!/usr/bin/python

# from subprocess import Popen
# from optparse import OptionParser
import subprocess,os,sys

INTERPRETER = 'bin/sh'

ABSPATH = os.path.abspath('')
DEVPATH = os.path.join(ABSPATH,'dev/')
SYSPATH = os.path.join(ABSPATH,'sys/')
RUNPATH = os.path.join(ABSPATH,'run/')
PROCPATH = os.path.join(ABSPATH,'proc/')

subprocess.call(['mount','--rbind','/dev',DEVPATH])
subprocess.call(['mount','--rbind','/sys',SYSPATH])
subprocess.call(['mount','--rbind','/run',RUNPATH])
subprocess.call(['mount','-t','proc','proc',PROCPATH])

subprocess.call(['chroot',ABSPATH,INTERPRETER])
