#.PHONY: all clean run iso

CC = x86_64-elf-gcc
LD = x86_64-elf-ld


CFLAGS = -Wall -Wextra -std=gnu11 -ffreestanding -fno-stack-protector \
         -fno-stack-check -fno-lto -fno-PIE -fno-pic -m64 -march=x86-64 \
         -mno-80387 -mno-mmx -mno-sse -mno-sse2 -mno-red-zone \
         -mcmodel=kernel -Ilimine -Isrc/includes -nostdlib

LDFLAGS = -nostdlib -static -no-pie -z text -z max-page-size=0x1000 \
          -T src/kernel/linker.ld

KERNEL_SRC = src/kernel/kernel.c \
             src/libs/graphics/graphics.c \
             src/libs/graphics/standard/screen.c \
             src/libs/string/string.c \
             src/libs/graphics/draw.c

KERNEL_OBJ = $(patsubst src/%.c,build/%.o,$(KERNEL_SRC))
KERNEL = kernel.elf
ISO = emexOS.iso

all: $(ISO) run
.PHONY: all clean run iso

build/%.o: src/%.c
	@mkdir -p $(dir $@)
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

$(KERNEL): $(KERNEL_OBJ)
	@echo "Linking kernel..."
	$(LD) $(LDFLAGS) $(KERNEL_OBJ) -o $(KERNEL)

$(ISO): $(KERNEL)
	@echo "create ISO..."
	rm -rf iso_root
	mkdir -p iso_root/boot
	mkdir -p iso_root/boot/limine
	mkdir -p iso_root/EFI/BOOT

	cp $(KERNEL) iso_root/boot/
	cp limine.conf iso_root/boot/limine/

	cp limine/limine-bios.sys iso_root/boot/limine/
	cp limine/limine-bios-cd.bin iso_root/boot/limine/
	cp limine/limine-uefi-cd.bin iso_root/boot/limine/
	cp limine/BOOTX64.EFI iso_root/EFI/BOOT/
	cp limine/BOOTIA32.EFI iso_root/EFI/BOOT/

	xorriso -as mkisofs -b boot/limine/limine-bios-cd.bin \
	        -no-emul-boot -boot-load-size 4 -boot-info-table \
	        --efi-boot boot/limine/limine-uefi-cd.bin \
	        -efi-boot-part --efi-boot-image --protective-msdos-label \
	        iso_root -o $(ISO)

	@echo "ISO createt to > $(ISO)"

run: $(ISO)
	qemu-system-x86_64 -cdrom $(ISO) -m 256M -enable-kvm 2>/dev/null || \
	qemu-system-x86_64 -cdrom $(ISO) -m 256M

clean:
	rm -rf build $(KERNEL) $(ISO) iso_root
