# Ferramentas
ASM = nasm
CC = i686-elf-gcc
LD = i686-elf-ld

# Flags
ASMFLAGS = -f bin
CFLAGS = -m16 -ffreestanding -fno-stack-protector -fno-builtin -nostdlib -Os
LDFLAGS = -r

# Targets
all: bootloader.bin kernel.bin disk.img

# Compila o bootloader
bootloader.bin: bootloader.s
	$(ASM) $(ASMFLAGS) bootloader.s -o bootloader.bin

# Compila o kernel
kernel_entry.o: kernel_entry.s
	$(ASM) -f elf32 kernel_entry.s -o kernel_entry.o

kernel.o: kernel.c
	$(CC) $(CFLAGS) -c kernel.c -o kernel.o

kernel.bin: kernel_entry.o kernel.o
	$(LD) -T kernel.ld kernel_entry.o kernel.o -o kernel.tmp
	i686-elf-objcopy -O binary kernel.tmp kernel.bin
	rm kernel.tmp

# Cria imagem de disco
disk.img: bootloader.bin kernel.bin
	dd if=/dev/zero of=disk.img bs=512 count=2880
	dd if=bootloader.bin of=disk.img bs=512 count=1 conv=notrunc
	dd if=kernel.bin of=disk.img bs=512 seek=1 conv=notrunc

# Executa no QEMU
run: disk.img
	qemu-system-i386 -fda disk.img

# Limpeza
clean:
	rm -f *.bin *.o disk.img

.PHONY: all run clean