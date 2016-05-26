#include "zmachine.h"

#define UNUSED(expr) do { (void)(expr); } while (0)

#define OP0(x) ((x)|0xb0)
#define OP1(x) ((x)|0x80):case((x)|0x90):case((x)|0xa0)
#define OP2(x) (x):case((x)|0x20):case((x)|0x40):case((x)|0x60):case((x)|0xc0)

#define OP1_READ_ARG(name) u16 name = pc_read_arg((opcode >> 4) & 3)
#define OP2_READ_ARGS(name1, name2) \
  u16 name1, name2; \
  if (opcode & 0x80) { \
    u8 argtypes = pc_get_u8(); \
	assert((argtypes & 0x0f) == 0x0f); \
	assert ((argtypes >> 6) != 3);		 \
	assert (((argtypes >> 4) & 3) != 3); \
	name1 = pc_read_arg(argtypes >> 6); \
	name2 = pc_read_arg((argtypes >> 4) & 3); \
  } else { \
    name1 = pc_read_arg(1 + ((opcode >> 6) & 1)); \
	name2 = pc_read_arg(1 + ((opcode >> 5) & 1)); \
  }

#define VAR_READ_0ARG \
  { u8 argtypes = pc_get_u8(); \
    assert ((argtypes & 0xff) == 0xff); \
  }
#define VAR_READ_1ARG(name) \
  u16 name; \
  UNUSED(name); \
  { u8 argtypes = pc_get_u8(); \
    assert ((argtypes & 0x3f) == 0x3f); \
	assert ((argtypes >> 6) != 3); \
	name = pc_read_arg(argtypes >> 6); \
  }
#define VAR_READ_2ARG(name1, name2) \
  u16 name1, name2; \
  UNUSED(name1); UNUSED(name2); \
  { u8 argtypes = pc_get_u8(); \
    assert ((argtypes & 0x0f) == 0x0f); \
	assert ((argtypes >> 6) != 3); \
	assert (((argtypes >> 4) & 3) != 3); \
	name1 = pc_read_arg(argtypes >> 6); \
	name2 = pc_read_arg((argtypes >> 4) & 3); \
  }
#define VAR_READ_3ARG(name1, name2, name3) \
  u16 name1, name2, name3; \
  { u8 argtypes = pc_get_u8(); \
    assert ((argtypes & 0x03) == 0x03); \
	assert ((argtypes >> 6) != 3); \
	assert (((argtypes >> 4) & 3) != 3); \
	assert (((argtypes >> 2) & 3) != 3); \
	name1 = pc_read_arg(argtypes >> 6); \
	name2 = pc_read_arg((argtypes >> 4) & 3); \
	name3 = pc_read_arg((argtypes >> 2) & 3); \
  }
#define VAR_READ_ARGS(name, numargs) \
  u16 name[4]; \
  u8 numargs = 0; \
  { u8 argtypes = pc_get_u8(); \
    for (int i = 0; i < 4; i++) { \
	    u8 at = ((argtypes >> (6 - 2 * i)) & 3); \
		if (at != 3) numargs++; \
	    name[i] = pc_read_arg(at); \
	} \
  }
#define VAR_READ_ARGS8(name, numargs) \
  u16 name[8]; \
  u8 numargs = 0; \
  { u16 argtypes = pc_get_u16(); \
    for (int i = 0; i < 8; i++) { \
	    u8 at = (argtypes >> (14 - 2 * i)) & 3; \
	    if (at != 3) numargs++; \
	    name[i] = pc_read_arg(at); \
	} \
  }

  
u32 ZString::print() {
	ZChars crs(addr);
	u8 alphabet = 0;
	for (u8 chr = crs.get_char(); !crs.at_end(); chr = crs.next_char()) {
		if (alphabet == 2 && chr == 7) {
			zstate.get_screen().putchr('\n');
			alphabet = 0;
			continue;
		} else if (alphabet == 2 && chr == 6) {
			u8 data = crs.next_char() << 5;
			data |= crs.next_char();
			if (crs.at_end()) break;
			// TODO: special values
			zstate.get_screen().putchr(data);
			alphabet = 0;
			continue;
		}
		switch (chr) {
			case 0: zstate.get_screen().putchr(' '); alphabet = 0; break;
			case 4: alphabet = 1; break;
			case 5: alphabet = 2; break;
			case 1: case 2: case 3: { // Abbreviations
				alphabet = 0;
				u8 abbrv = crs.next_char();
				if (crs.at_end()) break;
				abbrv |= (chr - 1) << 5;
				ZString(zstate.get_abbrv(abbrv)).print();
				break;
			}
			default:
				zstate.get_screen().putchr(get_char(alphabet, chr));
				alphabet = 0;
				break;
		}
	}
	return crs.end_addr();
}

u16 Dictionary::search(u16 encoded_text[3]) {
	s16 ne = number_entries();
	u8 el = entry_length();
	u16 ts = table_start();
	if (ne < 0) {
		ne = -ne;
	}
	// For now, do a brutal search
	// Dichotomy is for later
	for (int i = 0; i < ne; i++) {
		u16 offs = ts + i * (u16)el;
	    bool found = true;
		for (int j = 0; j < 3; j++) {
			found &= (encoded_text[j] == zstate.get_u16(offs + 2 * j));
		}
		if (found) {
			return offs;
		}
	}
	return 0;
}

void Dictionary::add_to_parse_buffer(u16 text_buffer, u16 parse_buffer,
                u8 &word_num, u8 &word_start, u8 length, bool flag) {
	u16 encoded_word[3];
	zstate.encode(text_buffer + 2 + word_start, length,
	              encoded_word, 9);
	u16 w = search(encoded_word);
	if (w != 0 || !flag) {
		u16 offset = parse_buffer + 2 + 4 * word_num;
		zstate.set_u16(offset, w);
		zstate.set_u8(offset + 2, length);
		zstate.set_u8(offset + 3, word_start + 2);
	}
	word_num++;
	word_start += length;
}
						   
void Dictionary::tokenize(u16 text_buffer, u16 parse_buffer, bool flag) {
	u8 number_chars = zstate.get_u8(text_buffer + 1);
	u8 word_start = 0;
	u8 word_num = 0;
	u8 max_words = zstate.get_u8(parse_buffer);
	for (u8 i = 0; i < number_chars && word_num < max_words; i++) {
		u8 c = zstate.get_u8(text_buffer + 2 + i);
		if (c == (char)' ') {
			if (word_start == i) {
				word_start++;
				continue;
			}
			add_to_parse_buffer(text_buffer, parse_buffer, word_num,
			                    word_start, i - word_start, flag);
			word_start++;
			continue;
		} else if (is_word_separator(c)) {
			if (word_start < i) {
				add_to_parse_buffer(text_buffer, parse_buffer, word_num,
				                    word_start, i - word_start, flag);
				if (word_num >= max_words) continue;
			}
			word_start = i;
			add_to_parse_buffer(text_buffer, parse_buffer, word_num,
			                    word_start, 1, flag);
		}
	}
	if (word_num < max_words && word_start < number_chars) {
		add_to_parse_buffer(text_buffer, parse_buffer, word_num,
		                    word_start, number_chars - word_start, flag);
	}
	zstate.set_u8(parse_buffer + 1, word_num);
}

void ZState::init_encoding_table() {
	for (u8 i = 0; i < 255; i++) {
		encoding_table[i] = 0;
	}
	// Go through all the chars in the alphabet table
	for (u8 i = 1; i < 26; i++) {
		encoding_table[alphabet_table[2 * 26 + i]] = (5 << 5) | (i + 6);
	}
	for (u8 i = 0; i < 26; i++) {
		encoding_table[alphabet_table[26 + i]] = (4 << 5) | (i + 6);
	}
	for (u8 i = 0; i < 26; i++) {
		encoding_table[alphabet_table[i]] = i + 6;
	}
	// Remaining chars
	for (u8 i = 0; i < 255; i++) {
		if (encoding_table[i] == 0 && i != ' ') {
			encoding_table[i] = 0x8000 | i;
		}
	}
}

const int MAX_SIZE_BOUND = (1<<9);
void ZState::encode(u16 addr, u8 length, u16* store_result, u8 max_size) {
	u8 store_temp[MAX_SIZE_BOUND]; // Easier not to check for overflows
	u8 index = 0;
	for (u8 k = 0; k < length && index < max_size; k++) {
		u8 c = get_u8(addr + k);
		u16 encoded = encoding_table[c];
		if (encoded & 0x8000) {
			store_temp[index++] = 5;
			store_temp[index++] = 6;
			store_temp[index++] = (encoded >> 5) & 0x1f;
			store_temp[index++] = encoded & 0x1f;
		} else if ((encoded >> 5) & 0x1f) { // Two chars
			store_temp[index++] = (encoded >> 5) & 0x1f;
			store_temp[index++] = encoded & 0x1f;
		} else {
			store_temp[index++] = encoded;
		}
	}
	// Pad with 5
	for (; index < max_size; index++) {
		store_temp[index] = 5; 
	}
	for (u8 i = 0; i < max_size / 3; i++) {
		store_result[i] = (store_temp[3 * i] << 10) |
		                  (store_temp[3 * i + 1] << 5) |
						   store_temp[3 * i + 2];
        if (i == max_size / 3 - 1) {
			store_result[i] |= 0x8000;
		}
	}
}

void ZState::call_routine(u32 routine_addr, u16 result_location,
	                      u16* args, u8 nargs) {
	if (routine_addr == 0) {
		if (result_location < 0x100) {
			set_var(result_location, 0);
		}
		return;
	}
	routine_push_u32(pc);
	routine_push(result_location);
	routine_push(stack_size);
	u8 local_vars = get_u8(routine_addr);
	for (int j = local_vars - 1; j >= 0; j--) {
		if (j < nargs) {
			routine_push(args[j]);
		} else {
			routine_push(0);
		}
	}
	routine_push((nargs << 4) | local_vars);
	pc = routine_addr + 1;
}

void ZState::routine_return(u16 value) {
	u8 num_local_vars = routine_pop() & 0xf;
	routine_stack_size -= num_local_vars;
	stack_size = routine_pop();
	u16 result_location = routine_pop();
	pc = routine_pop_u32();
	if (result_location < 0x100) {
		set_var(result_location, value);
	}
}

void ZState::run_step() {
	u16 opcode = pc_get_u8();
	if (opcode == 0xbe) { // Extended
		opcode = 0x100 | pc_get_u8();
	}
	switch(opcode) {
		case OP2(0x01): {
			if (opcode & 0x80) {
				VAR_READ_ARGS(args, k);
				bool do_branch = false;
				assert (k > 1);
				for (int i = 1; i < k; i++) {
					if (args[0] == args[i]) {
						do_branch = true;
						break;
					}
				}
				branch(do_branch);
			} else {
				OP2_READ_ARGS(a, b);
				branch(a == b);
			}
			break;
		}
		case OP2(0x02): {
			OP2_READ_ARGS(a, b);
			branch(((s16)a) < ((s16)b));
			break;
		}
		case OP2(0x03): {
			OP2_READ_ARGS(a, b);
			branch(((s16)a) > ((s16)b));
			break;
		}
		case OP2(0x04): {
			OP2_READ_ARGS(var, value);
			s16 new_value = (s16)get_var(var) - 1;
			set_var(var, (u16)new_value);
			branch(new_value < (s16)value);
			break;
		}
		case OP2(0x05): {
			OP2_READ_ARGS(var, value);
			s16 new_value = (s16)get_var(var) + 1;
			set_var(var, (u16)new_value);
			branch(new_value > (s16)value);
			break;
		}
		case OP2(0x06): {
			OP2_READ_ARGS(obj1, obj2);
			branch(Object(obj1).get_parent() == obj2);
			break;
		}
		case OP2(0x07): {
			OP2_READ_ARGS(bitmap, flags);
			branch((bitmap & flags) == flags);
			break;
		}
		case OP2(0x08): {
			OP2_READ_ARGS(arg1, arg2);
			u8 result = pc_get_u8();
			set_var(result, arg1 | arg2);
			break;
		}
		case OP2(0x09): {
			OP2_READ_ARGS(arg1, arg2);
			u8 result = pc_get_u8();
			set_var(result, arg1 & arg2);
			break;
		}
		case OP2(0x0a): {
			OP2_READ_ARGS(obj, attr);
			branch(Object(obj).get_attribute(attr));
			break;
		}
		case OP2(0x0b): {
			OP2_READ_ARGS(obj, attr);
			Object(obj).set_attribute(attr, true);
			break;
		}
		case OP2(0x0c): {
			OP2_READ_ARGS(obj, attr);
			Object(obj).set_attribute(attr, false);
			break;
		}
		case OP2(0x0d): {
			OP2_READ_ARGS(var, value);
			set_var_ind(var, value);
			break;
		}
		case OP2(0x0e): {
			OP2_READ_ARGS(obj, dest);
			Object o(obj);
			o.detach();
			o.set_parent(dest);
			if (dest) {
				o.set_sibling(Object(dest).get_child());
				Object(dest).set_child(obj);
			}
			break;
		}
		case OP2(0x0f): {
			OP2_READ_ARGS(array, index);
			set_var(pc_get_u8(), get_u16(array + 2 * (s16)index));
			break;
		}
		case OP2(0x10): {
			OP2_READ_ARGS(array, index);
			set_var(pc_get_u8(), get_u8(array + (s16)index));
			break;
		}
		case OP2(0x11): {
			OP2_READ_ARGS(obj, prop);
			Property p = Object(obj).get_props().find_prop(prop);
			u16 r;
			if (p.property_number() == prop) {
				switch (p.property_length()) {
					case 1: r = get_u8(p.property_start_address()); break;
					case 2: r = get_u16(p.property_start_address()); break;
					default: assert(false); break;
				}
			} else {
				r = get_u16(get_u16(0xa) + (prop - 1) * 2);
			}
			set_var(pc_get_u8(), r);
			break;
		}
		case OP2(0x12): {
			OP2_READ_ARGS(obj, prop);
			Property p = Object(obj).get_props().find_prop(prop);
			u16 r;
			if (p.property_number() == prop) {
				r = p.property_start_address();
			} else {
				r = 0;
			}
			set_var(pc_get_u8(), r);
			break;
		}
		case OP2(0x13): {
			OP2_READ_ARGS(obj, prop);
			set_var(pc_get_u8(), Object(obj).get_props().next_prop(prop));
			break;
		}
		case OP2(0x14): {
			OP2_READ_ARGS(arg1, arg2);
			u8 result = pc_get_u8();
			set_var(result, arg1 + arg2);
			break;
		}
		case OP2(0x15): {
			OP2_READ_ARGS(arg1, arg2);
			u8 result = pc_get_u8();
			set_var(result, arg1 - arg2);
			break;
		}
		case OP2(0x16): {
			OP2_READ_ARGS(arg1, arg2);
			u8 result = pc_get_u8();
			set_var(result, arg1 * arg2);
			break;
		}
		case OP2(0x17): {
			OP2_READ_ARGS(arg1, arg2);
			assert (arg2 != 0);
			u8 result = pc_get_u8();
			set_var(result, ((s16)arg1) / ((s16)arg2));
			break;
		}
		case OP2(0x18): {
			OP2_READ_ARGS(arg1, arg2);
			assert (arg2 != 0);
			u8 result = pc_get_u8();
			set_var(result, ((s16)arg1) % ((s16)arg2));
			break;
		}
		case OP2(0x19): {
			OP2_READ_ARGS(raddr, arg);
			call_routine(get_packed(raddr), pc_get_u8(), &arg, 1);
			break;
		}
		case OP2(0x1a): {
			OP2_READ_ARGS(raddr, arg);
			call_routine(get_packed(raddr), (u16)(-1), &arg, 1);
			break;
		}
		case OP0(0x00): {
			routine_return(1);
			break;
		}
		case OP0(0x01): {
			routine_return(0);
			break;
		}
		case OP0(0x02): {
			pc = ZString(pc).print();
			break;
		}
		case OP0(0x03): {
			pc = ZString(pc).print();
			screen.putchr('\n');
			routine_return(1);
			break;
		}
		case OP0(0x08): {
			routine_return(pop());
			break;
		}
		case OP0(0x0a): {
			exit(0);
			break;
		}
		case OP0(0x0b): {
			screen.putchr('\n');
			break;
		}
		case OP0(0x0d): {
			// TODO
			branch(true);
			break;
		}
		case OP0(0x0f): {
			branch(true);
			break;
		}
		case OP1(0x00): {
			OP1_READ_ARG(value);
			branch(value == 0);
			break;
		}
		case OP1(0x01): {
			OP1_READ_ARG(value);
			u16 sibling = Object(value).get_sibling();
			set_var(pc_get_u8(), sibling);
			branch(sibling != 0);
			break;
		}
		case OP1(0x02): {
			OP1_READ_ARG(value);
			u16 child = Object(value).get_child();
			set_var(pc_get_u8(), child);
			branch(child != 0);
			break;
		}
		case OP1(0x03): {
			OP1_READ_ARG(value);
			set_var(pc_get_u8(), Object(value).get_parent());
			break;
		}
		case OP1(0x04): {
			OP1_READ_ARG(prop_addr);
			u8 r;
			u8 x = get_u8(prop_addr - 1);
			if (x & 0x80) {
				r = x & 0x3f;
				if (r == 0) r = 64;
			} else {
				r = (x >> 6) + 1;
			}
			set_var(pc_get_u8(), r);
			break;
		}
		case OP1(0x05): {
			OP1_READ_ARG(var);
			set_var(var, get_var(var) + 1);
			break;
		}
		case OP1(0x06): {
			OP1_READ_ARG(var);
			set_var(var, get_var(var) - 1);
			break;
		}
		case OP1(0x07): {
			OP1_READ_ARG(addr);
			ZString(addr).print();
			break;
		}
		case OP1(0x08): {
			OP1_READ_ARG(raddr);
			call_routine(get_packed(raddr), pc_get_u8(), NULL, 0);
			break;
		}
		case OP1(0x09): {
			OP1_READ_ARG(obj);
			Object(obj).detach();
			break;
		}
		case OP1(0x0a): {
			OP1_READ_ARG(obj);
			ZString(Object(obj).get_props().short_name_addr()).print();
			break;
		}
		case OP1(0x0b): {
			OP1_READ_ARG(value);
			routine_return(value);
			break;
		}
		case OP1(0x0c): {
			OP1_READ_ARG(offset);
			jump(offset);
			break;
		}
		case OP1(0x0d): {
			OP1_READ_ARG(paddr);
			ZString(get_packed(paddr)).print();
			break;
		}
		case OP1(0x0e): {
			OP1_READ_ARG(var);
			u8 result = pc_get_u8();
			set_var(result, get_var_ind(var));
			break;
		}
		case OP1(0x0f): {
			OP1_READ_ARG(raddr);
			call_routine(get_packed(raddr), (u16)(-1), NULL, 0);
			break;
		}
		case 0xe0: {
			VAR_READ_ARGS(args, nargs);
			u8 result_loc = pc_get_u8();
			call_routine(get_packed(args[0]), result_loc, &args[1], nargs - 1);
			break;
		}
		case 0xe1: {
			VAR_READ_3ARG(array, index, value);
			set_u16(array + 2 * (s16)index, value);
			break;
		}
		case 0xe2: {
			VAR_READ_3ARG(array, index, value);
			set_u8(array + (s16)index, value);
			break;
		}
		case 0xe3: {
			VAR_READ_3ARG(obj, prop, value);
			Property p = Object(obj).get_props().find_prop(prop);
			assert (p.property_number() == prop);
			switch (p.property_length()) {
				case 1:
					set_u8(p.property_start_address(), value);
					break;
				case 2:
					set_u16(p.property_start_address(), value);
					break;
				default: assert(false); break;
			}
			break;
		}
		case 0xe4: {
			VAR_READ_ARGS(args, _);
			u16 text_buffer = args[0];
			u8 i = get_u8(text_buffer + 1);
			// TODO: time, routine
			for (; i < get_u8(text_buffer); i++) {
				char c = screen.getchr();
				if (c == '\n') {
					break;
				}
				c = tolower(c);
				set_u8(text_buffer + i + 2, (u8)c);
			}
			set_u8(text_buffer + 1, i);
			Dictionary dict(get_u16(0x8));
			dict.tokenize(text_buffer, args[1], false);
			set_var(pc_get_u8(), (u8)'\n');
			break;
		}
		case 0xe5: {
			VAR_READ_1ARG(chr);
			screen.putchr((char)chr);
			break;
		}
		case 0xe6: {
			VAR_READ_1ARG(n);
			screen.print_int((s16)n);
			break;
		}
		case 0xe7: {
			VAR_READ_1ARG(range);
			u16 r;
			if ((s16)range > 0) {
				r = rand_next(range) + 1;
			} else if (range == 0) {
				rand_reseed();
				r = 0;
			} else {
				rand_setseed(-range);
				r = 0;
			}
			set_var(pc_get_u8(), r);
			break;
		}
		case 0xe8: {
			VAR_READ_1ARG(value);
			push(value);
			break;
		}
		case 0xe9: {
			VAR_READ_1ARG(var);
			set_var_ind(var, pop());
			break;
		}
		case 0xea: {
			// TODO
			VAR_READ_1ARG(lines);
			break;
		}
		case 0xeb: {
			VAR_READ_1ARG(window);
			screen.set_window(window);
			break;
		}
		case 0xec: {
			VAR_READ_ARGS8(args, nargs);
			u8 result_loc = pc_get_u8();
			call_routine(get_packed(args[0]), result_loc, &args[1], nargs - 1);
			break;
		}
		case 0xef: {
			// TODO
			VAR_READ_2ARG(line, column);
			break;
		}
		case 0xf1: {
			VAR_READ_1ARG(style);
			screen.set_style(style);
			break;
		}
		case 0xf6: {
			VAR_READ_ARGS(args, numargs);
			assert(numargs <= 3);
			assert(args[0] == 1);
			// TODO: time, routine
			set_var(pc_get_u8(), getchar());
			break;
		}
		case 0xf8: {
			VAR_READ_1ARG(value);
			u8 result = pc_get_u8();
			set_var(result, ~value);
			break;
		}
		case 0xf9: {
			VAR_READ_ARGS(args, nargs);
			call_routine(get_packed(args[0]), (u16)(-1), &args[1], nargs - 1);
			break;
		}
		case 0xfa: {
			VAR_READ_ARGS8(args, nargs);
			call_routine(get_packed(args[0]), (u16)(-1), &args[1], nargs - 1);
			break;
		}
		case 0xfb: {
			VAR_READ_ARGS(args, _);
			u16 dict = args[2] ? args[2] : get_u16(0x8);
			Dictionary(dict).tokenize(args[0], args[1], args[3]);
			break;
		}
		case 0xff: {
			VAR_READ_1ARG(n);
			branch(n <= get_num_args());
			break;
		}
		case 0x102: {
			VAR_READ_2ARG(n, places);
			assert (-15 <= (s16)places && (s16) places <= 15);
			u16 r;
			if ((s16)places >= 0) {
				r = n << places;
			} else {
				r = n >> (-(s16)places);
			}
			set_var(pc_get_u8(), r);
			break;
		}
		case 0x103: {
			VAR_READ_2ARG(n, places);
			assert (-15 <= (s16)places && (s16) places <= 15);
			s16 r;
			if ((s16)places >= 0) {
				r = ((s16)n) << places;
			} else {
				r = ((s16)n) >> (-(s16)places);
			}
			set_var(pc_get_u8(), r);
			break;
		}
		case 0x109: {
			VAR_READ_0ARG;
			set_var(pc_get_u8(), (u16)(-1));
			break;
		}
		default:
			assert(false);
	}
}

void read_file(char* file)
{
	int src = find_file(file);
	if (src == 0) {
		printf("Error: file %s not found", file);
		exit(1);
	}

	int size = file_size(src);

	assert(size <= 512 * 1024);
	u8* zstate_addr = (u8*)malloc(size * sizeof(uint8_t));
	int index = 0;
	while (index < size) {
		int data_read = file_read(src, index, (void*)(zstate_addr + index), size - index);
		index += data_read;
	}
	zstate = ZState(zstate_addr, size);
}

void init() {
	u16 pc_initial = zstate.get_u16(0x6);
	zstate.set_pc(pc_initial);
	while (true) {
		zstate.run_step();
	}
}


int main(int argc, char** argv) {
	assert(argc >= 2);
	puts("Hello, world!");
	sleep(1000*1000);
	printf("Trying to open file %s...\n", argv[1]);
	sleep(1000 * 1000);
	read_file(argv[1]);
	init();
}
