#include "assert.h"

template<int N, typename T, T data[N]>
struct pool_allocator {
	static_assert(sizeof(T) >= sizeof(int),
		"pool_allocator requires a type bigger than int");
	int first_free;
	void init() {
		first_free = 0;
		for (int i = 0; i < N; i++) {
			*((int*)(&data[i])) = i + 1;
		}
	};
	int alloc() {
		assert(first_free < N, 17);
		int res = first_free;
		first_free = *((int*)(&data[res]));
		return res;
	};
	void dealloc(int i) {
		*((int*)(&data[i])) = first_free;
		first_free = i;
	};
};
