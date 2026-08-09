

CC = riscv64-linux-gnu-gcc
LD = riscv64-linux-gnu-ld
CFLAGS = -march=rv64gc -ffreestanding -nostdlib -I.
LDFLAGS = -Ttext 0x80200000  # 内核加载地址

OBJS = entry.o start.o main.o

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