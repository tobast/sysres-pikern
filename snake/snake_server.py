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
                if self.new_client(u):
                    self.action(u, data)
            

class Server():
    def __init__(self):
        self.connection = ClientConnection(SERVER_PORT, self.got_packet, self.new_client)
        self.connection.start()
        self.snakes = {}
        self.directions = {}
        self.snake_colors = {}
        self.apples = []
        self.stopped = False
        self.extend_sizes = {}
        for _ in range(NB_APPLES):
            self.add_apple()
        self.loop()
        
    def set_dir_event(self, d):
        return lambda e: self.set_direction(d)

    def snake_exists(self, p):
        return any(p in snake for snake in self.snakes.values())

    def add_apple(self):
        while True:
            x, y = randrange(WIDTH), randrange(HEIGHT)
            if self.snake_exists((x, y)) or (x, y) in self.apples:
                continue
            self.apples.append((x, y))
            return

    def is_valid(self, p):
        return 0 <= p[0] < WIDTH and 0 <= p[1] < HEIGHT

    def move(self):
        for i in self.snakes:
            snake = self.snakes[i]
            p = snake[-1]
            np = p2add(p, self.directions[i])
            if self.snake_exists(np) or not self.is_valid(np):
                self.stopped = True
                continue
            snake.append(np)
            if np in self.apples:
                self.apples.remove(np)
                self.extend_sizes[i] += 2
                self.add_apple()
            if self.extend_sizes[i] > 0:
                self.extend_sizes[i] -= 1
            else:
                del snake[0]

    def reset(self):
        self.stopped = False
        self.snakes = {}
        self.apples = []
        initial_length = 5
        ahead_length = 3
        w = initial_length + ahead_length
        for cid in self.directions:
            tries = 100
            for _ in range(tries):
                x = randrange(WIDTH - w)
                y = randrange(HEIGHT - w)
                if any(self.snake_exists((x + i, y)) or (x + i, y) in self.apples \
                       for i in range(w)):
                    continue
                self.snakes[cid] = [(x + i, y) for i in range(initial_length)]
                break
            self.directions[cid] = (1, 0)
            self.extend_sizes[cid] = 0
        for _ in range(NB_APPLES):
            self.add_apple()
            

    def loop(self):
        while True:
            if self.stopped:
                time.sleep(2.)
                self.reset()
            self.move()
            self.send_data()
            time.sleep(PERIOD / 1000.)

    def send_data(self):
        for i in self.snakes:
            pck = Packet()
            pck.add_uint8(SET_SNAKE)
            pck.add_uint16(i)
            pck.add_position_list(self.snakes[i])
            self.connection.send_all(pck.data)

        pck = Packet()
        pck.add_uint8(SET_APPLES)
        pck.add_position_list(self.apples)
        self.connection.send_all(pck.data)

    def got_packet(self, client_id, packet):
        pck = Packet(packet)
        command = pck.read_uint8()
        if command == SET_DIRECTION:
            self.directions[client_id] = DIRS[pck.read_uint8()]
        elif command == TOSERVER_INIT:
            rpck = Packet()
            rpck.add_uint8(TOCLIENT_INIT)
            rpck.add_uint16(client_id)
            self.connection.send(rpck.data, client_id)
                
            r, g, b = randrange(1 << 16), randrange(1 << 16), randrange(1 << 16)
            rpck = Packet()
            rpck.add_uint8(SET_SNAKE_COLOR)
            rpck.add_uint16(client_id)
            rpck.add_color((r, g, b))
            self.connection.send_all(rpck.data)

            for i in self.snake_colors:
                rpck = Packet()
                rpck.add_uint8(SET_SNAKE_COLOR)
                rpck.add_uint16(i)
                rpck.add_color(self.snake_colors[i])
                self.connection.send(rpck.data, client_id)

            self.snake_colors[client_id] = (r, g, b)

    def new_client(self, client_id):
        initial_length = 5
        ahead_length = 3
        w = initial_length + ahead_length
        tries = 100
        for _ in range(tries):
            x = randrange(WIDTH - w)
            y = randrange(HEIGHT - w)
            if any(self.snake_exists((x + i, y)) or (x + i, y) in self.apples \
                   for i in range(w)):
                continue
            self.snakes[client_id] = [(x + i, y) for i in range(initial_length)]
            break
        else:
            pck = Packet()
            pck.add_uint8(TOCLIENT_ACCESS_DENIED)
            self.connection.send(pck.data, client_id)
            return False
        self.directions[client_id] = (1, 0)
        self.extend_sizes[client_id] = 0
        print("Hello %d" % client_id)
        return True
        

    def mainloop(self):
        while True:
            pass

    
Server().mainloop()
