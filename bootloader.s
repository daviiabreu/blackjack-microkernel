org 0x0

_start:
    jmp main_entry_point

loading_msg: db "loading...", 0x0D, 0x0A, 0
kernel_loaded_msg: db "loaded! starting game", 0x0D, 0x0A, 0

print_string:
    lodsb
    cmp al, 0
    jz .end_print
    mov ah, 0x0e
    int 0x10
    jmp print_string
.end_print:
    ret

; carregar o kernel do disco
load_kernel:
    mov ax, 0x1000      ; Endereço onde o kernel será carregado
    mov es, ax
    mov bx, 0x0000      ; Offset

    mov ah, 0x02        ; Função de leitura do disco
    mov al, 5           ; Número de setores para ler (ajuste conforme necessário)
    mov ch, 0           ; Cilindro 0
    mov cl, 2           ; Setor 2 (bootloader está no setor 1)
    mov dh, 0           ; Cabeça 0
    mov dl, 0x00        ; Drive (0x00 = primeiro floppy)

    int 0x13            ; Chama interrupção do BIOS
    jc .disk_error      ; Se carry flag estiver ativa, houve erro

    ret

.disk_error:
    mov si, disk_error_msg
    call print_string
    jmp $

disk_error_msg: db "error!", 0x0D, 0x0A, 0

call_kernel:
    ; Setup proper environment for C code
    mov ax, 0x1000
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0xFFFF      ; Set stack at top of segment

    sti                 ; Enable interrupts for BIOS calls

    ; Salta para o código C carregado
    jmp 0x1000:0x0000   ; Far jump para o kernel

    ret

main_entry_point:
    cli
    mov ax, 0x7c0
    mov ds, ax
    mov es, ax
    mov ax, 0x00
    mov ss, ax
    mov sp, 0x7c00
    sti

    ; Mostra mensagem de carregamento
    mov si, loading_msg
    call print_string

    ; Carrega o kernel do disco
    call load_kernel

    ; Mostra mensagem de sucesso
    mov si, kernel_loaded_msg
    call print_string

    ; Chama o kernel (jogo de blackjack)
    call call_kernel

    ; Se chegou aqui, algo deu errado
    jmp $

times 510 - ($ - $$) db 0
dw 0xAA55