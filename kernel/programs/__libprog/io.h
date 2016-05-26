#pragma once

extern "C" {
void put_int(int v, unsigned base=10);
void put_uint(unsigned v, unsigned base=10);

void puts(const char* v, bool endl=true);

void printf(const char* fmt, ...);
}

