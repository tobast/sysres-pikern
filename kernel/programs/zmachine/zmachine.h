#include "syslib.h"
#include <cstdint>

//#define DEBUG(x) x
#define DEBUG(x)
#define assert(x) if (!(x)) exit()
#define NULL 0

typedef uint8_t u8;
typedef uint16_t u16;
typedef int16_t s16;
typedef uint32_t u32;

class Screen {
	u8 current_window;
	u8 current_style;
public:
	Screen () {
		current_window = 0;
		current_style = 0;
	};
	inline void set_window(u8 window) {
		current_window = window;
	};
	inline void putchr(char c) {
		if (current_window == 0) {
			putchar(c);
		}
	};
	inline char getchr() {
		return getchar();
	};
	inline void print(const char* s) {
		if (current_window == 0) {
			char c;
			while ((c = *(s++))) {
				putchr(c);
			}
		}
	};
	inline void print_int(int n) {
		u32 n0 = n;
		if (n < 0) {
			putchr('-');
			n0 = (u32)(-n);
		}
		if (n0 == 0) {
			putchr('0');
			return;
		}
		char buffer[10];
		int index = 0;
		while (n0 > 0) {
			buffer[index++] = '0' + (n0 % 10);
			n0 /= 10;
		}
		while (index >= 1) {
			putchr(buffer[--index]);
		}
	};
	inline void set_style(u8 style) {
		if (style == 0) {
			current_style = 0;
			print("\x1b[0m");
			return;
		}
		current_style = style;
		if (current_style & 4) {
			print("\x1b[3m");
		} else if (current_style & 2) {
			print("\x1b[1m");
		} else if (current_style & 1) {
			print("\x1b[7m");
		}
	};
};


const int STACK_SIZE = 16384;

u8* default_alphabet_table = (u8*)
	 "abcdefghijklmnopqrstuvwxyz"
	 "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	" \n0123456789.,!?_#'\"/\\-:()";


class ZState {
	u8* zstate;
	int size;
	u16* stack;
	u16 stack_size;
	u32 pc;
	u16* routine_stack;
	u16 routine_stack_size;
	int routine_locals;
	u8* alphabet_table;
	u16* encoding_table;
	Screen screen;
public:
	ZState() {};
	ZState(u8* addr, int size_) {
		zstate = addr;
		size = size_;
		stack = (u16*)(malloc(STACK_SIZE * sizeof(u16)));
		stack_size = 0;
		routine_stack = (u16*)(malloc(STACK_SIZE * sizeof(u16)));
		routine_stack_size = 0;
		pc = 0;
		alphabet_table = default_alphabet_table;
		encoding_table = (u16*)(malloc(256 * sizeof(u16)));
		init_encoding_table();
	};
	void init_encoding_table();
	void encode(u16 addr, u8 length, u16* store_result, u8 max_size);
	inline Screen & get_screen() {
		return screen;
	};
	inline u32 get_abbrv(u8 a) {
		return 2 * get_u16(get_u16(0x18) + 2 * a);
	};
	inline void set_alphabet_table(u8* t) {
		alphabet_table = t;
	};
	inline u8* get_alphabet_table() {
		return alphabet_table;
	};
	inline void routine_push(u16 n) {
		routine_stack[routine_stack_size++] = n;
	};
	inline void routine_push_u32(u32 n) {
		routine_push(n >> 16);
		routine_push(n & 0xffff);
	};
	inline u16 routine_pop() {
		return routine_stack[--routine_stack_size];
	};
	inline u16 get_local_var(u8 n) {
		return routine_stack[routine_stack_size - n - 1];
	};
	inline void set_local_var(u8 n, u16 value) {
		routine_stack[routine_stack_size - n - 1] = value;
	};
	inline u8 get_num_locals() {
		return routine_stack[routine_stack_size - 1] & 0xf;
	};
	inline u8 get_num_args() {
		return routine_stack[routine_stack_size - 1] >> 4;
	};
	inline u32 routine_pop_u32() {
		u32 lo = routine_pop();
		u32 hi = routine_pop();
		return (hi << 16) | lo;
	};
	inline void pop(u16 n) {
		stack_size -= n;
	};
	inline u16 pop() {
		return stack[--stack_size];
	};
	inline u16 top() {
		return stack[stack_size - 1];
	};
	inline void set_top(u16 value) {
		stack[stack_size - 1] = value;
	};
	inline void push(u16 value) {
		stack[stack_size++] = value;
	};
	inline u8* get_addr() {
		return zstate;
	};
	inline u8 get_u8(u32 addr) {
		return zstate[addr];
	};
	inline u16 get_u16(u32 addr) {
		return (u16)(zstate[addr] << 8) | (u16)(zstate[addr + 1]);
	};
	inline s16 get_s16(u32 addr) {
		return (s16)get_u16(addr);
	};
	inline void set_u8(u32 addr, u8 value) {
		zstate[addr] = value;
	};
	inline void set_u16(u32 addr, u16 value) {
		zstate[addr] = (u8)(value >> 8);
		zstate[addr + 1] = (u8)(value & 0xff);
	};
	inline void set_s16(u32 addr, s16 value) {
		set_u16(addr, (u16)(value));
	};
	inline u32 get_packed(u16 addr) {
		return 4 * addr;
	};
	inline void set_pc(u32 value) {
		pc = value;
	};
	inline u8 pc_get_u8() {
		return zstate[pc++];
	};
	inline u16 pc_get_u16() {
		u16 r = get_u16(pc);
		pc += 2;
		return r;
	};
	inline u16 get_var(u8 var) {
		if (var == 0) {
			return pop();
		} else if (var < 0x10) {
			return get_local_var(var);
		} else {
			return get_u16(get_u16(0xc) + 2 * (var - 0x10));
		}
	};
	inline u16 get_var_ind(u8 var) {
		if (var == 0) {
			return top();
		} else if (var < 0x10) {
			return get_local_var(var);
		} else {
			return get_u16(get_u16(0xc) + 2 * (var - 0x10));
		}
	};
	inline void set_var(u8 var, u16 value) {
		if (var == 0) {
			push(value);
		} else if (var < 0x10) {
			set_local_var(var, value);
		} else {
			set_u16(get_u16(0xc) + 2 * (var - 0x10), value);
		}
	};
	inline void set_var_ind(u8 var, u16 value) {
		if (var == 0) {
			set_top(value);
		} else if (var < 0x10) {
			set_local_var(var, value);
		} else {
			set_u16(get_u16(0xc) + 2 * (var - 0x10), value);
		}
	};
	inline u16 pc_read_arg(u8 arg_type) {
		switch (arg_type) {
			case 0: {
				return pc_get_u16();
			}
			case 1: {
				return pc_get_u8();
			}
			case 2: {
				u8 var = pc_get_u8();
				return get_var(var);
			}
			case 3: {
			    return 0;
			}
			default: {
			    assert(false);
				break;
			}
		}
	};
	inline void jump(s16 offs) {
		if (offs == 0 || offs == 1) {
			routine_return(offs);
		} else {
			pc += offs - 2;
		}
	};
	inline void branch(bool test_result) {
		u8 x = pc_get_u8();
		u8 y;
		if (!(x & 0x40)) {
			y = pc_get_u8();
		}
		if (((bool)(x & 0x80)) != test_result) return;
		if (x & 0x40) {
			jump(x & 0x3f);
		} else {
			s16 jp = (s16) ((x << 10) | (y << 2));
			jump(jp >> 2);
		}
	};
	void call_routine(u32 routine_addr, u16 result_location,
	                  u16* args, u8 nargs);
	void routine_return(u16 value);
	void run_step();
};
ZState zstate;

class Dictionary {
	u16 addr;
public:
	Dictionary(u16 addr_) {
		addr = addr_;
	};
	inline u8 n_word_separators() {
		return zstate.get_u8(addr);
	};
	inline bool is_word_separator(u8 x) {
		for (u8 i = 0; i < n_word_separators(); i++) {
			if (zstate.get_u8(addr + 1 + i) == x) {
				return true;
			}
		}
		return false;
	};
	inline u16 table_start() {
		return addr + n_word_separators() + 4;
	};
	inline u8 entry_length() {
		return zstate.get_u8(addr + n_word_separators() + 1);
	};
	inline s16 number_entries() {
		return (s16)zstate.get_u16(addr + n_word_separators() + 2);
	};
	u16 search(u16 encoded_text[3]);
	void add_to_parse_buffer(u16 text_buffer, u16 parse_buffer,
                 u8 &word_num, u8 &word_start, u8 length, bool flag);
	void tokenize(u16 text_buffer, u16 parse_buffer, bool flag);
};

class ZChars {
	u32 addr;
	u8 offs;
	bool end;
public:
	ZChars() {};
	ZChars(u32 addr_) {
		addr = addr_;
		offs = 0;
		end = false;
	}
	inline u8 get_char() {
		switch (offs) {
			case 0:
				return (zstate.get_u8(addr) >> 2) & 0x1f;
			case 1:
				return ((zstate.get_u8(addr) << 3) |
				        (zstate.get_u8(addr + 1) >> 5)) & 0x1f;
			case 2:
				return zstate.get_u8(addr + 1) & 0x1f;
			default:
			    assert(false);
		}
	};
	inline u8 next_char() {
		if (end) return 0xff;
		if (offs == 2) {
			offs = 0;
			if (zstate.get_u8(addr) & 0x80) {
				end = true;
				addr += 2;
				return 0xff;
			}
			addr += 2;
		} else {
			offs++;
		}
		return get_char();
	};
	inline bool at_end() {
		return end;
	};
	inline u32 end_addr() {
		if (end) return addr;
		int i = 0;
		while (!(zstate.get_u8(addr + 2 * i) & 0x80)) i++;
		return addr + 2 * i + 2;
	};
};

class ZString {
	u32 addr;
public:
	ZString() {};
	ZString(u32 addr_) {
		addr = addr_;
	};
	inline static u8 get_char(u8 alphabet, u8 c) {
		return zstate.get_alphabet_table()[alphabet*26+(c-6)];
	};
	/*
	inline static u8 process_char(u8 alph, u8 c) {
		if (alph > 2) {
			return 0;
		}
		switch (c) {
			case 0: putchar(' '); return 0;
			case 1: return 3;
			case 2: return 4;
			case 3: return 5;
			case 4: return 1;
			case 5: return 2;
			default: putchar(get_char(alph, c)); return 0;
		}
	}
	*/
	u32 print();
};

class Property {
	u16 addr;
public:
	Property (u16 addr_) {
		addr = addr_;
	};
	inline u16 get_addr() {
		return addr;
	};
	inline u8 property_number() {
		return zstate.get_u8(addr) & 0x3f;
	};
	inline u8 property_length() {
		u8 x = zstate.get_u8(addr);
		if (x & 0x80) {
			u8 r = zstate.get_u8(addr + 1) & 0x3f;
			if (r == 0) r = 64;
			return r;
		} else {
			return (x >> 6) + 1;
		}
	};
	inline u16 property_start_address() {
		return addr + (zstate.get_u8(addr) >> 7) + 1;
	};
	inline Property next_prop() {
		return Property(property_start_address() + property_length());
	};
};

class PropertyTable {
	u16 addr;
public:
	PropertyTable(u16 addr_) {
		addr = addr_;
	};
	inline u16 short_name_addr() {
		return addr + 1;
	};
	inline Property first_prop() {
		return Property(addr + 2 * zstate.get_u8(addr) + 1);
	};
	inline Property find_prop(u8 prop_number) {
		Property prop = first_prop();
		while (prop.property_number() > prop_number) {
			prop = prop.next_prop();
		}
		return prop;
	};
	inline u8 next_prop(u8 prop_number) {
		if (prop_number == 0) {
			return first_prop().property_number();
		}
		return find_prop(prop_number).next_prop().property_number();
	};
};

class Object {
	u16 addr;
public:
	Object(u16 number) {
		addr = zstate.get_u16(0xa) + 2 * 63 + 14 * (number - 1);
	};
	inline bool get_attribute(u8 attr_num) {
		u8 v = zstate.get_u8(addr + (attr_num >> 3));
		return (v >> (7 - (attr_num & 7))) & 1;
	};
	inline void set_attribute(u8 attr_num, bool value) {
		u8 shift_count = 7 - (attr_num & 7);
		u8 mask = ~(1 << shift_count);
		u16 adr = addr + (attr_num >> 3);
		zstate.set_u8(adr, (zstate.get_u8(adr) & mask) |
			(((u8) value) << shift_count));
	};
	inline u16 get_parent() {
		return zstate.get_u16(addr + 6);
	};
	inline void set_parent(u16 value) {
		zstate.set_u16(addr + 6, value);
	};
	inline u16 get_sibling() {
		return zstate.get_u16(addr + 8);
	};
	inline void set_sibling(u16 value) {
		zstate.set_u16(addr + 8, value);
	};
	inline u16 get_child() {
		return zstate.get_u16(addr + 10);
	};
	inline void set_child(u16 value) {
		zstate.set_u16(addr + 10, value);
	};
	inline PropertyTable get_props() {
		return PropertyTable(zstate.get_u16(addr + 12));
	};
	inline void detach() {
		if (get_parent() == 0) return;
		Object p(get_parent());
		if (addr == Object(p.get_child()).addr) {
			p.set_child(get_sibling());
		} else {
			Object c(p.get_child());
			while (Object(c.get_sibling()).addr != addr) {
				c = Object(c.get_sibling());
			}
			c.set_sibling(get_sibling());
		}
		set_parent(0);	
	};
};

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
void rand_setseed(int seed) {
	rand_state[0] = seed;
	rand_state[1] = seed + 17;
	rand_state[2] = seed + 42;
	rand_state[3] = seed + 51;
	rand_state[4] = seed + 23;
	rand_next(1);
	rand_next(1);
	rand_next(1);
}

inline char tolower(char c) {
	if ('A' <= c && c <= 'Z') {
		return c + ('a' - 'A');
	}
	return c;
}

void read_file(char* file);
void init();
