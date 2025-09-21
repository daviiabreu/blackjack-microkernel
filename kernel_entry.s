[bits 16]

; Entry point para o kernel C
global _start
extern kernel_main

_start:
    ; Garante que os segmentos estão corretos
    mov ax, cs
    mov ds, ax
    mov es, ax

    ; Chama a função principal do kernel
    call kernel_main

    ; Se retornar, fica em loop infinito
    jmp $