#pragma once

int atomic_cas(int *p, int old_value, int new_value);

struct mutex_t {
	int locked;
};

void mutex_init(mutex_t *mutex);
void mutex_lock(mutex_t *mutex);
void mutex_unlock(mutex_t *mutex);
