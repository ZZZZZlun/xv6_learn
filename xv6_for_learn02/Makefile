

CC = riscv64-linux-gnu-gcc
LD = riscv64-linux-gnu-ld
CFLAGS = -march=rv64gc -ffreestanding -nostdlib -I.
LDFLAGS = -Ttext 0x80200000  # 内核加载地址

OBJS = entry.o start.o main.o vm.o proc.o kalloc.o string.o spinlock.o


QEMU = qemu-system-riscv64
QEMUOPTS = -machine virt -bios none -kernel kernel -m 128M -smp 1 -nographic

#GDB 端口
GDBPORT = 1234

all:kernel

kernel: $(OBJS)
	$(LD) $(LDFLAGS) -e _entry -o kernel $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.S 
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f kernel *.o

run: kernel
	qemu-system-riscv64 -kernel kernel -nographic


#debug
qemu-gdb: kernel
	$(QEMU) $(QEMUOPTS) -s -S

#使用指定端口
qemu-gdb-port: kernel
	$(QEMU) $(QEMUOPTS) -gdb tcp::$(GDBPORT) -S

# 启动GDB客户端
gdb:
