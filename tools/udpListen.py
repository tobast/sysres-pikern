#!/usr/bin/python3

import socket
import datetime
import sys
import threading
import argparse


parser = argparse.ArgumentParser()
parser.add_argument('-f', '--from-port', help='Selects the destination port.'\
        +' 0 for any, defaults to 1.', default=1)
parser.add_argument('-p', '--port', help='Binds this port. Defaults to 3141.',\
        default=3141)
parser.add_argument('-a', '--bind-addr', help='Binds this address. Defaults '\
        +' to 0.0.0.0.', default="")
parser.add_argument('-s', '--send-port', help='Sends data to this port. '\
        +'Defaults to 3141.', default=3141)

parsed = parser.parse_args()

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

class Sender(threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)
        self.sock = None
        self.addr = None
    def run(self):
        while True:
            data = input()
            if self.sock == None or self.addr == None:
                continue
            self.sock.sendto(data.encode('utf-8'), self.addr)

class LogDump(threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)
    def run(self):
        global parsed
        port = int(parsed.port)
        from_port = int(parsed.from_port)
        send_port = int(parsed.send_port)

        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.bind((parsed.bind_addr,port))
        sender = Sender()
        sender.sock = sock
        sender.start()
        print("Listening on {}:{} from *:{}..." \
                .format(parsed.bind_addr, port, from_port))
        while True:
            data,addr = sock.recvfrom(4096)
            if(data == b'=PIKERN Hey, listen!='):
                sender.addr = (addr[0], send_port)
                sock.sendto(b"RPi", addr)
                time=datetime.datetime.now().strftime('%H:%M:%S')
                nextColorBar()
                print("[{}] Invited to {}".format(time,addr[0]))
                continue

            if(addr[1] != from_port): # Wrong incoming port
                continue
            print(data.decode('utf-8'), end='')

logDump = LogDump()
logDump.start()

