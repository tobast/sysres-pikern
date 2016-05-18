import socket
import datetime
import sys

addr,port = "10.0.0.1",3141
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
    print("[{}:{}, {}] {}".format(addr[0],addr[1],time,data.decode('utf-8')))
