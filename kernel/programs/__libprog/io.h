#pragma once

enum AnsiColors {
	ANSI_BLACK=0,
	ANSI_RED=1,
	ANSI_GREEN=2,
	ANSI_YELLOW=3,
	ANSI_BLUE=4,
	ANSI_MAGENTA=5,
	ANSI_CYAN=6,
	ANSI_WHITE=7,

	ANSI_COLOR_DEFAULT=9
};

enum AnsiFormatType {
	ANSI_DEFAULT = 0,
	ANSI_BOLD = 1,
	ANSI_FG = 30,
	ANSI_BG = 40
};

struct AnsiFormat {
	AnsiFormat() : formatType(ANSI_DEFAULT), formatMod(0) {}
	AnsiFormat(AnsiFormatType ft) :
		formatType(ft), formatMod(0) {}
	AnsiFormat(AnsiFormatType ft, AnsiColors col) :
		formatType(ft), formatMod(col) {}

	AnsiFormatType formatType;
	int formatMod;

	int operator()() const {
		return (int)formatType + formatMod;
	}
};

extern "C" {
void put_int(int v, unsigned base=10);
void put_uint(unsigned v, unsigned base=10);

void puts(const char* v, bool endl=true);

void printf(const char* fmt, ...);

bool str_cmp(const char* c1, const char* c2);
}

