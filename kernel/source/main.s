
.section .init
.globl _start
_start:
	ldr r0,=0x7E200000
	mov r1,#1
	lsl r1,#15
	str r1,[r0,#5]

loop$:

	mov r2,#0x3F0000
wait1$:
	sub r2,#1
	cmp r2,#0
	bne wait1$

	mov r1,#1
	lsl r1,#25
	str r1,[r0,#0x1C]

	mov r2,#0x3F0000
wait2$:
	sub r2,#1
	cmp r2,#0
	bne wait2$

	mov r1,#1
	lsl r1,#25
	str r1,[r0,#0x28]
	
	b loop$
