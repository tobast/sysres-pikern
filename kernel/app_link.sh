#!/bin/bash

arm-none-eabi-objdump -t $1 |
	grep __bss_end__ |
	cut -d' ' -f1 |
	python3 -c "import sys; x = int(input(), 16); sys.stdout.buffer.write(bytes((0x7f, 0x45, 0x4c, 0x46, (x >> 24) & 0xff, (x >> 16) & 0xff, (x >> 8) & 0xff, x & 0xff)))"
arm-none-eabi-objcopy $1 -O binary /dev/stdout

