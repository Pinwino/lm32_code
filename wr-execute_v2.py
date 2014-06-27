#! /usr/bin/python

import serial
import sys
import time
import re
from select import select

#def read_keyboard():
#	cmd = raw_input()

def read_keyboard(blocking=True):
	if not blocking:
		rlist, _, _ = select([sys.stdin], [], [], 0)
		if rlist:
			cmd = sys.stdin.readline()
		else:
			cmd=""
	else:
		cmd = raw_input()
		
	return cmd
    
def read_response(port,blocking=True):
	
	if blocking:
		response = port.read(1)
				
		while response != '#':
			response = port.read(1)
			sys.stdout.write(response)
		print '',
	else:
		nbytes = port.inWaiting()
		
		if nbytes != 256:
			response = port.read(nbytes)
			sys.stdout.write(response)
			sys.stdout.write(str(nbytes))
			

def execute_file(f,port,tintchar=0.01,eol=0xd):
	for cmd in f:
		cmd = cmd[:-1]
		print "Executing "+cmd+"..."
		for c in cmd:
			port.write(c)
			time.sleep(tintchar)
			
		port.write(chr(eol))
		
		read_response(port)
			
		print "Press any key to continue..."
		wait = raw_input()

def execute_program(dev,interactive=False,tintchar=0.01,eol=0xd,fcmd="cmd.txt",cmd_exit="exit",cmd_load="load",pbaudrate=115200,pparity=serial.PARITY_NONE,pstopbits=serial.STOPBITS_ONE,pbytesize=serial.EIGHTBITS,pxonxoff=False,prtscts=False,pdsrdtr=False):
	if interactive == False:
		f = open(fcmd)
		port = serial.Serial(dev, baudrate=pbaudrate,parity=pparity,stopbits=pstopbits,bytesize=pbytesize,xonxoff=pxonxoff,rtscts=prtscts,dsrdtr=pdsrdtr,timeout=0)
		
		execute_file(f,port,tintchar=tintchar,eol=eol)
		
		port.close()
		f.close()
	else:	
		port = serial.Serial(dev, baudrate=pbaudrate,parity=pparity,stopbits=pstopbits,bytesize=pbytesize,xonxoff=pxonxoff,rtscts=prtscts,dsrdtr=pdsrdtr,timeout=0)
		end = False
		
		cmd=""
		
		while not end:
			
			if cmd == cmd_exit:
				break
			elif cmd == cmd_load:
				fload = raw_input("File: ")
				f2 = open(fload)
				execute_file(f2,port,tintchar=tintchar,eol=eol)
				f2.close()
				cmd = ""
			else:
				for c in cmd:
					port.write(c)
					time.sleep(tintchar)
					
				port.write(chr(eol))
				
				read_response(port)
				
				cmd = read_keyboard()
											
		port.close()
			
print
print "============================================================================"
print "\t\t White-Rabbit Executer v1.0  \t\t"
print "============================================================================"


if len(sys.argv) != 2 and len(sys.argv) != 3:
	print "ERROR: use ",sys.argv[0]," <USB device>"
	print "ERROR: use ",sys.argv[0]," <USB device> <program_file>"
	print "Made by: Miguel Jimenez Lopez <klyone@ugr.es>, UGR"
	print
	sys.exit(-1)
	
dev = sys.argv[1]

if len(sys.argv) == 2:
	interactive_b = True
	fc = None
else:
	interactive_b = False
	fc = sys.argv[2]
	
execute_program(dev,interactive=interactive_b,fcmd=fc)
