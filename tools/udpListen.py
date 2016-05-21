#!/usr/bin/python3

import socket
import datetime
import sys

addr,port = "0.0.0.0",3141
if len(sys.argv) > 1:
    if len(sys.argv) > 2:
        port = int(argv[2])
        addr = argv[1]
    else:
        port = int(argv[1])

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((addr,port))
print("Listening on {}:{}...".format(addr,port))
while True:
    data,addr = sock.recvfrom(4096)
    time=datetime.datetime.now().strftime('%H:%M:%S')
#    spl = data.split(b'\n')
#    for (i,l) in enumerate(spl):
#        print("[{}:{}, {}] {}".format(addr[0],addr[1],time,l.decode('utf-8')),
#                end='\n' if i<len(spl)-1 else '')
    print(data.decode('utf-8'), end='')

