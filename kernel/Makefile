#########################################################
# Makefile
#  By Théophile Bastian and Nathanaël Courant
#
# Generates kernel.img
#########################################################

ARM_PREFIX = arm-none-eabi
CC = $(ARM_PREFIX)-g++
LD = $(ARM_PREFIX)-ld
AS = $(ARM_PREFIX)-as
OD = $(ARM_PREFIX)-objdump
OC = $(ARM_PREFIX)-objcopy

SDCARD_DEVICE = /dev/mmcblk0p1

CFLAGS = -O1 -std=c++11 -mfpu=vfp -mfloat-abi=hard -march=armv6zk \
	-mtune=arm1176jzf-s -nostartfiles

BUILD = _build/
SOURCE = src/
TARGET = kernel.img
LIST = $(BUILD)kernel.list
MAP = $(BUILD)kernel.map
LINKER = kernel.ld

BIN = $(BUILD)master.bin

OBJS = $(BUILD)malloc.o $(BUILD)blink.o

all: $(TARGET) $(LIST)

$(LIST): $(BUILD)output.elf
	$(OD) -d $< > $@

$(TARGET): $(BUILD)output.elf
	$(OC) $< -O binary $@

$(BUILD)output.elf: $(BIN) $(LINKER)
	$(LD) --no-undefined $(OBJS) -Map $(MAP) -o $@ -T $(LINKER)

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) --entry=main --specs=nosys.specs -I $(SOURCE) $^ -o $@

$(BUILD)%.o: $(SOURCE)%.cpp $(BUILD)
	$(CC) $(CFLAGS) -I $(SOURCE) -c $< -o $@

$(BUILD):
	mkdir $@

install: $(TARGET) $(SDCARD_DEVICE)
	mkdir -p ./mntpt
	sudo /usr/bin/mount $(SDCARD_DEVICE) ./mntpt
	cp "$(TARGET)" "./mntpt/"
	sync
	sudo /usr/bin/umount $(SDCARD_DEVICE)
	rmdir mntpt

checkinstall:
	mkdir ./mntpt
	sudo /usr/bin/mount $(SDCARD_DEVICE) ./mntpt
	@md5sum $(TARGET)
	@md5sum ./mntpt/$(TARGET)
	sudo /usr/bin/umount $(SDCARD_DEVICE)
	rmdir ./mntpt
		

clean:
	rm -rf $(BUILD)
	rm -f $(TARGET)
