/* the in-game blob: one section at 0x268800 (inside ms_net_patch_set's dead body, which ends at 0x26A2E8) */
SECTIONS
{
    .blob 0x268800 : {
        *(.data .data.*) *(.sdata .sdata.*) *(.rodata .rodata.*) *(.text .text.*)
        *(.bss .bss.*) *(.sbss .sbss.*) *(COMMON)
    }
    /DISCARD/ : { *(.comment) *(.note*) *(.reginfo) *(.MIPS.*) *(.mdebug*) *(.gnu.attributes) *(.pdr) }
}
