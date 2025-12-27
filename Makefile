CC = riscv64-unknown-elf-gcc
LD = riscv64-unknown-elf-ld
OBJCOPY = riscv64-unknown-elf-objcopy
OBJDUMP = riscv64-unknown-elf-objdump

CFLAGS = -O2 -march=rv64gc -mabi=lp64 -mcmodel=medany \
         -ffreestanding -nostdlib -nostartfiles -fno-builtin \
         -Iinclude

LDFLAGS = -nostdlib

# 用户程序编译
user/init.bin: user/entry.S user/usys.S user/ulib.c user/init.c user/user.h user/user.ld
	$(CC) $(CFLAGS) -Iuser -c user/entry.S -o user/entry.o
	$(CC) $(CFLAGS) -Iuser -c user/usys.S -o user/usys.o
	$(CC) $(CFLAGS) -Iuser -c user/ulib.c -o user/ulib.o
	$(CC) $(CFLAGS) -Iuser -c user/init.c -o user/init_main.o
	$(CC) $(CFLAGS) -T user/user.ld -o user/init.elf \
		user/entry.o user/usys.o user/ulib.o user/init_main.o
	$(OBJCOPY) -O binary user/init.elf user/init.bin
	$(OBJDUMP) -S user/init.elf > user/init.asm

# 将用户程序二进制转为内核可链接的目标文件
user/initcode.o: user/init.bin
	cd user && $(LD) -r -b binary -o initcode.o init.bin

# 编译内核
kernel.elf: user/initcode.o
	$(CC) $(CFLAGS) -c kernel/entry.S -o kernel/entry.o
	$(CC) $(CFLAGS) -c kernel/uart.c -o kernel/uart.o
	$(CC) $(CFLAGS) -c kernel/console.c -o kernel/console.o
	$(CC) $(CFLAGS) -c kernel/printf.c -o kernel/printf.o
	$(CC) $(CFLAGS) -c kernel/pmm.c -o kernel/pmm.o
	$(CC) $(CFLAGS) -c kernel/vm.c -o kernel/vm.o
	$(CC) $(CFLAGS) -c kernel/vm_init.c -o kernel/vm_init.o
	$(CC) $(CFLAGS) -c kernel/trap.c -o kernel/trap.o
	$(CC) $(CFLAGS) -c kernel/kernelvec.S -o kernel/kernelvec.o
	$(CC) $(CFLAGS) -c kernel/timer.c -o kernel/timer.o
	$(CC) $(CFLAGS) -c kernel/exception.c -o kernel/exception.o
	$(CC) $(CFLAGS) -c kernel/swtch.S -o kernel/swtch.o
	$(CC) $(CFLAGS) -c kernel/proc.c -o kernel/proc.o
	$(CC) $(CFLAGS) -c kernel/trampoline.S -o kernel/trampoline.o
	$(CC) $(CFLAGS) -c kernel/syscall.c -o kernel/syscall.o
	$(CC) $(CFLAGS) -c kernel/sysproc.c -o kernel/sysproc.o
	$(CC) $(CFLAGS) -c kernel/virtio_disk.c -o kernel/virtio_disk.o
	$(CC) $(CFLAGS) -c kernel/bio.c -o kernel/bio.o
	$(CC) $(CFLAGS) -c kernel/log.c -o kernel/log.o
	$(CC) $(CFLAGS) -c kernel/fs.c -o kernel/fs.o
	$(CC) $(CFLAGS) -c kernel/file.c -o kernel/file.o
	$(CC) $(CFLAGS) -c kernel/sysfile.c -o kernel/sysfile.o
	$(CC) $(CFLAGS) -c kernel/klog.c -o kernel/klog.o
	$(CC) $(CFLAGS) -c kernel/main.c -o kernel/main.o
	$(CC) $(CFLAGS) -T kernel/kernel.ld -o kernel.elf \
		kernel/entry.o kernel/uart.o kernel/console.o kernel/printf.o \
		kernel/pmm.o kernel/vm.o kernel/vm_init.o kernel/trap.o \
		kernel/kernelvec.o kernel/timer.o kernel/exception.o \
		kernel/swtch.o kernel/proc.o kernel/trampoline.o \
		kernel/syscall.o kernel/sysproc.o kernel/virtio_disk.o \
		kernel/bio.o kernel/log.o kernel/fs.o kernel/file.o kernel/sysfile.o \
		kernel/klog.o kernel/main.o \
		user/initcode.o

all: kernel.elf

mkfs/mkfs: mkfs/mkfs.c
	gcc -Werror -Wall -Iinclude -o mkfs/mkfs mkfs/mkfs.c

fs.img: mkfs/mkfs
	./mkfs/mkfs fs.img

run: all 
	rm -f fs.img
	./mkfs/mkfs fs.img
	qemu-system-riscv64 -machine virt -nographic \
		-bios /usr/lib/riscv64-linux-gnu/opensbi/generic/fw_jump.bin \
		-kernel kernel.elf \
		-global virtio-mmio.force-legacy=false \
		-drive file=fs.img,if=none,format=raw,id=x0 \
		-device virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0

clean:
	rm -f kernel/*.o user/*.o user/*.elf user/*.bin user/*.asm kernel.elf

.PHONY: all run clean