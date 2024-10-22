.PHONY: all
all:
	rm -f KlonOS.iso
	$(MAKE) -C kernel
	cp -v kernel/bin/KlonOS iso_root/boot/KlonOS
	xorriso -as mkisofs -b boot/limine/limine-bios-cd.bin \
        -no-emul-boot -boot-load-size 4 -boot-info-table \
        --efi-boot boot/limine/limine-uefi-cd.bin \
        -efi-boot-part --efi-boot-image --protective-msdos-label \
        ./iso_root -o KlonOS.iso
	limine/limine bios-install KlonOS.iso

.PHONY: clean
clean:
	$(MAKE) -C kernel clean

.PHONY: run
run:
	qemu-system-x86_64 -cdrom KlonOS.iso -boot d -machine q35 -S -s -serial file:serial.log -monitor stdio

.PHONY: debug
debug:
	gdb -x gdbcommands.txt