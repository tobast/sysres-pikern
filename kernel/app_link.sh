#!/bin/bash

tempfile=$(mktemp)
temp2=$(mktemp)
arm-none-eabi-objcopy "$1" -O binary "$tempfile"
arm-none-eabi-objdump -t "$1" |
	grep __bss_end__ |
	cut -d' ' -f1 |
	python3 -c "import sys; x = int(input(), 16); sys.stdout.buffer.write(bytes((0x7f, 0x45, 0x4c, 0x46, (x >> 24) & 0xff, (x >> 16) & 0xff, (x >> 8) & 0xff, x & 0xff)))" > "$temp2"
arm-none-eabi-objdump -h -j.got "$1" |
	grep ".got" |
	python3 -c "import sys; l = [x for x in input().split() if x != '']; assert(l[1]) == '.got'; u = int(l[2], 16) // 4; v = int(l[3], 16); sys.stdout.buffer.write(bytes(((v >> 24) & 0xff, (v >> 16) & 0xff, (v >> 8) & 0xff, v & 0xff, (u >> 24) & 0xff, (u >> 16) & 0xff, (u >> 8) & 0xff, u & 0xff)))" |
	cat "$temp2" - "$tempfile" > "$2"
rm "$tempfile" "$temp2"