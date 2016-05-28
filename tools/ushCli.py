#!/usr/bin/python3

import socket
import datetime
import sys
import threading
import time
import argparse


parser = argparse.ArgumentParser()
parser.add_argument('-f', '--from-port', help='Selects the destination port.'\
        +' 0 for any, defaults to 22.', default=22)
parser.add_argument('-p', '--port', help='Binds this port. Defaults to 4222.',\
        default=4222)
parser.add_argument('-a', '--bind-addr', help='Binds this address. Defaults '\
        +' to 0.0.0.0.', default="")
parser.add_argument('-s', '--send-port', help='Sends data to this port. '\
        +'Defaults to 22.', default=22)
parser.add_argument('--host', help="Sends data to this address. "\
        +"Defaults to the default Raspberry Pi address, 129.199.157.174.",\
        default='129.199.157.174')

parsed = parser.parse_args()

class Sender(threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)
        self.sock = None
        self.addr = None
        self.stop = False
        self.doSend = False
    def run(self):
        global parsed
        host = parsed.host
        port = int(parsed.send_port)

        self.addr = (host, port)

        if(self.addr == None):
            print('ERROR: Address unbound.', file=sys.stdout)
            sys.exit(1)

        self.sock.sendto(b'=Hey, listen!=\n', self.addr)

        while not self.doSend:
            time.sleep(0.01)
            if self.stop:
                exit(1)

        while True:
            try:
                data = input() + '\n'
            except EOFError:
                sys.exit(0)

            if self.addr == None:
                print('ERROR: Address unbound.', file=sys.stderr)
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

        sockTimeout = 0.5
        startWaitTime = time.time()
        while True:
            try:
                sock.settimeout(sockTimeout - (time.time() - startWaitTime))
                data,addr = sock.recvfrom(4096)
                if(addr[1] != from_port):
                    continue
                if(data == b"=Hello=\n"):
                    break
            except socket.timeout:
                print('ERROR: timeout.', file=sys.stderr)
                sender.stop = True
                sys.exit(1)
        sock.settimeout(0.1)
        sender.doSend = True

        while True:
            if not sender.is_alive():
                sys.exit(0)

            try:
                data,addr = sock.recvfrom(4096)
            except socket.timeout:
                continue

            if(addr[1] != from_port): # Wrong incoming port
                continue

            print(data.decode('utf-8'), end='')
            sys.stdout.flush()

logDump = LogDump()
logDump.start()

