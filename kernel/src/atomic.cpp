#include "atomic.h"
#include "assert.h"
#include "svc.h"
#include "interrupts.h"

int atomic_cas(int* p, int old_value, int new_value) {
	if (is_interrupt()) {
		int old = *p;
		if (old == old_value) {
			*p = new_value;
		}
		return old;
	}
	return atomic_cas_svc(p, old_value, new_value);
}

void mutex_init(mutex_t *mutex) {
	mutex->locked = 0;
}

// TODO: make mutexes have their own SVC to avoid busy wait
void mutex_lock(mutex_t *mutex) {
	while (atomic_cas(&mutex->locked, 0, 1) != 0);
}

void mutex_unlock(mutex_t *mutex) {
	assert(atomic_cas(&mutex->locked, 1, 0) == 1, 0xb0);
}
