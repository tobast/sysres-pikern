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

OBJS = $(BUILD)blink.o

all: $(TARGET) $(LIST)

$(LIST): $(BUILD)output.elf
	$(OD) -d $< > $@

$(TARGET): $(BUILD)output.elf
	$(OC) $< -O binary $@

$(BUILD)output.elf: $(OBJS) $(LINKER)
	$(LD) --no-undefined $(OBJS) -Map $(MAP) -o $@ -T $(LINKER)

$(BUILD)%.o: $(SOURCE)%.cpp $(BUILD)
	$(CC) $(CFLAGS) -I $(SOURCE) $< -o $@

$(BUILD):
	mkdir $@

install:
	mkdir ./mntpt
	mount $(SDCARD_DEVICE) ./mntpt
	cp "$(TARGET)" "./mntpt/"
	umount ./mntpt
	rmdir mntpt

checkinstall:
	mkdir ./mntpt
	mount $(SDCARD_DEVICE) ./mntpt
	@md5sum $(TARGET)
	@md5sum ./mntpt/$(TARGET)
	umount ./mntpt
	rmdir ./mntpt
		

clean:
	rm -rf $(BUILD)
	rm -f $(TARGET)
