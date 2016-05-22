
SERVER_PORT = 31412

PSIZE = 20
WIDTH = 30
HEIGHT = 30
PERIOD = 100

def p2add(u, v):
    return (u[0] + v[0], u[1] + v[1])

DIRS = [(0, 1), (1, 0), (-1, 0), (0, -1)]
NB_APPLES = 3

class Packet:
    def __init__(self, data = b''):
        self.start_index = 0
        self.data = data

    def add_position(self, p):
        self.data += bytes((p[0], p[1]))

    def add_uint16(self, n):
        self.data += bytes(((n >> 8) & 0xff, n & 0xff))

    def add_uint8(self, n):
        self.data += bytes((n,))

    def add_color(self, c):
        self.add_uint16(c[0])
        self.add_uint16(c[1])
        self.add_uint16(c[2])

    def add_position_list(self, l):
        self.add_uint16(len(l))
        for p in l:
            self.add_position(p)

    def read_position(self):
        r = self.data[self.start_index]
        s = self.data[self.start_index + 1]
        self.start_index += 2
        return (r, s)

    def read_uint16(self):
        r = self.data[self.start_index]
        s = self.data[self.start_index + 1]
        self.start_index += 2
        return (r << 8) | s

    def read_uint8(self):
        r = self.data[self.start_index]
        self.start_index += 1
        return r

    def read_position_list(self):
        l = []
        n = self.read_uint16()
        for i in range(n):
            l.append(self.read_position())
        return l

    def read_color(self):
        r = self.read_uint16()
        g = self.read_uint16()
        b = self.read_uint16()
        return (r, g, b)

TOSERVER_INIT = 0
TOCLIENT_INIT = 1
SET_SNAKE = 2
SET_APPLES = 3
SET_DIRECTION = 4
SET_SNAKE_COLOR = 5
TOCLIENT_ACCESS_DENIED = 6
