#include "svc.h"

#define SVC(decl, x) __attribute__((naked)) decl { \
	asm volatile ("svc #" #x "\n\tbx lr"); \
}

SVC(int get_pid(), 1)
SVC(int read(int, void*, int), 2)
SVC(int write(int, void*, int), 3)
SVC(void sleep(int), 4)
SVC(void* malloc_svc(uint32_t), 5)
SVC(void free_svc(void*), 6)
SVC(int atomic_cas_svc(int*, int, int), 7)
SVC(void kill(int, int), 8)
SVC(int wait(int), 9)
SVC(int is_ready_read(int), 10)
SVC(int is_ready_write(int), 11)
SVC(int is_process_alive(int), 12)
SVC(int find_file(const char*), 13)
SVC(int file_size(int), 14)
SVC(int file_read(int, int, void*, int), 15)
SVC(int nb_processes(), 16)
SVC(int get_processes(int*, int), 17)
SVC(void get_process_name(int, char*), 18)
SVC(char get_process_state(int), 19)
SVC(int udp_write(UdpSysData*), 20)

