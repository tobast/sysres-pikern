import sys
from tkinter import *
from random import randrange
import threading
import socket
from snake_common import *
import time

class ClientConnection(threading.Thread):
    def __init__(self, port, action, new_client):
        threading.Thread.__init__(self)
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.bind(("", port))
        self.clients_to_addr = {}
        self.addr_to_clients = {}
        self.min_client_id = 0
        self.action = action
        self.new_client = new_client

    def send(self, message, client):
        self.sock.sendto(message, self.clients_to_addr[client])

    def send_all(self, message):
        for addr in self.addr_to_clients:
            self.sock.sendto(message, addr)

    def run(self):
        while True:
            data, addr = self.sock.recvfrom(4096)
            if addr in self.addr_to_clients:
                self.action(self.addr_to_clients[addr], data)
            else:
                u = self.min_client_id
                self.min_client_id += 1
                self.addr_to_clients[addr] = u
                self.clients_to_addr[u] = addr
                self.new_client(u)
                self.action(u, data)
            

class Server():
    def __init__(self):
        self.connection = ClientConnection(SERVER_PORT, self.got_packet, self.new_client)
        self.connection.start()
        initial_length = 5
        self.snake = [(WIDTH // 2 - initial_length // 2 + i, HEIGHT // 2) for i in range(initial_length)]
        self.direction = (1, 0)
        self.apples = []
        self.stopped = False
        self.started = False
        self.extend_size = 0
        for _ in range(NB_APPLES):
            self.add_apple()
        self.loop()

    def new_client(self, client):
        self.started = True
        
    def set_dir_event(self, d):
        return lambda e: self.set_direction(d)

    def add_apple(self):
        while True:
            x, y = randrange(WIDTH), randrange(HEIGHT)
            if (x, y) in self.snake or (x, y) in self.apples:
                continue
            self.apples.append((x, y))
            return

    def is_valid(self, p):
        return 0 <= p[0] < WIDTH and 0 <= p[1] < HEIGHT

    def set_direction(self, newdir):
        self.direction = newdir

    def move(self):
        p = self.snake[-1]
        np = p2add(p, self.direction)
        if np in self.snake or not self.is_valid(np):
            self.stopped = True
        self.snake.append(np)
        if np in self.apples:
            self.apples.remove(np)
            self.extend_size += 2
            self.add_apple()
        if self.extend_size > 0:
            self.extend_size -= 1
        else:
            del self.snake[0]

    def loop(self):
        while True:
            if not self.started:
                continue
            if self.stopped:
                break
            self.move()
            self.send_data()
            time.sleep(PERIOD / 1000.)

    def send_data(self):
        pck = Packet()
        pck.add_uint8(SET_SNAKE)
        pck.add_position_list(self.snake)
        self.connection.send_all(pck.data)

        pck = Packet()
        pck.add_uint8(SET_APPLES)
        pck.add_position_list(self.apples)
        self.connection.send_all(pck.data)

    def got_packet(self, client_id, packet):
        assert (client_id == 0)
        pck = Packet(packet)
        command = pck.read_uint8()
        if command == SET_DIRECTION:
            self.direction = DIRS[pck.read_uint8()]

    def mainloop(self):
        while True:
            pass

    
Server().mainloop()
