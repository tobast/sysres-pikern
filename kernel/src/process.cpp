#include "assert.h"
#include "barriers.h"
#include "hardware_constants.h"
#include "interrupts.h"
#include "process.h"
#include "sleep.h"
#include "svc.h"
#include "uspi_interface.h"
#include "malloc.h"
#include "expArray.h"
#include "genericSocket.h"
#include "filesystem.h"
#include "networkCore.h"
#include "gpio.h" // FIXME DEBUG

struct context {
	s32 spsr;
	s32 lr;
	s32 r0;
	s32 r1;
	s32 r2;
	s32 r3;
	s32 r4;
	s32 r5;
	s32 r6;
	s32 r7;
	s32 r8;
	s32 r9;
	s32 r10;
	s32 r11;
	s32 r12;
	s32 r13;
	s32 r14;
	context () {
		r0 = r1 = r2 = r3 = r4 = r5 = r6 = r7 = 0;
		r8 = r9 = r10 = r11 = r12 = r13 = r14 = 0;
		spsr = lr = 0;
	};
};

enum pstate {
	PROCESS_ACTIVE,
	PROCESS_WAIT_READ,
	PROCESS_WAIT_WRITE,
	PROCESS_SLEEPING,
	PROCESS_INEXISTANT,
	PROCESS_ZOMBIE,
	PROCESS_WAITING
};

const int PNAME_MAX_SIZE = 32;
struct process {
	context cont;
	int next_process;
	int previous_process;
	pstate process_state;
	u64 state_info;
	char process_name[PNAME_MAX_SIZE];
	node* cwd;
};

const int NUMBER_INTERRUPTS = 64;
struct interrupt_context {
	TInterruptHandler* handler;
	void* param;
};
interrupt_context bound_interrupts[NUMBER_INTERRUPTS];
void init_interrupts() {
	for (int i = 0; i < NUMBER_INTERRUPTS; i++) {
		bound_interrupts[i].handler = NULL;
	}
}
void ConnectInterrupt(unsigned nIRQ, TInterruptHandler *pHandler, void *pParam) {
	bound_interrupts[nIRQ].handler = pHandler;
	bound_interrupts[nIRQ].param = pParam;
	hardware::IRQ_ENABLE_1[nIRQ >> 5] = 1 << (nIRQ & 31);
}


const int MAX_TIMERS = 256;
struct timer_context {
	TKernelTimerHandler* handler;
	void* param;
	void* context;
	u64 trigger_time;
};

timer_context timers[MAX_TIMERS];
u64 first_trigger = (u64)(-1);
void init_timers() {
	for (int i = 0; i < MAX_TIMERS; i++) {
		timers[i].handler = NULL;
	}
}
int get_timer_context() {
	for (int i = 0; i < MAX_TIMERS; i++) {
		if (timers[i].handler == NULL) {
			return i;
		}
	}
	assert(false, 0x13);
	return 0;
}
void recalc_trigger() {
	first_trigger = (u64)(-1);
	for (int i = 0; i < MAX_TIMERS; i++) {
		if (timers[i].handler != NULL) {
			first_trigger = min(first_trigger, timers[i].trigger_time);
		}
	}
}
void check_and_run_timers() {
	u64 current_time = elapsed_us();
	if (current_time < first_trigger) return;
	for (int i = 0; i < MAX_TIMERS; i++) {
		if (timers[i].handler != NULL && timers[i].trigger_time <= current_time) {
			timers[i].handler(i, timers[i].param, timers[i].context);
			timers[i].handler = NULL;
		}
	}
	recalc_trigger();
}
unsigned StartKernelTimer (unsigned nHzDelay,
		TKernelTimerHandler *pHandler, void *pParam, void *pContext) {
	u64 current_time = elapsed_us();
	int i = get_timer_context();
	timers[i].handler = pHandler;
	timers[i].param = pParam;
	timers[i].context = pContext;
	timers[i].trigger_time = current_time + nHzDelay * 1000000 / HZ;
	return (unsigned)i;
}
void CancelKernelTimer (unsigned timer) {
	timers[timer].handler = NULL;
}


// const int MAX_PROCESS = 128;
ExpArray<process> processes;
int active_process = 0;
int free_process = 1;

const int SOCKET_BUFFER_SIZE = 2048;
//const int MAX_SOCKETS = 128;
// TODO: file/socket descriptor table
//struct socket {
//	int start;
//	int length;
//	char buffer[SOCKET_BUFFER_SIZE];
//x};
ExpArray<GenericSocket*> sockets;
ExpArray<UdpSocket*> udpSockets;

int create_socket(GenericSocket *s) {
	if (s == NULL) {
		s = (GenericSocket*)malloc(sizeof(GenericSocket));
		*s = GenericSocket(true);
	}
	for (unsigned i = 0; i < sockets.size(); i++) {
		if (sockets[i] == NULL) {
			sockets[i] = s;
			return i;
		}
	}
	unsigned i = sockets.size();
	sockets.push_back(s);
	return i;
}

int create_udp_socket(UdpSocket *s) {
	if (s == NULL) {
		return -1;
	}
	for (unsigned i = 0; i < udpSockets.size(); i++) {
		if (udpSockets[i] == NULL) {
			udpSockets[i] = s;
			return i;
		}
	}
	unsigned i = udpSockets.size();
	udpSockets.push_back(s);
	return i;
}

void close_socket(int i) {
	// TODO: processes that use that socket
	// --> file descriptors table?
	(sockets[i])->GenericSocket::~GenericSocket();
	free(sockets[i]);
	sockets[i] = NULL;
}

void close_udp_socket(int i) {
	// TODO same as above
	(udpSockets[i])->UdpSocket::~UdpSocket();
	free(udpSockets[i]);
	udpSockets[i] = NULL;
}

/*
int sread(int i, void* addr, int num) {
	int do_read = min(sockets[i].length, num);
	int index = sockets[i].start;
	for (int k = 0; k < do_read; k++) {
		((char*)addr)[k] = sockets[i].buffer[index];
		index++;
		if (index == SOCKET_BUFFER_SIZE) {
			index = 0;
		}
	}
	sockets[i].start = index;
	sockets[i].length -= do_read;
	return do_read;
}
*/

/*
int swrite(int i, void* addr, int num) {
	int do_write = min(SOCKET_BUFFER_SIZE - sockets[i].length, num);
	int index = (sockets[i].start + sockets[i].length) % SOCKET_BUFFER_SIZE;
	for (int k = 0; k < do_write; k++) {
		sockets[i].buffer[index] = ((char*)addr)[k]; 
		index++;
		if (index == SOCKET_BUFFER_SIZE) {
			index = 0;
		}
	}
	sockets[i].length += do_write;
	return do_write;
}
*/

void reset_timer() {
	hardware::ARM_TIMER[3] = 0;
	hardware::ARM_TIMER[0] = 500;
}

void go_next_process(context* current_context) {
	processes[active_process].cont = *current_context;
	next_process();
	*current_context = processes[active_process].cont;
}

extern "C" void on_irq(void* stack_pointer) {
	dataMemoryBarrier();
	for (int i = 0; i < NUMBER_INTERRUPTS; i++) {
		if (hardware::IRQ_PENDING_1[i >> 5] & (1 << (i & 31))) {
			hardware::IRQ_PENDING_1[i >> 5] = 1 << (i & 31);
			if (bound_interrupts[i].handler != NULL) {
				bound_interrupts[i].handler(bound_interrupts[i].param);
				dataMemoryBarrier();
			} else {
				hardware::IRQ_DISABLE_1[i >> 5] = 1 << (i & 31);
			}
		}
	}
	dataMemoryBarrier();
	if ((*hardware::IRQ_BASIC_PENDING) & 1) {
		*hardware::IRQ_BASIC_PENDING = 1;
		check_and_run_timers();
		go_next_process((context*)stack_pointer);
		reset_timer();
	}
	dataMemoryBarrier();
}

extern "C" void on_svc(void* stack_pointer, int svc_number) {
	context* current_context = (context*)stack_pointer;
	switch (svc_number) {
		case SVC_GET_PID: {
			current_context->r0 = active_process;
			return;
		}
		case SVC_READ: {
			int socket = (int)current_context->r0;
			if(socket == -1) {// /dev/null
				current_context->r0 = 0;
				return;
			}

			void* addr = (void*)current_context->r1;
			int size = (int)current_context->r2;
			if (sockets[socket]->isEmpty()) {
				processes[active_process].process_state = PROCESS_WAIT_READ;
				go_next_process(current_context);
				reset_timer();
			} else {
				current_context->r0 = sockets[socket]->read(addr, size);
			}
			return;
		}
		case SVC_WRITE: {
			int socket = (int)current_context->r0;
			void* addr = (void*)current_context->r1;
			int size = (int)current_context->r2;
			if(socket == -1) { // /dev/null
				current_context->r0 = size;
				return;
			}

			if (sockets[socket]->isFull()) {
				processes[active_process].process_state = PROCESS_WAIT_WRITE;
				go_next_process(current_context);
				reset_timer();
			} else {
				current_context->r0 = sockets[socket]->write(addr, size);
			}
			return;
		}
		case SVC_SLEEP: {
			processes[active_process].process_state = PROCESS_SLEEPING;
			u32 wait_time = (u32)current_context->r0;
			u64 trigger_time = elapsed_us() + (u64)wait_time;
			processes[active_process].state_info = trigger_time;
			go_next_process(current_context);
			reset_timer();
			return;
		}
		case SVC_MALLOC: {
			current_context->r0 = (u32)malloc_nocheck(current_context->r0);
			return;
	    }
		case SVC_FREE: {
			free_nocheck((void*)current_context->r0);
			return;
	    }
		case SVC_ATOMIC_CAS: {
			s32* r = (s32*)current_context->r0;
			s32 y = current_context->r1;
			s32 z = current_context->r2;
			s32 old_val = *r;
			if (old_val == y) {
				*r = z;
			}
			current_context->r0 = old_val;
			return;
		}
		case SVC_KILL: {
			int i = current_context->r0;
			processes[i].process_state = PROCESS_ZOMBIE;
			processes[i].state_info = current_context->r1;
			if (i == active_process) {
				go_next_process(current_context);
				reset_timer();
			}
			return;
		}
		case SVC_WAIT: {
			int i = current_context->r0;
			if (i == active_process) {
				current_context->r0 = 0;
				return;
			}
			if (processes[i].process_state == PROCESS_ZOMBIE) {
				current_context->r0 = (s32)processes[i].state_info;
				delete_process(i);
				return;
			}
			else if(processes[i].process_state == PROCESS_INEXISTANT) {
				current_context->r0 = (s32)-1;
				return;
			}
			processes[active_process].process_state = PROCESS_WAITING;
			go_next_process(current_context);
			reset_timer();
			return;
		}
		case SVC_READY_READ: {
			if(current_context->r0 == -1) {
				current_context->r0 = false;
				return;
			}
			current_context->r0 = !(sockets[current_context->r0]->isEmpty());
			return;
		}
		case SVC_READY_WRITE: {
			if(current_context->r0 == -1) {
				current_context->r0 = true;
				return;
			}
			current_context->r0 = !(sockets[current_context->r0]->isFull());
			return;
		}
		case SVC_PROCESS_ALIVE: {
			int pid = current_context->r0;
			pstate state = processes[pid].process_state;
			current_context->r0 = (state != PROCESS_INEXISTANT) &&
			                      (state != PROCESS_ZOMBIE);
			return;
		}
		case SVC_FIND_FILE: {
			node *cwd = processes[active_process].cwd;
			folder *cwf = NULL;
			if (cwd != NULL && cwd->type == NODE_FOLDER) {
				cwf = cwd->node_folder;
			}
			current_context->r0 = (s32)follow_path((const char*)
				current_context->r0, cwf);
			return;
		}
		case SVC_FILE_SIZE: {
			node *nd = (node*)current_context->r0;
			if (nd->type != NODE_FILE) {
				current_context->r0 = -1;
				return;
			}
			current_context->r0 = nd->node_file->data.size();
			return;
		}
		case SVC_FILE_READ: {
			node *nd = (node*)current_context->r0;
			if (nd->type != NODE_FILE) {
				current_context->r0 = 0;
				return;
			}
			int offset = current_context->r1;
			char *addr = (char*)current_context->r2;
			int length = current_context->r3;
			int actual_read = min(length, ((int)nd->node_file->data.size()) - offset);
			char *dt = &nd->node_file->data[offset];
			for (int i = 0; i < actual_read; i++) {
				addr[i] = dt[i];
			}
			current_context->r0 = actual_read;
			return;
		}
		case SVC_NB_PROCESSES: {
			int r = 0;
			for (unsigned i = 1; i < processes.size(); i++) {
				pstate state = processes[i].process_state;
				if (state != PROCESS_INEXISTANT && state != PROCESS_ZOMBIE) {
					r++;
				}
			}
			current_context->r0 = r;
			return;
		}
		case SVC_GET_PROCESSES: {
			int* data = (int*)current_context->r0;
			int maxp = current_context->r1;
			int index = 0;
			for (unsigned i = 1; i < processes.size(); i++) {
				pstate state = processes[i].process_state;
				if (state != PROCESS_INEXISTANT && state != PROCESS_ZOMBIE) {
					data[index++] = i;
					if (index == maxp) {
						break;
					}
				}
			}
			current_context->r0 = index;
			return;
		}
		case SVC_GET_PROCESS_NAME: {
			int pid = current_context->r0;
			const char* name = process_name(pid);
			for (int i = 0; i < PNAME_MAX_SIZE; i++) {
			    ((char*)current_context->r1)[i] = name[i];
			}
			return;
		}
		case SVC_GET_PROCESS_STATE: {
			current_context->r0 = (int)(unsigned char)
				process_state(current_context->r0);
			return;
		}
		case SVC_SEND_UDP_PACKET: {
			current_context->r0 = nw::sendSysPacket(
					(UdpSysData*)current_context->r0);
			return;
		}
		case SVC_BIND_UDP: {
			current_context->r0 = create_udp_socket(
				nw::bindUdpPort((uint16_t)current_context->r0));
			return;
		}
		case SVC_READ_UDP: {
			nw::readUdpSocket(udpSockets[current_context->r0],
					(void*) current_context->r1,
					(unsigned) current_context->r2,
					(UdpSysRead*) current_context->r3 );
			current_context->r0 = ((UdpSysRead*)current_context->r3)->len;
			return;
		}
		case SVC_EXECUTE_FILE: {
			execution_context *ec =
				(execution_context*)(malloc(sizeof(execution_context)));
			*ec = *(execution_context*)current_context->r1;
			if (ec->cwd == 0) {
				ec->cwd = (int)processes[active_process].cwd;
			}
			current_context->r0 = run_process(
				(node*)current_context->r0, ec);
			return;
		}
		case SVC_IS_FOLDER: {
			current_context->r0 =
				((node*)current_context->r0)->type == NODE_FOLDER;
			return;
		}
		case SVC_NUM_CHILDREN: {
			node *nd = (node*)current_context->r0;
			if (nd->type != NODE_FOLDER) {
				current_context->r0 = -1;
				return;
			}
			current_context->r0 = nd->node_folder->contents.size();
			return;
		}
		case SVC_GET_CHILD: {
			node *nd = (node*)current_context->r0;
			int child_id = current_context->r1;
			if (nd->type != NODE_FOLDER) {
				current_context->r0 = 0;
				return;
			}
			if (child_id < 0 ||
					child_id > (int)nd->node_folder->contents.size()) {
				current_context->r0 = 0;
				return;
			}
			current_context->r0 = (int)nd->node_folder->contents[child_id];
			return;
		}
		case SVC_GET_NODE_NAME: {
			node *nd = (node*)current_context->r0;
			char *data = (char*)current_context->r1;
			int max_chars = current_context->r2;
			for (int i = 0; i < min(max_chars, MAX_NAME_LENGTH + 1); i++) {
				char c = nd->name[i];
				data[i] = c;
				if (c == '\0') break;
			}
			return;
		}
		case SVC_NEW_SOCKET: {
			current_context->r0 = create_socket();
			return;
		}
		case SVC_GET_CWD: {
			current_context->r0 = (int)processes[active_process].cwd;
			return;
		}
		case SVC_SET_CWD: {
			node* cwd = (node*)current_context->r0;
			if (cwd->type == NODE_FOLDER) {
				processes[active_process].cwd = cwd;
			}
			return;
		}
		case SVC_GET_PARENT: {
			current_context->r0 = (int)((node*)current_context->r0)->parent;
			return;
		}
		case SVC_GPIO_SET: {
			int i = current_context->r0;
			if (0 <= i && i < 8) {
				gpio::setVal(gpio::BYTE_PINS[i], (current_context->r1 != 0));
			}
			return;
		}
		default:
			// Invalid svc
			// Kill process?
			return;
	}
}

void init_process_table() {
	processes.init();
	process proc;
	processes.push_back(proc);
	processes[0].next_process = 0;
	processes[0].previous_process = 0;
	processes[0].process_state = PROCESS_ACTIVE;
	active_process = 0;
	free_process = 1;
	//for (int i = 1; i < MAX_PROCESS; i++) {
	//	processes[i].next_process = i + 1;
	//	processes[i].process_state = PROCESS_INEXISTANT;
	//}
	
	//free_socket = 0;
	sockets.init();
	//for (int i = 0; i < MAX_SOCKETS; i++) {
	//	sockets[i].start = i + 1;
	//}
	//sockets_allocator.init();

	set_irq_handler(&on_irq);
	set_svc_handler(&on_svc);

	init_timers();

	*hardware::IRQ_ENABLE_BASIC |= 1;
	hardware::ARM_TIMER[3] = 0;
	hardware::ARM_TIMER[7] = 0xff;
	hardware::ARM_TIMER[2] = 0x3e00a2;
	hardware::ARM_TIMER[0] = 500;
	hardware::ARM_TIMER[6] = 500;
}

// Add a process before active_process
void add_process(int i) {
	int u = active_process;
	int v = processes[u].previous_process;
	processes[u].previous_process = i;
	processes[v].next_process = i;
	processes[i].previous_process = v;
	processes[i].next_process = u;
	processes[i].process_state = PROCESS_ACTIVE;
	// There was no active process before
	if (active_process == 0) {
		active_process = i;
	}
}

void delete_process(int i) {
	int u = processes[i].previous_process;
	int v = processes[i].next_process;
	processes[u].next_process = v;
	processes[v].previous_process = u;
	processes[i].next_process = free_process;
	processes[i].process_state = PROCESS_INEXISTANT;
	free_process = i;
	// TODO
	if (active_process == i) {
		active_process = u;
		if (active_process == 0) {
			active_process = processes[u].previous_process;
		}
	}
}

int create_process() {
	int i = free_process;
	if (i >= (int)processes.size()) {
		process proc;
		processes.push_back(proc);
		free_process = i + 1;
	} else {
		free_process = processes[i].next_process;
	}
	add_process(i);
	return i;
}

bool ready(int i) {
	switch (processes[i].process_state) {
		case PROCESS_ACTIVE: {
			return true;
		}
		case PROCESS_WAIT_READ: {
			int sock = processes[i].cont.r0;
			if (!sockets[sock]->isEmpty()) {
				// TODO: quand y'aura le MMU, danger...
				int num_read = sockets[sock]->read(
						(void*)processes[i].cont.r1,
				        processes[i].cont.r2);
				processes[i].cont.r0 = num_read;
				processes[i].process_state = PROCESS_ACTIVE;
				return true;
			} else {
				return false;
			}
		}
		case PROCESS_WAIT_WRITE: {
			int sock = processes[i].cont.r0;
			if (sockets[sock]->isFull()) {
				// TODO: quand y'aura le MMU, danger...
				int num_written = sockets[sock]->write(
						(void*)processes[i].cont.r1,
				        processes[i].cont.r2);
				processes[i].cont.r0 = num_written;
				processes[i].process_state = PROCESS_ACTIVE;
				return true;
			} else {
				return false;
			}
		}
		case PROCESS_SLEEPING: {
			if (processes[i].state_info < elapsed_us()) {
				processes[i].process_state = PROCESS_ACTIVE;
				return true;
			}
			return false;
		}
		case PROCESS_ZOMBIE: {
			return false;
		}
		case PROCESS_WAITING: {
			int pid = processes[i].cont.r0;
			if (processes[pid].process_state == PROCESS_ZOMBIE) {
				processes[i].cont.r0 = (s32)processes[pid].state_info;
				delete_process(pid);
				processes[i].process_state = PROCESS_ACTIVE;
				return true;
			}
			return false;
		}
		default: return false;

	}
}

void next_process() {
	do {
		active_process = processes[active_process].next_process;
	} while (!ready(active_process));
	if (active_process == 0) {
		do {
			active_process = processes[active_process].next_process;
		} while (!ready(active_process));
	}
}

int async_start(async_func f, void* arg, s32 mode, const char* name,
	int cwd)
{
	int i = create_process();
	processes[i].cont = context();
	processes[i].cont.lr = ((s32)f) + 4;
	processes[i].cont.r0 = (s32)arg;
	// Find a suitable stack pointer; TODO: make it possible to free it
    // Make it 16-byte aligned.
	processes[i].cont.r13 = ((s32)(malloc(0x100000)) + 0x100000 - 1)
																& ~0xf;
	processes[i].cont.spsr = mode;
	if (name != NULL) {
		for (unsigned r = 0; r < sizeof(processes[i].process_name); r++) {
			char c = name[r];
			processes[i].process_name[r] = c;
			if (c == 0) break;
		}
		processes[i].process_name[(sizeof(processes[i].process_name)) - 1] = '\0';
	} else {
		processes[i].process_name[0] = '\0';
	}
	processes[i].cwd = (node*)cwd;
	return i;
}

ExpArray<int> alive_processes() {
	ExpArray<int> r;
	for (int i = 0; i < (int)processes.size(); i++) {
		if (processes[i].process_state != PROCESS_INEXISTANT) {
			r.push_back(i);
		}
	}
	return r;
}

const char* process_name(int pid) {
	return processes[pid].process_name;
}

char process_state(int pid) {
	switch (processes[pid].process_state) {
		case PROCESS_ACTIVE:
			return 'R';
		case PROCESS_SLEEPING:
			return 'S';
		case PROCESS_WAIT_READ:
		case PROCESS_WAIT_WRITE:
			return 'D';
		case PROCESS_ZOMBIE:
			return 'Z';
		case PROCESS_WAITING:
			return 'W';
		default:
			return ' ';
	}
}

__attribute__((naked))
void _async_go(context*) {
	asm(
		"mov sp, r0\n\t"
		"ldmfd sp!, {r0, lr}\n\t"
		"msr spsr_cxsf, r0\n\t"
		"ldmfd sp, {r0-lr}^\n\t"
		"add sp, sp, #60\n\t"
		"nop\n\t" // Needed?
		"subs pc, lr, #4"
	);
}
void _wait() {
	while (1) {
		asm volatile ("wfi");
	}
}
void async_go() {
	// Init the waiting process
	processes[0].cont = context();
	processes[0].cont.lr = ((s32)&_wait) + 4;
	processes[0].cont.r13 = (s32)(malloc(0x100));
	processes[0].process_state = PROCESS_ACTIVE;
	s32 cpsr = get_cpsr();
	cpsr &= ~0xdf;
	cpsr |= 0x53;
	processes[0].cont.spsr = 0x53; // SVC mode, enable IRQ, disable FIQ
	context* svc_stack_address = (context*)(0x6000);
	*svc_stack_address = processes[active_process].cont;
	_async_go(svc_stack_address);
}
