#!/usr/bin/python3

import socket
import datetime
import sys
import threading

addr,port = "",3141
if len(sys.argv) > 1:
    if len(sys.argv) > 2:
        port = int(argv[2])
        addr = argv[1]
    else:
        port = int(argv[1])

nextBgColor = 0
def nextColorBar():
    global nextBgColor
    COLOR_HEAD = '\033['
    COLOR_BG_START = 40
    COLOR_RESET = COLOR_HEAD+'39;49m'
    BG_COLORS = [ COLOR_HEAD+str(COLOR_BG_START+i)+'m' for i in range(1,8) ]

    print(BG_COLORS[nextBgColor]+\
            "                                                             "+\
            COLOR_RESET)
    nextBgColor = (nextBgColor+1)%len(BG_COLORS)

class LogDump(threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)
    def run(self):
        global addr,port
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.bind((addr,port))
        print("Listening on {}:{}...".format(addr,port))
        while True:
            data,addr = sock.recvfrom(4096)
            if(data == b'=PIKERN Hey, listen!='):
                sock.sendto(b"RPi", addr)
                time=datetime.datetime.now().strftime('%H:%M:%S')
                nextColorBar()
                print("[{}] Invited to {}".format(time,addr[0]))
                continue

            print(data.decode('utf-8'), end='')

logDump = LogDump()
logDump.start()

