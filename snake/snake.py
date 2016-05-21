import sys
from tkinter import *
from random import randrange
import threading
import socket
from snake_common import *

class ServerConnection(threading.Thread):
    def __init__(self, addr, port, client_port, action):
        threading.Thread.__init__(self)
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.dest = (addr, port)
        self.sock.bind(("", client_port))
        self.client_port = self.sock.getsockname()[1]
        self.action = action

    def send(self, message):
        self.sock.sendto(message, self.dest)

    def run(self):
        while 1:
            data, addr = self.sock.recvfrom(4096)
            if addr == self.dest:
                self.action(data)

class App:
    def __init__(self):
        self.tk = Tk()
        self.connection = ServerConnection("127.0.0.1", SERVER_PORT, 0, self.got_server_message)
        self.connection.start()
        pck = Packet()
        pck.add_uint8(TOSERVER_INIT)
        self.connection.send(pck.data)
        self.can = Canvas(self.tk, width = PSIZE * WIDTH, height = PSIZE * HEIGHT, bg = "white")
        self.can.pack()
        self.tk.after(0, self.loop)
        initial_length = 5
        self.snake = [(WIDTH // 2 - initial_length // 2 + i, HEIGHT // 2) for i in range(initial_length)]
        self.direction = (1, 0)
        self.apples = []
        self.stopped = False
        self.extend_size = 0
        self.tk.bind("<Up>", self.set_dir_event(3))
        self.tk.bind("<Down>", self.set_dir_event(0))
        self.tk.bind("<Left>", self.set_dir_event(2))
        self.tk.bind("<Right>", self.set_dir_event(1))

    def got_server_message(self, message):
        packet = Packet(message)
        message_type = packet.read_uint8()
        if message_type == SET_SNAKE:
            self.snake = packet.read_position_list()
        elif message_type == SET_APPLES:
            self.apples = packet.read_position_list()
        
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
        if len(self.snake) > 0 and p2add(self.snake[-1], DIRS[newdir]) == self.snake[-2]:
            return
        pck = Packet()
        pck.add_uint8(SET_DIRECTION)
        pck.add_uint8(newdir)
        self.connection.send(pck.data)

    def loop(self):
        if self.stopped: return
        self.tk.after(PERIOD, self.loop)
        self.draw()

    def draw(self):
        self.can.delete(ALL)
        for x, y in self.snake:
            self.can.create_rectangle(PSIZE * x, PSIZE * y, PSIZE * (x + 1), PSIZE * (y + 1), width = 0, fill = "green")
        for x, y in self.apples:
            self.can.create_rectangle(PSIZE * x, PSIZE * y, PSIZE * (x + 1), PSIZE * (y + 1), width = 0, fill = "red")

    def mainloop(self):
        self.tk.mainloop()

App().mainloop()
