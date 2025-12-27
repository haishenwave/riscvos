ENTRY(_start)

/*设置起始地址为0x80200000——OPENSBI */
    . = 0x80200000;
SECTIONS{
    .text : ALIGN(4) { 
        *(.text.entry)
        *(.text*) 
        *(.rodata*) 
    }
    etext = .;
    

    . = ALIGN(4096);
    .trampoline : {
        *(.text.trampoline)
    }

    .data : { *(.data*) }

    . = ALIGN(16);
    .sdata : {
        __global_pointer$ = . + 0x800;
        *(.sdata .sdata.* .gnu.linkonce.s.*)
    }

    bss_start = .;
    .bss (NOLOAD) : { *(.bss*) *(COMMON) }
    bss_end = .;

    . = ALIGN(16);
    . = . + 0x100; /*gap after bss*/

    stack_bottom = .;
    . = . + 8K;
    stack_top = .;

    end = .;
}