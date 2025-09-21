/* Linker script pro kernel */
ENTRY(_start)

SECTIONS
{
    . = 0x0;  /* Kernel carregado em 0x1000:0x0000 */

    .text : {
        *(.text)
    }

    .data : {
        *(.data)
    }

    .bss : {
        *(.bss)
    }
}