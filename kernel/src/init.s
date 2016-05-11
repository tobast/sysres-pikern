.section ".text.startup"

.global _start
.global _get_stack_pointer

_start:
	// Place stack pointer (growing downwards, 32k should be ok)
	ldr sp, =0x8000

	// Then run the startup function
	b _c_init

_exitloop: // If anything bad occurs, we want to loop here.
	b _exitloop

_get_stack_pointer:
    // Return the stack pointer value
    str     sp, [sp]
    ldr     r0, [sp]

    // Return from the function
    mov     pc, lr
