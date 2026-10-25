# Toolchain definitions
AS      := pdp11-aout-as
LD      := pdp11-aout-ld
OBJCOPY := pdp11-aout-objcopy
ROM_BASE := 0173000

# File definitions
SRCS := short.s
OBJS    := $(SRCS:.s=.o)
TARGET := lda_file.h
ROMS := boot_rom_mod.h m9312_console_mod.h m9312_rom1.h m9312_rom2.h 

.PHONY: all clean

all: $(TARGET) $(ROMS)

short.out: short.o

# Test code
lda_file.h: short.lda
#lda_file.h: benchmark.lda
	bin2h -s -i $< -o $@ 
	cp lda_file.h ../src/lda_file.h

# Make T11 boot code to execute at rom location in bare metal ld file
boot_rom_mod.h : boot_rom.h
	sed -i 's/data/boot_rom/g' boot_rom.h
	cp boot_rom.h ../src/boot_rom.h

# make a header file from M9312 console/diag PROM intel hex file
23-248F1_mod.hex : 23-248F1_mod.obj
	obj2bin.pl --console --out $@ $<

m9312_console_mod.h : 23-248F1_mod.hex
	hextoword $< $@ m9312_console_mod
	cp $@ ../src/$@

# make header files from M9312 RK/DL boot ROMs
23-756A9.hex : 23-756A9.obj
	obj2bin.pl --boot --out $@ $<

m9312_rom1.h : 23-756A9.hex
	hextoword $< $@ $(basename $@)
	cp $@ ../src/$@

23-751A9.hex : 23-751A9.obj
	obj2bin.pl --boot --out $@ $<

m9312_rom2.h : 23-751A9.hex
	hextoword $< $@ $(basename $@)
	cp $@ ../src/$@

# Pattern rule to assemble .s files into .o files
%.o: %.s
	$(AS) $(ASFLAGS) -als=$(basename $<).lst -o $@ $<

# Macro11
%.obj : %.mac
	macro11 -l $(basename $<).lst -o $@ $<

# Makes an lda file from macro11 objects
%.bic : %.obj
	obj2bin.pl --binary --memsize=65535 --out $@ $<

%.bin: %.o
	$(LD) -T rom.ld -o $@ $< 

# Create an lda file from pdp11-aout-as object file (note: always loads at zero)
%.lda: %.o
	bin2load -v -b 0 -a -o $@ -f $<

%_rom.lda: %.bin
	bin2load -v -b 0 -a -o $@ -f $<

# Create a C header file from lda file
%.h: %.lda
	bin2h -s -i $< -o $@ 

%.lst: %.o
	pdp11-aout-objdump -D -m pdp11 $< > $@

# Simulate our m9312 code
sim: 23-248F1_mod.bic 23-756A9.bic 23-751A9.bic run_m9312.ini
	pdp11 ./run_m9312.ini 

clean:
	rm -f $(OBJS) $(TARGET) *.lst *.h *.lda *.hex *.obj *.bic
