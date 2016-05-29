#include "syslib.h"
#include "proglib.h"

u32 rand_state[5];
u32 rand_mods[5] = {131071, 32767, 257, 65535, 2047};
u32 rand_a[5] = {67235, 12345, 195, 20671, 1379};
u32 rand_b[5] = {19237, 3176, 47, 9424, 871};
int rand_next(int n) {
	for (int i = 0; i < 5; i++) {
		rand_state[i] = (rand_state[i] * rand_a[i] + rand_b[i]) % rand_mods[i];
	}
	return rand_state[rand_state[4] % 4] % n;
}
void rand_reseed() {
	void* addr = malloc(1);
	u32 i = (u32)addr;
	free(addr);
	rand_state[0] = i;
	rand_state[1] = i + 17;
	rand_state[2] = i + 42;
	rand_state[3] = i + 51;
	rand_state[4] = i + 23;
	rand_next(1);
	rand_next(1);
	rand_next(1);
}

struct Channel {
	Channel() {}
	Channel(Ipv4Addr addr, uint16_t port) : addr(addr), port(port) {}
	Channel(const UdpSysRead& sysread) :
		addr(sysread.fromAddr), port(sysread.fromPort) {}

	bool operator == (const Channel& oth) const {
		return addr == oth.addr && port == oth.port;
	}

	Ipv4Addr addr;
	uint16_t port;
};
struct ChannelHasher {
	uint32_t operator()(const Channel& ch) const {
		return ch.addr ^ (((uint32_t)ch.port) << 16);
	}
};

typedef HashTable<Channel, int, ChannelHasher> ClientTable;

struct pos {
	int x;
	int y;
	inline pos() {};
	inline pos(int x_, int y_) : x(x_), y(y_) {};
	inline pos operator + (const pos &other) const {
		return pos(x + other.x, y + other.y);
	};
	inline pos operator - (const pos &other) const {
		return pos(x - other.x, y - other.y);
	};
	inline bool operator == (const pos &other) const {
		return (x == other.x) && (y == other.y);
	};
};

struct color {
	uint16_t r;
	uint16_t g;
	uint16_t b;
	color(uint16_t r, uint16_t g, uint16_t b) : r(r), g(g), b(b) {};
	color() {};
};

struct SnakeInfo {
	ExpArray<pos> snake;
	pos direction;
	color snake_color;
	int extend_size;
	SnakeInfo() {
		snake.init();
	}
};

class Packet {
public:
	char *buffer;
	int len;
	int start_pos;
	Packet(char *buffer, int len) :
		buffer(buffer), len(len), start_pos(0) {};
	uint8_t read_uint8() {
		return (uint8_t)buffer[start_pos++];
	};
	void add_uint8(uint8_t v) {
		buffer[len++] = v;
	};
	void add_uint16(uint16_t v) {
		buffer[len++] = (uint8_t)(v >> 8);
		buffer[len++] = (uint8_t)(v & 0xff);
	};
	void add_color(color c) {
		add_uint16(c.r);
		add_uint16(c.g);
		add_uint16(c.b);
	};
	void add_position(pos p) {
		add_uint16(p.x);
		add_uint16(p.y);
	};
	void add_position_list(ExpArray<pos> positions) {
		add_uint16(positions.size());
		for (uint16_t i = 0; i < positions.size(); i++) {
			add_position(positions[i]);
		}
	};
};

enum commands {
	TOSERVER_INIT = 0,
	TOCLIENT_INIT = 1,
	SET_SNAKE = 2,
	SET_APPLES = 3,
	SET_DIRECTION = 4,
	SET_SNAKE_COLOR = 5,
	TOCLIENT_ACCESS_DENIED = 6
};

class SnakeServer {
public:
	const uint16_t LISTEN_PORT = 31412;
	const int16_t WIDTH = 30;
	const int16_t HEIGHT = 30;
	pos DIRS[4];
	const uint16_t NB_APPLES = 3;
	char* send_buffer;
	bool stopped;

	ClientTable clientTable;
	ExpArray<Channel> channelTable;
	int udpSock;

	ExpArray<SnakeInfo> snakes;
	ExpArray<pos> apples;

	SnakeServer() {
		udpSock = udp_bind(LISTEN_PORT);
		if(udpSock < 0)
			exit(3);
		DIRS[0] = pos(0, 1);
		DIRS[1] = pos(1, 0);
		DIRS[2] = pos(-1, 0);
		DIRS[3] = pos(0, -1);
		send_buffer = (char*)malloc(65536);
		stopped = false;
	};

	bool snake_exists(pos p) {
		for (uint16_t i = 0; i < snakes.size(); i++) {
			for (uint16_t j = 0; j < snakes[i].snake.size(); j++) {
				if (snakes[i].snake[j] == p) {
					return true;
				}
			}
		}
		return false;
	};

	bool apple_exists(pos p) {
		for (uint16_t i = 0; i < apples.size(); i++) {
			if (apples[i] == p) {
				return true;
			}
		}
		return false;
	};

	bool is_valid(pos p) {
		return 0 <= p.x && p.x < WIDTH && 0 <= p.y && p.y < HEIGHT;
	};

	void run() {
		while (true) {
			process_incoming();
			move();
			send_data();
			sleep(100 * 1000);
		}
	};

	void move() {
		for (uint16_t i = 0; i < snakes.size(); i++) {
			ExpArray<pos> s = snakes[i].snake;
			pos p = s[s.size() - 1];
			pos np = p + snakes[i].direction;
			if (snake_exists(np) || !is_valid(np)) {
				stopped = true;
				return;
			}
			s.push_back(np);
			if (apple_exists(np)) {
				delete_apple(np);
				snakes[i].extend_size += 2;
				add_apple();
			}
			if (snakes[i].extend_size > 0) {
				snakes[i].extend_size--;
			} else {
				for (uint16_t j = 0; j < s.size() - 1; j++) {
					s[j] = s[j + 1];
				}
				s.pop_back();
			}
		}
	};

	void delete_apple(pos p) {
		for (uint16_t i = 0; i < apples.size(); i++) {
			if (apples[i] == p) {
				apples[i] = apples[apples.size() - 1];
				apples.pop_back();
				return;
			}
		}
	};

	void add_apple() {
		while (true) {
			uint16_t x = rand_next(WIDTH);
			uint16_t y = rand_next(HEIGHT);
			if (snake_exists(pos(x, y)) || apple_exists(pos(x, y))) {
				continue;
			}
			apples.push_back(pos(x, y));
			return;
		}
	};

	void process_packet(int clientId, char* buffer, int len) {
		Packet pck(buffer, len);
		commands cm = (commands)pck.read_uint8();
		if (cm == SET_DIRECTION) {
			snakes[clientId].direction = DIRS[pck.read_uint8()];
		} else if (cm == TOSERVER_INIT) {
			{
				Packet rpck(send_buffer, 0);
				rpck.add_uint8(TOCLIENT_INIT);
				rpck.add_uint16(clientId);
				sendClient(clientId, rpck.buffer, rpck.len);
			}

			uint16_t r = rand_next(1 << 16);
			uint16_t g = rand_next(1 << 16);
			uint16_t b = rand_next(1 << 16);
			snakes[clientId].snake_color = color(r, g, b);

			{
				Packet rpck(send_buffer, 0);
				rpck.add_uint8(SET_SNAKE_COLOR);
				rpck.add_uint16(clientId);
				rpck.add_color(color(r, g, b));
				sendAll(rpck.buffer, rpck.len);
			}
			
			for (uint16_t i = 0; i < snakes.size(); i++) {
				if (i == clientId) continue;
				Packet rpck(send_buffer, 0);
				rpck.add_uint8(SET_SNAKE_COLOR);
				rpck.add_uint16(i);
				rpck.add_color(snakes[i].snake_color);
				sendClient(clientId, rpck.buffer, rpck.len);
			}
		}
	};

	void new_client(int clientId) {
		const int initial_length = 5;
		const int ahead_length = 3;
		const int w = initial_length + ahead_length;
		const int tries = 100;
		for (int _ = 0; _ < tries; _++) {
			uint16_t x = rand_next(WIDTH - w);
			uint16_t y = rand_next(HEIGHT);
			bool ok = true;
			for (int i = 0; i < w; i++) {
				if (snake_exists(pos(x + i, y)) || apple_exists(pos(x + i, y))) {
					ok = false;
					break;
				}
			}
			if (ok) {
				SnakeInfo sn;
				for (int i = 0; i < w; i++) {
					sn.snake.push_back(pos(x + i, y));
				}
				sn.direction = pos(1, 0);
				sn.extend_size = 0;
				snakes.push_back(sn);
				printf("Hello %d\n", clientId);
				return;
			}
		}
		Packet rpck(send_buffer, 0);
		rpck.add_uint8(TOCLIENT_ACCESS_DENIED);
		sendClient(clientId, rpck.buffer, rpck.len);
	};

	void send_data() {
		for (uint16_t i = 0; i < snakes.size(); i++) {
			Packet pck(send_buffer, 0);
			pck.add_uint8(SET_SNAKE);
			pck.add_position_list(snakes[i].snake);
			sendAll(pck.buffer, pck.len);
		}

		Packet pck(send_buffer, 0);
		pck.add_uint8(SET_APPLES);
		pck.add_position_list(apples);
		sendAll(pck.buffer, pck.len);
	};

	void process_incoming() {
		const unsigned BUFFER_SIZE = 2048;
		while(true) {
			UdpSysRead readData;
			char buff[BUFFER_SIZE];
			int len = udp_read(udpSock, (void*)buff, BUFFER_SIZE, &readData);
			if (len == 0) {
				break;
			}

			ClientTable::Iterator itCli =
				clientTable.findIter(Channel(readData));
			
			if(itCli != clientTable.end()) { // Known client
				int clientId = *itCli;
				process_packet(clientId, buff, len);
			} else { // Unknown client
				int clientId = channelTable.size();
				Channel chan(readData);
				clientTable.insert(chan, clientId);
				channelTable.push_back(chan);
				new_client(clientId);
				process_packet(clientId, buff, len);
			}
		}
	};

	void sendClient(int clientId, char* buffer, int size) {
		UdpSysData destDat(channelTable[clientId].addr, LISTEN_PORT,
			channelTable[clientId].port, buffer, size);
		udp_write(&destDat);
	};

	void sendAll(char* buffer, int size) {
		for (uint16_t i = 0; i < channelTable.size(); i++) {
			sendClient(i, buffer, size);
		}
	};
};

int main(int, char**) {
	rand_reseed();
	SnakeServer server;
	server.run();
	return 0;
}
