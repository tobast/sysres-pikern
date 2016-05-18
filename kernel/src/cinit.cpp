// Defined in the linker script
extern int __bss_start__;
extern int __bss_end__;

// Defined in kernel.cpp
extern void kernel_main(void);

void* __dso_handle = 0x00;

extern "C" {
void _c_init()
{
    int* bss = &__bss_start__;
    int* bss_end = &__bss_end__;

	// Clear BSS
    while( bss < bss_end )
        *bss++ = 0;

    kernel_main();

	while(true); // If main ever returns, we should be trapped here.
}
}
