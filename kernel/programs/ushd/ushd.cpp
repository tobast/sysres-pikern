#include "syslib.h"
#include "proglib.h"

struct Channel {
	Channel() {}
	Channel(Ipv4Addr addr, uint16_t port) : addr(addr), port(port) {}
	Channel(const UdpSysRead& sysread) :
		addr(sysread.fromAddr), port(sysread.fromPort) {}

	bool operator==(const Channel& oth) const {
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

struct UshCli {
	UshCli(int pid, int in, int out) :
		pid(pid), stdinSock(in), stdoutSock(out) {}
	int pid;
	int stdinSock, stdoutSock;
};

typedef HashTable<Channel, UshCli, ChannelHasher> UshCliTable;

int main(int, char**) {
	const char* CHILD_NAME = "bin/ush";
	const unsigned CHILD_NAME_LEN = 8;
	const uint16_t LISTEN_PORT = 22;
	const unsigned BUFFER_SIZE = 2048;

	UshCliTable ushCliTable;

	int udpSock = udp_bind(22);
	if(udpSock < 0)
		exit(3);
	int ushClientFile = find_file(CHILD_NAME);
	if(ushClientFile == 0)
		exit(2);

	while(true) {
		// ==== READ INBOUND DATA ====
		UdpSysRead readData;
		char buff[BUFFER_SIZE];
		int len = udp_read(udpSock, (void*)buff, BUFFER_SIZE, &readData);
		
		if(len > 0) {
			UshCliTable::Iterator itCli =
				ushCliTable.findIter(Channel(readData));
			
			if(itCli != ushCliTable.end()) { // Known client, open session.
				UshCli cli = *itCli;

				if(!is_ready_write(cli.stdinSock))
					continue;
				int written = write(cli.stdinSock, buff, len);
				if(written != len) {
					sleep(100);
					if(is_ready_write(cli.stdinSock))
						write(cli.stdinSock, buff+written, len-written);
				}
			}
			else { // Unknown client
				execution_context context;
				context.stdin = new_socket();
				context.stdout = new_socket();
				context.argc = 1;
				context.argv = (char**)malloc(sizeof(char*));
				context.argv[0] = (char*) malloc(sizeof(char)*CHILD_NAME_LEN);
				for(unsigned pos=0; pos < CHILD_NAME_LEN; pos++)
					context.argv[0][pos] = CHILD_NAME[pos];

				int pid = execute_file(ushClientFile, &context);
				UshCli cli(pid, context.stdin, context.stdout);
				ushCliTable.insert(Channel(readData), cli);

				const char* motd = "Hello, world!\n";
				unsigned motdLen = 15;
				UdpSysData destData(readData.fromAddr, LISTEN_PORT,
						readData.fromPort, motd, motdLen);
				udp_write(&destData);
			}
		}
		else {
			bool didStuff=false;
			// ==== WRITE OUTBOUND DATA ====
			for(auto it = ushCliTable.begin(); it != ushCliTable.end(); ++it) {
				if(is_ready_read((*it).stdoutSock)) {
					didStuff = true;
					len = read((*it).stdoutSock, buff, BUFFER_SIZE);

					UdpSysData destDat(it.key().addr, LISTEN_PORT,
							it.key().port, buff, len);
					udp_write(&destDat);
				}
			}

			if(!didStuff)
				sleep(100);
		}
	}
}

